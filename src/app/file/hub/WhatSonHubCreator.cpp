#include "WhatSonHubCreator.hpp"

#include "WhatSonDebugTrace.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSaveFile>
#include <QtCore/qconfig.h>

#if QT_CONFIG(process)
#include <QProcess>
#endif

#include <utility>

WhatSonHubCreator::WhatSonHubCreator(
    QString workspaceRootPath,
    QString hubsRootPath)
    : m_workspaceRootPath(std::move(workspaceRootPath))
      , m_hubsRootPath(std::move(hubsRootPath))
{
}

WhatSonHubCreator::~WhatSonHubCreator() = default;

void WhatSonHubCreator::setWorkspaceRootPath(QString workspaceRootPath)
{
    WhatSon::Debug::trace(
        QStringLiteral("hub.creator"),
        QStringLiteral("setWorkspaceRootPath"),
        QStringLiteral("path=%1").arg(workspaceRootPath));
    m_workspaceRootPath = std::move(workspaceRootPath);
}

const QString& WhatSonHubCreator::workspaceRootPath() const noexcept
{
    return m_workspaceRootPath;
}

void WhatSonHubCreator::setHubsRootPath(QString hubsRootPath)
{
    WhatSon::Debug::trace(
        QStringLiteral("hub.creator"),
        QStringLiteral("setHubsRootPath"),
        QStringLiteral("path=%1").arg(hubsRootPath));
    m_hubsRootPath = std::move(hubsRootPath);
}

const QString& WhatSonHubCreator::hubsRootPath() const noexcept
{
    return m_hubsRootPath;
}

QString WhatSonHubCreator::creatorName() const
{
    return QStringLiteral("WhatSonHubCreator");
}

QStringList WhatSonHubCreator::requiredRelativePaths() const
{
    return {
        QStringLiteral(".whatson"),
        QStringLiteral("notes"),
        QStringLiteral("notes/drafts"),
        QStringLiteral("attachments"),
        QStringLiteral("assets"),
        QStringLiteral("indexes")
    };
}

bool WhatSonHubCreator::createHub(
    const QString& hubName,
    QString* outPackagePath,
    QString* errorMessage) const
{
    WhatSon::Debug::trace(
        QStringLiteral("hub.creator"),
        QStringLiteral("createHub.begin"),
        QStringLiteral("hubName=%1 workspace=%2 hubsRoot=%3")
        .arg(hubName, workspaceRootPath(), m_hubsRootPath));
    if (workspaceRootPath().trimmed().isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Workspace root path must not be empty.");
        }
        return false;
    }

    const QString hubsRootAbsolutePath = joinPath(workspaceRootPath(), m_hubsRootPath);
    if (!ensureDirectory(hubsRootAbsolutePath, errorMessage))
    {
        return false;
    }

    const QString hubRootPath = hubDirectoryPath(hubName);
    QDir hubRootDir(hubRootPath);
    if (hubRootDir.exists())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Hub already exists: %1").arg(hubRootPath);
        }
        return false;
    }

    if (!ensureDirectory(hubRootPath, errorMessage))
    {
        return false;
    }

    if (!createHubScaffold(hubRootPath, errorMessage))
    {
        WhatSon::Debug::trace(
            QStringLiteral("hub.creator"),
            QStringLiteral("createHub.failed.scaffold"),
            errorMessage != nullptr ? *errorMessage : QString());
        return false;
    }

    const QString packagePath = hubRootPath + packageExtension();
    if (!packageHubDirectory(hubRootPath, packagePath, errorMessage))
    {
        WhatSon::Debug::trace(
            QStringLiteral("hub.creator"),
            QStringLiteral("createHub.failed.package"),
            errorMessage != nullptr ? *errorMessage : QString());
        return false;
    }

    if (outPackagePath != nullptr)
    {
        *outPackagePath = packagePath;
    }
    WhatSon::Debug::trace(
        QStringLiteral("hub.creator"),
        QStringLiteral("createHub.success"),
        QStringLiteral("hubRoot=%1 package=%2").arg(hubRootPath, packagePath));
    return true;
}

QString WhatSonHubCreator::packageExtension() const
{
    return QStringLiteral(".wshub");
}

QString WhatSonHubCreator::manifestFileName() const
{
    return QStringLiteral("hub.json");
}

QString WhatSonHubCreator::sanitizeHubName(const QString& hubName) const
{
    QString normalized = hubName.trimmed().toLower();
    normalized.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral("-"));
    normalized.replace(QRegularExpression(QStringLiteral("[^a-z0-9._-]")), QStringLiteral(""));

    if (normalized.isEmpty())
    {
        WhatSon::Debug::trace(
            QStringLiteral("hub.creator"),
            QStringLiteral("sanitizeHubName.fallback"),
            QStringLiteral("input=%1").arg(hubName));
        return QStringLiteral("untitled-hub");
    }

    WhatSon::Debug::trace(
        QStringLiteral("hub.creator"),
        QStringLiteral("sanitizeHubName.success"),
        QStringLiteral("input=%1 output=%2").arg(hubName, normalized));
    return normalized;
}

QString WhatSonHubCreator::joinPath(const QString& left, const QString& right) const
{
    if (left.isEmpty())
    {
        return QDir::cleanPath(right);
    }
    if (right.isEmpty())
    {
        return QDir::cleanPath(left);
    }

    return QDir::cleanPath(left + QLatin1Char('/') + right);
}

