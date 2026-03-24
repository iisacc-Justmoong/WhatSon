#include "WhatSonLibraryFolderHierarchyMutationService.hpp"

#include "file/hierarchy/WhatSonFolderIdentity.hpp"
#include "file/hierarchy/folders/WhatSonFoldersHierarchyStore.hpp"
#include "file/note/WhatSonHubNoteMutationSupport.hpp"
#include "file/note/WhatSonNoteFolderSemantics.hpp"

#include <QSet>

namespace
{
    struct FolderHierarchyLookup final
    {
        QHash<QString, QStringList> folderUuidsByLeafKey;
        QHash<QString, QSet<QString>> ancestorLeafKeysByFolderUuid;
        QHash<QString, QString> displayPathByFolderUuid;
        QHash<QString, QString> folderUuidByPathKey;
    };

    QString normalizeFolderPath(QString value)
    {
        value = value.trimmed();
        value.replace(QLatin1Char('\\'), QLatin1Char('/'));
        while (value.contains(QStringLiteral("//")))
        {
            value.replace(QStringLiteral("//"), QStringLiteral("/"));
        }
        while (value.startsWith(QLatin1Char('/')))
        {
            value.remove(0, 1);
        }
        while (value.endsWith(QLatin1Char('/')))
        {
            value.chop(1);
        }
        return value;
    }

    QString normalizeFolderLookupKey(QString value)
    {
        return normalizeFolderPath(std::move(value)).toCaseFolded();
    }

    QString normalizeFolderUuid(QString value)
    {
        return WhatSon::FolderIdentity::normalizeFolderUuid(std::move(value));
    }

    QStringList folderPathSegments(const QString& folderPath)
    {
        const QString normalized = normalizeFolderPath(folderPath);
        if (normalized.isEmpty())
        {
            return {};
        }
        return normalized.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    }

    FolderHierarchyLookup buildFolderHierarchyLookup(const QVector<WhatSonFolderDepthEntry>& entries)
    {
        FolderHierarchyLookup lookup;

        for (const WhatSonFolderDepthEntry& entry : entries)
        {
            const QString pathKey = normalizeFolderLookupKey(entry.id);
            const QString folderUuid = normalizeFolderUuid(entry.uuid);
            if (pathKey.isEmpty())
            {
                continue;
            }

            if (!folderUuid.isEmpty())
            {
                lookup.displayPathByFolderUuid.insert(folderUuid, normalizeFolderPath(entry.id));
                lookup.folderUuidByPathKey.insert(pathKey, folderUuid);
            }

            QString leafKey = normalizeFolderLookupKey(entry.label);
            if (leafKey.isEmpty())
            {
                const QStringList segments = folderPathSegments(entry.id);
                if (!segments.isEmpty())
                {
                    leafKey = normalizeFolderLookupKey(segments.constLast());
                }
            }
            if (leafKey.isEmpty() || folderUuid.isEmpty())
            {
                continue;
            }

            QStringList& folderUuids = lookup.folderUuidsByLeafKey[leafKey];
            if (!folderUuids.contains(folderUuid))
            {
                folderUuids.push_back(folderUuid);
            }

            const QStringList segments = folderPathSegments(entry.id);
            QSet<QString>& ancestorLeafKeys = lookup.ancestorLeafKeysByFolderUuid[folderUuid];
            for (int index = 0; index + 1 < segments.size(); ++index)
            {
                const QString ancestorKey = normalizeFolderLookupKey(segments.at(index));
                if (!ancestorKey.isEmpty())
                {
                    ancestorLeafKeys.insert(ancestorKey);
                }
            }
        }

        return lookup;
    }

