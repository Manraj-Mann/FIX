// Simple high-performance TCP server using epoll and non-blocking sockets
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <memory>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include "fix_parser.hpp"

static constexpr size_t ENGINE_BUFFER_SIZE = 64 * 1024;

struct Connection {
    int fd = -1;
    size_t len = 0;
    char* buf = nullptr;
};

class EpollServer {
public:
    // non-template constructor: choose max connections up-front
    EpollServer(int port, size_t max_events = 1024, size_t max_connections = 4096);
    ~EpollServer();

    // start listening and run event loop (blocking)
    // MessageHandler should be an inlinable callable: void(int fd, const char* msg, size_t len)
    template <typename MessageHandler>
    void run(MessageHandler&& on_message) {
        setup_listener();
        running_ = true;

        std::vector<epoll_event> events(max_events_);

        epoll_event ev{};
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = listen_fd_;
        epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &ev);

        while (running_) {
            int n = epoll_wait(epoll_fd_, events.data(), (int)events.size(), 1000);
            if (n < 0) {
                if (errno == EINTR) continue;
                break;
            }

            for (int i = 0; i < n; ++i) {
                int fd = events[i].data.fd;
                uint32_t evts = events[i].events;

                if (fd == listen_fd_) {
                    // accept loop
                    while (true) {
                        sockaddr_in client_addr;
                        socklen_t addrlen = sizeof(client_addr);
                        int c = accept(listen_fd_, (sockaddr*)&client_addr, &addrlen);
                        if (c < 0) break;
                        set_nonblocking(c);
                        epoll_event cev{};
                        cev.events = EPOLLIN | EPOLLET;
                        cev.data.fd = c;
                        epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, c, &cev);

                        // allocate from free list
                        if (free_list_.empty()) {
                            close(c);
                            continue;
                        }
                        int idx = free_list_.back(); free_list_.pop_back();
                        connections_[idx].fd = c;
                        connections_[idx].len = 0;
                        if ((size_t)c < fd_to_idx_.size()) fd_to_idx_[c] = idx;
                    }
                    continue;
                }

                if (evts & (EPOLLHUP | EPOLLERR)) {
                    // cleanup connection
                    int idx = -1;
                    if ((size_t)fd < fd_to_idx_.size()) idx = fd_to_idx_[fd];
                    if (idx >= 0) {
                        connections_[idx].fd = -1;
                        connections_[idx].len = 0;
                        fd_to_idx_[fd] = -1;
                        free_list_.push_back(idx);
                    }
                    close(fd);
                    continue;
                }

                if (evts & EPOLLIN) {
                    char tmp[ENGINE_BUFFER_SIZE];
                    while (true) {
                        ssize_t r = ::recv(fd, tmp, sizeof(tmp), 0);
                        if (r <= 0) {
                            if (r == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)) {
                                int idx = -1;
                                if ((size_t)fd < fd_to_idx_.size()) idx = fd_to_idx_[fd];
                                if (idx >= 0) {
                                    connections_[idx].fd = -1;
                                    connections_[idx].len = 0;
                                    fd_to_idx_[fd] = -1;
                                    free_list_.push_back(idx);
                                }
                                close(fd);
                            }
                            break;
                        }

                        int idx = -1;
                        if ((size_t)fd < fd_to_idx_.size()) idx = fd_to_idx_[fd];
                        if (idx < 0) { close(fd); break; }

                        Connection &c = connections_[idx];
                        size_t to_copy = (size_t)r;
                        if (c.len + to_copy > ENGINE_BUFFER_SIZE) {
                            close(fd);
                            c.fd = -1;
                            c.len = 0;
                            fd_to_idx_[fd] = -1;
                            free_list_.push_back(idx);
                            break;
                        }

                        memcpy(c.buf + c.len, tmp, to_copy);
                        c.len += to_copy;

                        size_t consumed = fix::parse_messages(c.buf, c.len, [&](const char* msg, size_t len){
                            on_message(fd, msg, len);
                        });
                        if (consumed > 0) {
                            size_t remain = c.len - consumed;
                            if (remain > 0) memmove(c.buf, c.buf + consumed, remain);
                            c.len = remain;
                        }
                    }
                }
            }
        }

        // cleanup preallocated connections
        for (int fd = 0; fd < (int)fd_to_idx_.size(); ++fd) {
            int idx = fd_to_idx_[fd];
            if (idx >= 0) close(fd);
        }

        if (listen_fd_ >= 0) close(listen_fd_);
        if (epoll_fd_ >= 0) close(epoll_fd_);
    }

    // stop the server (thread-safe)
    void stop();

private:
    int port_;
    int listen_fd_;
    int epoll_fd_;
    bool running_;
    size_t max_events_;
    size_t max_connections_;
    std::unique_ptr<char[]> buffer_area_;
    std::vector<Connection> connections_;
    std::vector<int> fd_to_idx_;
    std::vector<int> free_list_;
    void setup_listener();
    static int set_nonblocking(int fd);
};