bool WhatSonHubCreator::ensureDirectory(const QString& absolutePath, QString* errorMessage) const
{
    WhatSon::Debug::trace(
        QStringLiteral("hub.creator"),
        QStringLiteral("ensureDirectory.begin"),
        QStringLiteral("path=%1").arg(absolutePath));
    QDir dir;
    if (dir.exists(absolutePath))
    {
        return true;
    }

    if (dir.mkpath(absolutePath))
    {
        WhatSon::Debug::trace(
            QStringLiteral("hub.creator"),
            QStringLiteral("ensureDirectory.created"),
            QStringLiteral("path=%1").arg(absolutePath));
        return true;
    }

    if (errorMessage != nullptr)
    {
        *errorMessage = QStringLiteral("Failed to create directory: %1").arg(absolutePath);
    }
    return false;
}

bool WhatSonHubCreator::writeTextFile(
    const QString& absolutePath,
    const QString& content,
    QString* errorMessage) const
{
    WhatSon::Debug::trace(
        QStringLiteral("hub.creator"),
        QStringLiteral("writeTextFile.begin"),
        QStringLiteral("path=%1 bytes=%2").arg(absolutePath).arg(content.toUtf8().size()));
    QSaveFile file(absolutePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to open file for writing: %1").arg(absolutePath);
        }
        return false;
    }

    const QByteArray bytes = content.toUtf8();
    if (file.write(bytes) != bytes.size())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to write file: %1").arg(absolutePath);
        }
        return false;
    }

    if (!file.commit())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to commit file: %1").arg(absolutePath);
        }
        return false;
    }

    WhatSon::Debug::trace(
        QStringLiteral("hub.creator"),
        QStringLiteral("writeTextFile.success"),
        QStringLiteral("path=%1").arg(absolutePath));
    return true;
}

QString WhatSonHubCreator::hubDirectoryPath(const QString& hubName) const
{
    const QString basePath = joinPath(workspaceRootPath(), m_hubsRootPath);
    return joinPath(basePath, sanitizeHubName(hubName));
}

bool WhatSonHubCreator::createHubScaffold(
    const QString& hubRootPath,
    QString* errorMessage) const
{
    WhatSon::Debug::trace(
        QStringLiteral("hub.creator"),
        QStringLiteral("createScaffold.begin"),
        QStringLiteral("hubRoot=%1").arg(hubRootPath));
    const QStringList paths = requiredRelativePaths();
    for (const QString& relativePath : paths)
    {
        const QString absolutePath = joinPath(hubRootPath, relativePath);
        if (!ensureDirectory(absolutePath, errorMessage))
        {
            return false;
        }
    }

    const QString manifestPath = joinPath(
        joinPath(hubRootPath, QStringLiteral(".whatson")),
        manifestFileName());

    QJsonObject manifest;
    manifest.insert(QStringLiteral("format"), QStringLiteral("wshub"));
    manifest.insert(QStringLiteral("version"), 1);
    manifest.insert(QStringLiteral("creator"), creatorName());
    manifest.insert(QStringLiteral("storage"), QStringLiteral("filesystem"));
    manifest.insert(QStringLiteral("notesRoot"), QStringLiteral("notes"));
    manifest.insert(QStringLiteral("createdAtUtc"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    manifest.insert(QStringLiteral("hubDirectory"), QFileInfo(hubRootPath).fileName());

    const QString manifestText = QString::fromUtf8(
        QJsonDocument(manifest).toJson(QJsonDocument::Indented));

    const bool ok = writeTextFile(manifestPath, manifestText, errorMessage);
    WhatSon::Debug::trace(
        QStringLiteral("hub.creator"),
        ok ? QStringLiteral("createScaffold.success") : QStringLiteral("createScaffold.failed"),
        ok
            ? QStringLiteral("manifest=%1").arg(manifestPath)
            : (errorMessage != nullptr ? *errorMessage : QString()));
    return ok;
}

bool WhatSonHubCreator::packageHubDirectory(
    const QString& hubRootPath,
    const QString& packagePath,
    QString* errorMessage) const
{
    WhatSon::Debug::trace(
        QStringLiteral("hub.creator"),
        QStringLiteral("package.begin"),
        QStringLiteral("hubRoot=%1 package=%2").arg(hubRootPath, packagePath));
#if !QT_CONFIG(process)
    Q_UNUSED(hubRootPath);
    Q_UNUSED(packagePath);
    if (errorMessage != nullptr)
    {
        *errorMessage = QStringLiteral(
            "Packaging is unavailable on this platform because Qt process support is disabled.");
    }
    return false;
#else
    QFile::remove(packagePath);

    const QFileInfo hubInfo(hubRootPath);
    const QString parentPath = hubInfo.absolutePath();
    const QString directoryName = hubInfo.fileName();

    QProcess process;
#if defined(Q_OS_MACOS)
    process.setProgram(QStringLiteral("/usr/bin/ditto"));
    process.setArguments({
        QStringLiteral("-c"),
        QStringLiteral("-k"),
        QStringLiteral("--sequesterRsrc"),
        QStringLiteral("--keepParent"),
        directoryName,
        packagePath
    });
#else
    process.setProgram(QStringLiteral("zip"));
    process.setArguments({
        QStringLiteral("-r"),
        packagePath,
        directoryName
    });
#endif
    process.setWorkingDirectory(parentPath);

    process.start();
    if (!process.waitForFinished(-1))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Packaging process timed out for: %1").arg(hubRootPath);
        }
        return false;
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
    {
        if (errorMessage != nullptr)
        {
            const QString stderrText = QString::fromLocal8Bit(process.readAllStandardError());
            *errorMessage = QStringLiteral("Packaging failed: %1").arg(stderrText.trimmed());
        }
        return false;
    }

    if (!QFileInfo::exists(packagePath))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Package file was not created: %1").arg(packagePath);
        }
        return false;
    }

    WhatSon::Debug::trace(
        QStringLiteral("hub.creator"),
        QStringLiteral("package.success"),
        QStringLiteral("package=%1").arg(packagePath));
    return true;
#endif
}
