cmake_minimum_required(VERSION 3.16)

project(EmbeddedAiRealityBridge LANGUAGES CXX)

# 中文注释：仓库根目录作为 CMake 入口，避免 VS Code/STM32Cube 插件误判子目录项目。
add_subdirectory(src/pc)
