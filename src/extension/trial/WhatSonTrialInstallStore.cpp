#include "extension/trial/WhatSonTrialInstallStore.hpp"

#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSettings>
#include <QVariant>

#include <utility>

namespace
{
    constexpr auto kInstallDateSchema = "whatson.trial.installDate";

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

    QString createCanonicalInstallDatePayload(const QString& settingsKey, const QString& dateText)
    {
        return QStringLiteral("whatson.trial.installDate.v1\n%1\n%2")
            .arg(settingsKey.trimmed(), dateText.trimmed());
    }

    QString createInstallDateSignature(
        const QString& settingsKey,
        const QString& dateText,
        const QString& signingSecret)
    {
        if (settingsKey.trimmed().isEmpty() || dateText.trimmed().isEmpty() || signingSecret.isEmpty())
        {
            return {};
        }

        return QString::fromLatin1(
            createHmacSha256Hex(
                signingSecret.toUtf8(),
                createCanonicalInstallDatePayload(settingsKey, dateText).toUtf8()));
    }

    QString createSignedInstallDateRecord(
        const QString& settingsKey,
        const QDate& installDate,
        const QString& signingSecret)
    {
        if (!installDate.isValid())
        {
            return {};
        }

        const QString dateText = installDate.toString(Qt::ISODate);
        const QString signature = createInstallDateSignature(settingsKey, dateText, signingSecret);
        if (signature.isEmpty())
        {
            return {};
        }

        QJsonObject root;
        root.insert(QStringLiteral("version"), 1);
        root.insert(QStringLiteral("schema"), QString::fromLatin1(kInstallDateSchema));
        root.insert(QStringLiteral("settingsKey"), settingsKey.trimmed());
        root.insert(QStringLiteral("installDate"), dateText);
        root.insert(QStringLiteral("signature"), signature);
        return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));
    }

    QDate loadSignedInstallDate(
        const QString& settingsKey,
        const QString& rawValue,
        const QString& signingSecret)
    {
        const QString trimmedValue = rawValue.trimmed();
        if (trimmedValue.isEmpty() || signingSecret.isEmpty())
        {
            return {};
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(trimmedValue.toUtf8(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject())
        {
            return {};
        }

        const QJsonObject root = document.object();
        if (root.value(QStringLiteral("schema")).toString().trimmed() != QString::fromLatin1(kInstallDateSchema)
            || root.value(QStringLiteral("version")).toInt() != 1
            || root.value(QStringLiteral("settingsKey")).toString().trimmed() != settingsKey.trimmed())
        {
            return {};
        }

        const QString dateText = root.value(QStringLiteral("installDate")).toString().trimmed();
        const QDate installDate = QDate::fromString(dateText, Qt::ISODate);
        const QString expectedSignature = createInstallDateSignature(settingsKey, dateText, signingSecret);
        const QString persistedSignature = root.value(QStringLiteral("signature")).toString().trimmed().toLower();
        if (!installDate.isValid() || expectedSignature.isEmpty() || persistedSignature != expectedSignature)
        {
            return {};
        }

        return installDate;
    }

    bool isSignedInstallDateRecord(const QString& settingsKey, const QString& rawValue)
    {
        const QString trimmedValue = rawValue.trimmed();
        if (trimmedValue.isEmpty())
        {
            return false;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(trimmedValue.toUtf8(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject())
        {
            return false;
        }

        const QJsonObject root = document.object();
        return root.value(QStringLiteral("schema")).toString().trimmed() == QString::fromLatin1(kInstallDateSchema)
            && root.value(QStringLiteral("version")).toInt() == 1
            && root.value(QStringLiteral("settingsKey")).toString().trimmed() == settingsKey.trimmed();
    }

    QString loadLegacySecureTextValue(
        const QString& settingsKey,
        const WhatSonTrialSecureStore& secureStore)
    {
        const WhatSonTrialSecureStoreReadResult secureResult = secureStore.readEntry(settingsKey);
        if (secureResult.status != WhatSonTrialSecureStoreStatus::Success)
        {
            return {};
        }

        const QString normalizedSecureValue = normalizeStoredDate(secureResult.value).toString(Qt::ISODate);
        if (!normalizedSecureValue.isEmpty())
        {
            return normalizedSecureValue;
        }

        (void)secureStore.removeEntry(settingsKey);
        return {};
    }
}

WhatSonTrialInstallStore::WhatSonTrialInstallStore(
    QString installDateSettingsKey,
    WhatSonTrialClientIdentityStore clientIdentityStore,
    WhatSonTrialSecureStore secureStore)
    : m_installDateSettingsKey(std::move(installDateSettingsKey))
    , m_clientIdentityStore(std::move(clientIdentityStore))
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
    QSettings settings;
    const QString rawSettingsValue = settings.value(m_installDateSettingsKey).toString().trimmed();
    const QString signingSecret = m_clientIdentityStore.loadRegisterIntegritySecret();
    const bool hasSignedInstallDateRecord =
        isSignedInstallDateRecord(m_installDateSettingsKey, rawSettingsValue);

    const QDate signedInstallDate = loadSignedInstallDate(
        m_installDateSettingsKey,
        rawSettingsValue,
        signingSecret);
    if (signedInstallDate.isValid())
    {
        return signedInstallDate;
    }

    const QDate legacySettingsDate = normalizeStoredDate(rawSettingsValue);
    if (legacySettingsDate.isValid() && !signingSecret.isEmpty())
    {
        settings.setValue(
            m_installDateSettingsKey,
            createSignedInstallDateRecord(m_installDateSettingsKey, legacySettingsDate, signingSecret));
        settings.sync();
        (void)m_secureStore.removeEntry(m_installDateSettingsKey);
        return legacySettingsDate;
    }

    const QString legacySecureText = loadLegacySecureTextValue(m_installDateSettingsKey, m_secureStore);
    const QDate legacySecureDate = QDate::fromString(legacySecureText, Qt::ISODate);
    if (legacySecureDate.isValid() && !signingSecret.isEmpty())
    {
        settings.setValue(
            m_installDateSettingsKey,
            createSignedInstallDateRecord(m_installDateSettingsKey, legacySecureDate, signingSecret));
        settings.sync();
        (void)m_secureStore.removeEntry(m_installDateSettingsKey);
        return legacySecureDate;
    }

    if (!hasSignedInstallDateRecord && (!rawSettingsValue.isEmpty() || settings.contains(m_installDateSettingsKey)))
    {
        settings.remove(m_installDateSettingsKey);
        settings.sync();
    }

    return {};
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
    return loadInstallDate();
}

void WhatSonTrialInstallStore::storeInstallDate(const QDate& installDate) const
{
    QSettings settings;
    if (!installDate.isValid())
    {
        settings.remove(m_installDateSettingsKey);
        settings.sync();
        (void)m_secureStore.removeEntry(m_installDateSettingsKey);
        return;
    }

    const QString signingSecret = m_clientIdentityStore.ensureRegisterIntegritySecret();
    const QString signedRecord = createSignedInstallDateRecord(
        m_installDateSettingsKey,
        installDate,
        signingSecret);
    if (signedRecord.isEmpty())
    {
        settings.remove(m_installDateSettingsKey);
        settings.sync();
        return;
    }

    settings.setValue(m_installDateSettingsKey, signedRecord);
    settings.sync();
    (void)m_secureStore.removeEntry(m_installDateSettingsKey);
}

void WhatSonTrialInstallStore::clear() const
{
    QSettings settings;
    settings.remove(m_installDateSettingsKey);
    settings.sync();
    (void)m_secureStore.removeEntry(m_installDateSettingsKey);
}
