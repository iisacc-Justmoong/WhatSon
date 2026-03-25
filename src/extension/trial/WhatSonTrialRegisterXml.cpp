#include "WhatSonTrialRegisterXml.hpp"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QUrl>
#include <QXmlStreamReader>

namespace
{
    QString createCanonicalRegisterPayload(const WhatSonTrialClientIdentity& identity)
    {
        return QStringLiteral("trial-register-v1\n%1\n%2")
            .arg(identity.deviceUuid, identity.key);
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

    QString createIntegritySignature(
        const WhatSonTrialClientIdentity& identity,
        const QString& registerIntegritySecret)
    {
        if (registerIntegritySecret.isEmpty())
        {
            return {};
        }

        return QString::fromLatin1(
            createHmacSha256Hex(
                registerIntegritySecret.toUtf8(),
                createCanonicalRegisterPayload(identity).toUtf8()));
    }

    QString escapeXmlText(QString value)
    {
        value.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
        value.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
        value.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
        value.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
        value.replace(QStringLiteral("'"), QStringLiteral("&apos;"));
        return value;
    }

    QString createRegisterXmlText(const WhatSonTrialClientIdentity& identity, const QString& signature)
    {
        QString text;
        text += QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        text += QStringLiteral("<trialRegister>\n");
        text += QStringLiteral("  <deviceUUID>")
            + escapeXmlText(identity.deviceUuid)
            + QStringLiteral("</deviceUUID>\n");
        text += QStringLiteral("  <key>")
            + escapeXmlText(identity.key)
            + QStringLiteral("</key>\n");
        text += QStringLiteral("  <signature algorithm=\"")
            + escapeXmlText(WhatSonTrialRegisterXml::signatureAlgorithmName())
            + QStringLiteral("\">")
            + escapeXmlText(signature)
            + QStringLiteral("</signature>\n");
        text += QStringLiteral("</trialRegister>\n");
        return text;
    }
}

WhatSonTrialRegisterXml::WhatSonTrialRegisterXml(WhatSonTrialClientIdentityStore clientIdentityStore)
    : m_clientIdentityStore(std::move(clientIdentityStore))
{
}

QString WhatSonTrialRegisterXml::registerFileName()
{
    return QStringLiteral("trial_register.xml");
}

QString WhatSonTrialRegisterXml::signatureAlgorithmName()
{
    return QStringLiteral("HMAC-SHA256");
}

QString WhatSonTrialRegisterXml::registerFilePath(const QString& hubRootPath) const
{
    const QString normalizedHubRootPath = normalizeHubRootPath(hubRootPath);
    if (normalizedHubRootPath.isEmpty())
    {
        return {};
    }

    return QDir(normalizedHubRootPath).filePath(QStringLiteral(".whatson/%1").arg(registerFileName()));
}

bool WhatSonTrialRegisterXml::exists(const QString& hubRootPath) const
{
    const QString path = registerFilePath(hubRootPath);
    return !path.isEmpty() && QFileInfo::exists(path);
}

WhatSonTrialRegisterLoadResult WhatSonTrialRegisterXml::loadRegister(const QString& hubRootPath) const
{
    const QString path = registerFilePath(hubRootPath);
    if (path.isEmpty())
    {
        return {
            {},
            QStringLiteral("Trial hub root path is invalid: %1").arg(hubRootPath),
            false
        };
    }

    QFile file(path);
    if (!file.exists())
    {
        return {};
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return {
            {},
            QStringLiteral("Failed to open trial register file: %1").arg(path),
            false
        };
    }

    QXmlStreamReader reader(file.readAll());
    WhatSonTrialClientIdentity identity;
    QString signature;
    QString signatureAlgorithm;
    while (!reader.atEnd())
    {
        reader.readNext();
        if (!reader.isStartElement())
        {
            continue;
        }

        if (reader.name() == QStringLiteral("deviceUUID"))
        {
            identity.deviceUuid = reader.readElementText().trimmed();
        }
        else if (reader.name() == QStringLiteral("key"))
        {
            identity.key = reader.readElementText().trimmed();
        }
        else if (reader.name() == QStringLiteral("signature"))
        {
            signatureAlgorithm = reader.attributes().value(QStringLiteral("algorithm")).toString().trimmed();
            signature = reader.readElementText().trimmed().toLower();
        }
    }

    if (reader.hasError())
    {
        return {
            {},
            QStringLiteral("Trial register XML is malformed: %1").arg(path),
            false
        };
    }

    const WhatSonTrialClientIdentity normalizedIdentity =
        WhatSonTrialClientIdentityStore::normalizeIdentity(identity);
    if (normalizedIdentity.key.isEmpty())
    {
        return {
            {},
            QStringLiteral("Trial register file is missing a valid key: %1").arg(path),
            false
        };
    }

    if (signatureAlgorithm != signatureAlgorithmName())
    {
        return {
            {},
            QStringLiteral("Trial register XML uses an unsupported signature algorithm: %1").arg(path),
            false
        };
    }

    if (signature.isEmpty())
    {
        return {
            {},
            QStringLiteral("Trial register XML is missing an integrity signature: %1").arg(path),
            false
        };
    }

    const QString registerIntegritySecret = m_clientIdentityStore.loadRegisterIntegritySecret();
    if (registerIntegritySecret.isEmpty())
    {
        return {
            {},
            QStringLiteral("Trial register integrity secret is unavailable for verification: %1").arg(path),
            false
        };
    }

    const QString expectedSignature = createIntegritySignature(normalizedIdentity, registerIntegritySecret);
    if (expectedSignature.isEmpty() || expectedSignature != signature)
    {
        return {
            {},
            QStringLiteral("Trial register XML integrity verification failed: %1").arg(path),
            false
        };
    }

    return {
        normalizedIdentity,
        {},
        true
    };
}

bool WhatSonTrialRegisterXml::writeRegister(
    const QString& hubRootPath,
    const WhatSonTrialClientIdentity& identity,
    QString* errorMessage) const
{
    const WhatSonTrialClientIdentity normalizedIdentity =
        WhatSonTrialClientIdentityStore::normalizeIdentity(identity);
    if (normalizedIdentity.deviceUuid.isEmpty() || normalizedIdentity.key.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Trial register identity is invalid.");
        }
        return false;
    }

