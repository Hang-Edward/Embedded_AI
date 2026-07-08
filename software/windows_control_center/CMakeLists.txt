cmake_minimum_required(VERSION 3.21)

project(EmbeddedAiControlCenter LANGUAGES CXX)

find_package(Qt6 QUIET COMPONENTS Widgets Network Concurrent)

if (BUILD_TESTING)
    find_package(Qt6 QUIET COMPONENTS Test)
endif()

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
    src/main.cpp
    src/AppConfig.cpp
    src/AgentWorkflowPolicy.cpp
    src/ApiResponseParser.cpp
    src/BasePage.cpp
    src/ChatInputPolicy.cpp
    src/ChatMessageWidget.cpp
    src/ChatPage.cpp
    src/ChatSessionStore.cpp
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
    src/SmoothScrollArea.cpp
    src/StatusCard.cpp
    src/SystemSelfTest.cpp
    src/SystemSelfTestReport.cpp
    src/Theme.cpp
)

target_include_directories(embedded_ai_control_center PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(embedded_ai_control_center PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
)

target_compile_definitions(embedded_ai_control_center PRIVATE
    QT_NO_KEYWORDS
)

if (BUILD_TESTING AND TARGET Qt6::Test)
    add_executable(embedded_ai_control_center_tests
        tests/CoreLogicTests.cpp
        src/AgentWorkflowPolicy.cpp
        src/ApiResponseParser.cpp
        src/ChatInputPolicy.cpp
        src/ChatSessionStore.cpp
        src/MarkdownLatexRenderer.cpp
        src/SystemSelfTestReport.cpp
    )

    target_include_directories(embedded_ai_control_center_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_libraries(embedded_ai_control_center_tests PRIVATE
        Qt6::Test
        Qt6::Widgets
        Qt6::Network
    )

    target_compile_definitions(embedded_ai_control_center_tests PRIVATE
        QT_NO_KEYWORDS
    )

    add_test(NAME windows_control_center_core
             COMMAND embedded_ai_control_center_tests)
    set_tests_properties(windows_control_center_core PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;EMBEDDED_AI_OFFLINE_LATEX=1")
endif()
