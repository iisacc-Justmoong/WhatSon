#include "WhatSonHubCreator.hpp"

#include "WhatSonDebugTrace.hpp"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSaveFile>

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
    WhatSon::Debug::traceSelf(this,
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
    WhatSon::Debug::traceSelf(this,
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

QStringList WhatSonHubCreator::requiredRelativePaths(const QString& hubName) const
{
    const QString contentsDirectory = hubContentsDirectoryName(hubName);
    return {
        QStringLiteral(".whatson"),
        contentsDirectory,
        joinPath(contentsDirectory, QStringLiteral("Library.wslibrary")),
        joinPath(contentsDirectory, QStringLiteral("Preset.wspreset")),
        hubResourcesDirectoryName(hubName)
    };
}

bool WhatSonHubCreator::createHub(
    const QString& hubName,
    QString* outPackagePath,
    QString* errorMessage) const
{
    WhatSon::Debug::traceSelf(this,
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

    return createHubAtPath(hubDirectoryPath(hubName), outPackagePath, errorMessage);
}

bool WhatSonHubCreator::createHubAtPath(
    const QString& hubPackagePath,
    QString* outPackagePath,
    QString* errorMessage) const
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.creator"),
                              QStringLiteral("createHubAtPath.begin"),
                              QStringLiteral("requestedPath=%1").arg(hubPackagePath));
    const QString trimmedHubPackagePath = hubPackagePath.trimmed();
    if (trimmedHubPackagePath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Hub package path must not be empty.");
        }
        return false;
    }

    QString normalizedHubPackagePath = QDir::cleanPath(trimmedHubPackagePath);
    if (!normalizedHubPackagePath.endsWith(packageExtension(), Qt::CaseInsensitive))
    {
        normalizedHubPackagePath += packageExtension();
    }

    const QFileInfo requestedPackageInfo(normalizedHubPackagePath);
    const QString absoluteHubPackagePath = QDir::cleanPath(requestedPackageInfo.absoluteFilePath());
    const QString hubParentDirectoryPath = requestedPackageInfo.absolutePath();
    const QString rawHubName = QFileInfo(absoluteHubPackagePath).completeBaseName();
    const QString sanitizedHubName = sanitizeHubName(rawHubName);

    if (hubParentDirectoryPath.trimmed().isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Hub parent directory must not be empty.");
        }
        return false;
    }

    if (!ensureDirectory(hubParentDirectoryPath, errorMessage))
    {
        return false;
    }

    const QFileInfo absoluteHubPackageInfo(absoluteHubPackagePath);
    if (absoluteHubPackageInfo.exists())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Hub already exists: %1").arg(absoluteHubPackagePath);
        }
        return false;
    }

    if (!ensureDirectory(absoluteHubPackagePath, errorMessage))
    {
        return false;
    }

    if (!createHubScaffold(absoluteHubPackagePath, sanitizedHubName, errorMessage))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("hub.creator"),
                                  QStringLiteral("createHubAtPath.failed.scaffold"),
                                  errorMessage != nullptr ? *errorMessage : QString());
        return false;
    }

    if (outPackagePath != nullptr)
    {
        *outPackagePath = absoluteHubPackagePath;
    }
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.creator"),
                              QStringLiteral("createHubAtPath.success"),
                              QStringLiteral("hubRoot=%1 hubName=%2")
                              .arg(absoluteHubPackagePath, sanitizedHubName));
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

QString WhatSonHubCreator::hubContentsDirectoryName(const QString& hubName) const
{
    return sanitizeHubName(hubName) + QStringLiteral(".wscontents");
}

QString WhatSonHubCreator::hubResourcesDirectoryName(const QString& hubName) const
{
    return sanitizeHubName(hubName) + QStringLiteral(".wsresources");
}

QString WhatSonHubCreator::hubStatFileName(const QString& hubName) const
{
    return sanitizeHubName(hubName) + QStringLiteral("Stat.wsstat");
}

