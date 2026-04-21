#include "app/viewmodel/detailPanel/session/WhatSonFoldersHierarchySessionService.hpp"

#include "app/models/file/hierarchy/WhatSonFolderDepthEntry.hpp"
#include "app/models/file/hierarchy/WhatSonFolderIdentity.hpp"
#include "app/models/file/hierarchy/folders/WhatSonFoldersHierarchyParser.hpp"
#include "app/models/file/hierarchy/folders/WhatSonFoldersHierarchyStore.hpp"
#include "app/models/file/note/WhatSonNoteFolderSemantics.hpp"
#include "app/viewmodel/hierarchy/library/LibraryHierarchyViewModelSupport.hpp"

#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QStringList>
#include <QVector>

namespace
{
    QString normalizeFolderPath(QString value)
    {
        return WhatSon::NoteFolders::normalizeFolderPath(std::move(value));
    }

    QString normalizeFolderLookupKey(QString value)
    {
        return normalizeFolderPath(std::move(value)).toCaseFolded();
    }

    QStringList folderSegments(const QString& folderPath)
    {
        return WhatSon::NoteFolders::folderPathSegments(folderPath);
    }

    QString buildFolderPath(const QString& parentPath, const QString& segment)
    {
        return WhatSon::NoteFolders::appendFolderPathSegment(parentPath, segment);
    }

    struct FolderLookup final
    {
        QHash<QString, int> indexByPathKey;
    };

    FolderLookup buildFolderLookup(const QVector<WhatSonFolderDepthEntry>& entries)
    {
        FolderLookup lookup;
        for (int index = 0; index < entries.size(); ++index)
        {
            const QString pathKey = normalizeFolderLookupKey(entries.at(index).id);
            if (!pathKey.isEmpty() && !lookup.indexByPathKey.contains(pathKey))
            {
                lookup.indexByPathKey.insert(pathKey, index);
            }
        }
        return lookup;
    }

    int insertionIndexForDepth(
        const QVector<WhatSonFolderDepthEntry>& entries,
        const FolderLookup& lookup,
        const QString& parentPathKey,
        const int depth)
    {
        if (depth <= 0)
        {
            return entries.size();
        }

        const int parentIndex = lookup.indexByPathKey.value(parentPathKey, -1);
        if (parentIndex < 0 || parentIndex >= entries.size())
        {
            return entries.size();
        }

        const int parentDepth = entries.at(parentIndex).depth;
        int insertIndex = parentIndex + 1;
        while (insertIndex < entries.size() && entries.at(insertIndex).depth > parentDepth)
        {
            ++insertIndex;
        }
        return insertIndex;
    }
}

WhatSonFoldersHierarchySessionService::WhatSonFoldersHierarchySessionService() = default;

WhatSonFoldersHierarchySessionService::~WhatSonFoldersHierarchySessionService() = default;

