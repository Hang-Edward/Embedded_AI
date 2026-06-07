cmake_minimum_required(VERSION 3.16)

project(EmbeddedAiPcBridge LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV REQUIRED COMPONENTS core imgcodecs videoio)
find_package(CURL REQUIRED)

add_executable(embedded_ai_pc_bridge
    src/Base64Encoder.cpp
    src/main.cpp
    src/App.cpp
    src/AuditLogStore.cpp
    src/CurlHttpClient.cpp
    src/DeviceComponent.cpp
    src/HttpClient.cpp
    src/ImageDataUrlBuilder.cpp
    src/KeyValueConfigFile.cpp
    src/MockAiVisionService.cpp
    src/NucleoOutputDevices.cpp
    src/OpenCvCameraService.cpp
    src/OutputDevice.cpp
    src/PrototypeDeviceSet.cpp
    src/QwenResponseParser.cpp
    src/QwenAsrService.cpp
    src/QwenVisionConfig.cpp
    src/QwenVisionService.cpp
    src/ShellAudioRecorder.cpp
    src/Console.cpp
    src/HardwareBridge.cpp
    src/SceneTask.cpp
    src/SerialPort.cpp
)

target_include_directories(embedded_ai_pc_bridge PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(embedded_ai_pc_bridge PRIVATE
    ${OpenCV_LIBS}
    CURL::libcurl
)

if (WIN32)
    target_compile_definitions(embedded_ai_pc_bridge PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()
