#pragma once

#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstddef>
#include <cstdint>
#include <arpa/inet.h>
#include <fcntl.h>

namespace network
{

    namespace utils
    {
        static inline void set_non_blocking(int fd) 
        {
            /*
                fcntl function in C/C++ is used to perform various operations on file descriptors.
                Here, it's being used to set the file descriptor (fd) to non-blocking mode.

                F_GETFL : This command retrieves the current file status flags for the file descriptor.
                F_SETFL : This command sets the file status flags for the file descriptor.

                O_NONBLOCK : This flag is used to set the file descriptor to non-blocking mode.
                             In non-blocking mode, operations that would normally block (like read or write)
                             will return immediately with a failure indication if they cannot be completed
                             right away, instead of waiting.

            */
            int flags = fcntl(fd, F_GETFL, 0);
            fcntl(fd, F_SETFL, flags | O_NONBLOCK);
        }

        static inline void set_reuse_address(int fd)
        {
            /*
                This function sets the SO_REUSEADDR option on a socket.
                This option allows a socket to bind to an address that is already in use.

                Why is this useful?
                When a server socket is closed, the operating system may keep the port in a TIME_WAIT state
                for a certain period of time. During this time, if you try to restart the server and bind to the same port,
                it may fail with an "Address already in use" error. By setting SO_REUSEADDR, you allow the server to
                rebind to the port even if it's in the TIME_WAIT state.

            */
            int opt = 1;
            setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        }

        static inline sockaddr_in create_ipv4_address(uint16_t port, uint32_t ip = INADDR_ANY)
        {
            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = htonl(ip);
            return addr;
        }


    } // namespace utils

    namespace tcp
    {
        
        class TcpConnection
        {
            public:
            
            explicit TcpConnection(int socket_fd)
                : m_socket_fd(socket_fd)
            {
                // disable Nagle's algorithm for lower latency
                if(m_socket_fd >= 0)
                {
                    disableNaglesAlgorithm();
                }
            }

            TcpConnection()
                : m_socket_fd(-1)
            {
            }

            ~TcpConnection()
            {
                if (m_socket_fd >= 0)
                {
                    // this methods comes from <unistd.h>
                    close(m_socket_fd);
                }
            }

            // disable copy constructor and assignment
            TcpConnection(const TcpConnection&) = delete;
            TcpConnection& operator=(const TcpConnection&) = delete;

            // move constructor
            TcpConnection(TcpConnection&& other) noexcept
                : m_socket_fd(other.m_socket_fd)
            {
                other.m_socket_fd = -1;
            }

            int getSocketFd() const
            {
                return m_socket_fd;
            }

            void setSocketFd(int socket_fd)
            {
                m_socket_fd = socket_fd;
                disableNaglesAlgorithm();
            }
            // send data over the TCP connection
            ssize_t send(const void * data , size_t size)
            {
                // MSG_NOSIGNAL : This flag prevents the generation of SIGPIPE signals.
                return ::send(m_socket_fd, data, size, MSG_NOSIGNAL);
            }

            // receive data from the TCP connection
            ssize_t receive(void * buffer, size_t size)
            {
                return ::recv(m_socket_fd, buffer, size, 0);
            }
            
            void clear_connection()
            {
                m_socket_fd = -1;
            }

            void close_connection()
            {
                if (m_socket_fd >= 0)
                {
                    close(m_socket_fd);
                    m_socket_fd = -1;
                }
            }
            
        private:
            int m_socket_fd{-1};

            // disable nagle's algorithm
            void disableNaglesAlgorithm()
            {
                /*
                    Why disabale Nagle's algorithm?
                    Nagle's algorithm is designed to reduce network congestion by combining a number of small outgoing
                    messages and sending them all at once. While this can be beneficial in many scenarios, it can introduce latency
                    in applications that require low-latency communication. 
                */ 
                int flag = 1;
                
                /*
                What this code does : 

                IPPROTO_TCP : This specifies that the option is for the TCP protocol.

                TCP_NODELAY : This is the specific option being set, which disables Nagle's algorithm.
                                This flag tells the TCP stack to send out packets immediately, 
                                without waiting to accumulate a larger amount of data.

                &flag       : This is a pointer to the value being set for the option. 
                                In this case, it's a pointer to an integer with a value of 1, 
                                which means "enable" the TCP_NODELAY option (i.e., disable Nagle's algorithm).
                
                sizeof(flag): This specifies the size of the option value being set.

                */ 

                setsockopt(m_socket_fd , IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
            }
        };
    }
    
}