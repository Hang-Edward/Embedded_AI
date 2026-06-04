cmake_minimum_required(VERSION 3.16)

project(EmbeddedAiPcBridge LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(embedded_ai_pc_bridge
    src/main.cpp
    src/App.cpp
    src/AuditLogStore.cpp
    src/Console.cpp
    src/HardwareBridge.cpp
    src/SceneTask.cpp
    src/SerialPort.cpp
)

target_include_directories(embedded_ai_pc_bridge PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (WIN32)
    target_compile_definitions(embedded_ai_pc_bridge PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()
