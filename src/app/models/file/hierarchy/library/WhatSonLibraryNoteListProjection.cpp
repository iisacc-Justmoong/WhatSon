#include "app/models/file/hierarchy/library/WhatSonLibraryNoteListProjection.hpp"

#include "app/models/calendar/ISystemCalendarStore.hpp"
#include "app/models/calendar/SystemCalendarStore.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/models/file/hierarchy/WhatSonFolderIdentity.hpp"
#include "app/models/file/hierarchy/library/LibraryHierarchyControllerSupport.hpp"
#include "app/models/file/hierarchy/library/LibraryNotePreviewText.hpp"
#include "app/models/file/note/WhatSonBookmarkColorPalette.hpp"
#include "app/models/file/note/WhatSonNoteFolderSemantics.hpp"

#include <QFileInfo>
#include <QSet>

#include <algorithm>
#include <utility>

namespace
{
    constexpr auto kLibraryDraftLabel = "Drafts";

    QString normalizeFolderPath(QString value)
    {
        return WhatSon::NoteFolders::normalizeFolderPath(std::move(value));
    }

    QString normalizeFolderLookupKey(QString value)
    {
        return normalizeFolderPath(std::move(value)).toCaseFolded();
    }

    QString normalizeFolderUuid(QString value)
    {
        return WhatSon::FolderIdentity::normalizeFolderUuid(std::move(value));
    }

    QString leafNameFromFolderPath(const QString& value)
    {
        return WhatSon::NoteFolders::leafFolderName(value);
    }

    QStringList folderPathSegments(const QString& folderPath)
    {
        return WhatSon::NoteFolders::folderPathSegments(folderPath);
    }

    bool isProtectedRootItem(const LibraryHierarchyItem& item)
    {
        return item.systemBucket != LibraryHierarchyItem::SystemBucket::None
            || (item.accent && item.depth == 0);
    }

    QStringList noteListFolders(const LibraryNoteRecord& note)
    {
        QStringList folders;
        folders.reserve(note.folders.size());
        for (const QString& folder : note.folders)
        {
            const QString displayFolder = WhatSon::NoteFolders::displayFolderPath(folder);
            if (!displayFolder.isEmpty())
            {
                folders.push_back(displayFolder);
            }
        }
        folders.removeDuplicates();
        if (folders.isEmpty())
        {
            folders.push_back(QString::fromLatin1(kLibraryDraftLabel));
        }
        return folders;
    }

    QStringList noteListTags(const LibraryNoteRecord& note)
    {
        QStringList tags;
        tags.reserve(note.tags.size());
        for (const QString& tag : note.tags)
        {
            const QString trimmed = tag.trimmed();
            if (!trimmed.isEmpty())
            {
                tags.push_back(trimmed);
            }
        }
        tags.removeDuplicates();
        return tags;
    }

    QString noteSearchableText(const LibraryNoteRecord& note, const QStringList& folderLabels)
    {
        QStringList parts;

        const QString firstLine = note.bodyFirstLine.trimmed();
        if (!firstLine.isEmpty())
        {
            parts.push_back(firstLine);
        }

        const QString bodyPlainText = note.bodyPlainText.trimmed();
        if (!bodyPlainText.isEmpty())
        {
            parts.push_back(bodyPlainText);
        }

        for (const QString& folder : folderLabels)
        {
            const QString trimmed = folder.trimmed();
            if (!trimmed.isEmpty())
            {
                parts.push_back(trimmed);
            }
        }

        for (const QString& tag : note.tags)
        {
            const QString trimmed = tag.trimmed();
            if (!trimmed.isEmpty())
            {
                parts.push_back(trimmed);
            }
        }

        return parts.join(QLatin1Char('\n'));
    }

    QString bookmarkColorHexFromNote(const LibraryNoteRecord& note)
    {
        if (!note.bookmarkColors.isEmpty())
        {
            return WhatSon::Bookmarks::bookmarkColorToHex(note.bookmarkColors.first());
        }
        return WhatSon::Bookmarks::defaultBookmarkColorHex();
    }

    struct FolderHierarchyLookup final
    {
        QHash<QString, QStringList> folderUuidsByLeafKey;
        QHash<QString, QSet<QString>> ancestorLeafKeysByFolderUuid;
        QHash<QString, QString> displayPathByFolderUuid;
        QHash<QString, QString> folderUuidByPathKey;
    };

