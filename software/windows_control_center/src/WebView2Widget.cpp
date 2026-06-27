#include "WebView2Widget.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QResizeEvent>
#include <QShowEvent>
#include <QStandardPaths>
#include <QUrl>
#include <QVBoxLayout>

#ifdef Q_OS_WIN
#include <windows.h>
#include <wrl/client.h>
#include <WebView2.h>
#endif

namespace {

QString winErrorMessage(HRESULT code) {
#ifdef Q_OS_WIN
    LPWSTR raw = nullptr;
    const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    const DWORD length = FormatMessageW(flags,
                                        nullptr,
                                        static_cast<DWORD>(code),
                                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                        reinterpret_cast<LPWSTR>(&raw),
                                        0,
                                        nullptr);
    QString result;
    if (length > 0 && raw != nullptr) {
        result = QString::fromWCharArray(raw, static_cast<int>(length)).trimmed();
        LocalFree(raw);
    }
    if (result.isEmpty()) {
        result = QStringLiteral("HRESULT=0x%1").arg(static_cast<quint32>(code), 8, 16, QLatin1Char('0'));
    }
    return result;
#else
    Q_UNUSED(code)
    return QStringLiteral("Unsupported platform");
#endif
}

QString webViewCacheRoot() {
    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir root(basePath);
    root.mkpath(QStringLiteral("cache"));
    return root.filePath(QStringLiteral("cache"));
}

} // namespace

#ifdef Q_OS_WIN

