#include "WhatSonLibraryFolderHierarchyMutationService.hpp"

#include "file/hierarchy/folders/WhatSonFoldersHierarchyStore.hpp"
#include "file/note/WhatSonHubNoteMutationSupport.hpp"
#include "file/note/WhatSonNoteFolderSemantics.hpp"

#include <QSet>

namespace
{
    struct FolderHierarchyLookup final
    {
        QHash<QString, QStringList> pathKeysByLeafKey;
        QHash<QString, QSet<QString>> ancestorLeafKeysByPathKey;
        QHash<QString, QString> displayPathByPathKey;
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
            if (pathKey.isEmpty())
            {
                continue;
            }

            lookup.displayPathByPathKey.insert(pathKey, normalizeFolderPath(entry.id));

            QString leafKey = normalizeFolderLookupKey(entry.label);
            if (leafKey.isEmpty())
            {
                const QStringList segments = folderPathSegments(entry.id);
                if (!segments.isEmpty())
                {
                    leafKey = normalizeFolderLookupKey(segments.constLast());
                }
            }
            if (leafKey.isEmpty())
            {
                continue;
            }

            QStringList& pathKeys = lookup.pathKeysByLeafKey[leafKey];
            if (!pathKeys.contains(pathKey))
            {
                pathKeys.push_back(pathKey);
            }

            const QStringList segments = folderPathSegments(entry.id);
            QSet<QString>& ancestorLeafKeys = lookup.ancestorLeafKeysByPathKey[pathKey];
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

    QStringList resolvedNoteFolderPathKeys(const LibraryNoteRecord& note, const FolderHierarchyLookup& lookup)
    {
        struct NoteFolderToken final
        {
            QString key;
            bool hierarchical = false;
        };

        QVector<NoteFolderToken> tokens;
        tokens.reserve(note.folders.size());
        QSet<QString> rawKeys;

        for (const QString& rawFolder : note.folders)
        {
            const QString normalizedFolder = normalizeFolderPath(rawFolder);
            const QString key = normalizeFolderLookupKey(normalizedFolder);
            if (key.isEmpty())
            {
                continue;
            }

            rawKeys.insert(key);
            tokens.push_back(NoteFolderToken{
                key,
                normalizedFolder.contains(QLatin1Char('/'))
            });
        }

        QStringList resolved;
        QSet<QString> resolvedSet;
        auto appendResolved = [&resolved, &resolvedSet](const QString& pathKey)
        {
            if (pathKey.isEmpty() || resolvedSet.contains(pathKey))
            {
                return;
            }
            resolvedSet.insert(pathKey);
            resolved.push_back(pathKey);
        };

        for (const NoteFolderToken& token : tokens)
        {
            if (token.hierarchical)
            {
                appendResolved(token.key);
            }
        }

        for (const NoteFolderToken& token : tokens)
        {
            if (token.hierarchical)
            {
                continue;
            }

            const QStringList candidates = lookup.pathKeysByLeafKey.value(token.key);
            if (candidates.isEmpty())
            {
                continue;
            }

            QStringList contextualMatches;
            for (const QString& candidatePathKey : candidates)
            {
                const QSet<QString> ancestorLeafKeys = lookup.ancestorLeafKeysByPathKey.value(candidatePathKey);
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
                    contextualMatches.push_back(candidatePathKey);
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

    QString remapFolderPathForMove(const QString& folderPath, const QHash<QString, QString>& movedFolderPathMap)
    {
        const QString normalizedPath = normalizeFolderPath(folderPath);
        if (normalizedPath.isEmpty() || movedFolderPathMap.isEmpty())
        {
            return normalizedPath;
        }

        const QString normalizedPathKey = normalizeFolderLookupKey(normalizedPath);
        QString bestMatch;
        QString replacement;
        for (auto it = movedFolderPathMap.constBegin(); it != movedFolderPathMap.constEnd(); ++it)
        {
            const QString sourcePathKey = normalizeFolderLookupKey(it.key());
            if (sourcePathKey.isEmpty())
            {
                continue;
            }
            if (normalizedPathKey != sourcePathKey
                && !normalizedPathKey.startsWith(sourcePathKey + QLatin1Char('/')))
            {
                continue;
            }
            if (sourcePathKey.size() > bestMatch.size())
            {
                bestMatch = sourcePathKey;
                replacement = normalizeFolderPath(it.value());
            }
        }

        if (bestMatch.isEmpty())
        {
            return normalizedPath;
        }

        QString suffix = normalizedPath.mid(bestMatch.size());
        while (suffix.startsWith(QLatin1Char('/')))
        {
            suffix.remove(0, 1);
        }

        return suffix.isEmpty() ? replacement : buildFolderPath(replacement, suffix);
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

    if (request.runtimeIndexLoaded && !request.movedFolderPathMap.isEmpty())
    {
        const FolderHierarchyLookup originalLookup = buildFolderHierarchyLookup(request.currentFolderEntries);
        const QString rewriteTimestamp = WhatSon::NoteMutationSupport::currentNoteTimestamp();

        for (int noteIndex = 0; noteIndex < request.notes.size(); ++noteIndex)
        {
            const LibraryNoteRecord& note = request.notes.at(noteIndex);
            const QStringList resolvedFolders = canonicalLeafFolderPaths(
                resolvedNoteFolderPathKeys(note, originalLookup));
            if (resolvedFolders.isEmpty())
            {
                continue;
            }

            QStringList remappedFolders;
            remappedFolders.reserve(resolvedFolders.size());
            QSet<QString> seenFolderKeys;
            bool noteChanged = false;

            for (const QString& resolvedFolder : resolvedFolders)
            {
                const QString remappedFolder = remapFolderPathForMove(resolvedFolder, request.movedFolderPathMap);
                const QString remappedKey = normalizeFolderLookupKey(remappedFolder);
                if (remappedKey.isEmpty()
                    || seenFolderKeys.contains(remappedKey)
                    || WhatSon::NoteFolders::usesReservedTodayFolderSegment(remappedFolder))
                {
                    continue;
                }
                seenFolderKeys.insert(remappedKey);
                remappedFolders.push_back(remappedFolder);
                if (normalizeFolderPath(resolvedFolder) != remappedFolder)
                {
                    noteChanged = true;
                }
            }

            if (!noteChanged)
            {
                continue;
            }

            const QString headerPath = WhatSon::NoteMutationSupport::resolveNoteHeaderPath(note);
            if (headerPath.isEmpty())
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = QStringLiteral("Failed to resolve note header path for: %1").arg(note.noteId);
                }
                return false;
            }

            WhatSonLocalNoteFileStore::ReadRequest readRequest;
            readRequest.noteId = note.noteId;
            readRequest.noteDirectoryPath = note.noteDirectoryPath;
            readRequest.noteHeaderPath = headerPath;

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

            WhatSonLocalNoteDocument nextDocument = previousDocument;
            nextDocument.headerStore.setFolders(remappedFolders);
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
