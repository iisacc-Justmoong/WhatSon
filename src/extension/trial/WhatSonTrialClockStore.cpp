#include "WhatSonTrialClockStore.hpp"

#include <QSettings>
#include <QVariant>

#include <utility>

namespace
{
    QDateTime normalizeUtcTimestamp(const QDateTime& timestamp)
    {
        if (!timestamp.isValid())
        {
            return {};
        }

        return timestamp.toUTC();
    }

    QDateTime normalizeStoredTimestamp(const QVariant& rawValue)
    {
        if (!rawValue.isValid())
        {
            return {};
        }

        const QDateTime directTimestamp = normalizeUtcTimestamp(rawValue.toDateTime());
        if (directTimestamp.isValid())
        {
            return directTimestamp;
        }

        const QString timestampText = rawValue.toString().trimmed();
        if (timestampText.isEmpty())
        {
            return {};
        }

        QDateTime parsedTimestamp = QDateTime::fromString(timestampText, Qt::ISODateWithMs);
        if (!parsedTimestamp.isValid())
        {
            parsedTimestamp = QDateTime::fromString(timestampText, Qt::ISODate);
        }

        return normalizeUtcTimestamp(parsedTimestamp);
    }

    QDateTime loadTimestamp(QSettings& settings, const QString& key)
    {
        const QVariant rawStoredValue = settings.value(key);
        const QDateTime normalizedTimestamp = normalizeStoredTimestamp(rawStoredValue);
        if (!normalizedTimestamp.isValid())
        {
            if (rawStoredValue.isValid())
            {
                settings.remove(key);
                settings.sync();
            }
            return {};
        }

        const QString normalizedTimestampText = normalizedTimestamp.toString(Qt::ISODateWithMs);
        if (rawStoredValue.toString() != normalizedTimestampText)
        {
            settings.setValue(key, normalizedTimestampText);
            settings.sync();
        }

        return normalizedTimestamp;
    }

    void storeTimestamp(QSettings& settings, const QString& key, const QDateTime& timestamp)
    {
        const QDateTime normalizedTimestamp = normalizeUtcTimestamp(timestamp);
        if (!normalizedTimestamp.isValid())
        {
            settings.remove(key);
            return;
        }

        settings.setValue(key, normalizedTimestamp.toString(Qt::ISODateWithMs));
    }

    WhatSonTrialClockCheck buildClockCheck(
        const QDateTime& nowUtc,
        const QDateTime& lastExitTimestampUtc,
        const QDateTime& lastSeenTimestampUtc)
    {
        WhatSonTrialClockCheck check;
        check.lastExitTimestampUtc = lastExitTimestampUtc;
        check.lastSeenTimestampUtc = lastSeenTimestampUtc;

        if (nowUtc.isValid() && lastSeenTimestampUtc.isValid() && nowUtc < lastSeenTimestampUtc)
        {
            check.clockRollbackDetected = true;
            check.rollbackSeconds = nowUtc.secsTo(lastSeenTimestampUtc);
        }

        return check;
    }
}

WhatSonTrialClockStore::WhatSonTrialClockStore(
    QString lastExitTimestampSettingsKey,
    QString lastSeenTimestampSettingsKey)
    : m_lastExitTimestampSettingsKey(std::move(lastExitTimestampSettingsKey))
    , m_lastSeenTimestampSettingsKey(std::move(lastSeenTimestampSettingsKey))
{
}

QString WhatSonTrialClockStore::defaultLastExitTimestampSettingsKey()
{
    return QStringLiteral("extension/trial/lastExitTimestampUtc");
}

QString WhatSonTrialClockStore::defaultLastSeenTimestampSettingsKey()
{
    return QStringLiteral("extension/trial/lastSeenTimestampUtc");
}

QString WhatSonTrialClockStore::lastExitTimestampSettingsKey() const
{
    return m_lastExitTimestampSettingsKey;
}

QString WhatSonTrialClockStore::lastSeenTimestampSettingsKey() const
{
    return m_lastSeenTimestampSettingsKey;
}

QDateTime WhatSonTrialClockStore::loadLastExitTimestampUtc() const
{
    QSettings settings;
    return loadTimestamp(settings, m_lastExitTimestampSettingsKey);
}

QDateTime WhatSonTrialClockStore::loadLastSeenTimestampUtc() const
{
    QSettings settings;
    return loadTimestamp(settings, m_lastSeenTimestampSettingsKey);
}

WhatSonTrialClockCheck WhatSonTrialClockStore::inspect(const QDateTime& nowUtc) const
{
    QSettings settings;
    const QDateTime normalizedNow = normalizeUtcTimestamp(
        nowUtc.isValid() ? nowUtc : QDateTime::currentDateTimeUtc());
    const QDateTime lastExitTimestampUtc = loadTimestamp(settings, m_lastExitTimestampSettingsKey);
    const QDateTime lastSeenTimestampUtc = loadTimestamp(settings, m_lastSeenTimestampSettingsKey);
    return buildClockCheck(normalizedNow, lastExitTimestampUtc, lastSeenTimestampUtc);
}

WhatSonTrialClockCheck WhatSonTrialClockStore::stampExitTimestamp(const QDateTime& exitTimestampUtc) const
{
    const QDateTime normalizedExitTimestampUtc = normalizeUtcTimestamp(
        exitTimestampUtc.isValid() ? exitTimestampUtc : QDateTime::currentDateTimeUtc());

    QSettings settings;
    const QDateTime storedLastSeenTimestampUtc = loadTimestamp(settings, m_lastSeenTimestampSettingsKey);
    const QDateTime nextLastSeenTimestampUtc =
        (!storedLastSeenTimestampUtc.isValid() || normalizedExitTimestampUtc > storedLastSeenTimestampUtc)
            ? normalizedExitTimestampUtc
            : storedLastSeenTimestampUtc;

    storeTimestamp(settings, m_lastExitTimestampSettingsKey, normalizedExitTimestampUtc);
    storeTimestamp(settings, m_lastSeenTimestampSettingsKey, nextLastSeenTimestampUtc);
    settings.sync();

    return buildClockCheck(normalizedExitTimestampUtc, normalizedExitTimestampUtc, nextLastSeenTimestampUtc);
}

void WhatSonTrialClockStore::clear() const
{
    QSettings settings;
    settings.remove(m_lastExitTimestampSettingsKey);
    settings.remove(m_lastSeenTimestampSettingsKey);
    settings.sync();
}
