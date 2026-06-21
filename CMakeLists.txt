cmake_minimum_required(VERSION 3.10)
project(HighPerf_DDoS_Windows)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ddos_flood
    main.cpp
    network_backend.h
    packet_builder.h
    flood_engine.h
)

# 1. 添加 WinDivert 的头文件搜索路径
target_include_directories(ddos_flood PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/WinDivert-2.2.2-A/include)

# 2. 链接 ws2_32 和指定路径下的 WinDivert.lib
target_link_libraries(ddos_flood PRIVATE ws2_32 ${CMAKE_CURRENT_SOURCE_DIR}/WinDivert-2.2.2-A/x64/WinDivert.lib)

# 针对 GCC/MinGW 的优化参数
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -static-libgcc -static-libstdc++")
endif()

# ==========================================
# 3. 配置 POST_BUILD 自动化复制脚本
# ==========================================

# 定义 run 文件夹的绝对路径
set(RUN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/run)

# 设置依赖文件的源路径
set(WINDIVERT_DLL ${CMAKE_CURRENT_SOURCE_DIR}/WinDivert-2.2.2-A/x64/WinDivert.dll)
set(WINDIVERT_SYS ${CMAKE_CURRENT_SOURCE_DIR}/WinDivert-2.2.2-A/x64/WinDivert64.sys)

# 在 ddos_flood 编译完成后执行以下命令
add_custom_command(TARGET ddos_flood POST_BUILD
    # 确保目标 run 文件夹存在，不存在则创建
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RUN_DIR}
    
    # 复制生成的可执行文件到 run 文件夹 (仅当文件有更新时才复制)
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:ddos_flood> ${RUN_DIR}
    
    # 复制 WinDivert.dll
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${WINDIVERT_DLL} ${RUN_DIR}
    
    # 复制 WinDivert64.sys
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${WINDIVERT_SYS} ${RUN_DIR}
    
    # 在控制台输出提示信息
    COMMENT "Copying ddos_flood.exe and dependencies to ${RUN_DIR}"
)
