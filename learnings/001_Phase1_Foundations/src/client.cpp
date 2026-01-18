#include <client.h>
#include "../../../common/timeutils.h"
#include <vector>
#include <algorithm>
#include <numeric>

Client::Client(const uint16_t port):
    m_port(port)
{
    // Constructor implementation (if needed)
}

bool Client::init()
{
    m_socket_fd = ::socket(AF_INET , SOCK_STREAM , 0);

    if(m_socket_fd < 0)
    {
        perror("Could not create socket");
        return false;
    }

    m_address = network::utils::create_ipv4_address(m_port , INADDR_ANY);
    return true;
}

bool Client::start(const int iterations)
{
    ::connect(m_socket_fd , (sockaddr*)&m_address , sizeof(m_address));

    network::tcp::TcpConnection connection(m_socket_fd);

    alignas(CACHE_LINE_SIZE) char send_buffer[1024];
    alignas(CACHE_LINE_SIZE) char receive_buffer[1024];

    std::vector<uint64_t> latencies;
    latencies.reserve(iterations);

    for(int i = 0; i < iterations; ++i)
    {
        uint64_t t0 = time_utils::now_ns();

        connection.send(send_buffer , sizeof(send_buffer));
        connection.receive(receive_buffer , sizeof(receive_buffer));

        uint64_t t1 = time_utils::now_ns();
        latencies.push_back(t1 - t0);

    }

    std::cout<<"Client completed "<<iterations<<" iterations"<<std::endl;

    std::sort(latencies.begin(), latencies.end());

    auto avg = std::accumulate(latencies.begin(), latencies.end(), 0ull) / latencies.size();

    std::cout << "Avg ns : " << avg << "\n";
    std::cout << "P50 ns : " << latencies[latencies.size() * 0.50] << "\n";
    std::cout << "P99 ns : " << latencies[latencies.size() * 0.99] << "\n";

    return true;

}

Client::~Client()
{
    if(m_socket_fd >= 0)
    {
        close(m_socket_fd);
        m_socket_fd = -1;
    }
}