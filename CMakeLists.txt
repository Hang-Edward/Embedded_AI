cmake_minimum_required(VERSION 3.16)

project(EmbeddedAiRealityBridge LANGUAGES CXX)

# 中文注释：仓库根目录作为 CMake 入口，避免 VS Code/STM32Cube 插件误判子目录项目。
add_subdirectory(src/pc)

# 中文注释：GUI 版本目前使用 Win32/ImGui 后端，树莓派默认只编译控制台版本。
option(BUILD_PC_GUI "Build the Windows desktop GUI prototype" ${WIN32})
if (BUILD_PC_GUI)
    add_subdirectory(src/pc_gui)
endif()
