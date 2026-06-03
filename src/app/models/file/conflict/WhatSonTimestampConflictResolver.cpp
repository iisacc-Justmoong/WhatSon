#include "app/models/file/conflict/WhatSonTimestampConflictResolver.hpp"

#include <QDateTime>
#include <QStringList>
#include <QTimeZone>

namespace
{
    QDateTime parseTimestamp(const QString& timestamp)
    {
        const QString value = timestamp.trimmed();
        if (value.isEmpty())
        {
            return {};
        }

        const QStringList formats{
            QStringLiteral("yyyy-MM-dd-hh-mm-ss"),
            QStringLiteral("yyyy-MM-ddTHH:mm:ss.zzzZ"),
            QStringLiteral("yyyy-MM-ddTHH:mm:ssZ"),
            QStringLiteral("yyyy-MM-ddTHH:mm:ss.zzz"),
            QStringLiteral("yyyy-MM-ddTHH:mm:ss")
        };
        for (const QString& format : formats)
        {
            QDateTime parsed = QDateTime::fromString(value, format);
            if (!parsed.isValid())
            {
                continue;
            }
            if (format.endsWith(QLatin1Char('Z')))
            {
                parsed.setTimeZone(QTimeZone::UTC);
            }
            return parsed;
        }

        QDateTime parsed = QDateTime::fromString(value, Qt::ISODateWithMs);
        if (parsed.isValid())
        {
            return parsed;
        }
        return QDateTime::fromString(value, Qt::ISODate);
    }

    int compareTimestamps(const QString& left, const QString& right)
    {
        const QDateTime parsedLeft = parseTimestamp(left);
        const QDateTime parsedRight = parseTimestamp(right);
        if (parsedLeft.isValid() && parsedRight.isValid())
        {
            if (parsedLeft < parsedRight)
            {
                return -1;
            }
            if (parsedLeft > parsedRight)
            {
                return 1;
            }
            return 0;
        }

        const QString normalizedLeft = left.trimmed();
        const QString normalizedRight = right.trimmed();
        if (normalizedLeft < normalizedRight)
        {
            return -1;
        }
        if (normalizedLeft > normalizedRight)
        {
            return 1;
        }
        return 0;
    }
}

bool WhatSonTimestampConflictResolver::isTimestampNewer(
    const QString& candidateLastModifiedAt,
    const QString& baselineLastModifiedAt) const
{
    const QString candidate = candidateLastModifiedAt.trimmed();
    if (candidate.isEmpty())
    {
        return false;
    }

    const QString baseline = baselineLastModifiedAt.trimmed();
    if (baseline.isEmpty())
    {
        return true;
    }

    return compareTimestamps(candidate, baseline) > 0;
}