QString WhatSonHubCreator::sanitizeHubName(const QString& hubName) const
{
    QString normalized = hubName.trimmed().toLower();
    normalized.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral("-"));
    normalized.replace(QRegularExpression(QStringLiteral("[^a-z0-9._-]")), QStringLiteral(""));

    if (normalized.isEmpty())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("hub.creator"),
                                  QStringLiteral("sanitizeHubName.fallback"),
                                  QStringLiteral("input=%1").arg(hubName));
        return QStringLiteral("untitled-hub");
    }

    WhatSon::Debug::traceSelf(this,
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
    WhatSon::Debug::traceSelf(this,
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
        WhatSon::Debug::traceSelf(this,
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
    WhatSon::Debug::traceSelf(this,
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

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.creator"),
                              QStringLiteral("writeTextFile.success"),
                              QStringLiteral("path=%1").arg(absolutePath));
    return true;
}

QString WhatSonHubCreator::hubDirectoryPath(const QString& hubName) const
{
    const QString basePath = joinPath(workspaceRootPath(), m_hubsRootPath);
    return joinPath(basePath, sanitizeHubName(hubName) + packageExtension());
}

bool WhatSonHubCreator::createHubScaffold(
    const QString& hubRootPath,
    const QString& hubName,
    QString* errorMessage) const
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.creator"),
                              QStringLiteral("createScaffold.begin"),
                              QStringLiteral("hubRoot=%1").arg(hubRootPath));

    const QString sanitizedHubName = sanitizeHubName(hubName);
    const QString contentsDirectory = hubContentsDirectoryName(sanitizedHubName);
    const QString resourcesDirectory = hubResourcesDirectoryName(sanitizedHubName);
    const QString statFileName = hubStatFileName(sanitizedHubName);
    const QString libraryRoot = joinPath(contentsDirectory, QStringLiteral("Library.wslibrary"));

    const QStringList paths = requiredRelativePaths(sanitizedHubName);
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
    manifest.insert(QStringLiteral("hubName"), sanitizedHubName);
    manifest.insert(QStringLiteral("contentsRoot"), contentsDirectory);
    manifest.insert(QStringLiteral("notesRoot"), libraryRoot);
    manifest.insert(QStringLiteral("resourcesRoot"), resourcesDirectory);
    manifest.insert(QStringLiteral("statFile"), statFileName);
    const QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    manifest.insert(QStringLiteral("createdAtUtc"), now);
    manifest.insert(QStringLiteral("lastModifiedAtUtc"), now);
    manifest.insert(QStringLiteral("hubDirectory"), QFileInfo(hubRootPath).fileName());

    const QString manifestText = QString::fromUtf8(
        QJsonDocument(manifest).toJson(QJsonDocument::Indented));

    if (!writeTextFile(manifestPath, manifestText, errorMessage))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("hub.creator"),
                                  QStringLiteral("createScaffold.failed.manifest"),
                                  errorMessage != nullptr ? *errorMessage : QString());
        return false;
    }

    const QString statPath = joinPath(hubRootPath, statFileName);

    QJsonObject statRoot;
    statRoot.insert(QStringLiteral("version"), 1);
    statRoot.insert(QStringLiteral("schema"), QStringLiteral("whatson.hub.stat"));
    statRoot.insert(QStringLiteral("hub"), sanitizedHubName);
    statRoot.insert(QStringLiteral("noteCount"), 0);
    statRoot.insert(QStringLiteral("resourceCount"), 0);
    statRoot.insert(QStringLiteral("characterCount"), 0);
    statRoot.insert(QStringLiteral("createdAtUtc"), now);
    statRoot.insert(QStringLiteral("lastModifiedAtUtc"), now);
    statRoot.insert(QStringLiteral("participants"), QJsonArray{});
    statRoot.insert(QStringLiteral("profileLastModifiedAtUtc"), QJsonObject{});

    const QString statText = QString::fromUtf8(
        QJsonDocument(statRoot).toJson(QJsonDocument::Indented));
    if (!writeTextFile(statPath, statText, errorMessage))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("hub.creator"),
                                  QStringLiteral("createScaffold.failed.stat"),
                                  errorMessage != nullptr ? *errorMessage : QString());
        return false;
    }

    const QString indexPath = joinPath(hubRootPath, joinPath(libraryRoot, QStringLiteral("index.wsnindex")));
    const QString indexText = QStringLiteral(
        "{\n  \"version\": 1,\n  \"schema\": \"whatson.library.index\",\n  \"notes\": []\n}\n");
    if (!writeTextFile(indexPath, indexText, errorMessage))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("hub.creator"),
                                  QStringLiteral("createScaffold.failed.index"),
                                  errorMessage != nullptr ? *errorMessage : QString());
        return false;
    }

    const QString tagsPath = joinPath(hubRootPath, joinPath(contentsDirectory, QStringLiteral("Tags.wstags")));
    const QString tagsText = QStringLiteral(
        "{\n  \"version\": 1,\n  \"schema\": \"whatson.tags.depth\",\n  \"tags\": []\n}\n");
    if (!writeTextFile(tagsPath, tagsText, errorMessage))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("hub.creator"),
                                  QStringLiteral("createScaffold.failed.tags"),
                                  errorMessage != nullptr ? *errorMessage : QString());
        return false;
    }

    const QString foldersPath = joinPath(
        hubRootPath,
        joinPath(contentsDirectory, QStringLiteral("Folders.wsfolders")));
    const QString foldersText =
        QStringLiteral("{\n  \"version\": 1,\n  \"schema\": \"whatson.folders.tree\",\n  \"folders\": []\n}\n");
    if (!writeTextFile(foldersPath, foldersText, errorMessage))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("hub.creator"),
                                  QStringLiteral("createScaffold.failed.folders"),
                                  errorMessage != nullptr ? *errorMessage : QString());
        return false;
    }

    const QString bookmarksPath = joinPath(
        hubRootPath,
        joinPath(contentsDirectory, QStringLiteral("Bookmarks.wsbookmarks")));
    const QString bookmarksText =
        QStringLiteral("{\n  \"version\": 1,\n  \"schema\": \"whatson.bookmarks.list\",\n  \"bookmarks\": []\n}\n");
    if (!writeTextFile(bookmarksPath, bookmarksText, errorMessage))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("hub.creator"),
                                  QStringLiteral("createScaffold.failed.bookmarks"),
                                  errorMessage != nullptr ? *errorMessage : QString());
        return false;
    }

    const QString progressPath = joinPath(
        hubRootPath,
        joinPath(contentsDirectory, QStringLiteral("Progress.wsprogress")));
    const QString progressText =
        QStringLiteral(
            "{\n  \"version\": 1,\n  \"schema\": \"whatson.progress.state\",\n  \"value\": 0,\n  \"states\": [\"Ready\", \"Pending\", \"InProgress\", \"Done\"]\n}\n");
    if (!writeTextFile(progressPath, progressText, errorMessage))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("hub.creator"),
                                  QStringLiteral("createScaffold.failed.progress"),
                                  errorMessage != nullptr ? *errorMessage : QString());
        return false;
    }

    const QString projectListsPath = joinPath(
        hubRootPath,
        joinPath(contentsDirectory, QStringLiteral("ProjectLists.wsproj")));
    if (!writeTextFile(
        projectListsPath,
        QStringLiteral("{\n  \"version\": 1,\n  \"schema\": \"whatson.projects.list\",\n  \"projects\": []\n}\n"),
        errorMessage))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("hub.creator"),
                                  QStringLiteral("createScaffold.failed.projectLists"),
                                  errorMessage != nullptr ? *errorMessage : QString());
        return false;
    }

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.creator"),
                              QStringLiteral("createScaffold.success"),
                              QStringLiteral("manifest=%1 stat=%2").arg(manifestPath, statPath));
    return true;
}
