/*
 * Embedded AI Reality Bridge — GUI 应用程序实现
 * 包含侧边栏导航、6个功能页面、后端操作封装和纹理管理。
 */

#include "AppGUI.h"
#include "imgui.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <cstdio>
#include <ctime>
#include <sstream>

// OpenGL 1.1 不定义 GL_CLAMP_TO_EDGE，手动补全
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

// ===================================================================
// 构造 / 析构
// ===================================================================

AppGUI::AppGUI() {
    // 启动时清理上一次留下的抓拍照片
    WIN32_FIND_DATAA ffd;
    HANDLE hFind = FindFirstFileA("captures\\*.jpg", &ffd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            DeleteFileA(("captures\\" + std::string(ffd.cFileName)).c_str());
        } while (FindNextFileA(hFind, &ffd));
        FindClose(hFind);
    }

    // 加载 Qwen 视觉配置
    QwenVisionConfigLoader loader;
    qwenConfig_ = loader.load(configPath_);
    if (qwenConfig_.enabled && qwenConfig_.hasApiKey()) {
        qwenVision_ = std::make_unique<QwenVisionService>(qwenConfig_, httpClient_);
    }

    // 初始化设置页输入
    std::strncpy(comPort_, "COM11", sizeof(comPort_) - 1);
    baudRate_ = 115200;
    settingsCamIdx_ = 0;
    std::strncpy(qwenEndpoint_, qwenConfig_.baseUrl.c_str(), sizeof(qwenEndpoint_) - 1);
    std::strncpy(qwenModel_, qwenConfig_.model.c_str(), sizeof(qwenModel_) - 1);
    std::strncpy(qwenApiKey_, qwenConfig_.apiKey.c_str(), sizeof(qwenApiKey_) - 1);
    std::strncpy(configPath_, qwenConfig_.configFilePath.c_str(), sizeof(configPath_) - 1);
    useQwen_ = qwenConfig_.enabled;

    // 启动时自动尝试连接串口硬件
    connectSerial();
}

AppGUI::~AppGUI() {
    releaseCameraTexture();
    disconnectSerial();
}

// ===================================================================
// 窗口 resize
// ===================================================================

void AppGUI::onResize(int /*width*/, int /*height*/) {
    // ImGui 自动处理 viewport resize
}

// ===================================================================
// 主渲染入口
// ===================================================================

void AppGUI::render() {
    const ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoBringToFrontOnFocus |
                             ImGuiWindowFlags_NoSavedSettings;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("MainWindow", nullptr, flags);
    ImGui::PopStyleVar(2);

    // ---- 顶部工具栏 ----
    renderTopBar();
    ImGui::Separator();

    // ---- 侧边栏 ----
    const float sidebarW = 170.0f;
    ImGui::BeginChild("Sidebar", ImVec2(sidebarW, ImGui::GetContentRegionAvail().y - 2), false,
                      ImGuiWindowFlags_NoScrollbar);
    renderSidebar();
    ImGui::EndChild();

    ImGui::SameLine();
    // 内容区
    ImGui::BeginChild("Content");
    renderPage();
    ImGui::EndChild();

    // ---- 底部状态栏 ----
    ImGui::Separator();
    renderStatusBar();

    ImGui::End();
}

// ===================================================================
// 顶部工具栏
// ===================================================================

void AppGUI::renderTopBar() {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
    ImGui::TextUnformatted("  Embedded AI Reality Bridge");
    ImGui::SameLine(ImGui::GetWindowWidth() - 60);
    if (ImGui::Button("Exit")) {
        PostQuitMessage(0);
    }
    ImGui::PopStyleColor();
}

// ===================================================================
// 底部状态栏
// ===================================================================

void AppGUI::renderStatusBar() {
    ImGui::TextUnformatted(serialConnected_
        ? "  COM: Connected"
        : "  COM: Disconnected");
    ImGui::SameLine(200);
    ImGui::TextUnformatted(useQwen_ ? "  AI: Qwen API" : "  AI: Mock");
    ImGui::SameLine(400);
    ImGui::Text("  Camera: idx %d", cameraIndex_);
}

