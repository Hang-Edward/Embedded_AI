cmake_minimum_required(VERSION 3.21)

project(EmbeddedAiControlCenter LANGUAGES CXX)

find_package(Qt6 QUIET COMPONENTS Widgets Network Concurrent)

if (NOT Qt6_FOUND)
    message(WARNING "Qt6 Widgets/Network/Concurrent not found. Skipping EmbeddedAiControlCenter.")
    return()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(embedded_ai_control_center WIN32
    resources.qrc
    app_icon.rc
    include/WebView2Widget.h
    src/main.cpp
    src/AppConfig.cpp
    src/BasePage.cpp
    src/ChatMessageWidget.cpp
    src/ChatPage.cpp
    src/DeepSeekChatClient.cpp
    src/MarkdownLatexRenderer.cpp
    src/ConnectionManager.cpp
    src/ConnectionState.cpp
    src/GlassSurface.cpp
    src/HardwarePage.cpp
    src/HistoryPage.cpp
    src/LogsPage.cpp
    src/CameraPage.cpp
    src/MainWindow.cpp
    src/QwenVisionQtClient.cpp
    src/SettingsPage.cpp
    src/StatusCard.cpp
    src/Theme.cpp
    src/WebView2Widget.cpp
)

target_include_directories(embedded_ai_control_center PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(WEBVIEW2_SDK_ROOT "${CMAKE_SOURCE_DIR}/third_party/webview2/1.0.4022.49")
set(WEBVIEW2_INCLUDE_DIR "${WEBVIEW2_SDK_ROOT}/build/native/include")
set(WEBVIEW2_LOADER_LIB "${WEBVIEW2_SDK_ROOT}/build/native/x64/WebView2Loader.dll.lib")

if (WIN32)
    if (NOT EXISTS "${WEBVIEW2_INCLUDE_DIR}/WebView2.h")
        message(FATAL_ERROR "WebView2 header not found: ${WEBVIEW2_INCLUDE_DIR}/WebView2.h")
    endif()
    if (NOT EXISTS "${WEBVIEW2_LOADER_LIB}")
        message(FATAL_ERROR "WebView2 loader import library not found: ${WEBVIEW2_LOADER_LIB}")
    endif()
    target_include_directories(embedded_ai_control_center PRIVATE
        "${WEBVIEW2_INCLUDE_DIR}"
    )
endif()

target_link_libraries(embedded_ai_control_center PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
)

if (WIN32)
    target_link_libraries(embedded_ai_control_center PRIVATE
        "${WEBVIEW2_LOADER_LIB}"
        ole32
    )
endif()

target_compile_definitions(embedded_ai_control_center PRIVATE
    QT_NO_KEYWORDS
)

if (WIN32 AND TARGET Qt6::windeployqt)
    # 让 build-qt 目录中的 exe 也能直接运行，避免误启动时加载到错误版本的 Qt DLL。
    add_custom_command(TARGET embedded_ai_control_center POST_BUILD
        COMMAND Qt6::windeployqt --release --no-translations $<TARGET_FILE:embedded_ai_control_center>
        COMMENT "Deploying Qt runtime beside the build executable"
    )
endif()
