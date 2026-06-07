/*
 * Embedded AI Reality Bridge — GUI 应用程序主类
 * 提供侧边栏导航和 6 个功能页面的渲染。
 */

#pragma once

#include "AuditLogStore.h"
#include "CurlHttpClient.h"
#include "HardwareBridge.h"
#include "MockAiVisionService.h"
#include "OpenCvCameraService.h"
#include "PrototypeDeviceSet.h"
#include "QwenVisionConfig.h"
#include "QwenVisionService.h"
#include "SerialPort.h"
#include "SceneTask.h"

#include <opencv2/core/mat.hpp>
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

// OpenGL 头（仅使用 GL 1.1 核心函数用于纹理操作）
#include <GL/gl.h>

class AppGUI {
public:
    AppGUI();
    ~AppGUI();

    // 每帧由主循环调用
    void render();
    // 窗口大小变化
    void onResize(int width, int height);

private:
    // ---- 页面枚举 ----
    enum class Page {
        Dashboard,   // 仪表盘
        Camera,      // 摄像头
        AIVision,    // AI 视觉分析
        Hardware,    // 硬件控制
        AuditLog,    // 审计日志
        Settings,    // 设置
        _Count
    };

    // ---- 渲染方法 ----
    void renderSidebar();   // 左侧导航栏
    void renderPage();      // 当前页面内容

    void pageDashboard();
    void pageCamera();
    void pageAIVision();
    void pageHardware();
    void pageAuditLog();
    void pageSettings();

    // ---- 工具栏 / 状态栏（窗口顶部 / 底部） ----
    void renderTopBar();
    void renderStatusBar();

    // ---- 后端操作 ----
    bool connectSerial();
    void disconnectSerial();
    void addEventLog(const std::string& msg);
    void refreshAuditLog();

    // ---- OpenGL 纹理管理 ----
    GLuint createTextureFromMat(const cv::Mat& mat);
    void   releaseCameraTexture();

    // ---- 页面状态 ----
    Page currentPage_ = Page::Dashboard;

    // ---- 后端对象（延迟创建） ----
    std::unique_ptr<SerialPort>         serial_;
    std::unique_ptr<HardwareBridge>     hardware_;
    std::unique_ptr<PrototypeDeviceSet> devices_;

    // ---- 后端对象（始终有效） ----
    OpenCvCameraService  camera_{0};
    MockAiVisionService  mockVision_;
    CurlHttpClient       httpClient_;
    AuditLogStore        auditLog_{"audit-log.dat"};
    QwenVisionConfig     qwenConfig_;
    std::unique_ptr<QwenVisionService> qwenVision_;

    // ---- 连接状态 ----
    bool   serialConnected_ = false;
    bool   useQwen_         = false;
    int    cameraIndex_     = 0;
    std::string connError_;

    // ---- 摄像头帧 & 纹理 ----
    cv::Mat lastFrame_;
    std::string lastCapturePath_;
    bool    frameCaptured_ = false;
    GLuint  cameraTexture_ = 0;
    int     texWidth_  = 0;
    int     texHeight_ = 0;

    // ---- 硬件控制 ----
    bool   ledOn_        = false;
    bool   buzzerOn_     = false;
    bool   vibrationOn_  = false;
    char   oledInput_[256]{};
    char   manualCmd_[128]{};
    std::string hwResponse_;

    // ---- AI 分析结果 ----
    std::string aiResult_;
    bool        hasResult_ = false;

    // ---- 设置页输入 ----
    char   comPort_[32]       = "COM11";
    int    baudRate_          = 115200;
    int    settingsCamIdx_    = 0;
    char   qwenEndpoint_[256] = "";
    char   qwenModel_[128]    = "";
    char   qwenApiKey_[256]   = "";
    char   configPath_[256]   = "config/qwen-vision.ini";

    // ---- 事件日志 ----
    std::vector<std::string> eventLog_;
    static constexpr size_t  kMaxEvents = 200;
    bool eventLogUpdated_ = false;

    // ---- 审计日志缓存 ----
    std::vector<AuditLogEntry> auditCache_;
};
