#pragma once

#include <QString>
#include <QVariantMap>

class WhatSonStructuredTagLinter final
{
public:
    WhatSonStructuredTagLinter();
    ~WhatSonStructuredTagLinter();

    QString normalizeStructuredSourceText(const QString& sourceText) const;
    QVariantMap buildBreakVerification(const QString& sourceText) const;
    QVariantMap buildStructuredVerification(const QString& sourceText) const;
};