// ===================================================================
// 侧边栏导航
// ===================================================================

void AppGUI::renderSidebar() {
    struct NavItem {
        Page  page;
        const char* icon;
        const char* label;
    };
    static const NavItem items[] = {
        { Page::Dashboard, "  ", "仪表盘" },
        { Page::Camera,    "  ", "摄像头" },
        { Page::AIVision,  "  ", "AI 视觉分析" },
        { Page::Hardware,  "  ", "硬件控制" },
        { Page::AuditLog,  "  ", "审计日志" },
        { Page::Settings,  "  ", "设置" }
    };

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

    for (const auto& item : items) {
        bool isActive = currentPage_ == item.page;
        if (isActive) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 0.40f));
        }
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%s %s", item.icon, item.label);
        if (ImGui::Button(buf, ImVec2(ImGui::GetContentRegionAvail().x, 36))) {
            currentPage_ = item.page;
        }
        if (isActive) {
            ImGui::PopStyleColor();
        }
    }

    ImGui::PopStyleVar(2);
}

// ===================================================================
// 页面分发
// ===================================================================

void AppGUI::renderPage() {
    switch (currentPage_) {
    case Page::Dashboard: pageDashboard(); break;
    case Page::Camera:    pageCamera();    break;
    case Page::AIVision:  pageAIVision();  break;
    case Page::Hardware:  pageHardware();  break;
    case Page::AuditLog:  pageAuditLog();  break;
    case Page::Settings:  pageSettings();  break;
    default: break;
    }
}

// ===================================================================
// 仪表盘
// ===================================================================

