#include "WhatSonWorkspaceHubCreator.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtCore/qconfig.h>

#if QT_CONFIG(process)
#include <QProcess>
#endif

#include <utility>

WhatSonWorkspaceHubCreator::WhatSonWorkspaceHubCreator(
    QString workspaceRootPath,
    QString hubsRootPath)
    : WhatSonHubCreator(std::move(workspaceRootPath)),
      m_hubsRootPath(std::move(hubsRootPath))
{
}

WhatSonWorkspaceHubCreator::~WhatSonWorkspaceHubCreator() = default;

void WhatSonWorkspaceHubCreator::setHubsRootPath(QString hubsRootPath)
{
    m_hubsRootPath = std::move(hubsRootPath);
}

const QString& WhatSonWorkspaceHubCreator::hubsRootPath() const noexcept
{
    return m_hubsRootPath;
}

QString WhatSonWorkspaceHubCreator::creatorName() const
{
    return QStringLiteral("WhatSonWorkspaceHubCreator");
}

QStringList WhatSonWorkspaceHubCreator::requiredRelativePaths() const
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

bool WhatSonWorkspaceHubCreator::createHub(
    const QString& hubName,
    QString* outPackagePath,
    QString* errorMessage) const
{
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
        return false;
    }

    const QString packagePath = hubRootPath + packageExtension();
    if (!packageHubDirectory(hubRootPath, packagePath, errorMessage))
    {
        return false;
    }

    if (outPackagePath != nullptr)
    {
        *outPackagePath = packagePath;
    }
    return true;
}

QString WhatSonWorkspaceHubCreator::packageExtension() const
{
    return QStringLiteral(".wshub");
}

QString WhatSonWorkspaceHubCreator::manifestFileName() const
{
    return QStringLiteral("hub.json");
}

QString WhatSonWorkspaceHubCreator::hubDirectoryPath(const QString& hubName) const
{
    const QString basePath = joinPath(workspaceRootPath(), m_hubsRootPath);
    return joinPath(basePath, sanitizeHubName(hubName));
}

bool WhatSonWorkspaceHubCreator::createHubScaffold(
    const QString& hubRootPath,
    QString* errorMessage) const
{
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

    return writeTextFile(manifestPath, manifestText, errorMessage);
}

bool WhatSonWorkspaceHubCreator::packageHubDirectory(
    const QString& hubRootPath,
    const QString& packagePath,
    QString* errorMessage) const
{
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

    return true;
#endif
}
