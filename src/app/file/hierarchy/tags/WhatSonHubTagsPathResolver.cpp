#include "WhatSonHubTagsPathResolver.hpp"

#include "WhatSonDebugTrace.hpp"
#include "hub/WhatSonHubPathUtils.hpp"

#include <QDir>
#include <QFileInfo>

namespace
{
    QString normalizedPath(const QString& input)
    {
        return WhatSon::HubPath::normalizePath(input);
    }
} // namespace

WhatSonHubTagsPathResolver::WhatSonHubTagsPathResolver() = default;

WhatSonHubTagsPathResolver::~WhatSonHubTagsPathResolver() = default;

bool WhatSonHubTagsPathResolver::resolveTagsFilePath(
    const QString& wshubPath,
    QString* outTagsFilePath,
    QString* errorMessage) const
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.tags.pathResolver"),
                              QStringLiteral("resolve.begin"),
                              QStringLiteral("wshubPath=%1").arg(wshubPath));
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
    const QString fixedInternalPath = WhatSon::HubPath::joinPath(hubDir.path(), QStringLiteral(".wscontents/Tags.wstags"));
    if (QFileInfo::exists(fixedInternalPath))
    {
        *outTagsFilePath = fixedInternalPath;
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("hub.tags.pathResolver"),
                                  QStringLiteral("resolve.success.fixed"),
                                  QStringLiteral("path=%1").arg(*outTagsFilePath));
        return true;
    }

    // Fallback: dynamic "<HubName>.wscontents/Tags.wstags".
    const QStringList contentsDirectories =
        hubDir.entryList(QStringList{QStringLiteral("*.wscontents")}, QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& contentsDirName : contentsDirectories)
    {
        const QString candidatePath = WhatSon::HubPath::joinPath(
            hubDir.path(),
            QDir(contentsDirName).filePath(QStringLiteral("Tags.wstags")));
        if (QFileInfo::exists(candidatePath))
        {
            *outTagsFilePath = candidatePath;
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("hub.tags.pathResolver"),
                                      QStringLiteral("resolve.success.dynamic"),
                                      QStringLiteral("path=%1").arg(*outTagsFilePath));
            return true;
        }
    }

    if (errorMessage != nullptr)
    {
        *errorMessage = QStringLiteral("Tags.wstags was not found inside .wshub: %1").arg(hubRootPath);
    }
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.tags.pathResolver"),
                              QStringLiteral("resolve.failed"),
                              errorMessage != nullptr ? *errorMessage : QString());
    return false;
}