    QStringList resolvedNoteFolderUuids(const LibraryNoteRecord& note, const FolderHierarchyLookup& lookup)
    {
        struct NoteFolderToken final
        {
            QString pathKey;
            QString folderUuid;
            bool hierarchical = false;
        };

        QVector<NoteFolderToken> tokens;
        tokens.reserve(std::max(note.folders.size(), note.folderUuids.size()));
        QSet<QString> rawKeys;

        const int tokenCount = std::max(note.folders.size(), note.folderUuids.size());
        for (int index = 0; index < tokenCount; ++index)
        {
            const QString rawFolder = index < note.folders.size() ? note.folders.at(index) : QString();
            const QString normalizedFolder = normalizeFolderPath(rawFolder);
            const QString pathKey = normalizeFolderLookupKey(normalizedFolder);
            const QString folderUuid = index < note.folderUuids.size()
                                           ? normalizeFolderUuid(note.folderUuids.at(index))
                                           : QString();
            if (!pathKey.isEmpty())
            {
                rawKeys.insert(pathKey);
            }
            if (pathKey.isEmpty() && folderUuid.isEmpty())
            {
                continue;
            }
            tokens.push_back(NoteFolderToken{
                pathKey,
                folderUuid,
                normalizedFolder.contains(QLatin1Char('/'))
            });
        }

        QStringList resolved;
        QSet<QString> resolvedSet;
        auto appendResolved = [&resolved, &resolvedSet](const QString& folderUuid)
        {
            if (folderUuid.isEmpty() || resolvedSet.contains(folderUuid))
            {
                return;
            }
            resolvedSet.insert(folderUuid);
            resolved.push_back(folderUuid);
        };

        for (const NoteFolderToken& token : tokens)
        {
            if (!token.folderUuid.isEmpty())
            {
                appendResolved(token.folderUuid);
            }
        }

        for (const NoteFolderToken& token : tokens)
        {
            if (token.hierarchical)
            {
                appendResolved(lookup.folderUuidByPathKey.value(token.pathKey));
            }
        }

        for (const NoteFolderToken& token : tokens)
        {
            if (token.hierarchical)
            {
                continue;
            }

            const QStringList candidates = lookup.folderUuidsByLeafKey.value(token.pathKey);
            if (candidates.isEmpty())
            {
                continue;
            }

            QStringList contextualMatches;
            for (const QString& candidateFolderUuid : candidates)
            {
                const QSet<QString> ancestorLeafKeys = lookup.ancestorLeafKeysByFolderUuid.value(candidateFolderUuid);
                bool matchesAllAncestors = true;
                for (const QString& ancestorLeafKey : ancestorLeafKeys)
                {
                    if (!rawKeys.contains(ancestorLeafKey))
                    {
                        matchesAllAncestors = false;
                        break;
                    }
                }

                if (matchesAllAncestors)
                {
                    contextualMatches.push_back(candidateFolderUuid);
                }
            }

            if (contextualMatches.size() == 1)
            {
                appendResolved(contextualMatches.constFirst());
                continue;
            }

            if (contextualMatches.isEmpty() && candidates.size() == 1)
            {
                appendResolved(candidates.constFirst());
            }
        }

        return resolved;
    }

    QStringList canonicalLeafFolderPaths(const QStringList& folderPaths)
    {
        QStringList canonicalPaths;
        canonicalPaths.reserve(folderPaths.size());

        for (const QString& rawPath : folderPaths)
        {
            const QString normalizedPath = normalizeFolderPath(rawPath);
            if (normalizedPath.isEmpty())
            {
                continue;
            }

            bool hasDescendant = false;
            for (const QString& comparePath : folderPaths)
            {
                const QString normalizedComparePath = normalizeFolderPath(comparePath);
                if (normalizedComparePath.isEmpty() || normalizedComparePath == normalizedPath)
                {
                    continue;
                }
                if (normalizedComparePath.startsWith(normalizedPath + QLatin1Char('/')))
                {
                    hasDescendant = true;
                    break;
                }
            }

            if (!hasDescendant && !canonicalPaths.contains(normalizedPath))
            {
                canonicalPaths.push_back(normalizedPath);
            }
        }

        return canonicalPaths;
    }

    QStringList canonicalLeafFolderUuids(const QStringList& folderUuids, const FolderHierarchyLookup& lookup)
    {
        QStringList canonicalUuids;
        canonicalUuids.reserve(folderUuids.size());

        for (const QString& rawFolderUuid : folderUuids)
        {
            const QString folderUuid = normalizeFolderUuid(rawFolderUuid);
            const QString folderPath = normalizeFolderPath(lookup.displayPathByFolderUuid.value(folderUuid));
            if (folderUuid.isEmpty() || folderPath.isEmpty())
            {
                continue;
            }

            bool hasDescendant = false;
            for (const QString& compareRawFolderUuid : folderUuids)
            {
                const QString compareFolderUuid = normalizeFolderUuid(compareRawFolderUuid);
                const QString compareFolderPath = normalizeFolderPath(
                    lookup.displayPathByFolderUuid.value(compareFolderUuid));
                if (compareFolderUuid.isEmpty()
                    || compareFolderPath.isEmpty()
                    || compareFolderUuid == folderUuid)
                {
                    continue;
                }
                if (compareFolderPath.startsWith(folderPath + QLatin1Char('/')))
                {
                    hasDescendant = true;
                    break;
                }
            }

            if (!hasDescendant && !canonicalUuids.contains(folderUuid))
            {
                canonicalUuids.push_back(folderUuid);
            }
        }

        return canonicalUuids;
    }

