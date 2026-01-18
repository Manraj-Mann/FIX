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
#include "tcp_connection.h"

#define CACHE_LINE_SIZE 64

class Client
{
    public:

    explicit Client(const uint16_t port);
    ~Client();
    bool start(const int iterations);
    bool init();

    private:

    int m_socket_fd{-1};
    uint16_t m_port{0};
    sockaddr_in m_address{};

};