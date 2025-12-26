#include "engine.hpp"

#include <stdexcept>
#include <iostream>

static constexpr int BACKLOG = 128;

int EpollServer::set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

EpollServer::EpollServer(int port, size_t max_events, size_t max_connections)
    : port_(port), listen_fd_(-1), epoll_fd_(-1), running_(false), max_events_(max_events), max_connections_(max_connections) {
    buffer_area_.reset(new char[max_connections_ * ENGINE_BUFFER_SIZE]);
    connections_.resize(max_connections_);
    fd_to_idx_.assign(65536, -1);
    free_list_.reserve(max_connections_);
    for (size_t i = 0; i < max_connections_; ++i) {
        connections_[i].fd = -1;
        connections_[i].len = 0;
        connections_[i].buf = buffer_area_.get() + i * ENGINE_BUFFER_SIZE;
        free_list_.push_back((int)i);
    }
}

EpollServer::~EpollServer() {
    stop();
}

void EpollServer::setup_listener() {
    listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) throw std::runtime_error("socket failed");

    int one = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    set_nonblocking(listen_fd_);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);

    if (bind(listen_fd_, (sockaddr*)&addr, sizeof(addr)) < 0) throw std::runtime_error("bind failed");
    if (listen(listen_fd_, BACKLOG) < 0) throw std::runtime_error("listen failed");

    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ < 0) throw std::runtime_error("epoll_create1 failed");
}

void EpollServer::stop() {
    running_ = false;
}