    FolderHierarchyLookup buildFolderHierarchyLookup(const QVector<LibraryHierarchyItem>& items)
    {
        FolderHierarchyLookup lookup;

        for (const LibraryHierarchyItem& item : items)
        {
            if (isProtectedRootItem(item))
            {
                continue;
            }

            const QString pathKey = normalizeFolderLookupKey(item.folderPath);
            const QString folderUuid = normalizeFolderUuid(item.folderUuid);
            if (pathKey.isEmpty())
            {
                continue;
            }

            if (!folderUuid.isEmpty())
            {
                lookup.displayPathByFolderUuid.insert(folderUuid, normalizeFolderPath(item.folderPath));
                lookup.folderUuidByPathKey.insert(pathKey, folderUuid);
            }

            QString leafKey = normalizeFolderLookupKey(item.label);
            if (leafKey.isEmpty())
            {
                leafKey = normalizeFolderLookupKey(leafNameFromFolderPath(item.folderPath));
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

            const QStringList segments = folderPathSegments(item.folderPath);
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

    struct ResolvedFolderBinding final
    {
        QString folderUuid;
        bool sourceWasLeafOnlyWithoutUuid = false;
    };

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
                WhatSon::NoteFolders::isHierarchicalFolderPath(normalizedFolder)
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

    QStringList effectiveNoteFolderUuids(
        const LibraryNoteRecord& note,
        const FolderHierarchyLookup& lookup)
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

    QStringList canonicalNoteFolderLabels(
        const LibraryNoteRecord& note,
        const FolderHierarchyLookup* lookup)
    {
        QStringList folders;
        QSet<QString> folderKeys;
        if (lookup != nullptr)
        {
            const QStringList resolvedFolderUuids = effectiveNoteFolderUuids(note, *lookup);
            folders.reserve(resolvedFolderUuids.size());
            for (const QString& folderUuid : resolvedFolderUuids)
            {
                const QString normalizedFolderPath = normalizeFolderPath(
                    lookup->displayPathByFolderUuid.value(folderUuid));
                const QString folderKey = normalizedFolderPath.toCaseFolded();
                if (!normalizedFolderPath.isEmpty() && !folderKeys.contains(folderKey))
                {
                    folders.push_back(WhatSon::NoteFolders::displayFolderPath(normalizedFolderPath));
                    folderKeys.insert(folderKey);
                }
            }
        }

        const QStringList mirroredFolders = noteListFolders(note);
        for (const QString& folderPath : mirroredFolders)
        {
            const QString normalizedFolderPath = normalizeFolderPath(folderPath);
            const QString folderKey = normalizedFolderPath.toCaseFolded();
            if (normalizedFolderPath.isEmpty() || folderKeys.contains(folderKey))
            {
                continue;
            }

            folders.push_back(WhatSon::NoteFolders::displayFolderPath(normalizedFolderPath));
            folderKeys.insert(folderKey);
        }

        if (folders.isEmpty())
        {
            folders = mirroredFolders;
        }
        return folders;
    }

    bool noteMatchesFolderScope(
        const LibraryNoteRecord& note,
        const QString& selectedFolderUuid,
        const FolderHierarchyLookup& lookup)
    {
        if (selectedFolderUuid.isEmpty())
        {
            return true;
        }

        const QStringList resolvedFolderUuids = effectiveNoteFolderUuids(note, lookup);
        for (const QString& folderUuid : resolvedFolderUuids)
        {
            if (folderUuid == selectedFolderUuid)
            {
                return true;
            }
        }

        return false;
    }

    QString noteListItemCacheKey(const QString& noteId, const QString& noteDirectoryPath)
    {
        return noteId.trimmed()
            + QLatin1Char('\n')
            + WhatSon::Hierarchy::LibrarySupport::normalizePath(noteDirectoryPath);
    }
} // namespace

void WhatSonLibraryNoteListProjection::setSystemCalendarStore(ISystemCalendarStore* store)
{
    if (m_systemCalendarStore == store)
    {
        return;
    }

    m_systemCalendarStore = store;
    invalidate();
}

ISystemCalendarStore* WhatSonLibraryNoteListProjection::systemCalendarStore() const noexcept
{
    return m_systemCalendarStore;
}

void WhatSonLibraryNoteListProjection::invalidate() const
{
    m_noteListItemCache.clear();
}

void WhatSonLibraryNoteListProjection::invalidateForNoteId(const QString& noteId) const
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return;
    }

    for (auto iterator = m_noteListItemCache.begin(); iterator != m_noteListItemCache.end();)
    {
        if (iterator.value().id.trimmed() == normalizedNoteId)
        {
            iterator = m_noteListItemCache.erase(iterator);
            continue;
        }
        ++iterator;
    }
}

QHash<QString, int> WhatSonLibraryNoteListProjection::folderNoteCountByFolderUuid(
    const QVector<LibraryHierarchyItem>& hierarchyItems,
    const QVector<LibraryNoteRecord>& notes,
    bool foldersHierarchyLoaded) const
{
    QHash<QString, int> countByFolderUuid;
    if (!foldersHierarchyLoaded)
    {
        return countByFolderUuid;
    }

    const FolderHierarchyLookup lookup = buildFolderHierarchyLookup(hierarchyItems);
    countByFolderUuid.reserve(hierarchyItems.size());

    for (const LibraryHierarchyItem& item : hierarchyItems)
    {
        const QString folderUuid = normalizeFolderUuid(item.folderUuid);
        if (!folderUuid.isEmpty() && !countByFolderUuid.contains(folderUuid))
        {
            countByFolderUuid.insert(folderUuid, 0);
        }
    }

    for (const LibraryNoteRecord& note : notes)
    {
        const QStringList effectiveFolderUuids = effectiveNoteFolderUuids(note, lookup);
        for (const QString& folderUuid : effectiveFolderUuids)
        {
            auto countIt = countByFolderUuid.find(folderUuid);
            if (countIt != countByFolderUuid.end())
            {
                ++countIt.value();
            }
        }
    }

    return countByFolderUuid;
}

QVector<LibraryNoteListItem> WhatSonLibraryNoteListProjection::buildNoteListItems(
    const QVector<LibraryHierarchyItem>& hierarchyItems,
    const QVector<LibraryNoteRecord>& notes,
    bool foldersHierarchyLoaded) const
{
    if (!m_noteListItemCache.isEmpty() && m_noteListItemCacheUsesFoldersHierarchy != foldersHierarchyLoaded)
    {
        m_noteListItemCache.clear();
    }
    m_noteListItemCacheUsesFoldersHierarchy = foldersHierarchyLoaded;

    QVector<LibraryNoteListItem> items;
    items.reserve(notes.size());
    const FolderHierarchyLookup lookup = buildFolderHierarchyLookup(hierarchyItems);
    const FolderHierarchyLookup* activeLookup = foldersHierarchyLoaded ? &lookup : nullptr;

    for (const LibraryNoteRecord& note : notes)
    {
        const LibraryNoteListItem item = buildNoteListItem(note, canonicalNoteFolderLabels(note, activeLookup));
        if (item.id.trimmed().isEmpty() && item.noteDirectoryPath.trimmed().isEmpty())
        {
            WhatSon::Debug::trace(
                QStringLiteral("library.noteListProjection"),
                QStringLiteral("buildNoteListItems.skipInvalidNote"),
                QStringLiteral("primaryText=%1 createdAt=%2 lastModifiedAt=%3")
                    .arg(item.primaryText)
                    .arg(item.createdAt)
                    .arg(item.lastModifiedAt));
            continue;
        }
        items.push_back(item);
    }

    return items;
}

QVector<LibraryNoteListItem> WhatSonLibraryNoteListProjection::buildFolderScopedNoteListItems(
    const QVector<LibraryHierarchyItem>& hierarchyItems,
    const QVector<LibraryNoteRecord>& notes,
    const QString& selectedFolderUuid) const
{
    const bool foldersHierarchyLoaded = true;
    if (!m_noteListItemCache.isEmpty() && m_noteListItemCacheUsesFoldersHierarchy != foldersHierarchyLoaded)
    {
        m_noteListItemCache.clear();
    }
    m_noteListItemCacheUsesFoldersHierarchy = foldersHierarchyLoaded;

    const FolderHierarchyLookup lookup = buildFolderHierarchyLookup(hierarchyItems);
    QVector<LibraryNoteListItem> items;
    items.reserve(notes.size());

    for (const LibraryNoteRecord& note : notes)
    {
        if (!noteMatchesFolderScope(note, selectedFolderUuid, lookup))
        {
            continue;
        }

        const LibraryNoteListItem item = buildNoteListItem(note, canonicalNoteFolderLabels(note, &lookup));
        if (item.id.trimmed().isEmpty() && item.noteDirectoryPath.trimmed().isEmpty())
        {
            WhatSon::Debug::trace(
                QStringLiteral("library.noteListProjection"),
                QStringLiteral("buildFolderScopedNoteListItems.skipInvalidNote"),
                QStringLiteral("primaryText=%1 createdAt=%2 lastModifiedAt=%3")
                    .arg(item.primaryText)
                    .arg(item.createdAt)
                    .arg(item.lastModifiedAt));
            continue;
        }
        items.push_back(item);
    }

    return items;
}

LibraryNoteListItem WhatSonLibraryNoteListProjection::buildNoteListItem(
    const LibraryNoteRecord& note,
    const QStringList& folderLabels) const
{
    QString noteId = note.noteId.trimmed();
    const QString noteDirectoryPath =
        WhatSon::Hierarchy::LibrarySupport::normalizePath(note.noteDirectoryPath);
    if (noteId.isEmpty() && !noteDirectoryPath.isEmpty())
    {
        noteId = QFileInfo(noteDirectoryPath).completeBaseName().trimmed();
        if (noteId.isEmpty())
        {
            noteId = QFileInfo(noteDirectoryPath).fileName().trimmed();
        }
        if (!noteId.isEmpty())
        {
            WhatSon::Debug::trace(
                QStringLiteral("library.noteListProjection"),
                QStringLiteral("buildNoteListItem.derivedNoteIdFromDirectoryPath"),
                QStringLiteral("noteDirectoryPath=%1 derivedNoteId=%2")
                    .arg(noteDirectoryPath)
                    .arg(noteId));
        }
    }

    const QString cacheKey = noteListItemCacheKey(noteId, noteDirectoryPath);
    if (!m_noteListItemCache.isEmpty())
    {
        const auto cachedIt = m_noteListItemCache.constFind(cacheKey);
        if (cachedIt != m_noteListItemCache.constEnd())
        {
            return cachedIt.value();
        }
    }

    LibraryNoteListItem item;
    item.id = noteId;
    item.noteDirectoryPath = noteDirectoryPath;
    item.primaryText = WhatSon::LibraryPreview::notePrimaryText(note);
    item.searchableText = noteSearchableText(note, folderLabels);
    item.bodyText = !note.bodySourceText.isEmpty()
        ? note.bodySourceText
        : note.bodyPlainText;
    item.createdAt = note.createdAt;
    item.lastModifiedAt = note.lastModifiedAt;
    item.image = note.bodyHasResource;
    item.imageSource = note.bodyFirstResourceThumbnailUrl;
    item.displayDate = m_systemCalendarStore
                           ? m_systemCalendarStore->formatNoteDate(note.lastModifiedAt, note.createdAt)
                           : SystemCalendarStore::formatNoteDateForSystem(note.lastModifiedAt, note.createdAt);
    item.folders = folderLabels;
    item.tags = noteListTags(note);
    item.bookmarked = note.bookmarked;
    item.bookmarkColor = bookmarkColorHexFromNote(note);

    WhatSon::Debug::trace(
        QStringLiteral("library.noteListProjection"),
        QStringLiteral("buildNoteListItem"),
        QStringLiteral("noteId=%1 noteDirectoryPath=%2 primaryText=%3 bodySourceText=%4 bodyPlainText=%5 bodyChosen=%6")
            .arg(item.id)
            .arg(item.noteDirectoryPath)
            .arg(WhatSon::Debug::summarizeText(item.primaryText, 48))
            .arg(WhatSon::Debug::summarizeText(note.bodySourceText, 48))
            .arg(WhatSon::Debug::summarizeText(note.bodyPlainText, 48))
            .arg(WhatSon::Debug::summarizeText(item.bodyText, 48)));
    if (!cacheKey.isEmpty())
    {
        m_noteListItemCache.insert(cacheKey, item);
    }
    return item;
}
