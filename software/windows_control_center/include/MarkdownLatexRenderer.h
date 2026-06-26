#pragma once

#include "AppConfig.h"

#include <QString>

class MarkdownLatexRenderer {
public:
    explicit MarkdownLatexRenderer(const AppConfig& config);

    QString renderToHtml(const QString& markdownText) const;

private:
    QString renderFormulaToImageHtml(const QString& latex, bool blockMode) const;
    QString downloadFormulaPng(const QString& latex, bool blockMode) const;
    QString cacheDirPath() const;

    const AppConfig& config_;
};
