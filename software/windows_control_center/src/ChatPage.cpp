#include "ChatPage.h"

#include "ChatMessageWidget.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QPointer>
#include <QPixmap>
#include <QScrollArea>
#include <QScrollBar>
#include <QSizePolicy>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>

namespace {

QString stageTitleText(const ConnectionState& state) {
    switch (state.assistantStatus) {
    case AssistantStatus::Ready:
        return QStringLiteral("任务舞台已就绪");
    case AssistantStatus::Listening:
        return QStringLiteral("正在接收语音指令");
    case AssistantStatus::Thinking:
        return QStringLiteral("AI 正在分析当前画面");
    case AssistantStatus::Warning:
        return QStringLiteral("系统可运行，但有待检查项");
    case AssistantStatus::Error:
        return QStringLiteral("任务流程中断");
    case AssistantStatus::Connecting:
        return QStringLiteral("正在建立桥接链路");
    case AssistantStatus::Offline:
        return QStringLiteral("桥接链路离线");
    }
    return QStringLiteral("任务舞台等待中");
}

QString stageStatusText(const ConnectionState& state) {
    if (!state.assistantStatusText.isEmpty()) {
        return state.assistantStatusText;
    }
    return QStringLiteral("等待树莓派返回最新状态。");
}

QString stageMetaText(const ConnectionState& state) {
    QStringList parts;
    parts << QStringLiteral("触发：三键键盘 K-B");
    parts << QStringLiteral("摄像头：Logitech C270");
    if (!state.activeHost.isEmpty()) {
        parts << QStringLiteral("主机：%1").arg(state.activeHost);
    }
    if (!state.recentRecords.isEmpty()) {
        parts << QStringLiteral("最新记录：%1").arg(state.recentRecords.first().title);
    } else if (!state.localFramePath.isEmpty()) {
        parts << QStringLiteral("画面缓存：已同步");
    } else {
        parts << QStringLiteral("画面缓存：等待同步");
    }
    return parts.join(QStringLiteral("    ·    "));
}

QString summaryOrFallback(const QString& text, const QString& fallback) {
    const QString trimmed = text.trimmed();
    return trimmed.isEmpty() ? fallback : trimmed;
}

QString formatFlowText(const QString& flowText) {
    const QString trimmed = flowText.trimmed();
    if (trimmed.isEmpty()) {
        return QStringLiteral("1. 等待树莓派返回完整流程日志\n2. 摄像头画面同步后会在这里显示执行摘要");
    }

    QStringList steps;
    const QStringList lines = trimmed.split('\n', Qt::SkipEmptyParts);
    for (const QString& rawLine : lines) {
        QString line = rawLine.trimmed();
        if (line.isEmpty()) {
            continue;
        }
        if (line.startsWith(QStringLiteral("✅"))
            || line.startsWith(QStringLiteral("⚠"))
            || line.startsWith(QStringLiteral("❌"))
            || line.startsWith(QStringLiteral("🌐"))
            || line.startsWith(QStringLiteral("🎤"))
            || line.startsWith(QStringLiteral("📷"))
            || line.startsWith(QStringLiteral("🧠"))) {
            steps << line;
            continue;
        }
        if (line.contains(QStringLiteral("录音"))) {
            steps << QStringLiteral("🎤 %1").arg(line);
        } else if (line.contains(QStringLiteral("摄像头")) || line.contains(QStringLiteral("画面"))) {
            steps << QStringLiteral("📷 %1").arg(line);
        } else if (line.contains(QStringLiteral("识别")) || line.contains(QStringLiteral("分析"))) {
            steps << QStringLiteral("🧠 %1").arg(line);
        } else if (line.contains(QStringLiteral("失败")) || line.contains(QStringLiteral("错误"))) {
            steps << QStringLiteral("❌ %1").arg(line);
        } else {
            steps << QStringLiteral("• %1").arg(line);
        }
    }

    QStringList displayLines;
    for (const QString& step : steps) {
        displayLines << step;
    }
    return displayLines.join('\n');
}

QString buildCompactOverview(const QString& userText, const QString& flowText) {
    QStringList sections;
    const QString normalizedUser = summaryOrFallback(
        userText,
        QStringLiteral("本次触发没有解析到清晰语音，系统可能已自动回退到场景描述。"));
    sections << QStringLiteral("触发内容\n%1").arg(normalizedUser);

    const QStringList lines = formatFlowText(flowText).split('\n', Qt::SkipEmptyParts);
    QStringList picked;
    for (const QString& line : lines) {
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty()) {
            continue;
        }
        picked << trimmed;
        if (picked.size() >= 3) {
            break;
        }
    }
    if (!picked.isEmpty()) {
        sections << QStringLiteral("流程摘要\n%1").arg(picked.join('\n'));
    }
    return sections.join(QStringLiteral("\n\n"));
}