class WebView2Widget::EnvironmentCompletedHandler final
    : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler {
public:
    explicit EnvironmentCompletedHandler(WebView2Widget* owner)
        : owner_(owner) {
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** object) override {
        if (object == nullptr) {
            return E_POINTER;
        }
        *object = nullptr;
        if (riid == IID_IUnknown || riid == IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler) {
            *object = static_cast<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*>(this);
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override {
        return static_cast<ULONG>(InterlockedIncrement(&refCount_));
    }

    ULONG STDMETHODCALLTYPE Release() override {
        const ULONG value = static_cast<ULONG>(InterlockedDecrement(&refCount_));
        if (value == 0) {
            delete this;
        }
        return value;
    }

    HRESULT STDMETHODCALLTYPE Invoke(HRESULT result, ICoreWebView2Environment* environment) override {
        return owner_ != nullptr ? owner_->handleEnvironmentCreated(result, environment) : E_FAIL;
    }

private:
    volatile long refCount_ = 1;
    WebView2Widget* owner_ = nullptr;
};

class WebView2Widget::ControllerCompletedHandler final
    : public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler {
public:
    explicit ControllerCompletedHandler(WebView2Widget* owner)
        : owner_(owner) {
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** object) override {
        if (object == nullptr) {
            return E_POINTER;
        }
        *object = nullptr;
        if (riid == IID_IUnknown || riid == IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler) {
            *object = static_cast<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler*>(this);
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override {
        return static_cast<ULONG>(InterlockedIncrement(&refCount_));
    }

    ULONG STDMETHODCALLTYPE Release() override {
        const ULONG value = static_cast<ULONG>(InterlockedDecrement(&refCount_));
        if (value == 0) {
            delete this;
        }
        return value;
    }

    HRESULT STDMETHODCALLTYPE Invoke(HRESULT result, ICoreWebView2Controller* controller) override {
        return owner_ != nullptr ? owner_->handleControllerCreated(result, controller) : E_FAIL;
    }

private:
    volatile long refCount_ = 1;
    WebView2Widget* owner_ = nullptr;
};

#endif

WebView2Widget::WebView2Widget(QWidget* parent)
    : QWidget(parent) {
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_DontCreateNativeAncestors);
    setAutoFillBackground(false);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    fallbackLabel_ = new QLabel(QStringLiteral("正在初始化内嵌浏览器..."), this);
    fallbackLabel_->setAlignment(Qt::AlignCenter);
    fallbackLabel_->setWordWrap(true);
    fallbackLabel_->setObjectName("chatPanelBody");
    layout->addWidget(fallbackLabel_, 1);
}

void WebView2Widget::setHtmlContent(const QString& html) {
    pendingHtml_ = html;
    if (initializationFinished_) {
        navigatePendingContent();
    } else {
        initializeIfNeeded();
    }
}

void WebView2Widget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    initializeIfNeeded();
}

void WebView2Widget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateControllerBounds();
}

void WebView2Widget::initializeIfNeeded() {
#ifndef Q_OS_WIN
    showFallbackText(QStringLiteral("当前平台不支持 WebView2。"));
    return;
#else
    if (initializationStarted_ || initializationFinished_) {
        return;
    }

    initializationStarted_ = true;
    const QString cacheRoot = webViewCacheRoot();
    const std::wstring userDataDir = QDir(cacheRoot).filePath(QStringLiteral("webview2-profile")).toStdWString();
    const HRESULT result = CreateCoreWebView2EnvironmentWithOptions(
        nullptr,
        userDataDir.c_str(),
        nullptr,
        new EnvironmentCompletedHandler(this));
    if (FAILED(result)) {
        initializationStarted_ = false;
        showFallbackText(QStringLiteral("WebView2 环境创建失败：%1").arg(winErrorMessage(result)));
    }
#endif
}

void WebView2Widget::updateControllerBounds() {
#ifdef Q_OS_WIN
    if (!controller_) {
        return;
    }
    RECT bounds {};
    bounds.left = 0;
    bounds.top = 0;
    bounds.right = width();
    bounds.bottom = height();
    controller_->put_Bounds(bounds);
#endif
}

void WebView2Widget::navigatePendingContent() {
#ifdef Q_OS_WIN
    if (!webView_) {
        return;
    }

    const QString targetPath = htmlCachePath();
    QFile file(targetPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        showFallbackText(QStringLiteral("无法写入聊天 HTML 缓存：%1").arg(targetPath));
        return;
    }
    file.write(pendingHtml_.toUtf8());
    file.close();

    const QString fileUrl = QUrl::fromLocalFile(targetPath).toString()
        + QStringLiteral("?v=%1").arg(QDateTime::currentMSecsSinceEpoch());
    webView_->Navigate(reinterpret_cast<LPCWSTR>(fileUrl.utf16()));
    fallbackLabel_->hide();
#endif
}

QString WebView2Widget::htmlCachePath() const {
    return QDir(webViewCacheRoot()).filePath(QStringLiteral("conversation.html"));
}

void WebView2Widget::showFallbackText(const QString& text) {
    initializationStarted_ = false;
    initializationFinished_ = false;
    if (fallbackLabel_ != nullptr) {
        fallbackLabel_->setText(text);
        fallbackLabel_->show();
    }
    Q_EMIT initializationFailed(text);
}

#ifdef Q_OS_WIN

HRESULT WebView2Widget::handleEnvironmentCreated(HRESULT result, ICoreWebView2Environment* environment) {
    if (FAILED(result) || environment == nullptr) {
        showFallbackText(QStringLiteral("WebView2 环境初始化失败：%1").arg(winErrorMessage(result)));
        return result;
    }

    environment_ = environment;
    const HWND parentWindow = reinterpret_cast<HWND>(winId());
    const HRESULT controllerResult = environment_->CreateCoreWebView2Controller(
        parentWindow,
        new ControllerCompletedHandler(this));
    if (FAILED(controllerResult)) {
        showFallbackText(QStringLiteral("WebView2 控制器创建失败：%1").arg(winErrorMessage(controllerResult)));
        return controllerResult;
    }
    return S_OK;
}

HRESULT WebView2Widget::handleControllerCreated(HRESULT result, ICoreWebView2Controller* controller) {
    if (FAILED(result) || controller == nullptr) {
        showFallbackText(QStringLiteral("WebView2 控制器初始化失败：%1").arg(winErrorMessage(result)));
        return result;
    }

    controller_ = controller;
    HRESULT coreResult = controller_->get_CoreWebView2(&webView_);
    if (FAILED(coreResult) || !webView_) {
        showFallbackText(QStringLiteral("WebView2 内核获取失败：%1").arg(winErrorMessage(coreResult)));
        return coreResult;
    }

    controller_->put_IsVisible(TRUE);
    updateControllerBounds();
    initializationFinished_ = true;
    fallbackLabel_->hide();
    navigatePendingContent();
    return S_OK;
}

#endif