bool WhatSonFoldersHierarchySessionService::ensureFolderEntry(
    const QString& noteDirectoryPath,
    const QString& requestedFolderPath,
    FolderResolution* outResolution,
    QString* errorMessage) const
{
    if (outResolution != nullptr)
    {
        *outResolution = FolderResolution{};
    }

    const QString normalizedRequestedPath = normalizeFolderPath(requestedFolderPath);
    if (normalizedRequestedPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Folder path must not be empty.");
        }
        return false;
    }

    if (WhatSon::NoteFolders::usesReservedTodayFolderSegment(normalizedRequestedPath))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Reserved Today folders cannot be edited from the detail panel.");
        }
        return false;
    }

    const QString foldersFilePath = resolveFoldersFilePath(noteDirectoryPath);
    if (foldersFilePath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Folders.wsfolders could not be resolved from the current note path.");
        }
        return false;
    }

    QVector<WhatSonFolderDepthEntry> folderEntries;
    if (QFileInfo(foldersFilePath).isFile())
    {
        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::LibrarySupport::readUtf8File(foldersFilePath, &rawText, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            return false;
        }

        WhatSonFoldersHierarchyStore hierarchyStore;
        WhatSonFoldersHierarchyParser hierarchyParser;
        QString parseError;
        if (!hierarchyParser.parse(rawText, &hierarchyStore, &parseError, nullptr))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            return false;
        }

        folderEntries = hierarchyStore.folderEntries();
    }

    FolderLookup lookup = buildFolderLookup(folderEntries);
    const QStringList segments = folderSegments(normalizedRequestedPath);
    QString canonicalPath;
    QString canonicalParentPath;
    QString finalFolderUuid;
    bool hierarchyChanged = false;
    bool folderCreated = false;

    for (int depth = 0; depth < segments.size(); ++depth)
    {
        const QString segment = segments.at(depth).trimmed();
        const QString requestedCumulativePath = buildFolderPath(canonicalParentPath, segment);
        const QString requestedPathKey = normalizeFolderLookupKey(requestedCumulativePath);
        const auto existingIt = lookup.indexByPathKey.constFind(requestedPathKey);

        if (existingIt != lookup.indexByPathKey.constEnd())
        {
            WhatSonFolderDepthEntry& existingEntry = folderEntries[existingIt.value()];
            canonicalPath = normalizeFolderPath(existingEntry.id);
            if (canonicalPath.isEmpty())
            {
                canonicalPath = requestedCumulativePath;
                existingEntry.id = canonicalPath;
                hierarchyChanged = true;
                lookup = buildFolderLookup(folderEntries);
            }

            QString normalizedUuid = WhatSon::FolderIdentity::normalizeFolderUuid(existingEntry.uuid);
            if (normalizedUuid.isEmpty())
            {
                normalizedUuid = WhatSon::FolderIdentity::createFolderUuid();
                existingEntry.uuid = normalizedUuid;
                hierarchyChanged = true;
            }

            finalFolderUuid = normalizedUuid;
            canonicalParentPath = canonicalPath;
            continue;
        }

        const QString newCanonicalPath = buildFolderPath(canonicalParentPath, segment);
        WhatSonFolderDepthEntry newEntry;
        newEntry.id = newCanonicalPath;
        newEntry.label = segment;
        newEntry.depth = depth;
        newEntry.uuid = WhatSon::FolderIdentity::createFolderUuid();

        const QString parentPathKey = normalizeFolderLookupKey(canonicalParentPath);
        const int insertIndex = insertionIndexForDepth(folderEntries, lookup, parentPathKey, depth);
        folderEntries.insert(insertIndex, newEntry);
        lookup = buildFolderLookup(folderEntries);

        hierarchyChanged = true;
        folderCreated = true;
        canonicalPath = newCanonicalPath;
        canonicalParentPath = canonicalPath;
        finalFolderUuid = newEntry.uuid;
    }

    if (hierarchyChanged)
    {
        WhatSonFoldersHierarchyStore stagedStore;
        stagedStore.setFolderEntries(folderEntries);

        QString writeError;
        if (!stagedStore.writeToFile(foldersFilePath, &writeError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = writeError;
            }
            return false;
        }
    }

    if (outResolution != nullptr)
    {
        outResolution->folderPath = canonicalPath;
        outResolution->folderUuid = finalFolderUuid;
        outResolution->foldersFilePath = foldersFilePath;
        outResolution->folderCreated = folderCreated;
        outResolution->hierarchyChanged = hierarchyChanged;
    }

    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }
    return true;
}

QString WhatSonFoldersHierarchySessionService::resolveFoldersFilePath(const QString& noteDirectoryPath)
{
    QString candidatePath = QDir::cleanPath(noteDirectoryPath.trimmed());
    if (candidatePath.isEmpty())
    {
        return {};
    }

    QFileInfo currentInfo(candidatePath);
    if (!currentInfo.isDir())
    {
        candidatePath = currentInfo.absolutePath();
        currentInfo.setFile(candidatePath);
    }

    while (currentInfo.exists())
    {
        const QString directoryName = currentInfo.fileName().trimmed();
        if (directoryName == QStringLiteral(".wscontents")
            || directoryName.endsWith(QStringLiteral(".wscontents"), Qt::CaseInsensitive))
        {
            return QDir(currentInfo.absoluteFilePath()).filePath(QStringLiteral("Folders.wsfolders"));
        }

        const QString parentPath = currentInfo.absolutePath();
        if (parentPath.isEmpty() || parentPath == currentInfo.absoluteFilePath())
        {
            break;
        }

        currentInfo.setFile(parentPath);
    }

    return {};
}