QString formatAnswerText(const QString& answerText) {
    const QString trimmed = answerText.trimmed();
    if (trimmed.isEmpty()) {
        return QStringLiteral("AI 回复尚未同步到本地控制中心。");
    }
    return trimmed;
}

QPixmap loadHeroPixmap(const QString& imagePath) {
    if (imagePath.isEmpty()) {
        return {};
    }
    QImageReader reader(imagePath);
    reader.setAutoTransform(true);
    const QImage image = reader.read();
    if (image.isNull()) {
        return {};
    }
    return QPixmap::fromImage(image);
}

void animateWidgetRefresh(QWidget* widget, int duration = 240) {
    if (widget == nullptr) {
        return;
    }
    widget->setProperty("flash", true);
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    QPointer<QWidget> guard(widget);
    QTimer::singleShot(duration, widget, [guard]() {
        if (guard == nullptr) {
            return;
        }
        guard->setProperty("flash", false);
        guard->style()->unpolish(guard);
        guard->style()->polish(guard);
        guard->update();
    });
}

void refreshLayoutAround(QWidget* widget) {
    QWidget* current = widget;
    int depth = 0;
    while (current != nullptr && depth < 6) {
        current->updateGeometry();
        if (current->layout() != nullptr) {
            current->layout()->invalidate();
            current->layout()->activate();
        }
        current = current->parentWidget();
        ++depth;
    }
}

void setAnimatedLabelText(QLabel* label, const QString& text, bool richText = false, int duration = 240) {
    if (label == nullptr) {
        return;
    }
    if (richText) {
        label->setTextFormat(Qt::RichText);
    } else {
        label->setTextFormat(Qt::PlainText);
    }
    if (label->text() == text) {
        return;
    }
    label->setText(text);
    label->updateGeometry();
    refreshLayoutAround(label);
    animateWidgetRefresh(label, duration);
}

} // namespace

