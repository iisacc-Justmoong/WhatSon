#include "WhatSonTrialInstallStore.hpp"

#include <QSettings>
#include <QVariant>

#include <utility>

namespace
{
    QDate normalizeStoredDate(const QVariant& rawValue)
    {
        if (!rawValue.isValid())
        {
            return {};
        }

        const QDate directDate = rawValue.toDate();
        if (directDate.isValid())
        {
            return directDate;
        }

        const QString dateText = rawValue.toString().trimmed();
        if (dateText.isEmpty())
        {
            return {};
        }

        return QDate::fromString(dateText, Qt::ISODate);
    }
}

WhatSonTrialInstallStore::WhatSonTrialInstallStore(QString installDateSettingsKey)
    : m_installDateSettingsKey(std::move(installDateSettingsKey))
{
}

QString WhatSonTrialInstallStore::defaultInstallDateSettingsKey()
{
    return QStringLiteral("extension/trial/installDate");
}

QString WhatSonTrialInstallStore::installDateSettingsKey() const
{
    return m_installDateSettingsKey;
}

QDate WhatSonTrialInstallStore::loadInstallDate() const
{
    QSettings settings;
    const QVariant rawStoredValue = settings.value(m_installDateSettingsKey);
    const QDate installDate = normalizeStoredDate(rawStoredValue);
    if (!installDate.isValid())
    {
        if (rawStoredValue.isValid())
        {
            settings.remove(m_installDateSettingsKey);
            settings.sync();
        }
        return {};
    }

    const QString normalizedDateText = installDate.toString(Qt::ISODate);
    if (rawStoredValue.toString() != normalizedDateText)
    {
        settings.setValue(m_installDateSettingsKey, normalizedDateText);
        settings.sync();
    }

    return installDate;
}

QDate WhatSonTrialInstallStore::ensureInstallDate(const QDate& today) const
{
    const QDate normalizedToday = today.isValid() ? today : QDate::currentDate();
    const QDate installDate = loadInstallDate();
    if (installDate.isValid())
    {
        return installDate;
    }

    storeInstallDate(normalizedToday);
    return normalizedToday;
}

void WhatSonTrialInstallStore::storeInstallDate(const QDate& installDate) const
{
    QSettings settings;
    if (!installDate.isValid())
    {
        settings.remove(m_installDateSettingsKey);
        settings.sync();
        return;
    }

    settings.setValue(m_installDateSettingsKey, installDate.toString(Qt::ISODate));
    settings.sync();
}

void WhatSonTrialInstallStore::clear() const
{
    QSettings settings;
    settings.remove(m_installDateSettingsKey);
    settings.sync();
}
