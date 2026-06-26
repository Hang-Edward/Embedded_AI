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
class QToolButton;

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
    QPushButton* makeNavButton(const QString& key, const QString& text, const QString& subtitle, const QString& glyph);
    void setMetricChipText(QLabel* chip, const QString& label, const QString& value, const QString& tone = QString());
    void setInspectorSectionExpanded(QToolButton* toggle, QWidget* container, bool expanded);
    void updateInspectorFocus(const ConnectionState& state);
    QString phaseTextFor(const ConnectionState& state) const;
    QString triggerTextFor(const ConnectionState& state) const;
    QString displayModeTextFor() const;
    QString nextActionTextFor(const ConnectionState& state) const;
    QString healthHeadlineTextFor(const ConnectionState& state) const;
    void startBeaconPulse();

    AppConfig config_;
    ConnectionManager connection_;
    QWidget* central_ = nullptr;
    QWidget* sidebar_ = nullptr;
    QLabel* heroEyebrow_ = nullptr;
    QLabel* connectionTitle_ = nullptr;
    QLabel* connectionSubtitle_ = nullptr;
    QLabel* headerStatusPill_ = nullptr;
    QLabel* headerHostPill_ = nullptr;
    QLabel* sidebarSummaryTitle_ = nullptr;
    QLabel* sidebarSummaryBody_ = nullptr;
    QLabel* lanWarning_ = nullptr;
    QLabel* actionBanner_ = nullptr;
    QLabel* networkChip_ = nullptr;
    QLabel* phaseChip_ = nullptr;
    QLabel* triggerChip_ = nullptr;
    QLabel* modeChip_ = nullptr;
    QLabel* healthHeadline_ = nullptr;
    QLabel* nextActionCard_ = nullptr;
    QLabel* statusBeacon_ = nullptr;
    QToolButton* sectionPrimaryToggle_ = nullptr;
    QToolButton* sectionRecoveryToggle_ = nullptr;
    QWidget* sectionPrimaryContainer_ = nullptr;
    QWidget* sectionRecoveryContainer_ = nullptr;
    QWidget* pageSurface_ = nullptr;
    QWidget* statusSurface_ = nullptr;
    QStackedWidget* stack_ = nullptr;
    QMap<QString, QPushButton*> navButtons_;
    QTimer* liveTimer_ = nullptr;
    QTimer* beaconTimer_ = nullptr;
    bool watchLive_ = true;
    QString lastStatusClass_;

    ChatPage* chatPage_ = nullptr;
    HardwarePage* hardwarePage_ = nullptr;
    HistoryPage* historyPage_ = nullptr;
    CameraPage* cameraPage_ = nullptr;
    LogsPage* logsPage_ = nullptr;
    SettingsPage* settingsPage_ = nullptr;
};