    QString buildFolderPath(const QString& parentPath, const QString& label)
    {
        const QString normalizedLabel = normalizeFolderPath(label);
        if (normalizedLabel.isEmpty())
        {
            return normalizeFolderPath(parentPath);
        }

        const QString normalizedParent = normalizeFolderPath(parentPath);
        if (normalizedParent.isEmpty())
        {
            return normalizedLabel;
        }
        return normalizedParent + QLatin1Char('/') + normalizedLabel;
    }

    bool folderBindingsMatch(
        const QStringList& lhsFolders,
        const QStringList& lhsFolderUuids,
        const QStringList& rhsFolders,
        const QStringList& rhsFolderUuids)
    {
        if (lhsFolders.size() != rhsFolders.size())
        {
            return false;
        }

        for (int index = 0; index < lhsFolders.size(); ++index)
        {
            const QString lhsUuid = index < lhsFolderUuids.size()
                                        ? normalizeFolderUuid(lhsFolderUuids.at(index))
                                        : QString();
            const QString rhsUuid = index < rhsFolderUuids.size()
                                        ? normalizeFolderUuid(rhsFolderUuids.at(index))
                                        : QString();
            if (!lhsUuid.isEmpty() || !rhsUuid.isEmpty())
            {
                if (lhsUuid != rhsUuid)
                {
                    return false;
                }
            }

            if (normalizeFolderLookupKey(lhsFolders.at(index)) != normalizeFolderLookupKey(rhsFolders.at(index)))
            {
                return false;
            }
        }

        return true;
    }
} // namespace

WhatSonLibraryFolderHierarchyMutationService::WhatSonLibraryFolderHierarchyMutationService() = default;

WhatSonLibraryFolderHierarchyMutationService::~WhatSonLibraryFolderHierarchyMutationService() = default;

