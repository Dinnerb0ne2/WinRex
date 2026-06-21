#pragma once
#include <windows.h>
#include <windivert.h>
#include <iostream>

class NetworkBackend {
public:
    NetworkBackend() : m_handle(INVALID_HANDLE_VALUE) {}
    ~NetworkBackend() { close(); }

    bool open() {
        // 使用 NETWORK 层，只处理 IP 包
        m_handle = WinDivertOpen("true", WINDIVERT_LAYER_NETWORK, 0, 0);
        if (m_handle == INVALID_HANDLE_VALUE) {
            return false;
        }
        return true;
    }

    void close() {
        if (m_handle != INVALID_HANDLE_VALUE) {
            WinDivertClose(m_handle);
            m_handle = INVALID_HANDLE_VALUE;
        }
    }

    bool send(const uint8_t* data, uint32_t len) {
        if (m_handle == INVALID_HANDLE_VALUE) return false;
        WINDIVERT_ADDRESS addr;
        memset(&addr, 0, sizeof(addr));
        addr.Outbound = 1; // 标记为出站包，WinDivert 会自动通过正确网卡发送
        
        UINT written = 0;
        return WinDivertSend(m_handle, data, len, &written, &addr);
    }

private:
    HANDLE m_handle;
};