    const QString registerIntegritySecret = m_clientIdentityStore.ensureRegisterIntegritySecret();
    const QString signature = createIntegritySignature(normalizedIdentity, registerIntegritySecret);
    if (signature.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to create the trial register integrity signature.");
        }
        return false;
    }

    const QString path = registerFilePath(hubRootPath);
    if (path.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Trial hub root path is invalid: %1").arg(hubRootPath);
        }
        return false;
    }

    const QString directoryPath = QFileInfo(path).absolutePath();
    if (!directoryPath.isEmpty() && !QDir().mkpath(directoryPath))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to create trial register directory: %1").arg(directoryPath);
        }
        return false;
    }

    const QByteArray bytes = createRegisterXmlText(normalizedIdentity, signature).toUtf8();
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to open trial register file for writing: %1").arg(path);
        }
        return false;
    }

    if (file.write(bytes) != bytes.size())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to write trial register file: %1").arg(path);
        }
        return false;
    }

    if (!file.commit())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to commit trial register file: %1").arg(path);
        }
        return false;
    }

    return true;
}

QString WhatSonTrialRegisterXml::normalizeHubRootPath(const QString& hubRootPath)
{
    const QString trimmedHubRootPath = hubRootPath.trimmed();
    if (trimmedHubRootPath.isEmpty())
    {
        return {};
    }

    const QUrl hubRootUrl(trimmedHubRootPath);
    if (hubRootUrl.isValid())
    {
        if (hubRootUrl.isLocalFile())
        {
            return QDir::cleanPath(hubRootUrl.toLocalFile());
        }

        if (!hubRootUrl.scheme().isEmpty())
        {
            return {};
        }
    }

    return QDir::cleanPath(trimmedHubRootPath);
}
