#include "WhatSonTrialClientIdentityStore.hpp"

#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QSettings>
#include <QSysInfo>
#include <QUuid>

#include <functional>
#include <utility>

namespace
{
    inline constexpr int kTrialClientKeyLength = 32;
    inline constexpr int kTrialRegisterIntegritySecretLength = 64;

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

    QString createHexKey(const int length)
    {
        static const QString kAlphabet = QStringLiteral("0123456789abcdef");

        QString key;
        key.reserve(length);
        for (int index = 0; index < length; ++index)
        {
            key.push_back(kAlphabet.at(QRandomGenerator::global()->bounded(kAlphabet.size())));
        }
        return key;
    }

    QString loadMirroredTextValue(
        const QString& settingsKey,
        const WhatSonTrialSecureStore& secureStore,
        const std::function<QString(QString)>& normalizeValue)
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

WhatSonTrialClientIdentityStore::WhatSonTrialClientIdentityStore(
    QString deviceUuidSettingsKey,
    QString clientKeySettingsKey,
    QString registerIntegritySecretSettingsKey,
    WhatSonTrialSecureStore secureStore)
    : m_deviceUuidSettingsKey(std::move(deviceUuidSettingsKey))
    , m_clientKeySettingsKey(std::move(clientKeySettingsKey))
    , m_registerIntegritySecretSettingsKey(std::move(registerIntegritySecretSettingsKey))
    , m_secureStore(std::move(secureStore))
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

QString WhatSonTrialClientIdentityStore::defaultRegisterIntegritySecretSettingsKey()
{
    return QStringLiteral("extension/trial/registerIntegritySecret");
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

QString WhatSonTrialClientIdentityStore::normalizeRegisterIntegritySecret(QString value)
{
    value = value.trimmed().toLower();
    if (value.size() != kTrialRegisterIntegritySecretLength)
    {
        return {};
    }

    for (const QChar& character : value)
    {
        const bool decimalDigit = character.isDigit();
        const bool hexLetter = character >= QLatin1Char('a') && character <= QLatin1Char('f');
        if (!decimalDigit && !hexLetter)
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

QString WhatSonTrialClientIdentityStore::registerIntegritySecretSettingsKey() const
{
    return m_registerIntegritySecretSettingsKey;
}

QString WhatSonTrialClientIdentityStore::loadDeviceUuid() const
{
    return loadMirroredTextValue(
        m_deviceUuidSettingsKey,
        m_secureStore,
        [](QString value) { return normalizeDeviceUuid(std::move(value)); });
}

QString WhatSonTrialClientIdentityStore::loadClientKey() const
{
    return loadMirroredTextValue(
        m_clientKeySettingsKey,
        m_secureStore,
        [](QString value) { return normalizeClientKey(std::move(value)); });
}

QString WhatSonTrialClientIdentityStore::loadRegisterIntegritySecret() const
{
    return loadMirroredTextValue(
        m_registerIntegritySecretSettingsKey,
        m_secureStore,
        [](QString value) { return normalizeRegisterIntegritySecret(std::move(value)); });
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

    ensureRegisterIntegritySecret();
    return identity;
}

QString WhatSonTrialClientIdentityStore::ensureRegisterIntegritySecret() const
{
    const QString secret = loadRegisterIntegritySecret();
    if (!secret.isEmpty())
    {
        return secret;
    }

    const QString createdSecret = createRegisterIntegritySecret();
    storeRegisterIntegritySecret(createdSecret);
    return createdSecret;
}

void WhatSonTrialClientIdentityStore::storeIdentity(const WhatSonTrialClientIdentity& identity) const
{
    const WhatSonTrialClientIdentity normalizedIdentity = normalizeIdentity(identity);

    storeMirroredTextValue(m_deviceUuidSettingsKey, normalizedIdentity.deviceUuid, m_secureStore);
    storeMirroredTextValue(m_clientKeySettingsKey, normalizedIdentity.key, m_secureStore);
}

void WhatSonTrialClientIdentityStore::clear() const
{
    storeMirroredTextValue(m_deviceUuidSettingsKey, {}, m_secureStore);
    storeMirroredTextValue(m_clientKeySettingsKey, {}, m_secureStore);
    storeMirroredTextValue(m_registerIntegritySecretSettingsKey, {}, m_secureStore);
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

QString WhatSonTrialClientIdentityStore::createRegisterIntegritySecret()
{
    return createHexKey(kTrialRegisterIntegritySecretLength);
}

void WhatSonTrialClientIdentityStore::storeRegisterIntegritySecret(const QString& value) const
{
    storeMirroredTextValue(
        m_registerIntegritySecretSettingsKey,
        normalizeRegisterIntegritySecret(value),
        m_secureStore);
}