void AppGUI::pageDashboard() {
    ImGui::TextUnformatted("\xe4\xbb\xaa\xe8\xa1\xa8\xe7\x9b\x98"); // 仪表盘
    ImGui::Separator();
    ImGui::Spacing();

    // 三列状态卡片
    const float cardW = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2) / 3.0f;

    // ---- 串口状态 ----
    ImGui::BeginChild("CardSerial", ImVec2(cardW, 100), true);
    ImGui::TextUnformatted("\xe4\xb8\xb2\xe5\x8f\xa3\xe8\xbf\x9e\xe6\x8e\xa5"); // 串口连接
    ImGui::Separator();
    if (serialConnected_) {
        ImGui::TextColored(ImVec4(0,1,0,1), "\xe2\x97\x8f \xe5\xb7\xb2\xe8\xbf\x9e\xe6\x8e\xa5"); // ● 已连接
        ImGui::TextUnformatted(serial_ ? serial_->portName().c_str() : "");
    } else {
        ImGui::TextColored(ImVec4(1,0,0,1), "\xe2\x97\x8b \xe6\x9c\xaa\xe8\xbf\x9e\xe6\x8e\xa5"); // ○ 未连接
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // ---- 摄像头状态 ----
    ImGui::BeginChild("CardCamera", ImVec2(cardW, 100), true);
    ImGui::TextUnformatted("\xe6\x91\x84\xe5\x83\x8f\xe5\xa4\xb4"); // 摄像头
    ImGui::Separator();
    ImGui::TextUnformatted(frameCaptured_
        ? "\xe2\x97\x8f \xe5\xb7\xb2\xe6\x8d\x95\xe8\x8e\xb7\xe5\xb8\xa7"  // ● 已捕获帧
        : "\xe2\x97\x8b \xe6\x9c\xaa\xe6\x8b\x8d\xe6\x91\x84");            // ○ 未拍摄
    ImGui::EndChild();

    ImGui::SameLine();

    // ---- AI 服务状态 ----
    ImGui::BeginChild("CardAI", ImVec2(cardW, 100), true);
    ImGui::TextUnformatted("AI \xe6\x9c\x8d\xe5\x8a\xa1"); // AI 服务
    ImGui::Separator();
    if (useQwen_ && qwenVision_) {
        ImGui::TextColored(ImVec4(0,1,0,1), "\xe2\x97\x8f Qwen API"); // ● Qwen API
    } else {
        ImGui::TextColored(ImVec4(0,0.8f,1,1), "\xe2\x97\x8f Mock AI"); // ● Mock AI
    }
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextUnformatted("\xe5\xbf\xab\xe9\x80\x9f\xe6\x93\x8d\xe4\xbd\x9c"); // 快速操作
    ImGui::Spacing();

    if (ImGui::Button("\xe6\x8d\x95\xe8\x8e\xb7\xe5\xb8\xa7\xe9\x9d\xa2 \xe5\xb9\xb6\xe5\x88\x86\xe6\x9e\x90", // 捕获画面并分析
                      ImVec2(220, 0))) {
        // 捕获 + 分析快捷操作
        std::string path = "captures/gui_capture_" + std::to_string(std::time(nullptr)) + ".jpg";
        auto result = camera_.captureFrame(path);
        if (result.success) {
            lastCapturePath_ = result.filePath;
            lastFrame_ = cv::imread(result.filePath);
            if (!lastFrame_.empty()) {
                releaseCameraTexture();
                cameraTexture_ = createTextureFromMat(lastFrame_);
                frameCaptured_ = true;
                texWidth_ = lastFrame_.cols;
                texHeight_ = lastFrame_.rows;
                addEventLog("Camera: captured " + result.filePath);
            }
            // 自动分析
            AiVisionService* svc = (useQwen_ && qwenVision_)
                ? static_cast<AiVisionService*>(qwenVision_.get())
                : static_cast<AiVisionService*>(&mockVision_);
            auto analysis = svc->analyzeImage(result.filePath, TaskType::SceneDescription);
            hasResult_ = analysis.success;
            if (analysis.success) {
                aiResult_ = analysis.title + "\n" + analysis.summary;
                addEventLog("AI: " + analysis.title + " — " + analysis.summary);
                currentPage_ = Page::AIVision;   // 分析完成后自动跳转到 AI 视觉页面展示结果
            } else {
                aiResult_ = "分析失败: " + analysis.message;
                addEventLog("AI: 分析失败 — " + analysis.message);
            }
        } else {
            addEventLog("Camera: " + result.message);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("\xe7\xa1\xac\xe4\xbb\xb6\xe8\x87\xaa\xe6\xa3\x80", // 硬件自检
                      ImVec2(140, 0))) {
        if (serialConnected_ && devices_) {
            bool ok = devices_->runSelfTest();
            addEventLog(ok ? "Self-test: PASS" : "Self-test: FAIL");
        } else {
            addEventLog("Self-test: serial not connected");
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("\xe5\x88\xb7\xe6\x96\xb0\xe5\xae\xa1\xe8\xae\xa1\xe6\x97\xa5\xe5\xbf\x97", // 刷新审计日志
                      ImVec2(160, 0))) {
        refreshAuditLog();
    }

    // 最近事件日志（仅新条目时自动滚到底部）
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextUnformatted("\xe6\x9c\x80\xe8\xbf\x91\xe4\xba\x8b\xe4\xbb\xb6"); // 最近事件
    ImGui::BeginChild("EventLog", ImVec2(0, ImGui::GetContentRegionAvail().y), true);
    if (eventLogUpdated_) {
        ImGui::SetScrollHereY(1.0f);
        eventLogUpdated_ = false;
    }
    for (const auto& ev : eventLog_) {
        ImGui::TextUnformatted(ev.c_str());
    }
    ImGui::EndChild();
}

// ===================================================================
// 摄像头
// ===================================================================

void AppGUI::pageCamera() {
    ImGui::TextUnformatted("\xe6\x91\x84\xe5\x83\x8f\xe5\xa4\xb4"); // 摄像头
    ImGui::Separator();
    ImGui::Spacing();

    // 控制按钮行
    if (ImGui::Button("\xe6\x8d\x95\xe8\x8e\xb7\xe5\xbd\x93\xe5\x89\x8d\xe5\xb8\xa7", // 捕获当前帧
                      ImVec2(180, 0))) {
        std::string path = "captures/gui_capture_" + std::to_string(std::time(nullptr)) + ".jpg";
        auto result = camera_.captureFrame(path);
        if (result.success) {
            lastCapturePath_ = result.filePath;
            lastFrame_ = cv::imread(result.filePath);
            if (!lastFrame_.empty()) {
                releaseCameraTexture();
                cameraTexture_ = createTextureFromMat(lastFrame_);
                frameCaptured_ = true;
                texWidth_ = lastFrame_.cols;
                texHeight_ = lastFrame_.rows;
                addEventLog("Camera: captured " + result.filePath);
            }
        } else {
            addEventLog("Camera: " + result.message);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("\xe6\xb8\x85\xe9\x99\xa4\xe5\x9b\xbe\xe5\x83\x8f", // 清除图像
                      ImVec2(120, 0))) {
        releaseCameraTexture();
        frameCaptured_ = false;
        lastFrame_ = cv::Mat();
        lastCapturePath_.clear();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 显示捕获的图像
    if (frameCaptured_ && cameraTexture_ != 0) {
        float maxW = ImGui::GetContentRegionAvail().x;
        float maxH = ImGui::GetContentRegionAvail().y - 30;
        float scale = std::min(maxW / texWidth_, maxH / texHeight_);
        float dispW = texWidth_ * scale;
        float dispH = texHeight_ * scale;

        ImGui::Image((ImTextureID)(intptr_t)cameraTexture_, ImVec2(dispW, dispH));
        ImGui::Text("Size: %d x %d  |  File: %s", texWidth_, texHeight_, lastCapturePath_.c_str());
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1),
            "\xe6\x9a\x82\xe6\x97\xa0\xe5\x9b\xbe\xe5\x83\x8f\xef\xbc\x8c\xe8\xaf\xb7\xe7\x82\xb9\xe5\x87\xbb"
            "\xe2\x80\x9c\xe6\x8d\x95\xe8\x8e\xb7\xe5\xbd\x93\xe5\x89\x8d\xe5\xb8\xa7\xe2\x80\x9d"); // 暂无图像
    }
}

// ===================================================================
// AI 视觉分析
// ===================================================================

void AppGUI::pageAIVision() {
    ImGui::TextUnformatted("AI \xe8\xa7\x86\xe8\xa7\x89\xe5\x88\x86\xe6\x9e\x90"); // AI 视觉分析
    ImGui::Separator();
    ImGui::Spacing();

    // 模式选择
    ImGui::TextUnformatted("\xe5\x88\x86\xe6\x9e\x90\xe6\xa8\xa1\xe5\xbc\x8f\xef\xbc\x9a"); // 分析模式：
    ImGui::SameLine();
    if (ImGui::RadioButton("Mock AI", !useQwen_)) {
        useQwen_ = false;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Qwen API", useQwen_)) {
        useQwen_ = true;
    }
    ImGui::Spacing();

    // 当前图像缩略图
    if (frameCaptured_ && cameraTexture_ != 0) {
        float thumbW = std::min(320.0f, ImGui::GetContentRegionAvail().x * 0.4f);
        float scale = thumbW / texWidth_;
        ImGui::Image((ImTextureID)(intptr_t)cameraTexture_, ImVec2(thumbW, texHeight_ * scale));
        ImGui::SameLine();
    }

    // 分析按钮
    ImGui::BeginDisabled(!frameCaptured_);
    if (ImGui::Button("\xe5\xbc\x80\xe5\xa7\x8b\xe5\x88\x86\xe6\x9e\x90", // 开始分析
                      ImVec2(160, 36))) {
        AiVisionService* svc = (useQwen_ && qwenVision_)
            ? static_cast<AiVisionService*>(qwenVision_.get())
            : static_cast<AiVisionService*>(&mockVision_);
        auto analysis = svc->analyzeImage(lastCapturePath_, TaskType::SceneDescription);
        hasResult_ = analysis.success;
        if (analysis.success) {
            aiResult_ = analysis.title + "\n" + analysis.summary;
            addEventLog("AI: analysis complete (mode: " + std::string(useQwen_ ? "Qwen" : "Mock") + ")");
        } else {
            aiResult_ = "Analysis failed: " + analysis.message;
            addEventLog("AI: analysis failed - " + analysis.message);
        }
    }
    ImGui::EndDisabled();

    if (!frameCaptured_) {
        ImGui::TextColored(ImVec4(1,1,0,1),
            "\xe8\xaf\xb7\xe5\x85\x88\xe5\x9c\xa8\xe2\x80\x9c\xe6\x91\x84\xe5\x83\x8f\xe5\xa4\xb4\xe2\x80\x9d"
            "\xe9\xa1\xb5\xe6\x8d\x95\xe8\x8e\xb7\xe4\xb8\x80\xe5\xb8\xa7\xe5\x9b\xbe\xe5\x83\x8f"); // 请先在"摄像头"页...
    }

    ImGui::Spacing();

    // 分析结果显示
    if (hasResult_ && !aiResult_.empty()) {
        ImGui::TextUnformatted("\xe5\x88\x86\xe6\x9e\x90\xe7\xbb\x93\xe6\x9e\x9c\xef\xbc\x9a"); // 分析结果：
        ImGui::Separator();
        ImGui::BeginChild("AIResult", ImVec2(0, ImGui::GetContentRegionAvail().y), true);
        ImGui::TextWrapped("%s", aiResult_.c_str());
        ImGui::EndChild();
    }

    // Qwen 配置摘要（Qwen 模式时显示）
    if (useQwen_) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("Qwen Config:");
        ImGui::Text("  Endpoint: %s", qwenConfig_.baseUrl.c_str());
        ImGui::Text("  Model: %s", qwenConfig_.model.c_str());
        ImGui::Text("  API Key: %s", qwenConfig_.hasApiKey() ? "[configured]" : "[not set]");
    }
}

// ===================================================================
// 硬件控制
// ===================================================================

void AppGUI::pageHardware() {
    ImGui::TextUnformatted("\xe7\xa1\xac\xe4\xbb\xb6\xe6\x8e\xa7\xe5\x88\xb6"); // 硬件控制
    ImGui::Separator();
    ImGui::Spacing();

    // 连接状态
    if (serialConnected_) {
        ImGui::TextColored(ImVec4(0,1,0,1), "\xe2\x97\x8f \xe5\xb7\xb2\xe8\xbf\x9e\xe6\x8e\xa5 %s",
                           serial_ ? serial_->portName().c_str() : "");
    } else {
        ImGui::TextColored(ImVec4(1,0,0,1), "\xe2\x97\x8b \xe6\x9c\xaa\xe8\xbf\x9e\xe6\x8e\xa5\xe4\xb8\xb2\xe5\x8f\xa3"); // ○ 未连接串口
        if (!connError_.empty()) {
            ImGui::TextColored(ImVec4(1,0.5f,0,1), "%s", connError_.c_str());
        }
        ImGui::Spacing();
        if (ImGui::Button("\xe8\xbf\x9e\xe6\x8e\xa5\xe4\xb8\xb2\xe5\x8f\xa3", // 连接串口
                          ImVec2(140, 0))) {
            if (connectSerial()) {
                addEventLog("Serial: connected to " + std::string(comPort_));
            } else {
                addEventLog("Serial: connection failed");
            }
        }
        return; // 未连接时不显示控制面板
    }

    // ---- 断开按钮 ----
    if (ImGui::Button("\xe6\x96\xad\xe5\xbc\x80\xe8\xbf\x9e\xe6\x8e\xa5", // 断开连接
                      ImVec2(120, 0))) {
        disconnectSerial();
        addEventLog("Serial: disconnected");
        return;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ---- LED 控制 ----
    ImGui::TextUnformatted("LED");
    ImGui::SameLine(80);
    if (ImGui::Button(ledOn_ ? "\xe5\x85\xb3\xe9\x97\xad LED" : "\xe6\x89\x93\xe5\xbc\x80 LED", // 关闭 LED / 打开 LED
                      ImVec2(120, 0))) {
        ledOn_ = !ledOn_;
        if (hardware_) {
            bool ok = hardware_->setLed(ledOn_);
            hwResponse_ = ok ? "OK" : "FAIL";
            addEventLog(std::string("LED ") + (ledOn_ ? "ON" : "OFF") + " -> " + hwResponse_);
        }
    }
    ImGui::SameLine();
    ImGui::TextUnformatted(ledOn_ ? "\xe2\x9c\x93 ON" : "\xe2\x9c\x97 OFF"); // ✓ ON / ✗ OFF

    // ---- 蜂鸣器 ----
    ImGui::Spacing();
    ImGui::TextUnformatted("\xe8\x9c\x82\xe9\xb8\xa3\xe5\x99\xa8"); // 蜂鸣器
    ImGui::SameLine(80);
    if (ImGui::Button(buzzerOn_ ? "\xe5\x85\xb3\xe9\x97\xad\xe8\x9c\x82\xe9\xb8\xa3" : "\xe6\x89\x93\xe5\xbc\x80\xe8\x9c\x82\xe9\xb8\xa3", // 关闭蜂鸣 / 打开蜂鸣
                      ImVec2(120, 0))) {
        buzzerOn_ = !buzzerOn_;
        if (hardware_) {
            bool ok = hardware_->setBuzzer(buzzerOn_);
            hwResponse_ = ok ? "OK" : "FAIL";
            addEventLog(std::string("Buzzer ") + (buzzerOn_ ? "ON" : "OFF") + " -> " + hwResponse_);
        }
    }
    ImGui::SameLine();
    ImGui::TextUnformatted(buzzerOn_ ? "\xe2\x9c\x93 ON" : "\xe2\x9c\x97 OFF");

    // ---- 震动模块 ----
    ImGui::Spacing();
    ImGui::TextUnformatted("\xe9\x9c\x87\xe5\x8a\xa8\xe6\xa8\xa1\xe5\x9d\x97"); // 震动模块
    ImGui::SameLine(80);
    if (ImGui::Button(vibrationOn_ ? "\xe5\x85\xb3\xe9\x97\xad\xe9\x9c\x87\xe5\x8a\xa8" : "\xe6\x89\x93\xe5\xbc\x80\xe9\x9c\x87\xe5\x8a\xa8", // 关闭震动 / 打开震动
                      ImVec2(120, 0))) {
        vibrationOn_ = !vibrationOn_;
        if (hardware_) {
            bool ok = hardware_->setVibration(vibrationOn_);
            hwResponse_ = ok ? "OK" : "FAIL";
            addEventLog(std::string("Vibration ") + (vibrationOn_ ? "ON" : "OFF") + " -> " + hwResponse_);
        }
    }
    ImGui::SameLine();
    ImGui::TextUnformatted(vibrationOn_ ? "\xe2\x9c\x93 ON" : "\xe2\x9c\x97 OFF");

    // ---- OLED 文本 ----
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextUnformatted("OLED \xe6\x98\xbe\xe7\xa4\xba"); // OLED 显示
    ImGui::InputText("##oled", oledInput_, sizeof(oledInput_));
    ImGui::SameLine();
    if (ImGui::Button("\xe5\x8f\x91\xe9\x80\x81", ImVec2(80, 0))) { // 发送
        if (hardware_ && std::strlen(oledInput_) > 0) {
            bool ok = hardware_->showOledText(oledInput_);
            hwResponse_ = ok ? "OK" : "FAIL";
            addEventLog(std::string("OLED: ") + oledInput_ + " -> " + hwResponse_);
        }
    }

    // ---- 手动命令 ----
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextUnformatted("\xe6\x89\x8b\xe5\x8a\xa8\xe5\x91\xbd\xe4\xbb\xa4"); // 手动命令
    ImGui::InputText("##cmd", manualCmd_, sizeof(manualCmd_));
    ImGui::SameLine();
    if (ImGui::Button("\xe5\x8f\x91\xe9\x80\x81\xe5\x91\xbd\xe4\xbb\xa4", // 发送命令
                      ImVec2(100, 0))) {
        if (serial_ && std::strlen(manualCmd_) > 0) {
            std::string fullCmd = manualCmd_;
            std::string resp = serial_->readAvailable(50); // 清除之前残留
            serial_->writeLine(fullCmd);
            hwResponse_ = serial_->readAvailable(200);
            addEventLog("CMD: " + fullCmd + " -> " + hwResponse_);
        }
    }

    // ---- 响应日志 ----
    if (!hwResponse_.empty()) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("\xe5\x93\x8d\xe5\xba\x94"); // 响应
        ImGui::BeginChild("HWResponse", ImVec2(0, 80), true);
        ImGui::TextUnformatted(hwResponse_.c_str());
        ImGui::EndChild();
    }
}

// ===================================================================
// 审计日志
// ===================================================================

void AppGUI::pageAuditLog() {
    ImGui::TextUnformatted("\xe5\xae\xa1\xe8\xae\xa1\xe6\x97\xa5\xe5\xbf\x97"); // 审计日志
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("\xe5\x88\xb7\xe6\x96\xb0", ImVec2(100, 0))) { // 刷新
        refreshAuditLog();
    }
    ImGui::SameLine();
    if (ImGui::Button("\xe6\xb8\x85\xe7\xa9\xba\xe7\xbc\x93\xe5\xad\x98", // 清空缓存
                      ImVec2(100, 0))) {
        auditCache_.clear();
    }
    ImGui::Spacing();

    if (auditCache_.empty()) {
        refreshAuditLog();
    }

    if (auditCache_.empty()) {
        ImGui::TextColored(ImVec4(0.5f,0.5f,0.5f,1),
            "\xe6\x9a\x82\xe6\x97\xa0\xe5\xae\xa1\xe8\xae\xa1\xe8\xae\xb0\xe5\xbd\x95"); // 暂无审计记录
        return;
    }

    // 表格
    const float tableH = ImGui::GetContentRegionAvail().y;
    ImGui::BeginChild("AuditTable", ImVec2(0, tableH), true);

    ImGui::Columns(4, "AuditColumns", false);
    ImGui::TextUnformatted("ID"); ImGui::NextColumn();
    ImGui::TextUnformatted("\xe6\x97\xb6\xe9\x97\xb4"); ImGui::NextColumn(); // 时间
    ImGui::TextUnformatted("\xe5\x8a\xa8\xe4\xbd\x9c"); ImGui::NextColumn(); // 动作
    ImGui::TextUnformatted("\xe7\x8a\xb6\xe6\x80\x81"); ImGui::NextColumn(); // 状态
    ImGui::Separator();

    for (const auto& entry : auditCache_) {
        ImGui::Text("%u", entry.id); ImGui::NextColumn();
        ImGui::TextUnformatted(entry.timestamp.c_str()); ImGui::NextColumn();
        ImGui::TextUnformatted(entry.action.c_str()); ImGui::NextColumn();
        ImGui::TextUnformatted(auditStatusToText(entry.status).c_str()); ImGui::NextColumn();
    }

    ImGui::Columns(1);
    ImGui::EndChild();
}

