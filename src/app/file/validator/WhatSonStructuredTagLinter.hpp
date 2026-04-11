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
    QVariantMap buildAgendaVerification(
        const QString& sourceText,
        int parsedAgendaCount,
        int parsedTaskCount,
        int invalidAgendaChildCount) const;
    QVariantMap buildCalloutVerification(
        const QString& sourceText,
        int parsedCalloutCount) const;
    QVariantMap buildStructuredVerification(
        const QVariantMap& agendaVerification,
        const QVariantMap& calloutVerification,
        const QString& sourceText) const;
};
