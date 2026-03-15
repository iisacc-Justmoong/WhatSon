#include "WhatSonHubStructureValidator.hpp"

#include <QDir>
#include <QFileInfo>

WhatSonHubStructureValidator::WhatSonHubStructureValidator() = default;

WhatSonHubStructureValidator::~WhatSonHubStructureValidator() = default;

QString WhatSonHubStructureValidator::normalizePath(const QString& path) const
{
    const QString trimmed = path.trimmed();
    if (trimmed.isEmpty())
    {
        return {};
    }

    return QDir::cleanPath(trimmed);
}

bool WhatSonHubStructureValidator::resolveContentsDirectories(
    const QString& wshubPath,
    QStringList* outContentsDirectories,
    QString* errorMessage) const
{
    if (outContentsDirectories == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("outContentsDirectories must not be null.");
        }
        return false;
    }

    outContentsDirectories->clear();

    const QString hubRootPath = normalizePath(wshubPath);
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

    if (!hubInfo.isDir() || !hubInfo.fileName().endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("wshubPath must be an unpacked .wshub directory: %1").arg(hubRootPath);
        }
        return false;
    }

    const QDir hubDir(hubRootPath);
    const QString fixedInternalPath = hubDir.filePath(QStringLiteral(".wscontents"));
    if (QFileInfo(fixedInternalPath).isDir())
    {
        outContentsDirectories->push_back(QDir::cleanPath(fixedInternalPath));
    }

    const QStringList dynamicContentsDirectories = hubDir.entryList(
        QStringList{QStringLiteral("*.wscontents")},
        QDir::Dirs | QDir::NoDotAndDotDot,
        QDir::Name);
    for (const QString& directoryName : dynamicContentsDirectories)
    {
        outContentsDirectories->push_back(QDir::cleanPath(hubDir.filePath(directoryName)));
    }

    outContentsDirectories->removeDuplicates();
    if (!outContentsDirectories->isEmpty())
    {
        return true;
    }

    if (errorMessage != nullptr)
    {
        *errorMessage = QStringLiteral("No *.wscontents directory was found inside .wshub: %1").arg(hubRootPath);
    }
    return false;
}

QStringList WhatSonHubStructureValidator::resolveLibraryRoots(const QString& wshubPath) const
{
    QStringList contentsDirectories;
    if (!resolveContentsDirectories(wshubPath, &contentsDirectories, nullptr))
    {
        return {};
    }

    QStringList libraryRoots;
    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QDir contentsDir(contentsDirectory);
        const QString fixedLibraryPath = contentsDir.filePath(QStringLiteral("Library.wslibrary"));
        if (QFileInfo(fixedLibraryPath).isDir())
        {
            libraryRoots.push_back(QDir::cleanPath(fixedLibraryPath));
        }

        const QStringList dynamicLibraries = contentsDir.entryList(
            QStringList{QStringLiteral("*.wslibrary")},
            QDir::Dirs | QDir::NoDotAndDotDot,
            QDir::Name);
        for (const QString& directoryName : dynamicLibraries)
        {
            libraryRoots.push_back(QDir::cleanPath(contentsDir.filePath(directoryName)));
        }
    }

    libraryRoots.removeDuplicates();
    return libraryRoots;
}

QString WhatSonHubStructureValidator::resolvePrimaryLibraryPath(const QString& wshubPath, QString* errorMessage) const
{
    const QStringList libraryRoots = resolveLibraryRoots(wshubPath);
    if (!libraryRoots.isEmpty())
    {
        return libraryRoots.first();
    }

    if (errorMessage != nullptr)
    {
        *errorMessage = QStringLiteral("No Library.wslibrary directory found inside: %1").arg(normalizePath(wshubPath));
    }
    return {};
}

QString WhatSonHubStructureValidator::resolveHubStatPath(const QString& wshubPath) const
{
    const QString normalizedWshubPath = normalizePath(wshubPath);
    if (normalizedWshubPath.isEmpty())
    {
        return {};
    }

    const QDir hubDir(normalizedWshubPath);
    const QStringList statFiles = hubDir.entryList(
        QStringList{QStringLiteral("*.wsstat")},
        QDir::Files | QDir::NoDotAndDotDot,
        QDir::Name);
    if (statFiles.isEmpty())
    {
        return {};
    }

    return QDir::cleanPath(hubDir.filePath(statFiles.first()));
}
