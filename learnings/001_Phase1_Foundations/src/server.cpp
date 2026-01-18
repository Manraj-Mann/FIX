#include <server.h>
#include "../../../common/logs.h"
#include "../../../common/timeutils.h"
#include "../../../common/constants.h"
#include <vector>
#include <algorithm>
#include <numeric>

Server::Server(uint16_t port)
    :m_port(port)
{
    // Constructor implementation (if needed)
}

bool Server::init()
{
    m_socket_fd = ::socket(AF_INET , SOCK_STREAM , 0);

    if(m_socket_fd < 0)
    {
        perror("Could not create socket");
        return false;
    }

    // set socket to non-blocking mode
    network::utils::set_non_blocking(m_socket_fd);

    // set reuse address option
    network::utils::set_reuse_address(m_socket_fd);

    m_address = network::utils::create_ipv4_address(m_port , INADDR_ANY);

    return true;
}

bool Server::start()
{
    /*
        Bind the socket to the specified IP address and port
    */
    if(::bind(m_socket_fd , (sockaddr*)&m_address , sizeof(m_address)) < 0)
    {
        perror("Bind failed");
        return false;
    }

    /*
        Listen for incoming connections
        SOMAXCONN : This constant defines the maximum length for the queue of pending connections.
                    It is used here to specify that the socket should allow the maximum number of
                    pending connections as defined by the system.
    */
    if(::listen(m_socket_fd , SOMAXCONN) < 0)
    {
        perror("Listen failed");
        return false;
    }

    return true;
}

void Server::add_socket_in_epoll(int epoll_fd , int socket_fd)
{
    /*
    Create an epoll instance to monitor multiple file descriptors
    epoll_create1 : This function creates a new epoll instance and returns a file descriptor
                    referring to that instance.

    The argument '0' indicates that no special flags are being set for the epoll instance.
    */

    // Add the server socket to the epoll instance to monitor for incoming connections
    epoll_event event{};

    // Monitor for read (incoming connection) events
    event.events = EPOLLIN;

    // set the socket fd to the event data
    event.data.fd = socket_fd;

    // Add the socket file descriptor to the epoll instance
    epoll_ctl(epoll_fd , EPOLL_CTL_ADD , socket_fd , &event);
    
}
void Server::run(const size_t max_socket_fds , const size_t buffer_size , const size_t max_events)
{
    int epoll_fd = epoll_create1(0);
    add_socket_in_epoll(epoll_fd , m_socket_fd);

    // Event array to hold the events returned by epoll_wait
    epoll_event events[max_events];

    // socket  // fd → connection mapping (FAST, no hashing)
    // network::tcp::TcpConnection * connections[max_socket_fds]{};

    /**
     * Remove the heap allocation by using a fixed-size array.
     * This avoids dynamic memory allocation overhead and fragmentation,
     */

    alignas(CACHE_LINE_SIZE) network::tcp::TcpConnection connections[max_socket_fds]{};

    bool fd_in_use[max_socket_fds]{false};

    alignas(CACHE_LINE_SIZE) char buffer[buffer_size];

    // Latency tracking
    std::vector<uint64_t> latencies;
    latencies.reserve(10*CONSTANTS::ITERATIONS);  // Pre-allocate space for latencies

    while (true)
    {
        /*
            Wait for events on the epoll instance
            -1 : This indicates that epoll_wait should block indefinitely until at least one event occurs.

            events : This is a pointer to an array of epoll_event structures that will be filled
                     with the events that have occurred.

            max_events : This specifies the maximum number of events that can be returned
                          at one time and should not exceed the size of the events array.

            nfds    : Number of file descriptors with events

        */

        LOG_INFO("Waiting for events...");

        int nfds = epoll_wait(epoll_fd , events , max_events , -1 );

        /*
            Process each event returned by epoll_wait 
        */
        for (int fd = 0; fd < nfds; fd++)
        {
           int socket_fd = events[fd].data.fd;
           
           /**
            * 
            * If the event is on the server socket, 
            * it indicates a new incoming connection.
            * 
            * We need to accept the new connection and 
            * add it to our epoll instance for monitoring.
            * 
            */
           if(socket_fd == m_socket_fd)
           {
                while(true)
                {
                    int client_socket_fd = ::accept(m_socket_fd , nullptr , nullptr);
                    if(client_socket_fd < 0)
                    {
                        // No more incoming connections to accept
                        break;
                    }

                    LOG_INFO("Accepting new connection...");
                    network::utils::set_non_blocking(client_socket_fd);

                    connections[client_socket_fd].setSocketFd(client_socket_fd);
                    fd_in_use[client_socket_fd] = true;
                    
                    add_socket_in_epoll(epoll_fd , client_socket_fd);
                    
                }
           }
           else
           {
                // get the connection object
                // network::tcp::TcpConnection connection = connections[socket_fd];
                network::tcp::TcpConnection * connection = &connections[socket_fd];

                // receive data from the client
                while (true)
                {
                    // Start time measurement
                    uint64_t t0 = time_utils::now_ns();

                    ssize_t bytes_received = connection->receive(buffer, buffer_size);

                    // if bytes_received is 0, the client has closed the connection
                    if(bytes_received == 0)
                    {
                        LOG_INFO("Client closed connection");
                        // client closed
                        epoll_ctl(epoll_fd , EPOLL_CTL_DEL , socket_fd , nullptr);
                        // delete connection;
                        // connections[socket_fd] = nullptr;
                        connections[socket_fd].clear_connection();
                        fd_in_use[socket_fd] = false;

                        // Print latency statistics
                        if (!latencies.empty())
                        {
                            std::cout << "Server completed " << latencies.size() << " echo operations" << std::endl;

                            std::sort(latencies.begin(), latencies.end());

                            auto avg = std::accumulate(latencies.begin(), latencies.end(), 0ull) / latencies.size();

                            std::cout << "Avg ns : " << avg << "\n";
                            std::cout << "P50 ns : " << latencies[latencies.size() * 0.50] << "\n";
                            std::cout << "P99 ns : " << latencies[latencies.size() * 0.99] << "\n";
                        }

                        break;
                    }

                    /*
                     * If bytes_received is negative, 
                     * it indicates an error or that no more data is available.
                     */
                    if(bytes_received < 0)
                    {
                        // EAGAIN or EWOULDBLOCK : No more data to read
                        break;
                    }

                    // Echo the received data back to the client
                    connection->send(buffer , bytes_received);

                    // End time measurement
                    uint64_t t1 = time_utils::now_ns();
                    latencies.push_back(t1 - t0);

                }

           }
        }
        

    }
    
}

Server::~Server()
{
    if(m_socket_fd >= 0)
    {
        close(m_socket_fd);
    }
}