bool WhatSonLibraryFolderHierarchyMutationService::commitMutation(
    Request request,
    Result* outResult,
    QString* errorMessage) const
{
    struct PendingNoteFolderRewrite final
    {
        int noteIndex = -1;
        WhatSonLocalNoteDocument previousDocument;
        WhatSonLocalNoteDocument nextDocument;
    };

    QVector<PendingNoteFolderRewrite> pendingNoteWrites;
    pendingNoteWrites.reserve(request.notes.size());

    if (request.runtimeIndexLoaded
        && (!request.currentFolderEntries.isEmpty() || !request.stagedFolderEntries.isEmpty()))
    {
        const FolderHierarchyLookup originalLookup = buildFolderHierarchyLookup(request.currentFolderEntries);
        const FolderHierarchyLookup stagedLookup = buildFolderHierarchyLookup(request.stagedFolderEntries);
        const QString rewriteTimestamp = WhatSon::NoteMutationSupport::currentNoteTimestamp();

        for (int noteIndex = 0; noteIndex < request.notes.size(); ++noteIndex)
        {
            const LibraryNoteRecord& note = request.notes.at(noteIndex);
            const QStringList resolvedFolderUuids = canonicalLeafFolderUuids(
                resolvedNoteFolderUuids(note, originalLookup),
                originalLookup);
            if (resolvedFolderUuids.isEmpty())
            {
                continue;
            }

            QStringList remappedFolders;
            QStringList remappedFolderUuids;
            remappedFolders.reserve(resolvedFolderUuids.size());
            remappedFolderUuids.reserve(resolvedFolderUuids.size());
            QSet<QString> seenFolderUuids;

            for (const QString& resolvedFolderUuid : resolvedFolderUuids)
            {
                const QString remappedFolderUuid = normalizeFolderUuid(resolvedFolderUuid);
                const QString remappedFolder = normalizeFolderPath(
                    stagedLookup.displayPathByFolderUuid.value(remappedFolderUuid));
                if (remappedFolderUuid.isEmpty()
                    || remappedFolder.isEmpty()
                    || seenFolderUuids.contains(remappedFolderUuid)
                    || WhatSon::NoteFolders::usesReservedTodayFolderSegment(remappedFolder))
                {
                    continue;
                }
                seenFolderUuids.insert(remappedFolderUuid);
                remappedFolders.push_back(remappedFolder);
                remappedFolderUuids.push_back(remappedFolderUuid);
            }

            WhatSonLocalNoteFileStore::ReadRequest readRequest;
            readRequest.noteId = note.noteId;
            readRequest.noteDirectoryPath = note.noteDirectoryPath;
            readRequest.noteHeaderPath = WhatSon::NoteMutationSupport::resolveNoteHeaderPath(note);

            WhatSonLocalNoteDocument previousDocument;
            QString readError;
            if (!m_localNoteFileStore.readNote(std::move(readRequest), &previousDocument, &readError))
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = readError;
                }
                return false;
            }

            if (folderBindingsMatch(
                previousDocument.headerStore.folders(),
                previousDocument.headerStore.folderUuids(),
                remappedFolders,
                remappedFolderUuids))
            {
                continue;
            }

            WhatSonLocalNoteDocument nextDocument = previousDocument;
            nextDocument.headerStore.setFolderBindings(remappedFolders, remappedFolderUuids);
            nextDocument.headerStore.setLastModifiedAt(rewriteTimestamp);

            PendingNoteFolderRewrite rewrite;
            rewrite.noteIndex = noteIndex;
            rewrite.previousDocument = std::move(previousDocument);
            rewrite.nextDocument = std::move(nextDocument);
            pendingNoteWrites.push_back(std::move(rewrite));
        }
    }

    auto rollbackNoteWrites = [this](const QVector<PendingNoteFolderRewrite>& writes)
    {
        for (int index = writes.size() - 1; index >= 0; --index)
        {
            const PendingNoteFolderRewrite& rewrite = writes.at(index);
            WhatSonLocalNoteFileStore::UpdateRequest rollbackRequest;
            rollbackRequest.document = rewrite.previousDocument;
            rollbackRequest.persistHeader = true;
            rollbackRequest.persistBody = false;
            m_localNoteFileStore.updateNote(std::move(rollbackRequest), nullptr, nullptr);
        }
    };

    QVector<PendingNoteFolderRewrite> appliedNoteWrites;
    appliedNoteWrites.reserve(pendingNoteWrites.size());
    for (const PendingNoteFolderRewrite& rewrite : pendingNoteWrites)
    {
        WhatSonLocalNoteFileStore::UpdateRequest updateRequest;
        updateRequest.document = rewrite.nextDocument;
        updateRequest.persistHeader = true;
        updateRequest.persistBody = false;

        WhatSonLocalNoteDocument updatedDocument;
        QString writeError;
        if (!m_localNoteFileStore.updateNote(std::move(updateRequest), &updatedDocument, &writeError))
        {
            rollbackNoteWrites(appliedNoteWrites);
            if (errorMessage != nullptr)
            {
                *errorMessage = writeError;
            }
            return false;
        }

        PendingNoteFolderRewrite appliedRewrite = rewrite;
        appliedRewrite.nextDocument = std::move(updatedDocument);
        appliedNoteWrites.push_back(std::move(appliedRewrite));
    }

    if (!request.foldersFilePath.trimmed().isEmpty())
    {
        WhatSonFoldersHierarchyStore stagedStore;
        stagedStore.setFolderEntries(request.stagedFolderEntries);

        QString writeError;
        if (!stagedStore.writeToFile(request.foldersFilePath, &writeError))
        {
            rollbackNoteWrites(appliedNoteWrites);
            if (errorMessage != nullptr)
            {
                *errorMessage = writeError;
            }
            return false;
        }
    }

    for (const PendingNoteFolderRewrite& rewrite : appliedNoteWrites)
    {
        LibraryNoteRecord& note = request.notes[rewrite.noteIndex];
        WhatSon::NoteMutationSupport::syncNoteRecordFromDocument(&note, rewrite.nextDocument);
    }

    if (outResult != nullptr)
    {
        outResult->notes = std::move(request.notes);
    }

    return true;
}