// ===================================================================
// 设置
// ===================================================================

void AppGUI::pageSettings() {
    ImGui::TextUnformatted("\xe8\xae\xbe\xe7\xbd\xae"); // 设置
    ImGui::Separator();
    ImGui::Spacing();

    // ---- 串口设置 ----
    ImGui::TextUnformatted("\xe4\xb8\xb2\xe5\x8f\xa3\xe9\x85\x8d\xe7\xbd\xae"); // 串口配置
    ImGui::InputText("COM \xe5\x8f\xa3", comPort_, sizeof(comPort_)); // COM 口
    ImGui::InputInt("\xe6\xb3\xa2\xe7\x89\xb9\xe7\x8e\x87", &baudRate_); // 波特率
    ImGui::Spacing();

    // ---- 摄像头设置 ----
    ImGui::Separator();
    ImGui::TextUnformatted("\xe6\x91\x84\xe5\x83\x8f\xe5\xa4\xb4\xe9\x85\x8d\xe7\xbd\xae"); // 摄像头配置
    ImGui::InputInt("\xe6\x91\x84\xe5\x83\x8f\xe5\xa4\xb4\xe7\xb4\xa2\xe5\xbc\x95", &settingsCamIdx_); // 摄像头索引
    ImGui::Spacing();

    // ---- Qwen API 设置 ----
    ImGui::Separator();
    ImGui::TextUnformatted("Qwen \xe8\xa7\x86\xe8\xa7\x89 API \xe9\x85\x8d\xe7\xbd\xae"); // Qwen 视觉 API 配置
    ImGui::InputText("Config \xe6\x96\x87\xe4\xbb\xb6\xe8\xb7\xaf\xe5\xbe\x84", configPath_, sizeof(configPath_)); // Config 文件路径
    ImGui::InputText("API Endpoint", qwenEndpoint_, sizeof(qwenEndpoint_));
    ImGui::InputText("\xe6\xa8\xa1\xe5\x9e\x8b", qwenModel_, sizeof(qwenModel_)); // 模型
    ImGui::InputText("API Key", qwenApiKey_, sizeof(qwenApiKey_),
                     ImGuiInputTextFlags_Password);

    ImGui::Spacing();

    // ---- 应用按钮 ----
    if (ImGui::Button("\xe5\xba\x94\xe7\x94\xa8\xe8\xae\xbe\xe7\xbd\xae", // 应用设置
                      ImVec2(160, 36))) {
        // 更新串口参数（断开后重新连接）
        if (serialConnected_) {
            disconnectSerial();
        }
        cameraIndex_ = settingsCamIdx_;
        camera_ = OpenCvCameraService(cameraIndex_);

        // 更新 Qwen 配置
        qwenConfig_.baseUrl = qwenEndpoint_;
        qwenConfig_.model = qwenModel_;
        qwenConfig_.apiKey = qwenApiKey_;
        if (qwenConfig_.hasApiKey()) {
            qwenVision_ = std::make_unique<QwenVisionService>(qwenConfig_, httpClient_);
        } else {
            qwenVision_.reset();
            useQwen_ = false;
        }

        addEventLog("Settings: applied");
    }

    // ---- 错误信息 ----
    if (!connError_.empty()) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1,0.5f,0,1), "%s", connError_.c_str());
    }
}

