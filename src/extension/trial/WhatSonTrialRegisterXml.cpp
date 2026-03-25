#include "WhatSonTrialRegisterXml.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QUrl>
#include <QXmlStreamReader>

namespace
{
    QString escapeXmlText(QString value)
    {
        value.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
        value.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
        value.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
        value.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
        value.replace(QStringLiteral("'"), QStringLiteral("&apos;"));
        return value;
    }

    QString createRegisterXmlText(const WhatSonTrialClientIdentity& identity)
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
        text += QStringLiteral("</trialRegister>\n");
        return text;
    }
}

QString WhatSonTrialRegisterXml::registerFileName()
{
    return QStringLiteral("trial_register.xml");
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

WhatSonTrialClientIdentity WhatSonTrialRegisterXml::loadRegister(
    const QString& hubRootPath,
    QString* errorMessage) const
{
    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }

    const QString path = registerFilePath(hubRootPath);
    if (path.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Trial hub root path is invalid: %1").arg(hubRootPath);
        }
        return {};
    }

    QFile file(path);
    if (!file.exists())
    {
        return {};
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to open trial register file: %1").arg(path);
        }
        return {};
    }

    QXmlStreamReader reader(file.readAll());
    WhatSonTrialClientIdentity identity;
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
    }

    if (reader.hasError())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Trial register XML is malformed: %1").arg(path);
        }
        return {};
    }

    const WhatSonTrialClientIdentity normalizedIdentity =
        WhatSonTrialClientIdentityStore::normalizeIdentity(identity);
    if (normalizedIdentity.key.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Trial register file is missing a valid key: %1").arg(path);
        }
        return {};
    }

    return normalizedIdentity;
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

    const QByteArray bytes = createRegisterXmlText(normalizedIdentity).toUtf8();
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
