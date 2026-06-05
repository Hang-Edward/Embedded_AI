cmake_minimum_required(VERSION 3.16)

project(EmbeddedAiGui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV REQUIRED COMPONENTS core imgcodecs videoio)
find_package(CURL REQUIRED)

set(IMGUI_DIR third_party/imgui)

# 中文注释：Dear ImGui 核心 + Win32/OpenGL3 后端。
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# 中文注释：复用 PC 后端源文件，排除 Console.cpp / App.cpp / main.cpp。
set(BACKEND_SRC ../pc/src)
set(BACKEND_SOURCES
    ${BACKEND_SRC}/SerialPort.cpp
    ${BACKEND_SRC}/HardwareBridge.cpp
    ${BACKEND_SRC}/OpenCvCameraService.cpp
    ${BACKEND_SRC}/MockAiVisionService.cpp
    ${BACKEND_SRC}/QwenVisionService.cpp
    ${BACKEND_SRC}/QwenVisionConfig.cpp
    ${BACKEND_SRC}/CurlHttpClient.cpp
    ${BACKEND_SRC}/HttpClient.cpp
    ${BACKEND_SRC}/Base64Encoder.cpp
    ${BACKEND_SRC}/ImageDataUrlBuilder.cpp
    ${BACKEND_SRC}/QwenResponseParser.cpp
    ${BACKEND_SRC}/AuditLogStore.cpp
    ${BACKEND_SRC}/PrototypeDeviceSet.cpp
    ${BACKEND_SRC}/NucleoOutputDevices.cpp
    ${BACKEND_SRC}/OutputDevice.cpp
    ${BACKEND_SRC}/DeviceComponent.cpp
    ${BACKEND_SRC}/KeyValueConfigFile.cpp
    ${BACKEND_SRC}/SceneTask.cpp
)

# 中文注释：GUI 应用程序源文件。
set(GUI_SOURCES
    src/main.cpp
    src/AppGUI.cpp
)

add_executable(embedded_ai_gui
    ${GUI_SOURCES}
    ${IMGUI_SOURCES}
    ${BACKEND_SOURCES}
)

target_include_directories(embedded_ai_gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/${IMGUI_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/${IMGUI_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/../pc/include
)

target_link_libraries(embedded_ai_gui PRIVATE
    ${OpenCV_LIBS}
    CURL::libcurl
    opengl32
    gdi32
    dwmapi
)

if (WIN32)
    target_compile_definitions(embedded_ai_gui PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()
