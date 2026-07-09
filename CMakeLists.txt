cmake_minimum_required(VERSION 3.16)

project(EmbeddedAiRealityBridge LANGUAGES CXX)

include(CTest)

# Hardware-side Raspberry Pi bridge and NUCLEO-facing console program.
add_subdirectory(hardware/pi_bridge)

# New Qt desktop control center. It is skipped automatically when Qt6 is absent.
option(BUILD_WINDOWS_CONTROL_CENTER "Build the Qt Windows control center" ON)
if (BUILD_WINDOWS_CONTROL_CENTER)
    add_subdirectory(software/windows_control_center)
endif()
