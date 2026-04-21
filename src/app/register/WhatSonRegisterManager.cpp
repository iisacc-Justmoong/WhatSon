#include "app/register/WhatSonRegisterManager.hpp"

#if defined(WHATSON_IS_TRIAL_BUILD)
#include "extension/trial/WhatSonTrialClientIdentityStore.hpp"
#endif

#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSettings>

#include <utility>

#if defined(WHATSON_IS_TRIAL_BUILD)
namespace
{
    constexpr auto kAuthenticationSchema = "whatson.register.authentication";

    QByteArray createHmacSha256Hex(QByteArray key, const QByteArray& message)
    {
        constexpr int blockSize = 64;
        if (key.size() > blockSize)
        {
            key = QCryptographicHash::hash(key, QCryptographicHash::Sha256);
        }
        if (key.size() < blockSize)
        {
            key = key.leftJustified(blockSize, '\0', true);
        }

        QByteArray innerPad(blockSize, char(0x36));
        QByteArray outerPad(blockSize, char(0x5c));
        for (int index = 0; index < blockSize; ++index)
        {
            innerPad[index] = char(innerPad[index] ^ key[index]);
            outerPad[index] = char(outerPad[index] ^ key[index]);
        }

        const QByteArray innerHash =
            QCryptographicHash::hash(innerPad + message, QCryptographicHash::Sha256);
        return QCryptographicHash::hash(outerPad + innerHash, QCryptographicHash::Sha256).toHex();
    }

    QString createCanonicalAuthenticationPayload(const QString& settingsKey)
    {
        return QStringLiteral("whatson.register.auth.v1\n%1\ntrue").arg(settingsKey.trimmed());
    }

    QString createAuthenticationSignature(const QString& settingsKey, const QString& signingSecret)
    {
        if (settingsKey.trimmed().isEmpty() || signingSecret.isEmpty())
        {
            return {};
        }

        return QString::fromLatin1(
            createHmacSha256Hex(
                signingSecret.toUtf8(),
                createCanonicalAuthenticationPayload(settingsKey).toUtf8()));
    }

    QString createSignedAuthenticationRecord(const QString& settingsKey, const QString& signingSecret)
    {
        const QString signature = createAuthenticationSignature(settingsKey, signingSecret);
        if (signature.isEmpty())
        {
            return {};
        }

        QJsonObject root;
        root.insert(QStringLiteral("version"), 1);
        root.insert(QStringLiteral("schema"), QString::fromLatin1(kAuthenticationSchema));
        root.insert(QStringLiteral("settingsKey"), settingsKey.trimmed());
        root.insert(QStringLiteral("authenticated"), true);
        root.insert(QStringLiteral("signature"), signature);
        return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));
    }

    bool loadSignedAuthenticationState(const QString& settingsKey)
    {
        QSettings settings;
        const QVariant rawValue = settings.value(settingsKey);
        if (!rawValue.isValid())
        {
            return false;
        }

        const QString rawText = rawValue.toString().trimmed();
        if (rawText.isEmpty())
        {
            if (settings.contains(settingsKey))
            {
                settings.remove(settingsKey);
                settings.sync();
            }
            return false;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(rawText.toUtf8(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject())
        {
            settings.remove(settingsKey);
            settings.sync();
            return false;
        }

        const QJsonObject root = document.object();
        if (root.value(QStringLiteral("schema")).toString().trimmed() != QString::fromLatin1(kAuthenticationSchema)
            || root.value(QStringLiteral("version")).toInt() != 1
            || root.value(QStringLiteral("settingsKey")).toString().trimmed() != settingsKey.trimmed()
            || !root.value(QStringLiteral("authenticated")).toBool(false))
        {
            settings.remove(settingsKey);
            settings.sync();
            return false;
        }

        const QString signingSecret = WhatSonTrialClientIdentityStore().loadRegisterIntegritySecret();
        if (signingSecret.isEmpty())
        {
            return false;
        }

        const QString expectedSignature = createAuthenticationSignature(settingsKey, signingSecret);
        const QString persistedSignature = root.value(QStringLiteral("signature")).toString().trimmed().toLower();
        if (persistedSignature != expectedSignature)
        {
            settings.remove(settingsKey);
            settings.sync();
            return false;
        }

        return true;
    }

    void clearPersistedAuthenticationState(const QString& settingsKey)
    {
        QSettings settings;
        if (!settings.contains(settingsKey))
        {
            return;
        }

        settings.remove(settingsKey);
        settings.sync();
    }
} // namespace
#endif

WhatSonRegisterManager::WhatSonRegisterManager(QString authenticatedSettingsKey, QObject* parent)
    : QObject(parent)
    , m_authenticatedSettingsKey(std::move(authenticatedSettingsKey))
{
    reload();
}

QString WhatSonRegisterManager::defaultAuthenticatedSettingsKey()
{
    return QStringLiteral("register/authenticated");
}

QString WhatSonRegisterManager::authenticatedSettingsKey() const
{
    return m_authenticatedSettingsKey;
}

bool WhatSonRegisterManager::authenticated() const noexcept
{
    return m_authenticated;
}

void WhatSonRegisterManager::reload()
{
#if defined(WHATSON_IS_TRIAL_BUILD)
    setAuthenticatedState(loadSignedAuthenticationState(m_authenticatedSettingsKey), true);
#else
    QSettings settings;
    setAuthenticatedState(settings.value(m_authenticatedSettingsKey, false).toBool(), true);
#endif
}

void WhatSonRegisterManager::setAuthenticated(const bool authenticated)
{
#if defined(WHATSON_IS_TRIAL_BUILD)
    if (!authenticated)
    {
        clearPersistedAuthenticationState(m_authenticatedSettingsKey);
        setAuthenticatedState(false, true);
        return;
    }

    WhatSonTrialClientIdentityStore clientIdentityStore;
    const QString signingSecret = clientIdentityStore.ensureRegisterIntegritySecret();
    const QString signedRecord = createSignedAuthenticationRecord(m_authenticatedSettingsKey, signingSecret);
    if (signedRecord.isEmpty())
    {
        clearPersistedAuthenticationState(m_authenticatedSettingsKey);
        setAuthenticatedState(false, true);
        return;
    }

    QSettings settings;
    settings.setValue(m_authenticatedSettingsKey, signedRecord);
    settings.sync();
    setAuthenticatedState(true, true);
#else
    QSettings settings;
    settings.setValue(m_authenticatedSettingsKey, authenticated);
    settings.sync();
    setAuthenticatedState(authenticated, true);
#endif
}

void WhatSonRegisterManager::clearAuthentication()
{
    setAuthenticated(false);
}

void WhatSonRegisterManager::setAuthenticatedState(const bool authenticated, const bool emitSignal)
{
    if (m_authenticated == authenticated)
    {
        return;
    }

    m_authenticated = authenticated;
    if (emitSignal)
    {
        emit authenticatedChanged();
    }
}
