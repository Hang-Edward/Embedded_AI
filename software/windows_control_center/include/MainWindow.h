#pragma once

#include "AppConfig.h"
#include "ConnectionManager.h"

#include <QHBoxLayout>
#include <QMainWindow>
#include <QMap>
#include <QVBoxLayout>

class CameraPage;
class ChatPage;
class HardwarePage;
class HistoryPage;
class QLabel;
class LogsPage;
class QPushButton;
class QStackedWidget;
class SettingsPage;
class QTimer;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void buildUi();
    void buildSidebar(QHBoxLayout* root);
    void buildHeader(QVBoxLayout* rightSide);
    void buildPages(QVBoxLayout* rightSide);
    void switchPage(const QString& key);
    void applyConnectionState(const ConnectionState& state);
    void updateResponsiveMode();
    void setWatchLive(bool enabled);
    QPushButton* makeNavButton(const QString& key, const QString& text);

    AppConfig config_;
    ConnectionManager connection_;
    QWidget* central_ = nullptr;
    QWidget* sidebar_ = nullptr;
    QLabel* connectionTitle_ = nullptr;
    QLabel* connectionSubtitle_ = nullptr;
    QLabel* lanWarning_ = nullptr;
    QLabel* readyDot_ = nullptr;
    QLabel* readyText_ = nullptr;
    QLabel* actionBanner_ = nullptr;
    QStackedWidget* stack_ = nullptr;
    QMap<QString, QPushButton*> navButtons_;
    QPushButton* watchButton_ = nullptr;
    QTimer* liveTimer_ = nullptr;
    bool watchLive_ = true;

    ChatPage* chatPage_ = nullptr;
    HardwarePage* hardwarePage_ = nullptr;
    HistoryPage* historyPage_ = nullptr;
    CameraPage* cameraPage_ = nullptr;
    LogsPage* logsPage_ = nullptr;
    SettingsPage* settingsPage_ = nullptr;
};