ChatPage::ChatPage(QWidget* parent)
    : BasePage("实时对话", "按下三键键盘 K-B 后，这里聚焦当前画面、执行过程与最终回答。", parent) {
    auto* workspaceRow = new QWidget(this);
    workspaceRow->setObjectName("chatWorkspaceRow");
    auto* workspaceLayout = new QHBoxLayout(workspaceRow);
    workspaceLayout->setContentsMargins(0, 0, 0, 0);
    workspaceLayout->setSpacing(14);

    auto* leftWorkspace = new QWidget(this);
    leftWorkspace->setObjectName("chatWorkspaceLeft");
    leftWorkspace->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto* leftLayout = new QVBoxLayout(leftWorkspace);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(12);

    auto* stagePanel = new QWidget(this);
    stagePanel->setObjectName("chatStagePanel");
    stagePanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    auto* stageLayout = new QVBoxLayout(stagePanel);
    stageLayout->setContentsMargins(18, 16, 18, 16);
    stageLayout->setSpacing(8);

    stageTitle_ = new QLabel(QStringLiteral("任务舞台已就绪"), stagePanel);
    stageTitle_->setObjectName("chatStageTitle");
    stageStatus_ = new QLabel(QStringLiteral("连接成功后，这里会即时显示录音、识别与分析阶段。"), stagePanel);
    stageStatus_->setObjectName("chatStageStatus");
    stageStatus_->setWordWrap(true);
    stageStatus_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    stageMeta_ = new QLabel(QStringLiteral("触发：三键键盘 K-B    ·    摄像头：Logitech C270"), stagePanel);
    stageMeta_->setObjectName("chatStageMeta");
    stageMeta_->setWordWrap(true);
    stageMeta_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    stageLayout->addWidget(stageTitle_);
    stageLayout->addWidget(stageStatus_);
    stageLayout->addWidget(stageMeta_);

    auto* visualCard = new QWidget(this);
    visualCard->setObjectName("chatVisualCard");
    visualCard->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto* visualLayout = new QVBoxLayout(visualCard);
    visualLayout->setContentsMargins(16, 16, 16, 16);
    visualLayout->setSpacing(10);

    auto* visualTitle = new QLabel(QStringLiteral("实时画面"), visualCard);
    visualTitle->setObjectName("chatPanelTitle");
    visualStatus_ = new QLabel(QStringLiteral("等待摄像头同步最新画面"), visualCard);
    visualStatus_->setObjectName("chatPanelSubtle");
    visualStatus_->setWordWrap(true);
    visualStatus_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    visualFrame_ = new QLabel(QStringLiteral("当树莓派抓取到最新照片后，这里会展示大图预览。"), visualCard);
    visualFrame_->setObjectName("chatImageHero");
    visualFrame_->setAlignment(Qt::AlignCenter);
    visualFrame_->setWordWrap(true);
    visualFrame_->setMinimumSize(500, 300);
    visualFrame_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    visualFrame_->setMinimumHeight(300);

    visualLayout->addWidget(visualTitle);
    visualLayout->addWidget(visualStatus_);
    visualLayout->addWidget(visualFrame_, 1);

    sectionCaption_ = new QLabel(QStringLiteral("Conversation"), this);
    sectionCaption_->setObjectName("chatSectionLabel");

    auto* conversationCard = new QWidget(this);
    conversationCard->setObjectName("chatConversationCard");
    conversationCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto* conversationLayout = new QVBoxLayout(conversationCard);
    conversationLayout->setContentsMargins(18, 18, 18, 18);
    conversationLayout->setSpacing(12);

    chatScroll_ = new QScrollArea(this);
    chatScroll_->setWidgetResizable(true);
    chatScroll_->setObjectName("chatScroll");
    chatScroll_->setFrameShape(QFrame::NoFrame);
    chatScroll_->setAttribute(Qt::WA_TranslucentBackground, true);
    chatScroll_->viewport()->setAttribute(Qt::WA_TranslucentBackground, true);
    chatScroll_->viewport()->setAutoFillBackground(false);

    auto* inner = new QWidget(chatScroll_);
    inner->setAttribute(Qt::WA_TranslucentBackground, true);
    inner->setAutoFillBackground(false);
    messages_ = new QVBoxLayout(inner);
    messages_->setContentsMargins(8, 8, 8, 8);
    messages_->setSpacing(16);
    messages_->addStretch(1);
    chatScroll_->setWidget(inner);

    conversationLayout->addWidget(sectionCaption_);
    conversationLayout->addWidget(chatScroll_, 1);

    auto* rightWorkspace = new QWidget(this);
    rightWorkspace->setObjectName("chatWorkspaceRight");
    rightWorkspace->setMinimumWidth(360);
    rightWorkspace->setMaximumWidth(430);
    rightWorkspace->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto* rightLayout = new QVBoxLayout(rightWorkspace);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(12);

    auto* overviewCard = new QWidget(this);
    overviewCard->setObjectName("chatPanelCard");
    overviewCard->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    auto* overviewLayout = new QVBoxLayout(overviewCard);
    overviewLayout->setContentsMargins(16, 16, 16, 16);
    overviewLayout->setSpacing(10);
    overviewLayout->setSizeConstraint(QLayout::SetMinimumSize);
    auto* overviewTitle = new QLabel(QStringLiteral("本轮摘要"), overviewCard);
    overviewTitle->setObjectName("chatPanelTitle");
    auto* userCaption = new QLabel(QStringLiteral("触发与流程"), overviewCard);
    userCaption->setObjectName("chatPanelSubtle");
    userSummary_ = new QLabel(QStringLiteral("暂无触发记录。"), overviewCard);
    userSummary_->setObjectName("chatPanelBody");
    userSummary_->setWordWrap(true);
    userSummary_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    overviewLayout->addWidget(overviewTitle);
    overviewLayout->addWidget(userCaption);
    overviewLayout->addWidget(userSummary_);

    auto* answerCard = new QWidget(this);
    answerCard->setObjectName("chatAnswerCard");
    answerCard->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    auto* answerLayout = new QVBoxLayout(answerCard);
    answerLayout->setContentsMargins(18, 18, 18, 18);
    answerLayout->setSpacing(8);
    answerLayout->setSizeConstraint(QLayout::SetMinimumSize);
    auto* answerTitle = new QLabel(QStringLiteral("回答正文"), answerCard);
    answerTitle->setObjectName("chatPanelTitle");
    answerSummary_ = new QLabel(QStringLiteral("等待第一条 AI 回答。"), answerCard);
    answerSummary_->setObjectName("chatAnswerBody");
    answerSummary_->setWordWrap(true);
    answerSummary_->setTextFormat(Qt::PlainText);
    answerSummary_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    answerLayout->addWidget(answerTitle);
    answerLayout->addWidget(answerSummary_, 1);

    leftLayout->addWidget(stagePanel, 0);
    leftLayout->addWidget(conversationCard, 1);

    rightLayout->addWidget(visualCard, 7);
    rightLayout->addWidget(overviewCard, 3);
    rightLayout->addWidget(answerCard, 5);

    workspaceLayout->addWidget(leftWorkspace, 9);
    workspaceLayout->addWidget(rightWorkspace, 5);

    bodyLayout()->addWidget(workspaceRow, 1);
    appendDemoConversation();
}

