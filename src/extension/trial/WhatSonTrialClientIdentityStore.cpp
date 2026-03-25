#include "WhatSonTrialClientIdentityStore.hpp"

#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QSettings>
#include <QSysInfo>
#include <QUuid>

#include <utility>

namespace
{
    inline constexpr int kTrialClientKeyLength = 32;

    QString createAlphaNumericKey(const int length)
    {
        static const QString kAlphabet =
            QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

        QString key;
        key.reserve(length);
        for (int index = 0; index < length; ++index)
        {
            key.push_back(kAlphabet.at(QRandomGenerator::global()->bounded(kAlphabet.size())));
        }
        return key;
    }
}

WhatSonTrialClientIdentityStore::WhatSonTrialClientIdentityStore(
    QString deviceUuidSettingsKey,
    QString clientKeySettingsKey)
    : m_deviceUuidSettingsKey(std::move(deviceUuidSettingsKey))
    , m_clientKeySettingsKey(std::move(clientKeySettingsKey))
{
}

QString WhatSonTrialClientIdentityStore::defaultDeviceUuidSettingsKey()
{
    return QStringLiteral("extension/trial/deviceUuid");
}

QString WhatSonTrialClientIdentityStore::defaultClientKeySettingsKey()
{
    return QStringLiteral("extension/trial/clientKey");
}

QString WhatSonTrialClientIdentityStore::normalizeDeviceUuid(QString value)
{
    value = value.trimmed();
    if (value.isEmpty())
    {
        return {};
    }

    if (value.startsWith(QLatin1Char('{')) && value.endsWith(QLatin1Char('}')))
    {
        value = value.mid(1, value.size() - 2);
    }

    const QUuid uuid = QUuid::fromString(QStringLiteral("{%1}").arg(value));
    if (uuid.isNull())
    {
        return {};
    }

    return uuid.toString(QUuid::WithoutBraces);
}

QString WhatSonTrialClientIdentityStore::normalizeClientKey(QString value)
{
    value = value.trimmed();
    if (value.size() != kTrialClientKeyLength)
    {
        return {};
    }

    for (const QChar& character : value)
    {
        if (!character.isLetterOrNumber())
        {
            return {};
        }
    }

    return value;
}

WhatSonTrialClientIdentity WhatSonTrialClientIdentityStore::normalizeIdentity(WhatSonTrialClientIdentity identity)
{
    identity.deviceUuid = normalizeDeviceUuid(std::move(identity.deviceUuid));
    identity.key = normalizeClientKey(std::move(identity.key));
    return identity;
}

QString WhatSonTrialClientIdentityStore::deviceUuidSettingsKey() const
{
    return m_deviceUuidSettingsKey;
}

QString WhatSonTrialClientIdentityStore::clientKeySettingsKey() const
{
    return m_clientKeySettingsKey;
}

QString WhatSonTrialClientIdentityStore::loadDeviceUuid() const
{
    QSettings settings;
    const QString rawStoredValue = settings.value(m_deviceUuidSettingsKey).toString();
    const QString normalizedValue = normalizeDeviceUuid(rawStoredValue);
    if (normalizedValue.isEmpty())
    {
        if (!rawStoredValue.trimmed().isEmpty())
        {
            settings.remove(m_deviceUuidSettingsKey);
            settings.sync();
        }
        return {};
    }

    if (normalizedValue != rawStoredValue)
    {
        settings.setValue(m_deviceUuidSettingsKey, normalizedValue);
        settings.sync();
    }

    return normalizedValue;
}

QString WhatSonTrialClientIdentityStore::loadClientKey() const
{
    QSettings settings;
    const QString rawStoredValue = settings.value(m_clientKeySettingsKey).toString();
    const QString normalizedValue = normalizeClientKey(rawStoredValue);
    if (normalizedValue.isEmpty())
    {
        if (!rawStoredValue.trimmed().isEmpty())
        {
            settings.remove(m_clientKeySettingsKey);
            settings.sync();
        }
        return {};
    }

    if (normalizedValue != rawStoredValue)
    {
        settings.setValue(m_clientKeySettingsKey, normalizedValue);
        settings.sync();
    }

    return normalizedValue;
}

WhatSonTrialClientIdentity WhatSonTrialClientIdentityStore::loadIdentity() const
{
    return {
        loadDeviceUuid(),
        loadClientKey()
    };
}

WhatSonTrialClientIdentity WhatSonTrialClientIdentityStore::ensureIdentity() const
{
    WhatSonTrialClientIdentity identity = loadIdentity();
    bool changed = false;

    if (identity.deviceUuid.isEmpty())
    {
        identity.deviceUuid = createDeviceUuid();
        changed = true;
    }

    if (identity.key.isEmpty())
    {
        identity.key = createClientKey();
        changed = true;
    }

    if (changed)
    {
        storeIdentity(identity);
    }

    return identity;
}

void WhatSonTrialClientIdentityStore::storeIdentity(const WhatSonTrialClientIdentity& identity) const
{
    const WhatSonTrialClientIdentity normalizedIdentity = normalizeIdentity(identity);

    QSettings settings;
    if (normalizedIdentity.deviceUuid.isEmpty())
    {
        settings.remove(m_deviceUuidSettingsKey);
    }
    else
    {
        settings.setValue(m_deviceUuidSettingsKey, normalizedIdentity.deviceUuid);
    }

    if (normalizedIdentity.key.isEmpty())
    {
        settings.remove(m_clientKeySettingsKey);
    }
    else
    {
        settings.setValue(m_clientKeySettingsKey, normalizedIdentity.key);
    }

    settings.sync();
}

void WhatSonTrialClientIdentityStore::clear() const
{
    QSettings settings;
    settings.remove(m_deviceUuidSettingsKey);
    settings.remove(m_clientKeySettingsKey);
    settings.sync();
}

QString WhatSonTrialClientIdentityStore::createDeviceUuid()
{
    const QByteArray machineUniqueId = QSysInfo::machineUniqueId();
    if (!machineUniqueId.isEmpty())
    {
        const QByteArray hashedUniqueId =
            QCryptographicHash::hash(machineUniqueId, QCryptographicHash::Sha256).left(16);
        const QUuid stableDeviceUuid = QUuid::fromRfc4122(hashedUniqueId);
        if (!stableDeviceUuid.isNull())
        {
            return stableDeviceUuid.toString(QUuid::WithoutBraces);
        }
    }

    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString WhatSonTrialClientIdentityStore::createClientKey()
{
    return createAlphaNumericKey(kTrialClientKeyLength);
}
