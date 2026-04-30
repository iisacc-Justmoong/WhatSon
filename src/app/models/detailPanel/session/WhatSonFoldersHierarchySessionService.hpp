#pragma once

#include <QString>

class WhatSonFoldersHierarchySessionService final
{
public:
    struct FolderResolution final
    {
        QString folderPath;
        QString folderUuid;
        QString foldersFilePath;
        bool folderCreated = false;
        bool hierarchyChanged = false;
    };

    WhatSonFoldersHierarchySessionService();
    ~WhatSonFoldersHierarchySessionService();

    bool ensureFolderEntry(
        const QString& noteDirectoryPath,
        const QString& requestedFolderPath,
        FolderResolution* outResolution = nullptr,
        QString* errorMessage = nullptr) const;

private:
    static QString resolveFoldersFilePath(const QString& noteDirectoryPath);
};