void ChatPage::appendDemoConversation() {
    lastSessionKey_.clear();
    clearMessages();
    const int insertAt = qMax(0, messages_->count() - 1);
    auto* system = new ChatMessageWidget(ChatMessageWidget::Role::System, this);
    system->setMessage("等待硬件触发",
        "连接成功后，按下三键键盘 K-B 即可开始 5 秒语音输入。完成后这里会显示摄像头画面、识别流程和 AI 回答。");
    messages_->insertWidget(insertAt, system);

    ConnectionState placeholder;
    placeholder.assistantStatus = AssistantStatus::Connecting;
    updateStagePanel(placeholder);
    updateOverviewPanels(placeholder);
}

void ChatPage::setLatestSession(const ConnectionState& state) {
    updateStagePanel(state);
    updateOverviewPanels(state);

    QString newKey;
    if (state.recentRecords.isEmpty()) {
        newKey = "empty|" + state.assistantStatusText + "|" + state.localFramePath;
    } else {
        const ConversationRecord& record = state.recentRecords.first();
        newKey = record.title + "|" + record.userText + "|" + record.flowText + "|" + record.aiText + "|" + record.imagePath + "|" + state.localFramePath;
    }
    if (newKey == lastSessionKey_) {
        return;
    }
    lastSessionKey_ = newKey;

    clearMessages();
    const int insertAt = qMax(0, messages_->count() - 1);

    if (state.recentRecords.isEmpty()) {
        auto* system = new ChatMessageWidget(ChatMessageWidget::Role::System, this);
        system->setMessage("暂无成功对话",
            state.assistantStatusText.isEmpty()
                ? "暂时没有从树莓派读取到完整的 AI 分析记录。"
                : state.assistantStatusText,
            state.localFramePath);
        messages_->insertWidget(insertAt, system);
        return;
    }

    const ConversationRecord& record = state.recentRecords.first();
    const QString imagePath = record.imagePath.isEmpty() ? state.localFramePath : record.imagePath;

    auto* user = new ChatMessageWidget(ChatMessageWidget::Role::User, this);
    user->setMessage("用户输入 / 现场画面", record.userText, imagePath);
    messages_->insertWidget(insertAt, user);

    auto* flow = new ChatMessageWidget(ChatMessageWidget::Role::System, this);
    flow->setMessage("执行流程", record.flowText.isEmpty() ? "暂无流程日志。" : record.flowText);
    messages_->insertWidget(insertAt + 1, flow);

    auto* assistant = new ChatMessageWidget(ChatMessageWidget::Role::Assistant, this);
    assistant->setMessage("AI 回答 / 分析", record.aiText.isEmpty() ? "当前日志中没有解析到 AI 回复。" : record.aiText);
    messages_->insertWidget(insertAt + 2, assistant);

    if (chatScroll_ != nullptr && chatScroll_->verticalScrollBar() != nullptr) {
        QTimer::singleShot(0, this, [this]() {
            if (chatScroll_ != nullptr && chatScroll_->verticalScrollBar() != nullptr) {
                chatScroll_->verticalScrollBar()->setValue(chatScroll_->verticalScrollBar()->maximum());
            }
        });
    }
}