// ===================================================================
// 串口连接 / 断开
// ===================================================================

bool AppGUI::connectSerial() {
    disconnectSerial();

    serial_ = std::make_unique<SerialPort>(comPort_, static_cast<DWORD>(baudRate_));
    if (!serial_->open()) {
        connError_ = std::string("Failed to open ") + comPort_;
        serial_.reset();
        serialConnected_ = false;
        return false;
    }

    hardware_ = std::make_unique<HardwareBridge>(*serial_);
    devices_  = std::make_unique<PrototypeDeviceSet>(*hardware_);

    // 验证通信：PING
    if (!hardware_->ping()) {
        connError_ = "PING failed - device not responding";
        disconnectSerial();
        return false;
    }

    connError_.clear();
    serialConnected_ = true;
    return true;
}

void AppGUI::disconnectSerial() {
    if (serial_) {
        serial_->close();
    }
    serial_.reset();
    hardware_.reset();
    devices_.reset();
    serialConnected_ = false;
    ledOn_ = false;
    buzzerOn_ = false;
    vibrationOn_ = false;
}

// ===================================================================
// 事件日志
// ===================================================================

void AppGUI::addEventLog(const std::string& msg) {
    std::time_t now = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&now));
    eventLog_.push_back(std::string("[") + buf + "] " + msg);
    eventLogUpdated_ = true;
    if (eventLog_.size() > kMaxEvents) {
        eventLog_.erase(eventLog_.begin(), eventLog_.begin() + (eventLog_.size() - kMaxEvents));
    }
}

// ===================================================================
// 审计日志刷新
// ===================================================================

void AppGUI::refreshAuditLog() {
    auditCache_ = auditLog_.readAll();
}

// ===================================================================
// OpenGL 纹理管理
// ===================================================================

GLuint AppGUI::createTextureFromMat(const cv::Mat& mat) {
    if (mat.empty()) return 0;

    cv::Mat rgb;
    cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rgb.cols, rgb.rows, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, rgb.data);
    return tex;
}

void AppGUI::releaseCameraTexture() {
    if (cameraTexture_ != 0) {
        glDeleteTextures(1, &cameraTexture_);
        cameraTexture_ = 0;
    }
}
