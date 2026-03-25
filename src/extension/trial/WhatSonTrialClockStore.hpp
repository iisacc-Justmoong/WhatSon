#pragma once

#include <QDateTime>
#include <QString>

struct WhatSonTrialClockCheck final
{
    QDateTime lastExitTimestampUtc;
    QDateTime lastSeenTimestampUtc;
    qint64 rollbackSeconds = 0;
    bool clockRollbackDetected = false;

    bool operator==(const WhatSonTrialClockCheck& other) const = default;
};

class WhatSonTrialClockStore final
{
public:
    explicit WhatSonTrialClockStore(
        QString lastExitTimestampSettingsKey = defaultLastExitTimestampSettingsKey(),
        QString lastSeenTimestampSettingsKey = defaultLastSeenTimestampSettingsKey());

    static QString defaultLastExitTimestampSettingsKey();
    static QString defaultLastSeenTimestampSettingsKey();

    QString lastExitTimestampSettingsKey() const;
    QString lastSeenTimestampSettingsKey() const;

    QDateTime loadLastExitTimestampUtc() const;
    QDateTime loadLastSeenTimestampUtc() const;

    WhatSonTrialClockCheck inspect(const QDateTime& nowUtc = QDateTime::currentDateTimeUtc()) const;
    WhatSonTrialClockCheck stampExitTimestamp(const QDateTime& exitTimestampUtc = QDateTime::currentDateTimeUtc()) const;
    void clear() const;

private:
    QString m_lastExitTimestampSettingsKey;
    QString m_lastSeenTimestampSettingsKey;
};
