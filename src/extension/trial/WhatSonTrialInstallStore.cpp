#include "WhatSonTrialInstallStore.hpp"

#include <QSettings>
#include <QVariant>

#include <functional>
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

    QString loadMirroredTextValue(
        const QString& settingsKey,
        const WhatSonTrialSecureStore& secureStore,
        const std::function<QString(const QString&)>& normalizeValue)
    {
        QSettings settings;

        const QString rawSettingsValue = settings.value(settingsKey).toString();
        const QString normalizedSettingsValue = normalizeValue(rawSettingsValue);
        if (normalizedSettingsValue.isEmpty())
        {
            if (!rawSettingsValue.trimmed().isEmpty() || settings.contains(settingsKey))
            {
                settings.remove(settingsKey);
                settings.sync();
            }
        }
        else if (rawSettingsValue != normalizedSettingsValue)
        {
            settings.setValue(settingsKey, normalizedSettingsValue);
            settings.sync();
        }

        const WhatSonTrialSecureStoreReadResult secureResult = secureStore.readEntry(settingsKey);
        const QString normalizedSecureValue = normalizeValue(secureResult.value);
        if (secureResult.status == WhatSonTrialSecureStoreStatus::Success && !normalizedSecureValue.isEmpty())
        {
            if (normalizedSettingsValue != normalizedSecureValue)
            {
                settings.setValue(settingsKey, normalizedSecureValue);
                settings.sync();
            }
            return normalizedSecureValue;
        }

        if (secureResult.status == WhatSonTrialSecureStoreStatus::Success && normalizedSecureValue.isEmpty())
        {
            (void)secureStore.removeEntry(settingsKey);
        }

        if (!normalizedSettingsValue.isEmpty())
        {
            if (secureResult.status == WhatSonTrialSecureStoreStatus::NotFound
                || (secureResult.status == WhatSonTrialSecureStoreStatus::Success && normalizedSecureValue.isEmpty()))
            {
                (void)secureStore.writeEntry(settingsKey, normalizedSettingsValue);
            }
            return normalizedSettingsValue;
        }

        return {};
    }

    void storeMirroredTextValue(
        const QString& settingsKey,
        const QString& normalizedValue,
        const WhatSonTrialSecureStore& secureStore)
    {
        QSettings settings;
        if (normalizedValue.isEmpty())
        {
            settings.remove(settingsKey);
            settings.sync();
            (void)secureStore.removeEntry(settingsKey);
            return;
        }

        settings.setValue(settingsKey, normalizedValue);
        settings.sync();
        (void)secureStore.writeEntry(settingsKey, normalizedValue);
    }
}

WhatSonTrialInstallStore::WhatSonTrialInstallStore(
    QString installDateSettingsKey,
    WhatSonTrialSecureStore secureStore)
    : m_installDateSettingsKey(std::move(installDateSettingsKey))
    , m_secureStore(std::move(secureStore))
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
    const QString installDateText = loadMirroredTextValue(
        m_installDateSettingsKey,
        m_secureStore,
        [](const QString& value) {
            const QDate date = normalizeStoredDate(value);
            return date.isValid() ? date.toString(Qt::ISODate) : QString();
        });
    return QDate::fromString(installDateText, Qt::ISODate);
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
    storeMirroredTextValue(
        m_installDateSettingsKey,
        installDate.isValid() ? installDate.toString(Qt::ISODate) : QString(),
        m_secureStore);
}

void WhatSonTrialInstallStore::clear() const
{
    storeMirroredTextValue(m_installDateSettingsKey, {}, m_secureStore);
}
