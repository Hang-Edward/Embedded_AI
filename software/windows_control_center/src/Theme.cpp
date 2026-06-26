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
    font-size: 19px;
    font-weight: 820;
    color: #ffffff;
}
QLabel#sidebarTag, QLabel#sidebarSectionLabel {
    color: rgba(206, 228, 255, 0.74);
    font-size: 11px;
    font-weight: 700;
    letter-spacing: 0.8px;
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
    background: rgba(7, 18, 40, 0.34);
    border: 1px solid rgba(185, 225, 255, 0.12);
    border-radius: 14px;
    color: #e9f4ff;
    font-size: 12px;
    font-weight: 800;
    padding: 10px 12px;
    text-align: left;
}
QToolButton#inspectorToggle:hover {
    background: rgba(18, 39, 80, 0.42);
    border-color: rgba(205, 234, 255, 0.22);
}
QToolButton#inspectorToggle[focused="true"] {
    background: rgba(28, 61, 124, 0.46);
    border-color: rgba(170, 220, 255, 0.30);
}
QToolButton#inspectorToggle::menu-indicator {
    image: none;
    width: 0px;
}
QWidget#inspectorSectionBody {
    background: transparent;
}
QWidget#sidebarSummaryCard {
    background: rgba(8, 18, 39, 0.48);
    border: 1px solid rgba(188, 225, 255, 0.15);
    border-radius: 18px;
}
QLabel#sidebarSummaryTitle {
    color: #f4f8ff;
    font-size: 15px;
    font-weight: 800;
}
QLabel#sidebarSummaryBody {
    color: #b8cce5;
    font-size: 12px;
    line-height: 148%;
}
QWidget#sidebarReconnectWrap {
    background: transparent;
}
QLabel#connectionTitle {
    font-size: 17px;
    font-weight: 800;
    color: #ffffff;
}
QWidget#headerMetrics {
    background: transparent;
}
QLabel#headerStatusPill, QLabel#headerMetaPill {
    padding: 9px 14px;
    border-radius: 14px;
    font-size: 12px;
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
    color: rgba(213, 235, 255, 0.78);
    font-size: 12px;
    font-weight: 700;
    letter-spacing: 0.5px;
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
QLabel#statusColumnTitle {
    font-size: 18px;
    font-weight: 850;
    color: #ffffff;
}
QWidget#statusColumnHeader {
    background: rgba(7, 18, 40, 0.52);
    border: 1px solid rgba(185, 225, 255, 0.18);
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
    color: #bfd0e8;
    line-height: 145%;
}
QLabel#healthHeadline {
    color: #eaf4ff;
    font-size: 15px;
    font-weight: 800;
    background: rgba(6, 17, 38, 0.46);
    border: 1px solid rgba(184, 220, 255, 0.14);
    border-radius: 16px;
    line-height: 145%;
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
    background: rgba(8, 20, 42, 0.56);
    border: 1px solid rgba(170, 215, 255, 56);
    border-radius: 16px;
    color: #e6f3ff;
    font-weight: 800;
    line-height: 145%;
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
    background: rgba(7, 18, 40, 0.46);
    border: 1px solid rgba(185, 225, 255, 0.16);
    border-radius: 18px;
    color: #e7f4ff;
    line-height: 145%;
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
    background: rgba(7, 18, 40, 0.50);
    border: 1px solid rgba(185, 225, 255, 0.18);
    border-radius: 18px;
    color: #e7f4ff;
    line-height: 148%;
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
    background: rgba(8, 19, 41, 0.48);
    border: 1px solid rgba(185, 225, 255, 0.16);
    border-radius: 20px;
}
QWidget#chatStagePanel[flash="true"] {
    border-color: rgba(214, 240, 255, 0.34);
    background: rgba(18, 38, 78, 0.54);
}
QLabel#chatStageTitle {
    color: #ffffff;
    font-size: 19px;
    font-weight: 850;
}
QLabel#chatStageStatus {
    color: #dcecff;
    font-size: 15px;
    font-weight: 650;
    line-height: 145%;
}
QLabel#chatStageMeta {
    color: #9fb8d7;
    font-size: 12px;
    line-height: 150%;
}
QWidget#chatVisualCard {
    background: rgba(7, 17, 36, 0.56);
    border: 1px solid rgba(185, 225, 255, 0.14);
    border-radius: 22px;
}
QWidget#chatConversationCard {
    background: rgba(7, 17, 36, 0.58);
    border: 1px solid rgba(185, 225, 255, 0.14);
    border-radius: 22px;
}
QWidget#chatComposerCard {
    background: rgba(6, 16, 35, 0.72);
    border: 1px solid rgba(168, 214, 255, 0.14);
    border-radius: 18px;
}
QTextEdit#chatComposerEdit {
    background: rgba(5, 13, 29, 0.84);
    color: #eef6ff;
    border: 1px solid rgba(170, 220, 255, 0.16);
    border-radius: 14px;
    padding: 10px 12px;
    selection-background-color: rgba(83, 160, 255, 165);
}
QCheckBox#chatSceneCheck {
    color: #dcecff;
    spacing: 8px;
}
QCheckBox#chatSceneCheck::indicator {
    width: 18px;
    height: 18px;
    border-radius: 6px;
    border: 1px solid rgba(170, 220, 255, 0.26);
    background: rgba(9, 20, 42, 0.76);
}
QCheckBox#chatSceneCheck::indicator:checked {
    background: rgba(81, 154, 255, 0.88);
    border-color: rgba(214, 240, 255, 0.42);
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
    background: rgba(7, 17, 36, 0.52);
    border: 1px solid rgba(185, 225, 255, 0.15);
    border-radius: 20px;
}
QWidget#chatPanelCard[flash="true"] {
    border-color: rgba(214, 240, 255, 0.34);
    background: rgba(18, 38, 78, 0.54);
}
QWidget#chatAnswerCard {
    background: rgba(7, 16, 34, 0.68);
    border: 1px solid rgba(139, 196, 255, 0.18);
    border-radius: 22px;
}
QWidget#chatAnswerCard[flash="true"] {
    border-color: rgba(214, 240, 255, 0.34);
    background: rgba(16, 28, 58, 0.80);
}
QLabel#chatPanelTitle {
    color: #ffffff;
    font-size: 15px;
    font-weight: 850;
}
QLabel#chatPanelSubtle {
    color: #9fb8d7;
    font-size: 12px;
    line-height: 145%;
}
QLabel#chatPanelBody {
    color: #dcecff;
    font-size: 14px;
    line-height: 158%;
}
QLabel#chatAnswerBody {
    color: #f1f7ff;
    font-size: 15px;
    line-height: 162%;
}
QLabel#chatSectionLabel {
    color: rgba(212, 232, 255, 0.82);
    font-size: 11px;
    font-weight: 800;
    letter-spacing: 0.8px;
    text-transform: uppercase;
}
QLabel#chatImageHero {
    background: rgba(3, 10, 24, 0.88);
    border: 1px solid rgba(155, 210, 255, 0.18);
    border-radius: 20px;
    color: #c7d9ef;
    padding: 12px;
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
    background: rgba(9, 20, 42, 132);
    border: 1px solid rgba(182, 223, 255, 0.24);
}
QWidget#messageBubble[role="system"] {
    background: rgba(8, 18, 38, 0.98);
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
QScrollArea#chatScroll > QWidget > QWidget {
    background: transparent;
}
QScrollArea#settingsScroll,
QScrollArea#settingsScroll::viewport,
QWidget#settingsContent {
    background: transparent;
    border: none;
}
QScrollArea#chatScroll QWidget,
QScrollArea#chatScroll QAbstractScrollArea,
QScrollArea#chatScroll QAbstractScrollArea::viewport {
    background: transparent;
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
