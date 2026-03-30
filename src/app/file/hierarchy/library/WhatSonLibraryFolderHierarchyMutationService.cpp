#include "WhatSonLibraryFolderHierarchyMutationService.hpp"

#include "file/hierarchy/WhatSonFolderIdentity.hpp"
#include "file/hierarchy/folders/WhatSonFoldersHierarchyStore.hpp"
#include "file/note/WhatSonHubNoteMutationSupport.hpp"
#include "file/note/WhatSonNoteFolderBindingService.hpp"
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

    struct ResolvedFolderBinding final
    {
        QString folderUuid;
        bool sourceWasLeafOnlyWithoutUuid = false;
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

    QVector<ResolvedFolderBinding> resolvedNoteFolderBindings(
        const LibraryNoteRecord& note,
        const FolderHierarchyLookup& lookup)
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

        QVector<ResolvedFolderBinding> resolved;
        QHash<QString, int> resolvedIndexByFolderUuid;
        auto appendResolved = [&resolved, &resolvedIndexByFolderUuid](const QString& folderUuid,
                                                                      const NoteFolderToken& token)
        {
            const QString normalizedFolderUuid = normalizeFolderUuid(folderUuid);
            if (normalizedFolderUuid.isEmpty())
            {
                return;
            }

            const bool sourceWasLeafOnlyWithoutUuid = token.folderUuid.isEmpty() && !token.hierarchical;
            const auto existingIt = resolvedIndexByFolderUuid.constFind(normalizedFolderUuid);
            if (existingIt != resolvedIndexByFolderUuid.constEnd())
            {
                resolved[existingIt.value()].sourceWasLeafOnlyWithoutUuid =
                    resolved.at(existingIt.value()).sourceWasLeafOnlyWithoutUuid && sourceWasLeafOnlyWithoutUuid;
                return;
            }

            resolvedIndexByFolderUuid.insert(normalizedFolderUuid, resolved.size());
            resolved.push_back(ResolvedFolderBinding{
                normalizedFolderUuid,
                sourceWasLeafOnlyWithoutUuid,
            });
        };

        for (const NoteFolderToken& token : tokens)
        {
            if (!token.folderUuid.isEmpty())
            {
                appendResolved(token.folderUuid, token);
            }
        }

        for (const NoteFolderToken& token : tokens)
        {
            if (token.hierarchical)
            {
                appendResolved(lookup.folderUuidByPathKey.value(token.pathKey), token);
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
                appendResolved(contextualMatches.constFirst(), token);
                continue;
            }

            if (contextualMatches.isEmpty() && candidates.size() == 1)
            {
                appendResolved(candidates.constFirst(), token);
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

    QStringList effectiveNoteFolderUuids(const LibraryNoteRecord& note, const FolderHierarchyLookup& lookup)
    {
        const QVector<ResolvedFolderBinding> resolvedBindings = resolvedNoteFolderBindings(note, lookup);
        QStringList effectiveFolderUuids;
        effectiveFolderUuids.reserve(resolvedBindings.size());

        for (const ResolvedFolderBinding& binding : resolvedBindings)
        {
            const QString folderUuid = normalizeFolderUuid(binding.folderUuid);
            const QString folderPath = normalizeFolderPath(lookup.displayPathByFolderUuid.value(folderUuid));
            if (folderUuid.isEmpty() || folderPath.isEmpty())
            {
                continue;
            }

            bool suppressAsLegacyContextOnlyAncestor = false;
            if (binding.sourceWasLeafOnlyWithoutUuid)
            {
                for (const ResolvedFolderBinding& otherBinding : resolvedBindings)
                {
                    const QString otherFolderUuid = normalizeFolderUuid(otherBinding.folderUuid);
                    const QString otherFolderPath = normalizeFolderPath(
                        lookup.displayPathByFolderUuid.value(otherFolderUuid));
                    if (otherFolderUuid.isEmpty()
                        || otherFolderPath.isEmpty()
                        || otherFolderUuid == folderUuid
                        || !otherBinding.sourceWasLeafOnlyWithoutUuid)
                    {
                        continue;
                    }
                    if (otherFolderPath.startsWith(folderPath + QLatin1Char('/')))
                    {
                        suppressAsLegacyContextOnlyAncestor = true;
                        break;
                    }
                }
            }

            if (!suppressAsLegacyContextOnlyAncestor && !effectiveFolderUuids.contains(folderUuid))
            {
                effectiveFolderUuids.push_back(folderUuid);
            }
        }

        return effectiveFolderUuids;
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

    const WhatSonNoteFolderBindingService noteFolderBindingService;
    QVector<PendingNoteFolderRewrite> pendingNoteWrites;
    pendingNoteWrites.reserve(request.notes.size());

    if (request.runtimeIndexLoaded
        && (!request.currentFolderEntries.isEmpty() || !request.stagedFolderEntries.isEmpty()))
    {
        const FolderHierarchyLookup originalLookup = buildFolderHierarchyLookup(request.currentFolderEntries);
        const FolderHierarchyLookup stagedLookup = buildFolderHierarchyLookup(request.stagedFolderEntries);

        for (int noteIndex = 0; noteIndex < request.notes.size(); ++noteIndex)
        {
            const LibraryNoteRecord& note = request.notes.at(noteIndex);
            const QStringList resolvedFolderUuids = effectiveNoteFolderUuids(note, originalLookup);
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

            WhatSonLocalNoteDocument previousDocument;
            QString readError;
            if (!m_noteFolderBindingRepository.readDocument(note, &previousDocument, &readError))
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = readError;
                }
                return false;
            }

            const WhatSonNoteFolderBindingService::Bindings previousBindings = noteFolderBindingService.bindings(
                previousDocument.headerStore.folders(),
                previousDocument.headerStore.folderUuids());
            const WhatSonNoteFolderBindingService::Bindings remappedBindings = noteFolderBindingService.bindings(
                remappedFolders,
                remappedFolderUuids);
            if (noteFolderBindingService.matches(previousBindings, remappedBindings))
            {
                continue;
            }

            WhatSonLocalNoteDocument nextDocument = previousDocument;
            nextDocument.headerStore.setFolderBindings(remappedBindings.folders, remappedBindings.folderUuids);

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
            m_noteFolderBindingRepository.writeDocument(rewrite.previousDocument, nullptr, nullptr);
        }
    };

    QVector<PendingNoteFolderRewrite> appliedNoteWrites;
    appliedNoteWrites.reserve(pendingNoteWrites.size());
    for (const PendingNoteFolderRewrite& rewrite : pendingNoteWrites)
    {
        WhatSonLocalNoteDocument updatedDocument;
        QString writeError;
        if (!m_noteFolderBindingRepository.writeDocument(rewrite.nextDocument, &updatedDocument, &writeError))
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
