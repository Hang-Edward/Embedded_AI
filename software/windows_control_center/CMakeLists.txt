cmake_minimum_required(VERSION 3.21)

project(EmbeddedAiControlCenter LANGUAGES CXX)

find_package(Qt6 QUIET COMPONENTS Widgets Network)

if (NOT Qt6_FOUND)
    message(WARNING "Qt6 Widgets/Network not found. Skipping EmbeddedAiControlCenter.")
    return()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(embedded_ai_control_center WIN32
    src/main.cpp
    src/AppConfig.cpp
    src/BasePage.cpp
    src/ChatMessageWidget.cpp
    src/ChatPage.cpp
    src/ConnectionManager.cpp
    src/ConnectionState.cpp
    src/HardwarePage.cpp
    src/HistoryPage.cpp
    src/LogsPage.cpp
    src/CameraPage.cpp
    src/MainWindow.cpp
    src/SettingsPage.cpp
    src/StatusCard.cpp
    src/Theme.cpp
)

target_include_directories(embedded_ai_control_center PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(embedded_ai_control_center PRIVATE
    Qt6::Widgets
    Qt6::Network
)

target_compile_definitions(embedded_ai_control_center PRIVATE
    QT_NO_KEYWORDS
)
