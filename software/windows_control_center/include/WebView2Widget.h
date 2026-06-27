#pragma once

#include <QWidget>

class QLabel;
class QString;

#ifdef Q_OS_WIN
#include <wrl/client.h>
#include <WebView2.h>
#endif

class WebView2Widget : public QWidget {
    Q_OBJECT

public:
    explicit WebView2Widget(QWidget* parent = nullptr);
    void setHtmlContent(const QString& html);

Q_SIGNALS:
    void initializationFailed(const QString& reason);

protected:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void initializeIfNeeded();
    void updateControllerBounds();
    void navigatePendingContent();
    QString htmlCachePath() const;
    void showFallbackText(const QString& text);

#ifdef Q_OS_WIN
    class EnvironmentCompletedHandler;
    class ControllerCompletedHandler;

    HRESULT handleEnvironmentCreated(HRESULT result, ICoreWebView2Environment* environment);
    HRESULT handleControllerCreated(HRESULT result, ICoreWebView2Controller* controller);
#endif

    QLabel* fallbackLabel_ = nullptr;
    QString pendingHtml_;
    bool initializationStarted_ = false;
    bool initializationFinished_ = false;

#ifdef Q_OS_WIN
    Microsoft::WRL::ComPtr<ICoreWebView2Environment> environment_;
    Microsoft::WRL::ComPtr<ICoreWebView2Controller> controller_;
    Microsoft::WRL::ComPtr<ICoreWebView2> webView_;
#endif
};
