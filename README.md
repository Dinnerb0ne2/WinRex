# WinRex

高性能 Windows 网络压力测试工具（DDoS 模拟器）

> ⚠️ **免责声明**：本项目仅供安全研究和教育目的使用。严禁用于任何非法攻击活动。使用者须自行承担一切法律责任。


## 简介

WinRex 是一个基于 **WinDivert** 的高性能网络数据包发送工具，专为 Windows 平台设计。它支持多线程并发、精确的 PPS（包/秒）速率控制和 CPU 亲和性绑定，可用于网络压力测试和性能评估。

## 功能特点

- **多线程并发**：支持自定义线程数量，充分利用多核 CPU
- **CPU 亲和性绑定**：自动将线程绑定到不同 CPU 核心，提升性能
- **精确速率控制**：支持目标 PPS 控制，实现稳定的发包速率
- **实时统计**：实时记录并显示已发送的数据包总数
- **高优先级调度**：线程以时间关键优先级运行，减少延迟
- **预生成数据包池**：减少运行时开销，提升发包效率

## 技术栈

- **语言**：C++17
- **构建工具**：CMake 3.10+
- **核心依赖**：WinDivert 2.2.2

## 构建

### 前置条件

1. 下载 [WinDivert 2.2.2](https://github.com/basil00/Divert/releases)
2. 将 WinDivert 目录放置于项目根目录下，结构如下：

```
WinRex/
├── WinDivert-2.2.2-A/
│   ├── include/
│   │   └── windivert.h
│   └── x64/
│       ├── WinDivert.lib
│       ├── WinDivert.dll
│       └── WinDivert64.sys
├── CMakeLists.txt
├── flood_engine.h
├── network_backend.h
├── packet_builder.h
└── main.cpp
```

### 编译步骤

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

构建完成后，可执行文件及依赖（`WinDivert.dll`、`WinDivert64.sys`）会自动复制到 `run/` 目录下。

## 使用

**必须以管理员身份运行**（WinDivert 需要管理员权限）。

```bash
# 进入运行目录
cd run

# 运行（具体参数请参考程序帮助）
ddos_flood.exe
```

## 文件结构

```
├── CMakeLists.txt          # CMake 构建配置
├── flood_engine.h          # 核心发包引擎
├── network_backend.h       # 网络后端（WinDivert 封装）
├── packet_builder.h        # 数据包构造器
└── main.cpp                # 程序入口
```

## 许可

MIT License