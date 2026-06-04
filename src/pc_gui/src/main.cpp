/*
 * Embedded AI Reality Bridge — GUI 应用程序入口
 * 功能：创建 Win32 窗口、初始化 OpenGL 渲染上下文、初始化 Dear ImGui，
 *       进入消息循环，由 AppGUI 负责页面渲染。
 */

#include "AppGUI.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl3.h"

#include <windows.h>
#include <GL/gl.h>
#include <string>
#include <cstdlib>

// 全局引用（窗口过程回调需要）
static HWND        g_hWnd   = nullptr;
static HDC         g_hDC    = nullptr;
static HGLRC       g_hRC    = nullptr;
static AppGUI*     g_app    = nullptr;

// ImGui_ImplWin32_WndProcHandler 被后端故意包裹在 #if 0 中以避免依赖 <windows.h>
// 此处手动前向声明（参见 imgui_impl_win32.h 注释）
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 窗口回调函数（前置声明）
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int main() {
    const char* CLASS_NAME = "EmbeddedAIGuiClass";
    const int   WIN_W = 1280;
    const int   WIN_H = 800;

    // ---------- 1. 注册窗口类 ----------
    WNDCLASSEX wc = {};
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = GetModuleHandle(nullptr);
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = CLASS_NAME;
    if (!RegisterClassEx(&wc)) {
        MessageBoxA(nullptr, "窗口类注册失败", "错误", MB_ICONERROR);
        return 1;
    }

    // ---------- 2. 创建窗口 ----------
    RECT rc = { 0, 0, WIN_W, WIN_H };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    g_hWnd = CreateWindowEx(
        0, CLASS_NAME, "Embedded AI Reality Bridge",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, wc.hInstance, nullptr
    );
    if (!g_hWnd) {
        MessageBoxA(nullptr, "窗口创建失败", "错误", MB_ICONERROR);
        return 1;
    }
    ShowWindow(g_hWnd, SW_SHOWDEFAULT);

    // ---------- 3. 创建 OpenGL 渲染上下文 ----------
    g_hDC = GetDC(g_hWnd);
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;
    int pf = ChoosePixelFormat(g_hDC, &pfd);
    if (!pf || !SetPixelFormat(g_hDC, pf, &pfd)) {
        MessageBoxA(nullptr, "设置像素格式失败", "错误", MB_ICONERROR);
        return 1;
    }
    g_hRC = wglCreateContext(g_hDC);
    if (!g_hRC || !wglMakeCurrent(g_hDC, g_hRC)) {
        MessageBoxA(nullptr, "创建 OpenGL 上下文失败", "错误", MB_ICONERROR);
        return 1;
    }

    // ---------- 4. 初始化 Dear ImGui ----------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = "imgui.ini";
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplWin32_Init(g_hWnd);
    ImGui_ImplOpenGL3_Init("#version 130");

    // 深色风格
    ImGui::StyleColorsDark();

    // 加载中文字体（尝试系统字体，失败则使用默认字体）
    {
        const char* fontPaths[] = {
            "C:/Windows/Fonts/msyh.ttc",      // Microsoft YaHei
            "C:/Windows/Fonts/simhei.ttf",    // SimHei
            "C:/Windows/Fonts/msyhbd.ttc",    // Microsoft YaHei Bold
        };
        ImFontConfig cfg;
        cfg.MergeMode = false;
        ImFont* font = nullptr;
        for (auto path : fontPaths) {
            font = io.Fonts->AddFontFromFileTTF(path, 15.0f, &cfg, io.Fonts->GetGlyphRangesChineseFull());
            if (font) break;
        }
        if (!font) {
            // 回退：使用默认字体并尝试放大
            font = io.Fonts->AddFontDefault();
        }
        io.FontDefault = font;
    }

    // ---------- 5. 创建应用实例 ----------
    AppGUI app;
    g_app = &app;

    // 启用垂直同步（限制帧率到显示器刷新率，降低 CPU/GPU 占用）
    typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC)(int);
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT =
        (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    if (wglSwapIntervalEXT) wglSwapIntervalEXT(1);

    // ---------- 6. 主消息循环 ----------
    bool running = true;
    while (running) {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (!running) break;

        // Dear ImGui 新帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 渲染应用页面
        app.render();

        // ImGui 绘制 + OpenGL 交换缓冲
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.10f, 0.10f, 0.12f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SwapBuffers(g_hDC);

        // 让出 CPU 时间片，降低无谓的忙等（配合 vsync 后帧率稳定在显示器刷新率）
        Sleep(1);
    }

    // ---------- 7. 清理 ----------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    wglMakeCurrent(nullptr, nullptr);
    if (g_hRC) wglDeleteContext(g_hRC);
    if (g_hDC) ReleaseDC(g_hWnd, g_hDC);
    DestroyWindow(g_hWnd);

    return 0;
}

// ---------- 窗口过程函数 ----------
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (g_app && wParam != SIZE_MINIMIZED)
            g_app->onResize(LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}
