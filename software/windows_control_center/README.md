# Embedded AI Windows Control Center

这是项目新的 Windows 展示应用，技术路线固定为：

```text
Qt 6 + C++ + Widgets + QSS
```

它不是网页壳，也不是 Electron/Tauri。界面、页面切换、状态卡片、对话窗口和后续 SSH 连接逻辑都用 C++/Qt 实现，方便满足 C++ 大作业对代码规模、UI、文件读写、类、继承和多态的要求。

## 第一版内容

当前第一版先做假数据 UI 和工程骨架：

- 主窗口和左侧页面切换。
- Chat 页面：类似网页版 LLM 的对话窗口。
- Hardware 页面：展示树莓派、NUCLEO、摄像头、麦克风、Qwen 配置、网络状态。
- Camera 页面：展示最近一次拍照结果占位。
- Logs 页面：展示树莓派服务日志占位。
- Settings 页面：保存 SSH 用户、默认 IP、手动 SSH 命令、树莓派项目路径和认证方式。
- Reconnect 按钮：触发连接检测流程。
- 响应式布局：窗口可拖拽缩放，窄窗口下侧边栏会收窄，文本使用自动换行。

## 连接策略

应用启动后会按这个顺序检测树莓派：

```text
1. 最近一次成功 IP
2. ssh ch@172.20.10.6
3. 用户在设置页手动输入 ssh ch@ip
```

如果 SSH 失败，并且检测到 PC 本机 IP 与目标树莓派 IP 疑似不在同一局域网，界面会提示用户检查网络。常见 `/24` 网络可以用 IP 前三段粗略判断，例如 `172.20.10.x`；严格判断需要看子网掩码，所以界面只会写“疑似不在同一局域网”。

## 后续真数据接入

后续会通过 SSH 做这些事：

- 检查树莓派是否在线。
- 读取 `~/Embedded_AI/logs/embedded-ai.log`。
- 检查 `systemctl --user status embedded-ai.service`。
- 启动、停止、重启树莓派服务。
- 读取 `captures/latest-frame.jpg` 并显示到 Camera/Chat 页面。
- 手动触发一次分析。

密码保存和 SSH key 会放在设置页里，但第一版不会把密码写进源码或日志。

## 构建

当前机器需要先安装 Qt 6。安装好后可用 CMake 构建：

```powershell
cd "D:\VScode Projects\Embedded_AI"
cmake -S . -B build-qt -G Ninja -DBUILD_WINDOWS_CONTROL_CENTER=ON -DBUILD_LEGACY_IMGUI_GUI=OFF
cmake --build build-qt
```

生成目标：

```text
build-qt/software/windows_control_center/embedded_ai_control_center.exe
```

如果 CMake 输出 `Qt6 Widgets/Network not found`，说明当前环境还没有 Qt 6，桌面应用会被跳过，但硬件桥接程序仍可继续构建。

## 代码结构

```text
include/
  AppConfig.h
  BasePage.h
  ChatPage.h
  ConnectionManager.h
  HardwarePage.h
  MainWindow.h
  ...
src/
  AppConfig.cpp
  ChatPage.cpp
  ConnectionManager.cpp
  HardwarePage.cpp
  MainWindow.cpp
  Theme.cpp
  ...
```

页面类继承关系：

```text
QWidget -> BasePage -> ChatPage / HardwarePage / CameraPage / LogsPage / SettingsPage
QWidget -> StatusCard
QWidget -> ChatMessageWidget
QMainWindow -> MainWindow
QObject -> ConnectionManager
```