void ChatPage::clearMessages() {
    while (messages_->count() > 1) {
        QLayoutItem* item = messages_->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void ChatPage::updateStagePanel(const ConnectionState& state) {
    setAnimatedLabelText(stageTitle_, stageTitleText(state), false, 220);
    setAnimatedLabelText(stageStatus_, stageStatusText(state), false, 220);
    setAnimatedLabelText(stageMeta_, stageMetaText(state), false, 220);
}

void ChatPage::updateOverviewPanels(const ConnectionState& state) {
    if (state.recentRecords.isEmpty()) {
        setAnimatedLabelText(userSummary_, summaryOrFallback(
            state.assistantStatusText,
            QStringLiteral("等待用户通过三键键盘 K-B 发起一次新的语音分析。")), false, 230);
        setAnimatedLabelText(answerSummary_, formatAnswerText(QStringLiteral("系统尚未收到可展示的 AI 回复。")), false, 230);
    } else {
        const ConversationRecord& record = state.recentRecords.first();
        setAnimatedLabelText(userSummary_, buildCompactOverview(record.userText, record.flowText), false, 230);
        setAnimatedLabelText(answerSummary_, formatAnswerText(record.aiText), false, 230);
    }

    const QString imagePath = !state.recentRecords.isEmpty() && !state.recentRecords.first().imagePath.isEmpty()
        ? state.recentRecords.first().imagePath
        : state.localFramePath;
    const QPixmap pixmap = loadHeroPixmap(imagePath);
    const QString oldKey = visualFrame_->property("contentKey").toString();
    if (!pixmap.isNull()) {
        visualFrame_->setPixmap(pixmap.scaled(620, 360, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        visualFrame_->setProperty("contentKey", imagePath);
        setAnimatedLabelText(visualStatus_, QStringLiteral("最新画面已同步，当前用于视觉分析与结果展示。"), false, 220);
    } else if (!imagePath.isEmpty()) {
        visualFrame_->setPixmap(QPixmap());
        visualFrame_->setText(QStringLiteral("图片已缓存，但当前无法显示。\n%1").arg(imagePath));
        visualFrame_->setProperty("contentKey", QStringLiteral("error:%1").arg(imagePath));
        setAnimatedLabelText(visualStatus_, QStringLiteral("本地存在图片路径，但 Qt 暂未成功解码。"), false, 220);
    } else {
        visualFrame_->setPixmap(QPixmap());
        visualFrame_->setText(QStringLiteral("等待树莓派同步首张摄像头画面。"));
        visualFrame_->setProperty("contentKey", QStringLiteral("empty"));
        setAnimatedLabelText(visualStatus_, QStringLiteral("摄像头还没有返回可展示的大图。"), false, 220);
    }
    if (oldKey != visualFrame_->property("contentKey").toString()) {
        animateWidgetRefresh(visualFrame_, 260);
    }
}
