#include "engine.hpp"

#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    const int port = 9878;
    EpollServer server(port);

    std::atomic<uint64_t> processed{0};

    std::thread server_thread([&]{
        server.run([&](int fd, const char* msg, size_t len){
            (void)fd; (void)msg; (void)len;
            processed.fetch_add(1, std::memory_order_relaxed);
        });
    });

    // give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    const int num_clients = 8;
    const int messages_per_client = 20000;

    std::string fix_msg;
    auto soh = [](void){ return char(1); }();
    fix_msg += "8=FIX.4.4"; fix_msg.push_back(soh);
    fix_msg += "9=12"; fix_msg.push_back(soh);
    fix_msg += "35=D"; fix_msg.push_back(soh);
    fix_msg += "49=SENDER"; fix_msg.push_back(soh);
    fix_msg += "56=TGT"; fix_msg.push_back(soh);
    fix_msg += "10=000"; fix_msg.push_back(soh);

    auto client_worker = [&](int id){
        int s = ::socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0) return;
        sockaddr_in sa{};
        sa.sin_family = AF_INET;
        sa.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &sa.sin_addr);
        if (connect(s, (sockaddr*)&sa, sizeof(sa)) < 0) { close(s); return; }

        for (int i = 0; i < messages_per_client; ++i) {
            ssize_t w = send(s, fix_msg.data(), fix_msg.size(), 0);
            if (w <= 0) break;
        }

        // wait a bit then close
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        close(s);
    };

    auto t0 = std::chrono::steady_clock::now();

    std::vector<std::thread> clients;
    for (int i = 0; i < num_clients; ++i) clients.emplace_back(client_worker, i);
    for (auto &t : clients) t.join();

    // wait for server to process remaining
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto t1 = std::chrono::steady_clock::now();

    double secs = std::chrono::duration<double>(t1 - t0).count();
    uint64_t count = processed.load();
    std::cout << "Processed messages: " << count << " in " << secs << "s => " << (count / secs) << " msg/s\n";

    server.stop();
    server_thread.join();
    return 0;
}
