#pragma once

#include <QByteArray>
#include <QDebug>
#include <QString>
#include <QtGlobal>

namespace WhatSon::Debug
{
    inline bool parseBoolFlag(const QByteArray& rawValue, bool* outRecognized = nullptr)
    {
        const QByteArray normalized = rawValue.trimmed().toLower();
        const bool isTrue = normalized == "1"
            || normalized == "true"
            || normalized == "yes"
            || normalized == "on";
        const bool isFalse = normalized == "0"
            || normalized == "false"
            || normalized == "no"
            || normalized == "off";

        if (outRecognized != nullptr)
        {
            *outRecognized = isTrue || isFalse;
        }

        if (isTrue)
        {
            return true;
        }
        if (isFalse)
        {
            return false;
        }
        return true;
    }

    inline bool isEnabled()
    {
        static const bool enabled = []()
        {
            // Default-on for local development visibility.
            const QByteArray raw = qgetenv("WHATSON_DEBUG_MODE");
            if (raw.trimmed().isEmpty())
            {
                return true;
            }

            bool recognized = false;
            const bool parsed = parseBoolFlag(raw, &recognized);
            return recognized ? parsed : true;
        }();

        return enabled;
    }

    inline QString normalizeSegment(const QString& value, const QString& fallback)
    {
        const QString trimmed = value.trimmed();
        return trimmed.isEmpty() ? fallback : trimmed;
    }

    inline void trace(
        const QString& scope,
        const QString& action,
        const QString& detail = QString())
    {
        if (!isEnabled())
        {
            return;
        }

        const QString prefix = QStringLiteral("[whatson:debug][%1][%2]")
            .arg(
                normalizeSegment(scope, QStringLiteral("general")),
                normalizeSegment(action, QStringLiteral("event")));

        if (detail.trimmed().isEmpty())
        {
            qWarning().noquote() << prefix;
            return;
        }

        qWarning().noquote() << prefix << detail;
    }
} // namespace WhatSon::Debug
