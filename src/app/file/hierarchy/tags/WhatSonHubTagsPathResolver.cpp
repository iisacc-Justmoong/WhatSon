#include "WhatSonHubTagsPathResolver.hpp"

#include <QDir>
#include <QFileInfo>

namespace
{
    QString normalizedPath(const QString& input)
    {
        const QString trimmed = input.trimmed();
        if (trimmed.isEmpty())
        {
            return {};
        }
        return QDir::cleanPath(trimmed);
    }
} // namespace

WhatSonHubTagsPathResolver::WhatSonHubTagsPathResolver() = default;

WhatSonHubTagsPathResolver::~WhatSonHubTagsPathResolver() = default;

bool WhatSonHubTagsPathResolver::resolveTagsFilePath(
    const QString& wshubPath,
    QString* outTagsFilePath,
    QString* errorMessage) const
{
    if (outTagsFilePath == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("outTagsFilePath must not be null.");
        }
        return false;
    }

    const QString hubRootPath = normalizedPath(wshubPath);
    if (hubRootPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("wshubPath must not be empty.");
        }
        return false;
    }

    const QFileInfo hubInfo(hubRootPath);
    if (!hubInfo.exists())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("wshubPath does not exist: %1").arg(hubRootPath);
        }
        return false;
    }
    if (!hubInfo.fileName().endsWith(QStringLiteral(".wshub")))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Path is not a .wshub package: %1").arg(hubRootPath);
        }
        return false;
    }
    if (!hubInfo.isDir())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage =
                QStringLiteral("Only unpacked .wshub directories are supported: %1").arg(hubRootPath);
        }
        return false;
    }

    const QDir hubDir(hubRootPath);

    // Preferred fixed internal path.
    const QString fixedInternalPath = hubDir.filePath(QStringLiteral(".wscontents/Tags.wstags"));
    if (QFileInfo::exists(fixedInternalPath))
    {
        *outTagsFilePath = fixedInternalPath;
        return true;
    }

    // Fallback: dynamic "<HubName>.wscontents/Tags.wstags".
    const QStringList contentsDirectories =
        hubDir.entryList(QStringList{QStringLiteral("*.wscontents")}, QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& contentsDirName : contentsDirectories)
    {
        const QString candidatePath = hubDir.filePath(
            QDir(contentsDirName).filePath(QStringLiteral("Tags.wstags")));
        if (QFileInfo::exists(candidatePath))
        {
            *outTagsFilePath = candidatePath;
            return true;
        }
    }

    if (errorMessage != nullptr)
    {
        *errorMessage = QStringLiteral("Tags.wstags was not found inside .wshub: %1").arg(hubRootPath);
    }
    return false;
}
