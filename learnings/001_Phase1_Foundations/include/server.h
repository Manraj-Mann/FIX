#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <tcp_connection.h>

#define CACHE_LINE_SIZE 64

class Server
{
public:
    explicit Server(uint16_t port);
    ~Server();
    bool start();
    bool init();
    void run(const size_t max_socket_fds , const size_t buffer_size , const size_t max_events);
private:
    void add_socket_in_epoll(int epoll_fd , int socket_fd);
    network::tcp::TcpConnection m_connection;
    int m_socket_fd{-1};
    uint16_t m_port{0};
    sockaddr_in m_address{};
};