#include "Theme.h"

QString Theme::styleSheet() {
    return R"QSS(
* {
    font-family: "Microsoft YaHei UI", "Segoe UI", sans-serif;
    font-size: 14px;
}
QMainWindow {
    color: #edf6ff;
    background: #050b19;
}
QWidget#central, QStackedWidget#pageStack, QStackedWidget#pageStack > QWidget {
    background: transparent;
    border: none;
}
QWidget#statusCard {
    background: rgba(19, 39, 76, 108);
    border: 1px solid rgba(215, 239, 255, 62);
    border-radius: 16px;
}
QWidget#sidebar, QWidget#glassHeader, QWidget#mainContentSurface, QWidget#statusColumn {
    background: transparent;
}
QWidget#messageBubble {
    background: rgba(8, 18, 38, 118);
    border: 1px solid rgba(170, 220, 255, 58);
    border-radius: 18px;
}
QPushButton {
    min-height: 34px;
}
QPushButton#navButton {
    min-height: 86px;
    background: transparent;
    border: none;
    padding: 0;
}
QPushButton#primaryButton {
    background: rgba(72, 145, 255, 138);
    color: white;
    border: 1px solid rgba(210, 235, 255, 90);
    border-radius: 13px;
    padding: 9px 15px;
    font-weight: 700;
}
QPushButton#primaryButton:hover {
    background: rgba(98, 170, 255, 170);
}
QPushButton#primaryButton:pressed {
    background: rgba(45, 110, 220, 190);
    padding-top: 10px;
    padding-left: 16px;
}
QPushButton#secondaryButton {
    min-height: 38px;
    background: rgba(255, 255, 255, 14);
    color: #dcecff;
    border: 1px solid rgba(195, 229, 255, 48);
    border-radius: 12px;
    padding: 8px 14px;
}
QPushButton#secondaryButton:hover {
    background: rgba(91, 163, 255, 62);
    border-color: rgba(198, 232, 255, 92);
}
QPushButton#secondaryButton:pressed {
    background: rgba(60, 126, 225, 104);
}
QGroupBox#settingsGroup {
    color: #f3f8ff;
    font-weight: 800;
    background: rgba(9, 24, 52, 72);
    border: 1px solid rgba(205, 234, 255, 45);
    border-radius: 16px;
    margin-top: 12px;
    padding: 16px 12px 12px 12px;
}
QGroupBox#settingsGroup::title {
    subcontrol-origin: margin;
    left: 16px;
    padding: 0 7px;
    color: #dcecff;
}
QLabel#appTitle {
    font-size: 18px;
    font-weight: 860;
    color: #ffffff;
}
QLabel#sidebarTag, QLabel#sidebarSectionLabel {
    color: rgba(206, 228, 255, 0.66);
    font-size: 10px;
    font-weight: 700;
    letter-spacing: 1.1px;
    text-transform: uppercase;
}
QLabel#inspectorSectionLabel {
    color: rgba(214, 233, 255, 0.70);
    font-size: 11px;
    font-weight: 800;
    letter-spacing: 0.8px;
    text-transform: uppercase;
    padding-left: 2px;
}
QToolButton#inspectorToggle {
    background: rgba(7, 18, 40, 0.28);
    border: 1px solid rgba(185, 225, 255, 0.10);
    border-radius: 13px;
    color: #e9f4ff;
    font-size: 11px;
    font-weight: 800;
    padding: 9px 11px;
    text-align: left;
}
QToolButton#inspectorToggle:hover {
    background: rgba(18, 39, 80, 0.34);
    border-color: rgba(205, 234, 255, 0.18);
}
QToolButton#inspectorToggle[focused="true"] {
    background: rgba(28, 61, 124, 0.38);
    border-color: rgba(170, 220, 255, 0.24);
}
QToolButton#inspectorToggle::menu-indicator {
    image: none;
    width: 0px;
}
QWidget#inspectorSectionBody {
    background: transparent;
}
QWidget#sidebarSummaryCard {
    background: rgba(7, 17, 36, 0.50);
    border: 1px solid rgba(188, 225, 255, 0.12);
    border-radius: 20px;
}
QLabel#sidebarSummaryTitle {
    color: #f4f8ff;
    font-size: 14px;
    font-weight: 800;
}
QLabel#sidebarSummaryBody {
    color: #a8bedb;
    font-size: 12px;
    line-height: 152%;
}
QWidget#sidebarReconnectWrap {
    background: transparent;
}
QLabel#connectionTitle {
    font-size: 16px;
    font-weight: 800;
    color: #ffffff;
}
QWidget#headerMetrics {
    background: transparent;
}
QLabel#headerStatusPill, QLabel#headerMetaPill {
    padding: 8px 13px;
    border-radius: 14px;
    font-size: 11px;
    font-weight: 700;
    color: #eaf4ff;
    background: rgba(8, 20, 42, 0.40);
    border: 1px solid rgba(188, 225, 255, 0.14);
}
QLabel#headerStatusPill[status="ready"] {
    background: rgba(18, 84, 61, 0.46);
    border-color: rgba(81, 252, 181, 0.28);
}
QLabel#headerStatusPill[status="busy"], QLabel#headerStatusPill[status="connecting"] {
    background: rgba(104, 74, 16, 0.46);
    border-color: rgba(255, 214, 96, 0.28);
}
QLabel#headerStatusPill[status="warning"] {
    background: rgba(76, 61, 20, 0.40);
    border-color: rgba(255, 210, 112, 0.24);
}
QLabel#headerStatusPill[status="error"] {
    background: rgba(104, 24, 42, 0.46);
    border-color: rgba(255, 132, 156, 0.28);
}
QWidget#statusColumn {
    background: transparent;
}
QLabel#heroEyebrow {
    color: rgba(213, 235, 255, 0.68);
    font-size: 11px;
    font-weight: 700;
    letter-spacing: 1.1px;
    text-transform: uppercase;
}
QLabel#connectionSubtitle, QLabel#pageSubtitle, QLabel#cardDetail {
    color: #bfd0e8;
}
QLabel#pageTitle {
    font-size: 28px;
    font-weight: 900;
    color: #ffffff;
}
QLabel#pageTitle[chatPage="true"] {
    font-size: 22px;
    font-weight: 860;
    color: #f3f8ff;
}
QLabel#statusColumnTitle {
    font-size: 17px;
    font-weight: 850;
    color: #ffffff;
}
QWidget#statusColumnHeader {
    background: rgba(6, 16, 35, 0.56);
    border: 1px solid rgba(185, 225, 255, 0.15);
    border-radius: 18px;
}
QLabel#statusBeacon {
    border-radius: 7px;
    background: rgba(154, 212, 255, 0.36);
    border: 1px solid rgba(227, 244, 255, 0.46);
}
QLabel#statusBeacon[pulse="bright"] { background: rgba(198, 232, 255, 0.92); }
QLabel#statusBeacon[pulse="dim"] { background: rgba(154, 212, 255, 0.36); }
QLabel#statusBeacon[status="ready"] { background: rgba(73, 255, 186, 0.90); border-color: rgba(212, 255, 235, 0.66); }
QLabel#statusBeacon[status="busy"], QLabel#statusBeacon[status="connecting"] { background: rgba(255, 204, 95, 0.88); border-color: rgba(255, 243, 204, 0.64); }
QLabel#statusBeacon[status="error"] { background: rgba(255, 117, 148, 0.88); border-color: rgba(255, 220, 230, 0.64); }
QLabel#statusColumnSubtitle, QLabel#statusHint {
    color: #b6cae3;
    line-height: 156%;
}
QLabel#healthHeadline {
    color: #eaf4ff;
    font-size: 13px;
    font-weight: 800;
    background: rgba(6, 17, 38, 0.34);
    border: 1px solid rgba(184, 220, 255, 0.10);
    border-radius: 16px;
    line-height: 154%;
}
QLabel#healthHeadline[flash="true"] {
    border-color: rgba(206, 236, 255, 0.34);
    background: rgba(20, 42, 84, 0.52);
}
QLabel#readyText {
    color: #e5f1ff;
    font-weight: 800;
}
QLabel#readyDot {
    border-radius: 8px;
    background: #64748b;
}
QLabel#readyDot[status="ready"] { background: #34d399; }
QLabel#readyDot[status="busy"], QLabel#readyDot[status="warning"] { background: #fbbf24; }
QLabel#readyDot[status="error"] { background: #fb7185; }
QLabel#actionBanner {
    background: rgba(8, 20, 42, 0.42);
    border: 1px solid rgba(170, 215, 255, 0.14);
    border-radius: 16px;
    color: #e6f3ff;
    font-weight: 800;
    line-height: 154%;
}
QLabel#actionBanner[flash="true"] {
    border-color: rgba(218, 242, 255, 0.36);
}
QLabel#actionBanner[status="ready"] {
    background: rgba(17, 95, 66, 158);
    border-color: rgba(71, 255, 177, 120);
}
QLabel#actionBanner[status="busy"], QLabel#actionBanner[status="warning"] {
    background: rgba(109, 78, 14, 168);
    border-color: rgba(255, 210, 91, 135);
    color: #fff7d6;
}
QLabel#actionBanner[status="error"] {
    background: rgba(110, 24, 40, 178);
    border-color: rgba(255, 128, 150, 145);
    color: #ffe4e8;
}
QLabel#metricChip {
    background: rgba(7, 18, 40, 0.34);
    border: 1px solid rgba(185, 225, 255, 0.10);
    border-radius: 16px;
    color: #e7f4ff;
    line-height: 150%;
}
QLabel#metricChip[flash="true"] {
    border-color: rgba(214, 240, 255, 0.34);
    background: rgba(22, 42, 82, 0.48);
}
QLabel#metricChip[tone="ready"] {
    background: rgba(18, 84, 61, 0.42);
    border-color: rgba(81, 252, 181, 0.28);
}
QLabel#metricChip[tone="busy"] {
    background: rgba(104, 74, 16, 0.42);
    border-color: rgba(255, 214, 96, 0.30);
}
QLabel#metricChip[tone="warning"] {
    background: rgba(76, 61, 20, 0.38);
    border-color: rgba(255, 210, 112, 0.24);
}
QLabel#metricChip[tone="error"] {
    background: rgba(104, 24, 42, 0.42);
    border-color: rgba(255, 132, 156, 0.28);
}
QLabel#metricChip[tone="neutral"] {
    background: rgba(24, 47, 92, 0.34);
    border-color: rgba(134, 191, 255, 0.20);
}
QLabel#statusActionCard {
    background: rgba(7, 18, 40, 0.36);
    border: 1px solid rgba(185, 225, 255, 0.11);
    border-radius: 16px;
    color: #e7f4ff;
    line-height: 152%;
}
QLabel#statusActionCard[flash="true"] {
    border-color: rgba(214, 240, 255, 0.34);
    background: rgba(22, 42, 82, 0.50);
}
QLabel#statusActionCard[tone="ready"] {
    background: rgba(18, 84, 61, 0.42);
    border-color: rgba(81, 252, 181, 0.28);
}
QLabel#statusActionCard[tone="busy"] {
    background: rgba(104, 74, 16, 0.42);
    border-color: rgba(255, 214, 96, 0.30);
}
QLabel#statusActionCard[tone="warning"] {
    background: rgba(76, 61, 20, 0.38);
    border-color: rgba(255, 210, 112, 0.24);
}
QWidget#chatStagePanel {
    background: rgba(7, 17, 37, 0.38);
    border: 1px solid rgba(185, 225, 255, 0.10);
    border-radius: 22px;
}
QWidget#chatStagePanel[flash="true"] {
    border-color: rgba(214, 240, 255, 0.34);
    background: rgba(18, 38, 78, 0.54);
}
QLabel#chatStageTitle {
    color: #ffffff;
    font-size: 16px;
    font-weight: 840;
}
QLabel#chatStageStatus {
    color: #dcecff;
    font-size: 13px;
    font-weight: 600;
    line-height: 160%;
}
QLabel#chatStageMeta {
    color: #89a5c8;
    font-size: 11px;
    line-height: 160%;
}
QWidget#chatVisualCard {
    background: rgba(7, 16, 34, 0.54);
    border: 1px solid rgba(185, 225, 255, 0.12);
    border-radius: 20px;
}
QWidget#chatConversationCard {
    background: rgba(22, 46, 90, 0.05);
    border: 1px solid rgba(190, 227, 255, 0.13);
    border-radius: 26px;
}
QWidget#chatConversationViewport,
QWidget#chatConversationHost,
QScrollArea#chatScroll,
QScrollArea#chatScroll > QWidget,
QScrollArea#chatScroll::viewport {
    background: transparent;
    border: none;
}
QWidget#chatComposerCard {
    background: rgba(20, 42, 82, 0.12);
    border: 1px solid rgba(168, 214, 255, 0.13);
    border-radius: 18px;
}
QTextEdit#chatComposerEdit,
QPlainTextEdit#chatComposerEdit {
    background: rgba(16, 35, 70, 0.12);
    color: #eef6ff;
    border: 1px solid rgba(170, 220, 255, 0.16);
    border-radius: 16px;
    padding: 14px 16px;
    selection-background-color: rgba(83, 160, 255, 165);
}
QTextEdit#chatComposerEdit[busy="true"],
QPlainTextEdit#chatComposerEdit[busy="true"] {
    background: rgba(14, 28, 56, 0.16);
}
QTextEdit#chatComposerEdit::viewport,
QPlainTextEdit#chatComposerEdit::viewport {
    background: transparent;
    border: none;
}
QCheckBox#chatSceneCheck {
    color: #e4f1ff;
    background: transparent;
    spacing: 8px;
    font-weight: 600;
}
QScrollArea#chatInsightScroll,
QScrollArea#chatInsightScroll QWidget,
QScrollArea#chatInsightScroll QAbstractScrollArea,
QScrollArea#chatInsightScroll QAbstractScrollArea::viewport,
QWidget#chatInsightHost,
QScrollArea#statusScroll,
QScrollArea#statusScroll QWidget,
QScrollArea#statusScroll QAbstractScrollArea,
QScrollArea#statusScroll QAbstractScrollArea::viewport,
QWidget#statusHost {
    background: transparent;
    border: none;
}
QWidget#chatPanelCard {
    background: rgba(7, 17, 36, 0.48);
    border: 1px solid rgba(185, 225, 255, 0.11);
    border-radius: 18px;
}
QWidget#chatPanelCard[flash="true"] {
    border-color: rgba(214, 240, 255, 0.34);
    background: rgba(18, 38, 78, 0.54);
}
QWidget#chatAnswerCard {
    background: rgba(7, 16, 34, 0.54);
    border: 1px solid rgba(139, 196, 255, 0.12);
    border-radius: 18px;
}
QWidget#chatAnswerCard[flash="true"] {
    border-color: rgba(214, 240, 255, 0.34);
    background: rgba(16, 28, 58, 0.72);
}
QLabel#chatPanelTitle {
    color: #ffffff;
    font-size: 13px;
    font-weight: 850;
}
QLabel#chatPanelSubtle {
    color: #8aa3c5;
    font-size: 11px;
    line-height: 150%;
}
QLabel#chatPanelBody {
    color: #d4e5f8;
    font-size: 13px;
    line-height: 158%;
}
QLabel#chatAnswerBody {
    color: #e9f3ff;
    font-size: 13px;
    line-height: 160%;
}
QLabel#chatSectionLabel {
    color: rgba(212, 232, 255, 0.58);
    font-size: 9px;
    font-weight: 800;
    letter-spacing: 2.1px;
    text-transform: uppercase;
}
QLabel#chatImageHero {
    background: rgba(3, 10, 24, 0.88);
    border: 1px solid rgba(155, 210, 255, 0.14);
    border-radius: 18px;
    color: #c7d9ef;
    padding: 10px;
}
QLabel#chatImageHero[flash="true"] {
    border-color: rgba(214, 240, 255, 0.34);
    background: rgba(10, 20, 42, 0.92);
}
QLabel#cardTitle, QLabel#messageTitle {
    font-weight: 800;
    color: #ffffff;
}
QWidget#messageBubble[role="assistant"] {
    background: rgba(7, 17, 38, 112);
    border: 1px solid rgba(182, 223, 255, 0.24);
}
QWidget#thinkingMessage QWidget#messageBubble {
    background: rgba(7, 17, 38, 126);
    border: 1px solid rgba(153, 205, 255, 0.22);
}
QWidget#messageBubble[role="system"] {
    background: rgba(8, 18, 38, 0.30);
    border: 1px solid rgba(170, 220, 255, 0.12);
}
QLabel#messageBody {
    color: #dcecff;
    line-height: 158%;
}
QTextBrowser#messageBody {
    background: transparent;
    border: none;
    color: #dcecff;
}
QTextBrowser#messageBody a {
    color: #9fd2ff;
    text-decoration: none;
}
QTextBrowser#messageBody a:hover {
    color: #d6edff;
    text-decoration: underline;
}
QTextBrowser#messageBody QScrollBar:vertical,
QTextBrowser#messageBody QScrollBar:horizontal {
    width: 0px;
    height: 0px;
    background: transparent;
}
QLabel#avatar {
    background: rgba(75, 150, 255, 118);
    color: white;
    border: 1px solid rgba(218, 240, 255, 88);
    border-radius: 18px;
    font-weight: 800;
}
QLabel#avatar[role="assistant"] {
    background: rgba(103, 126, 255, 0.34);
    border-color: rgba(211, 223, 255, 0.30);
}
QLabel#avatar[role="system"] {
    background: rgba(105, 125, 154, 0.24);
    border-color: rgba(210, 224, 240, 0.24);
    color: #d9e6f3;
}
QLabel[level="ok"] { background: #34d399; border-radius: 6px; }
QLabel[level="warn"] { background: #fbbf24; border-radius: 6px; }
QLabel[level="error"] { background: #fb7185; border-radius: 6px; }
QLabel[level="checking"], QLabel[level="unknown"] { background: #94a3b8; border-radius: 6px; }
QLabel#cameraPreview, QLabel#messageImage {
    background: rgba(5, 13, 30, 84);
    border: 1px solid rgba(155, 210, 255, 42);
    border-radius: 18px;
    color: #c7d9ef;
}
QTextEdit#logView, QLineEdit, QComboBox {
    background: rgba(8, 18, 38, 118);
    color: #ecf6ff;
    border: 1px solid rgba(170, 220, 255, 58);
    border-radius: 14px;
    padding: 10px;
    selection-background-color: rgba(83, 160, 255, 165);
}
QLineEdit, QComboBox {
    min-height: 38px;
    max-height: 38px;
    padding: 2px 12px;
}
QComboBox::drop-down {
    width: 34px;
    border: none;
}
QListWidget#recentList {
    background: rgba(8, 18, 38, 108);
    color: #e6f3ff;
    border: 1px solid rgba(170, 220, 255, 55);
    border-radius: 16px;
    padding: 8px;
}
QListWidget#recentList::item {
    padding: 10px;
    border-radius: 11px;
}
QListWidget#recentList::item:selected {
    background: rgba(74, 144, 255, 145);
    color: white;
}
QScrollArea#chatScroll {
    border: 0;
    background: transparent;
}
QScrollArea#chatScroll::viewport {
    background: transparent;
}
QScrollArea#settingsScroll,
QScrollArea#settingsScroll::viewport,
QWidget#settingsContent {
    background: transparent;
    border: none;
}
QScrollBar:vertical {
    background: rgba(4, 12, 29, 105);
    width: 13px;
    margin: 5px 2px 5px 2px;
    border: 1px solid rgba(151, 207, 255, 28);
    border-radius: 6px;
}
QScrollBar::handle:vertical {
    min-height: 44px;
    background: rgba(117, 187, 255, 132);
    border: 1px solid rgba(220, 243, 255, 92);
    border-radius: 5px;
}
QScrollBar::handle:vertical:hover {
    background: rgba(143, 207, 255, 185);
    border-color: rgba(235, 249, 255, 145);
}
QScrollBar::handle:vertical:pressed {
    background: rgba(91, 164, 242, 218);
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0px;
    background: transparent;
    border: none;
}
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
    background: transparent;
}
)QSS";
}
