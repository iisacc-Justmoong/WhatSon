#include "LibraryHierarchyViewModel.hpp"

#include "calendar/SystemCalendarStore.hpp"
#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/library/WhatSonLibraryHierarchyCreator.hpp"
#include "file/hierarchy/library/WhatSonLibraryHierarchyStore.hpp"
#include "file/hierarchy/projects/WhatSonProjectsHierarchyParser.hpp"
#include "file/hierarchy/projects/WhatSonProjectsHierarchyStore.hpp"
#include "file/note/WhatSonBookmarkColorPalette.hpp"
#include "file/note/WhatSonHubNoteDeletionService.hpp"
#include "file/note/WhatSonNoteAttachManagerCreator.hpp"
#include "file/note/WhatSonNoteBodyPersistence.hpp"
#include "file/note/WhatSonNoteBodyCreator.hpp"
#include "file/note/WhatSonNoteHeaderCreator.hpp"
#include "file/note/WhatSonNoteHeaderParser.hpp"
#include "file/note/WhatSonNoteHeaderStore.hpp"
#include "file/note/WhatSonNoteLinkManagerCreator.hpp"
#include "viewmodel/hierarchy/library/LibraryHierarchyViewModelSupport.hpp"
#include "viewmodel/sidebar/SidebarHierarchyLvrsSupport.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSet>
#include <QVariantMap>

#include <algorithm>
#include <limits>
#include <utility>

namespace
{
    constexpr int kMaxNoteListSummaryLines = 5;
    constexpr auto kNoteTimestampFormat = "yyyy-MM-dd-hh-mm-ss";
    constexpr auto kLibraryDraftLabel = "Draft";
    constexpr auto kLibraryAllLabel = "All Library";
    constexpr auto kLibraryTodayLabel = "Today";

    QStringList noteListFolders(const LibraryNoteRecord& note);

    QString truncateToMaxLines(const QString& value, int maxLines)
    {
        if (maxLines <= 0)
        {
            return {};
        }

        const QStringList lines = value.split(QLatin1Char('\n'));
        if (lines.size() <= maxLines)
        {
            return value;
        }

        QStringList truncated;
        truncated.reserve(maxLines);
        for (int index = 0; index < maxLines; ++index)
        {
            truncated.push_back(lines.at(index));
        }
        return truncated.join(QLatin1Char('\n'));
    }

    QString notePrimaryText(const LibraryNoteRecord& note)
    {
        const QString bodyPlainText = truncateToMaxLines(note.bodyPlainText.trimmed(), kMaxNoteListSummaryLines);
        if (!bodyPlainText.isEmpty())
        {
            return bodyPlainText;
        }
        return {};
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

    QStringList noteListFolders(const LibraryNoteRecord& note)
    {
        QStringList folders;
        folders.reserve(note.folders.size());
        for (const QString& folder : note.folders)
        {
            const QString trimmed = folder.trimmed();
            if (!trimmed.isEmpty())
            {
                folders.push_back(trimmed);
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

    QString bookmarkColorHexFromNote(const LibraryNoteRecord& note)
    {
        if (!note.bookmarkColors.isEmpty())
        {
            return WhatSon::Bookmarks::bookmarkColorToHex(note.bookmarkColors.first());
        }
        return WhatSon::Bookmarks::defaultBookmarkColorHex();
    }

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

    QString leafNameFromFolderPath(const QString& value)
    {
        const QString normalized = normalizeFolderPath(value);
        if (normalized.isEmpty())
        {
            return {};
        }

        const int slashIndex = normalized.lastIndexOf(QLatin1Char('/'));
        if (slashIndex < 0)
        {
            return normalized;
        }

        return normalized.mid(slashIndex + 1);
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

    bool isProtectedRootItem(const LibraryHierarchyItem& item);

    struct FolderHierarchyLookup final
    {
        QHash<QString, QStringList> pathKeysByLeafKey;
        QHash<QString, QSet<QString>> ancestorLeafKeysByPathKey;
        QHash<QString, QString> displayPathByPathKey;
    };

    QStringList canonicalLeafFolderPaths(const QStringList& folderPaths);

    QStringList folderPathSegments(const QString& folderPath)
    {
        const QString normalized = normalizeFolderPath(folderPath);
        if (normalized.isEmpty())
        {
            return {};
        }
        return normalized.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    }

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
            if (pathKey.isEmpty())
            {
                continue;
            }

            lookup.displayPathByPathKey.insert(pathKey, normalizeFolderPath(item.folderPath));

            QString leafKey = normalizeFolderLookupKey(item.label);
            if (leafKey.isEmpty())
            {
                leafKey = normalizeFolderLookupKey(leafNameFromFolderPath(item.folderPath));
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

            const QStringList segments = folderPathSegments(item.folderPath);
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

    QStringList resolvedNoteFolderPathKeys(
        const LibraryNoteRecord& note,
        const FolderHierarchyLookup& lookup)
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

    QStringList canonicalNoteFolderLabels(
        const LibraryNoteRecord& note,
        const FolderHierarchyLookup* lookup)
    {
        QStringList folders;
        if (lookup != nullptr)
        {
            const QStringList resolvedFolderPathKeys = canonicalLeafFolderPaths(
                resolvedNoteFolderPathKeys(note, *lookup));
            folders.reserve(resolvedFolderPathKeys.size());
            for (const QString& folderPathKey : resolvedFolderPathKeys)
            {
                const QString normalizedFolderPath = normalizeFolderPath(
                    lookup->displayPathByPathKey.value(folderPathKey, folderPathKey));
                if (!normalizedFolderPath.isEmpty() && !folders.contains(normalizedFolderPath))
                {
                    folders.push_back(normalizedFolderPath);
                }
            }
        }

        if (folders.isEmpty())
        {
            folders = noteListFolders(note);
        }
        return folders;
    }

    bool noteMatchesFolderScope(
        const LibraryNoteRecord& note,
        const QString& selectedPathKey,
        const FolderHierarchyLookup& lookup)
    {
        if (selectedPathKey.isEmpty())
        {
            return true;
        }

        const QStringList resolvedFolderPathKeys = resolvedNoteFolderPathKeys(note, lookup);
        for (const QString& folderPathKey : resolvedFolderPathKeys)
        {
            if (folderPathKey == selectedPathKey)
            {
                return true;
            }
        }

        return false;
    }

    QString currentNoteTimestamp()
    {
        return QDateTime::currentDateTime().toString(QString::fromLatin1(kNoteTimestampFormat));
    }

    QStringList noteIdsFromRecords(const QVector<LibraryNoteRecord>& notes)
    {
        QStringList noteIds;
        noteIds.reserve(notes.size());

        for (const LibraryNoteRecord& note : notes)
        {
            const QString noteId = note.noteId.trimmed();
            if (!noteId.isEmpty())
            {
                noteIds.push_back(noteId);
            }
        }

        return noteIds;
    }

    QString randomAlphaNumericSegment(int length)
    {
        static const QString upperAlphabet = QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        static const QString lowerAlphabet = QStringLiteral("abcdefghijklmnopqrstuvwxyz");
        static const QString digits = QStringLiteral("0123456789");
        static const QString alphaNumericAlphabet = upperAlphabet + lowerAlphabet + digits;

        if (length <= 0)
        {
            return {};
        }

        QString segment;
        segment.reserve(length);
        segment.push_back(upperAlphabet.at(QRandomGenerator::global()->bounded(upperAlphabet.size())));
        if (length > 1)
        {
            segment.push_back(lowerAlphabet.at(QRandomGenerator::global()->bounded(lowerAlphabet.size())));
        }
        if (length > 2)
        {
            segment.push_back(digits.at(QRandomGenerator::global()->bounded(digits.size())));
        }
        for (int index = segment.size(); index < length; ++index)
        {
            segment.push_back(
                alphaNumericAlphabet.at(QRandomGenerator::global()->bounded(alphaNumericAlphabet.size())));
        }

        for (int index = segment.size() - 1; index > 0; --index)
        {
            const int swapIndex = QRandomGenerator::global()->bounded(index + 1);
            if (swapIndex != index)
            {
                const QChar currentValue = segment.at(index);
                segment[index] = segment.at(swapIndex);
                segment[swapIndex] = currentValue;
            }
        }

        return segment;
    }

    QString createUniqueNoteId(
        const QString& libraryPath,
        const QVector<LibraryNoteRecord>& existingNotes)
    {
        QSet<QString> existingKeys;
        existingKeys.reserve(existingNotes.size());
        for (const LibraryNoteRecord& note : existingNotes)
        {
            const QString noteIdKey = note.noteId.trimmed().toCaseFolded();
            if (!noteIdKey.isEmpty())
            {
                existingKeys.insert(noteIdKey);
            }
        }

        const QDir libraryDir(libraryPath);
        for (int attempt = 0; attempt < 4096; ++attempt)
        {
            const QString candidate = randomAlphaNumericSegment(16) + QLatin1Char('-')
                + randomAlphaNumericSegment(16);
            const QString candidateKey = candidate.toCaseFolded();
            if (existingKeys.contains(candidateKey))
            {
                continue;
            }

            if (libraryDir.exists(candidate + QStringLiteral(".wsnote")))
            {
                continue;
            }

            return candidate;
        }

        return {};
    }

    QString resolvePrimaryLibraryPathFromWshub(
        const QString& wshubPath,
        QString* errorMessage = nullptr)
    {
        QStringList contentsDirectories;
        if (!WhatSon::Hierarchy::LibrarySupport::resolveContentsDirectories(
            wshubPath,
            &contentsDirectories,
            errorMessage))
        {
            return {};
        }

        for (const QString& contentsDirectory : std::as_const(contentsDirectories))
        {
            const QString fixedLibraryPath = QDir(contentsDirectory).filePath(QStringLiteral("Library.wslibrary"));
            if (QFileInfo(fixedLibraryPath).isDir())
            {
                return QDir::cleanPath(fixedLibraryPath);
            }

            const QDir contentsDir(contentsDirectory);
            const QStringList dynamicLibraries = contentsDir.entryList(
                QStringList{QStringLiteral("*.wslibrary")},
                QDir::Dirs | QDir::NoDotAndDotDot,
                QDir::Name);
            if (!dynamicLibraries.isEmpty())
            {
                return QDir::cleanPath(contentsDir.filePath(dynamicLibraries.first()));
            }
        }

        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("No Library.wslibrary directory found inside: %1").arg(wshubPath);
        }
        return {};
    }

    QString resolveHubStatPathFromWshub(const QString& wshubPath)
    {
        const QString normalizedWshubPath = WhatSon::Hierarchy::LibrarySupport::normalizePath(wshubPath);
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

    bool ensureDirectoryPath(const QString& directoryPath, QString* errorMessage = nullptr)
    {
        if (directoryPath.trimmed().isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Directory path must not be empty.");
            }
            return false;
        }

        QDir directory;
        if (directory.mkpath(directoryPath))
        {
            return true;
        }

        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to create directory: %1").arg(directoryPath);
        }
        return false;
    }

    QString createAttachmentManifestText(const QString& noteId)
    {
        return QStringLiteral(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<!DOCTYPE WHATSONNOTEPAINT>\n"
                "<contents id=\"%1\">\n"
                "  <body>\n"
                "  </body>\n"
                "</contents>\n")
            .arg(noteId);
    }

    QString createLinkManifestText(const QString& noteId, const QString& schema)
    {
        QJsonObject root;
        root.insert(QStringLiteral("version"), 1);
        root.insert(QStringLiteral("schema"), schema);
        root.insert(QStringLiteral("noteId"), noteId);
        root.insert(QStringLiteral("links"), QJsonArray{});
        return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
    }

    bool writeUtf8File(const QString& filePath, const QString& text, QString* errorMessage = nullptr)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to write file: %1").arg(filePath);
            }
            return false;
        }

        if (file.write(text.toUtf8()) < 0)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to write file bytes: %1").arg(filePath);
            }
            return false;
        }
        return true;
    }

    QString resolveNoteHeaderPath(const LibraryNoteRecord& note)
    {
        const QString directPath = WhatSon::Hierarchy::LibrarySupport::normalizePath(note.noteHeaderPath);
        if (!directPath.isEmpty() && QFileInfo(directPath).isFile())
        {
            return directPath;
        }

        const QString noteDirectoryPath = WhatSon::Hierarchy::LibrarySupport::normalizePath(note.noteDirectoryPath);
        if (noteDirectoryPath.isEmpty())
        {
            return {};
        }

        const QDir noteDir(noteDirectoryPath);
        if (!noteDir.exists())
        {
            return {};
        }

        const QString noteStem = QFileInfo(noteDirectoryPath).completeBaseName().trimmed();
        if (!noteStem.isEmpty())
        {
            const QString stemHeaderPath = noteDir.filePath(noteStem + QStringLiteral(".wsnhead"));
            if (QFileInfo(stemHeaderPath).isFile())
            {
                return QDir::cleanPath(stemHeaderPath);
            }
        }

        const QString canonicalHeaderPath = noteDir.filePath(QStringLiteral("note.wsnhead"));
        if (QFileInfo(canonicalHeaderPath).isFile())
        {
            return QDir::cleanPath(canonicalHeaderPath);
        }

        const QFileInfoList headerCandidates = noteDir.entryInfoList(
            QStringList{QStringLiteral("*.wsnhead")},
            QDir::Files,
            QDir::Name);
        QString draftHeaderPath;
        for (const QFileInfo& fileInfo : headerCandidates)
        {
            const QString loweredName = fileInfo.fileName().toCaseFolded();
            if (loweredName.contains(QStringLiteral(".draft.")))
            {
                if (draftHeaderPath.isEmpty())
                {
                    draftHeaderPath = fileInfo.absoluteFilePath();
                }
                continue;
            }
            return QDir::cleanPath(fileInfo.absoluteFilePath());
        }

        if (!draftHeaderPath.isEmpty())
        {
            return QDir::cleanPath(draftHeaderPath);
        }

        return {};
    }

    int indexOfNoteRecordById(const QVector<LibraryNoteRecord>& notes, const QString& noteId)
    {
        const QString normalizedNoteId = noteId.trimmed();
        if (normalizedNoteId.isEmpty())
        {
            return -1;
        }

        for (int index = 0; index < notes.size(); ++index)
        {
            if (notes.at(index).noteId.trimmed() == normalizedNoteId)
            {
                return index;
            }
        }

        return -1;
    }

    QStringList folderAssignmentForDrop(const QString& folderPath)
    {
        const QString normalizedFolderPath = normalizeFolderPath(folderPath);
        if (normalizedFolderPath.isEmpty())
        {
            return {};
        }

        return {normalizedFolderPath};
    }

    void applyChevronByDepth(QVector<LibraryHierarchyItem>* items)
    {
        if (items == nullptr)
        {
            return;
        }

        for (int index = 0; index < items->size(); ++index)
        {
            const int nextIndex = index + 1;
            const bool hasChild = nextIndex < items->size()
                && items->at(nextIndex).depth > items->at(index).depth;
            (*items)[index].showChevron = hasChild;
        }
    }

    bool isProtectedRootItem(const LibraryHierarchyItem& item)
    {
        return item.systemBucket != LibraryHierarchyItem::SystemBucket::None
            || (item.accent && item.depth == 0);
    }

    bool isSystemBucketItem(const LibraryHierarchyItem& item)
    {
        return item.systemBucket != LibraryHierarchyItem::SystemBucket::None;
    }

    LibraryHierarchyItem makeSystemBucketItem(
        const QString& label,
        LibraryHierarchyItem::SystemBucket systemBucket)
    {
        LibraryHierarchyItem item;
        item.depth = 0;
        item.accent = true;
        item.expanded = false;
        item.label = label;
        item.systemBucket = systemBucket;
        item.showChevron = false;
        return item;
    }

    QVector<LibraryHierarchyItem> prependSystemBuckets(QVector<LibraryHierarchyItem> items)
    {
        QVector<LibraryHierarchyItem> combined;
        combined.reserve(3 + items.size());
        combined.push_back(
            makeSystemBucketItem(QLatin1String(kLibraryAllLabel), LibraryHierarchyItem::SystemBucket::All));
        combined.push_back(
            makeSystemBucketItem(QLatin1String(kLibraryDraftLabel), LibraryHierarchyItem::SystemBucket::Draft));
        combined.push_back(
            makeSystemBucketItem(QLatin1String(kLibraryTodayLabel), LibraryHierarchyItem::SystemBucket::Today));

        for (LibraryHierarchyItem& item : items)
        {
            combined.push_back(std::move(item));
        }

        return combined;
    }

    QVector<LibraryHierarchyItem> buildFolderItems(const QVector<WhatSonFolderDepthEntry>& entries)
    {
        QVector<LibraryHierarchyItem> items;
        items.reserve(entries.size());

        for (const WhatSonFolderDepthEntry& entry : entries)
        {
            LibraryHierarchyItem item;
            item.depth = std::max(0, entry.depth);
            item.label = entry.label.trimmed();
            item.accent = false;
            item.expanded = false;
            item.folderPath = normalizeFolderPath(entry.id);
            item.showChevron = true;

            if (item.label.isEmpty())
            {
                WhatSon::Debug::trace(
                    QStringLiteral("library.viewmodel"),
                    QStringLiteral("buildFolderItems.emptyLabelKept"));
            }

            items.push_back(std::move(item));
        }

        applyChevronByDepth(&items);
        return items;
    }

    void finalizeFolderItems(QVector<LibraryHierarchyItem>* items, bool preserveExistingPaths)
    {
        if (items == nullptr)
        {
            return;
        }

        QStringList pathStack;
        for (LibraryHierarchyItem& item : *items)
        {
            item.label = item.label.trimmed();

            if (item.systemBucket != LibraryHierarchyItem::SystemBucket::None)
            {
                item.depth = 0;
                item.accent = true;
                item.folderPath.clear();
                continue;
            }

            int depth = std::max(0, item.depth);
            if (depth > pathStack.size())
            {
                depth = pathStack.size();
            }
            while (pathStack.size() > depth)
            {
                pathStack.removeLast();
            }
            item.depth = depth;

            const QString parentPath = (depth > 0 && !pathStack.isEmpty()) ? pathStack.constLast() : QString();
            QString folderPath = preserveExistingPaths ? normalizeFolderPath(item.folderPath) : QString();
            if (folderPath.isEmpty())
            {
                folderPath = buildFolderPath(parentPath, item.label);
            }
            else if (!parentPath.isEmpty()
                && folderPath != parentPath
                && !folderPath.startsWith(parentPath + QLatin1Char('/')))
            {
                folderPath = buildFolderPath(parentPath, leafNameFromFolderPath(folderPath));
            }
            item.folderPath = folderPath;

            if (pathStack.size() <= depth)
            {
                pathStack.push_back(item.folderPath);
            }
            else
            {
                pathStack[depth] = item.folderPath;
                pathStack = pathStack.mid(0, depth + 1);
            }
        }

        applyChevronByDepth(items);
    }

    QVector<WhatSonFolderDepthEntry> folderEntriesFromItems(const QVector<LibraryHierarchyItem>& items)
    {
        QVector<WhatSonFolderDepthEntry> entries;
        entries.reserve(items.size());

        for (const LibraryHierarchyItem& item : items)
        {
            const QString label = item.label.trimmed();
            if (label.isEmpty())
            {
                continue;
            }
            if (isProtectedRootItem(item))
            {
                continue;
            }

            WhatSonFolderDepthEntry entry;
            entry.id = normalizeFolderPath(item.folderPath);
            entry.label = label;
            entry.depth = std::max(0, item.depth);
            if (entry.id.isEmpty())
            {
                entry.id = label;
            }
            entries.push_back(std::move(entry));
        }

        return entries;
    }

    int subtreeEndIndexExclusive(const QVector<LibraryHierarchyItem>& items, int startIndex)
    {
        if (startIndex < 0 || startIndex >= items.size())
        {
            return startIndex;
        }

        const int baseDepth = items.at(startIndex).depth;
        int endIndex = startIndex + 1;
        while (endIndex < items.size() && items.at(endIndex).depth > baseDepth)
        {
            ++endIndex;
        }
        return endIndex;
    }

    bool indexInsideSubtree(int index, int subtreeStart, int subtreeEndExclusive)
    {
        return index >= subtreeStart && index < subtreeEndExclusive;
    }

    enum class FolderDropPlacement
    {
        Before,
        After,
        Child,
        RootTop
    };

    struct FolderMoveOperation final
    {
        int sourceEndIndex = -1;
        int sourceCount = 0;
        int normalizedInsertIndex = -1;
        int newBaseDepth = 0;
    };

    bool isEditableFolderItem(const QVector<LibraryHierarchyItem>& items, int index)
    {
        return index >= 0 && index < items.size() && !isProtectedRootItem(items.at(index));
    }

    bool resolveFolderMoveOperation(
        const QVector<LibraryHierarchyItem>& items,
        int firstEditableInsertIndex,
        int sourceIndex,
        int targetIndex,
        FolderDropPlacement placement,
        FolderMoveOperation* outOperation = nullptr)
    {
        if (!isEditableFolderItem(items, sourceIndex))
        {
            return false;
        }

        const int sourceEndIndex = subtreeEndIndexExclusive(items, sourceIndex);
        const int sourceCount = sourceEndIndex - sourceIndex;
        if (sourceCount <= 0)
        {
            return false;
        }

        int rawInsertIndex = firstEditableInsertIndex;
        int newBaseDepth = 0;

        switch (placement)
        {
        case FolderDropPlacement::RootTop:
            rawInsertIndex = firstEditableInsertIndex;
            newBaseDepth = 0;
            break;
        case FolderDropPlacement::Before:
            if (!isEditableFolderItem(items, targetIndex) || sourceIndex == targetIndex)
            {
                return false;
            }
            if (indexInsideSubtree(targetIndex, sourceIndex, sourceEndIndex))
            {
                return false;
            }
            rawInsertIndex = targetIndex;
            newBaseDepth = items.at(targetIndex).depth;
            break;
        case FolderDropPlacement::After:
            if (!isEditableFolderItem(items, targetIndex) || sourceIndex == targetIndex)
            {
                return false;
            }
            if (indexInsideSubtree(targetIndex, sourceIndex, sourceEndIndex))
            {
                return false;
            }
            rawInsertIndex = subtreeEndIndexExclusive(items, targetIndex);
            newBaseDepth = items.at(targetIndex).depth;
            break;
        case FolderDropPlacement::Child:
            if (!isEditableFolderItem(items, targetIndex) || sourceIndex == targetIndex)
            {
                return false;
            }
            if (indexInsideSubtree(targetIndex, sourceIndex, sourceEndIndex))
            {
                return false;
            }
            rawInsertIndex = subtreeEndIndexExclusive(items, targetIndex);
            newBaseDepth = items.at(targetIndex).depth + 1;
            break;
        }

        int normalizedInsertIndex = rawInsertIndex;
        if (normalizedInsertIndex > sourceIndex)
        {
            normalizedInsertIndex -= sourceCount;
        }
        const int maxInsertIndex = std::max(0, static_cast<int>(items.size()) - sourceCount);
        normalizedInsertIndex = std::clamp(normalizedInsertIndex, 0, maxInsertIndex);

        if (normalizedInsertIndex == sourceIndex && newBaseDepth == items.at(sourceIndex).depth)
        {
            return false;
        }

        if (outOperation != nullptr)
        {
            outOperation->sourceEndIndex = sourceEndIndex;
            outOperation->sourceCount = sourceCount;
            outOperation->normalizedInsertIndex = normalizedInsertIndex;
            outOperation->newBaseDepth = newBaseDepth;
        }
        return true;
    }

    QVector<LibraryHierarchyItem> stageFolderMoveItems(
        const QVector<LibraryHierarchyItem>& items,
        int sourceIndex,
        const FolderMoveOperation& operation)
    {
        const int depthDelta = operation.newBaseDepth - items.at(sourceIndex).depth;

        QVector<LibraryHierarchyItem> movedItems;
        movedItems.reserve(operation.sourceCount);
        for (int index = sourceIndex; index < operation.sourceEndIndex; ++index)
        {
            LibraryHierarchyItem item = items.at(index);
            item.depth = std::max(0, item.depth + depthDelta);
            movedItems.push_back(std::move(item));
        }

        QVector<LibraryHierarchyItem> stagedItems = items;
        stagedItems.remove(sourceIndex, operation.sourceCount);

        for (int offset = 0; offset < movedItems.size(); ++offset)
        {
            stagedItems.insert(operation.normalizedInsertIndex + offset, std::move(movedItems[offset]));
        }

        finalizeFolderItems(&stagedItems, false);
        return stagedItems;
    }

    QHash<QString, QString> movedFolderPathMapForOperation(
        const QVector<LibraryHierarchyItem>& originalItems,
        int sourceIndex,
        const FolderMoveOperation& operation,
        const QVector<LibraryHierarchyItem>& stagedItems)
    {
        QHash<QString, QString> movedPathMap;
        for (int offset = 0; offset < operation.sourceCount; ++offset)
        {
            const QString sourcePath = normalizeFolderPath(originalItems.at(sourceIndex + offset).folderPath);
            const QString targetPath = normalizeFolderPath(
                stagedItems.at(operation.normalizedInsertIndex + offset).folderPath);
            if (sourcePath.isEmpty() || targetPath.isEmpty() || sourcePath == targetPath)
            {
                continue;
            }
            movedPathMap.insert(normalizeFolderLookupKey(sourcePath), targetPath);
        }
        return movedPathMap;
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

        return suffix.isEmpty()
                   ? replacement
                   : buildFolderPath(replacement, suffix);
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
} // namespace

LibraryHierarchyViewModel::LibraryHierarchyViewModel(QObject* parent)
    : QObject(parent)
      , m_itemModel(this)
      , m_noteListModel(this)
{
    WhatSon::Debug::traceSelf(this, QStringLiteral("library.viewmodel"), QStringLiteral("ctor"));
    QObject::connect(
        &m_itemModel,
        &LibraryHierarchyModel::itemCountChanged,
        this,
        [this](int)
        {
            updateItemCount();
            setSelectedIndex(m_selectedIndex);
        });
    QObject::connect(
        &m_noteListModel,
        &LibraryNoteListModel::itemCountChanged,
        this,
        [this](int)
        {
            updateNoteItemCount();
        });
    syncModel();
    refreshNoteListForSelection();
}

LibraryHierarchyViewModel::~LibraryHierarchyViewModel() = default;

LibraryHierarchyModel* LibraryHierarchyViewModel::itemModel() noexcept
{
    return &m_itemModel;
}

LibraryNoteListModel* LibraryHierarchyViewModel::noteListModel() noexcept
{
    return &m_noteListModel;
}

void LibraryHierarchyViewModel::setSystemCalendarStore(SystemCalendarStore* store)
{
    if (m_systemCalendarStore == store)
    {
        return;
    }

    if (m_systemCalendarStoreChangedConnection)
    {
        QObject::disconnect(m_systemCalendarStoreChangedConnection);
    }

    m_systemCalendarStore = store;
    if (m_systemCalendarStore)
    {
        m_systemCalendarStoreChangedConnection = QObject::connect(
            m_systemCalendarStore,
            &SystemCalendarStore::systemInfoChanged,
            this,
            [this]()
            {
                refreshNoteListForSelection();
            });
    }
    else
    {
        m_systemCalendarStoreChangedConnection = {};
    }

    refreshNoteListForSelection();
}

SystemCalendarStore* LibraryHierarchyViewModel::systemCalendarStore() const noexcept
{
    return m_systemCalendarStore;
}

int LibraryHierarchyViewModel::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

int LibraryHierarchyViewModel::itemCount() const noexcept
{
    return m_itemCount;
}

int LibraryHierarchyViewModel::noteItemCount() const noexcept
{
    return m_noteItemCount;
}

bool LibraryHierarchyViewModel::loadSucceeded() const noexcept
{
    return m_loadSucceeded;
}

QString LibraryHierarchyViewModel::lastLoadError() const
{
    return m_lastLoadError;
}

void LibraryHierarchyViewModel::setSelectedIndex(int index)
{
    applySelectedIndex(index, false);
}

void LibraryHierarchyViewModel::applySelectedIndex(int index, bool forceReapply)
{
    const int maxIndex = m_itemModel.rowCount() - 1;
    int clamped = index;
    if (maxIndex < 0)
    {
        clamped = -1;
    }
    else
    {
        clamped = std::clamp(index, -1, maxIndex);
    }

    if (m_selectedIndex == clamped)
    {
        if (!forceReapply)
        {
            return;
        }

        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("reapplySelectedIndex"),
                                  QStringLiteral("value=%1").arg(m_selectedIndex));
        refreshNoteListForSelection();
        emit selectedIndexChanged();
        return;
    }

    m_selectedIndex = clamped;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("setSelectedIndex"),
                              QStringLiteral("value=%1").arg(m_selectedIndex));
    refreshNoteListForSelection();
    emit selectedIndexChanged();
}

void LibraryHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("setDepthItems.begin"),
                              QStringLiteral("count=%1").arg(depthItems.size()));

    if (depthItems.isEmpty() && m_runtimeIndexLoaded)
    {
        if (m_foldersHierarchyLoaded)
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("library.viewmodel"),
                                      QStringLiteral("setDepthItems.keepFoldersHierarchy"),
                                      QStringLiteral("folderCount=%1").arg(m_items.size()));
            refreshNoteListForSelection();
            return;
        }

        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("setDepthItems.useIndexedBuckets"),
                                  QStringLiteral("all=%1 draft=%2 today=%3")
                                  .arg(m_libraryAll.notes().size())
                                  .arg(m_libraryDraft.notes().size())
                                  .arg(m_libraryToday.notes().size()));
        applyIndexedBuckets();
        setSelectedIndex(-1);
        return;
    }

    QVector<LibraryHierarchyItem> parsedItems;
    parsedItems.reserve(depthItems.size());

    int ordinal = 1;
    for (const QVariant& entry : depthItems)
    {
        parsedItems.push_back(parseItem(entry, ordinal));
        ++ordinal;
    }

    m_items = m_runtimeIndexLoaded ? prependSystemBuckets(std::move(parsedItems)) : std::move(parsedItems);
    finalizeFolderItems(&m_items, true);
    m_foldersHierarchyLoaded = m_runtimeIndexLoaded;
    rebuildBucketRanges();
    m_createdFolderSequence = nextFolderSequence(m_items);
    syncModel();
    setSelectedIndex(-1);
    if (m_runtimeIndexLoaded)
    {
        refreshNoteListForSelection();
    }
    else
    {
        m_noteListModel.setItems({});
        updateNoteItemCount();
    }
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("setDepthItems.success"),
                              QStringLiteral("itemCount=%1 nextFolderSeq=%2 foldersLoaded=%3")
                              .arg(m_items.size())
                              .arg(m_createdFolderSequence)
                              .arg(m_foldersHierarchyLoaded ? QStringLiteral("1") : QStringLiteral("0")));
}

bool LibraryHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    m_foldersFilePath.clear();

    QString indexError;
    if (!m_libraryAll.indexFromWshub(wshubPath, &indexError))
    {
        m_libraryDraft.clear();
        m_libraryToday.clear();
        m_runtimeIndexLoaded = false;
        m_foldersHierarchyLoaded = false;
        if (errorMessage != nullptr)
        {
            *errorMessage = indexError;
        }
        m_noteListModel.setItems({});
        updateNoteItemCount();
        updateLoadState(false, indexError);
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("loadFromWshub.failed"),
                                  QStringLiteral("path=%1 reason=%2").arg(wshubPath, indexError));
        return false;
    }

    m_libraryDraft.rebuild(m_libraryAll.notes());
    m_libraryToday.rebuild(m_libraryAll.notes());
    m_runtimeIndexLoaded = true;

    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::LibrarySupport::resolveContentsDirectories(wshubPath, &contentsDirectories, &resolveError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = resolveError;
        }
        updateLoadState(false, resolveError);
        return false;
    }

    WhatSonProjectsHierarchyParser foldersParser;
    QVector<WhatSonFolderDepthEntry> folderEntries;
    bool foldersFileFound = false;

    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Folders.wsfolders"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }

        foldersFileFound = true;
        if (m_foldersFilePath.isEmpty())
        {
            m_foldersFilePath = filePath;
        }

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::LibrarySupport::readUtf8File(filePath, &rawText, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            updateLoadState(false, readError);
            return false;
        }

        WhatSonProjectsHierarchyStore foldersStore;
        QString parseError;
        if (!foldersParser.parse(rawText, &foldersStore, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            updateLoadState(false, parseError);
            return false;
        }

        const QVector<WhatSonFolderDepthEntry> parsedEntries = foldersStore.folderEntries();
        for (const WhatSonFolderDepthEntry& entry : parsedEntries)
        {
            folderEntries.push_back(entry);
        }
    }

    if (m_foldersFilePath.isEmpty() && !contentsDirectories.isEmpty())
    {
        m_foldersFilePath = QDir(contentsDirectories.first()).filePath(QStringLiteral("Folders.wsfolders"));
    }

    if (!folderEntries.isEmpty())
    {
        m_items = prependSystemBuckets(buildFolderItems(folderEntries));
        finalizeFolderItems(&m_items, true);
        m_foldersHierarchyLoaded = true;
        rebuildBucketRanges();
        m_createdFolderSequence = nextFolderSequence(m_items);
        syncModel();
        setSelectedIndex(-1);
        refreshNoteListForSelection();

        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("loadFromWshub.folderHierarchy"),
                                  QStringLiteral("path=%1 folderCount=%2 all=%3 draft=%4 today=%5")
                                  .arg(wshubPath)
                                  .arg(m_items.size())
                                  .arg(m_libraryAll.notes().size())
                                  .arg(m_libraryDraft.notes().size())
                                  .arg(m_libraryToday.notes().size()));
        updateLoadState(true);
        return true;
    }

    applyIndexedBuckets();
    setSelectedIndex(-1);

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("loadFromWshub.success"),
                              QStringLiteral("foldersFileFound=%1 all=%2 draft=%3 today=%4")
                              .arg(foldersFileFound ? QStringLiteral("1") : QStringLiteral("0"))
                              .arg(m_libraryAll.notes().size())
                              .arg(m_libraryDraft.notes().size())
                              .arg(m_libraryToday.notes().size()));
    updateLoadState(true);
    return true;
}

void LibraryHierarchyViewModel::applyRuntimeSnapshot(
    const QString& wshubPath,
    QVector<LibraryNoteRecord> allNotes,
    QVector<LibraryNoteRecord> draftNotes,
    QVector<LibraryNoteRecord> todayNotes,
    QVector<WhatSonFolderDepthEntry> folderEntries,
    QString foldersFilePath,
    bool loadSucceeded,
    QString errorMessage)
{
    m_foldersFilePath = foldersFilePath.trimmed();

    if (!loadSucceeded)
    {
        m_libraryAll.clear();
        m_libraryDraft.clear();
        m_libraryToday.clear();
        m_runtimeIndexLoaded = false;
        m_foldersHierarchyLoaded = false;
        m_items.clear();
        m_bucketRanges.clear();
        syncModel();
        m_noteListModel.setItems({});
        updateNoteItemCount();
        updateLoadState(false, errorMessage);
        return;
    }

    m_libraryAll.setIndexedNotes(wshubPath, std::move(allNotes));
    m_libraryDraft.setNotes(std::move(draftNotes));
    m_libraryToday.setNotes(std::move(todayNotes));
    m_runtimeIndexLoaded = true;

    if (!folderEntries.isEmpty())
    {
        m_items = prependSystemBuckets(buildFolderItems(folderEntries));
        finalizeFolderItems(&m_items, true);
        m_foldersHierarchyLoaded = true;
        rebuildBucketRanges();
        m_createdFolderSequence = nextFolderSequence(m_items);
        syncModel();
        setSelectedIndex(-1);
        refreshNoteListForSelection();
    }
    else
    {
        applyIndexedBuckets();
        setSelectedIndex(-1);
    }

    updateLoadState(true);
}

QVariantList LibraryHierarchyViewModel::depthItems() const
{
    QVariantList serializedItems;
    serializedItems.reserve(m_items.size());
    for (int index = 0; index < m_items.size(); ++index)
    {
        const LibraryHierarchyItem& item = m_items.at(index);
        QString itemKey = normalizeFolderPath(item.folderPath);
        if (item.systemBucket == LibraryHierarchyItem::SystemBucket::All)
        {
            itemKey = QStringLiteral("bucket:all");
        }
        else if (item.systemBucket == LibraryHierarchyItem::SystemBucket::Draft)
        {
            itemKey = QStringLiteral("bucket:draft");
        }
        else if (item.systemBucket == LibraryHierarchyItem::SystemBucket::Today)
        {
            itemKey = QStringLiteral("bucket:today");
        }
        else if (itemKey.isEmpty())
        {
            itemKey = QStringLiteral("item:%1").arg(index);
        }

        serializedItems.push_back(QVariantMap{
            {"label", item.label},
            {"id", item.folderPath},
            {"itemId", index},
            {"key", itemKey},
            {"depth", item.depth},
            {"accent", item.accent},
            {"expanded", item.expanded},
            {"draggable", canMoveFolder(index)},
            {"showChevron", item.showChevron}
        });
    }
    return serializedItems;
}

QVariantList LibraryHierarchyViewModel::hierarchyModel() const
{
    return depthItems();
}

QString LibraryHierarchyViewModel::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    return m_items.at(index).label;
}

bool LibraryHierarchyViewModel::canRenameItem(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return false;
    }

    const LibraryHierarchyItem& item = m_items.at(index);
    if (isProtectedRootItem(item))
    {
        return false;
    }

    if (m_runtimeIndexLoaded && !m_foldersHierarchyLoaded)
    {
        return false;
    }

    return true;
}

bool LibraryHierarchyViewModel::renameItem(int index, const QString& displayName)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("renameItem.begin"),
                              QStringLiteral("index=%1 label=%2").arg(index).arg(displayName));
    if (!canRenameItem(index))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("renameItem.rejected"),
                                  QStringLiteral("reason=canRenameItem false index=%1").arg(index));
        return false;
    }

    const QString trimmedName = displayName.trimmed();
    if (trimmedName.isEmpty())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("renameItem.rejected"),
                                  QStringLiteral("reason=empty label index=%1").arg(index));
        return false;
    }

    if (m_items.at(index).label == trimmedName)
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("renameItem.skipped"),
                                  QStringLiteral("reason=same label index=%1").arg(index));
        return true;
    }

    QVector<LibraryHierarchyItem> stagedItems = m_items;
    stagedItems[index].label = trimmedName;
    finalizeFolderItems(&stagedItems, false);

    WhatSonProjectsHierarchyStore stagedStore;
    stagedStore.setFolderEntries(folderEntriesFromItems(stagedItems));

    if (!m_foldersFilePath.trimmed().isEmpty())
    {
        QString writeError;
        if (!stagedStore.writeToFile(m_foldersFilePath, &writeError))
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("library.viewmodel"),
                                      QStringLiteral("renameItem.writeFailed"),
                                      QStringLiteral("index=%1 path=%2 reason=%3").arg(index).arg(
                                          m_foldersFilePath, writeError));
            return false;
        }
    }

    m_items = std::move(stagedItems);
    syncModel();
    applySelectedIndex(m_selectedIndex, true);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("renameItem"),
                              QStringLiteral("index=%1 label=%2").arg(index).arg(trimmedName));
    return true;
}

bool LibraryHierarchyViewModel::setItemExpanded(int index, bool expanded)
{
    if (index < 0 || index >= m_items.size())
    {
        return false;
    }

    if (isProtectedRootItem(m_items.at(index)))
    {
        return false;
    }

    if (m_items.at(index).expanded == expanded)
    {
        return true;
    }

    m_items[index].expanded = expanded;
    m_itemModel.setItemExpanded(index, expanded);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("setItemExpanded"),
                              QStringLiteral("index=%1 expanded=%2")
                              .arg(index)
                              .arg(expanded ? QStringLiteral("true") : QStringLiteral("false")));
    return true;
}

bool LibraryHierarchyViewModel::renameEnabled() const noexcept
{
    return true;
}

bool LibraryHierarchyViewModel::createFolderEnabled() const noexcept
{
    return true;
}

bool LibraryHierarchyViewModel::deleteFolderEnabled() const noexcept
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        return false;
    }

    return !isProtectedRootItem(m_items.at(m_selectedIndex));
}

void LibraryHierarchyViewModel::createFolder()
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("createFolder.begin"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2").arg(m_selectedIndex).arg(m_items.size()));
    int insertIndex = m_items.size();
    int folderDepth = 0;
    int expandedParentIndex = -1;

    if (m_selectedIndex >= 0 && m_selectedIndex < m_items.size())
    {
        if (isProtectedRootItem(m_items.at(m_selectedIndex)))
        {
            insertIndex = firstEditableInsertIndex();
        }
        else
        {
            const int selectedDepth = m_items.at(m_selectedIndex).depth;
            folderDepth = selectedDepth + 1;
            expandedParentIndex = m_selectedIndex;

            insertIndex = m_selectedIndex + 1;
            while (insertIndex < m_items.size() && m_items.at(insertIndex).depth > selectedDepth)
            {
                ++insertIndex;
            }
        }
    }

    const QString folderLabel = QStringLiteral("Untitled");
    ++m_createdFolderSequence;
    LibraryHierarchyItem newItem;
    newItem.depth = folderDepth;
    newItem.label = folderLabel;
    newItem.accent = false;
    newItem.expanded = false;
    newItem.showChevron = true;

    QVector<LibraryHierarchyItem> stagedItems = m_items;
    if (expandedParentIndex >= 0 && expandedParentIndex < stagedItems.size())
    {
        stagedItems[expandedParentIndex].expanded = true;
    }
    stagedItems.insert(insertIndex, std::move(newItem));
    if (insertIndex >= 0 && insertIndex < stagedItems.size())
    {
        stagedItems[insertIndex].label = folderLabel;
    }
    finalizeFolderItems(&stagedItems, false);

    WhatSonProjectsHierarchyStore stagedStore;
    stagedStore.setFolderEntries(folderEntriesFromItems(stagedItems));
    if (!m_foldersFilePath.trimmed().isEmpty())
    {
        QString writeError;
        if (!stagedStore.writeToFile(m_foldersFilePath, &writeError))
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("library.viewmodel"),
                                      QStringLiteral("createFolder.writeFailed"),
                                      QStringLiteral("insertIndex=%1 path=%2 reason=%3").arg(insertIndex).arg(
                                          m_foldersFilePath, writeError));
            return;
        }
    }

    m_items = std::move(stagedItems);
    m_foldersHierarchyLoaded = true;
    rebuildBucketRanges();
    syncModel();
    setSelectedIndex(insertIndex);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("createFolder"),
                              QStringLiteral("insertIndex=%1 depth=%2 itemCount=%3")
                              .arg(insertIndex)
                              .arg(folderDepth)
                              .arg(m_items.size()));
}

void LibraryHierarchyViewModel::deleteSelectedFolder()
{
    const int startIndex = m_selectedIndex;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("deleteSelectedFolder.begin"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2").arg(startIndex).arg(m_items.size()));
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("deleteSelectedFolder.rejected"),
                                  QStringLiteral("reason=selection out of range selectedIndex=%1").arg(startIndex));
        return;
    }

    if (isProtectedRootItem(m_items.at(m_selectedIndex)))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("deleteSelectedFolder.rejected"),
                                  QStringLiteral("reason=protected system bucket selectedIndex=%1").arg(startIndex));
        return;
    }

    const int baseDepth = m_items.at(startIndex).depth;

    int removeCount = 1;
    while (startIndex + removeCount < m_items.size()
        && m_items.at(startIndex + removeCount).depth > baseDepth)
    {
        ++removeCount;
    }

    QVector<LibraryHierarchyItem> stagedItems = m_items;
    stagedItems.remove(startIndex, removeCount);
    finalizeFolderItems(&stagedItems, false);

    WhatSonProjectsHierarchyStore stagedStore;
    stagedStore.setFolderEntries(folderEntriesFromItems(stagedItems));
    if (!m_foldersFilePath.trimmed().isEmpty())
    {
        QString writeError;
        if (!stagedStore.writeToFile(m_foldersFilePath, &writeError))
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("library.viewmodel"),
                                      QStringLiteral("deleteSelectedFolder.writeFailed"),
                                      QStringLiteral("startIndex=%1 path=%2 reason=%3").arg(startIndex).arg(
                                          m_foldersFilePath, writeError));
            return;
        }
    }

    m_items = std::move(stagedItems);
    m_foldersHierarchyLoaded = !folderEntriesFromItems(m_items).isEmpty();
    rebuildBucketRanges();
    syncModel();
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("deleteSelectedFolder"),
                              QStringLiteral("startIndex=%1 removeCount=%2 remaining=%3")
                              .arg(startIndex)
                              .arg(removeCount)
                              .arg(m_items.size()));

    if (m_items.isEmpty())
    {
        setSelectedIndex(-1);
        return;
    }

    applySelectedIndex(std::min(startIndex, static_cast<int>(m_items.size() - 1)), true);
}

bool LibraryHierarchyViewModel::canMoveFolder(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return false;
    }

    return !isProtectedRootItem(m_items.at(index));
}

bool LibraryHierarchyViewModel::canAcceptFolderDropBefore(int sourceIndex, int targetIndex) const
{
    return resolveFolderMoveOperation(
        m_items,
        firstEditableInsertIndex(),
        sourceIndex,
        targetIndex,
        FolderDropPlacement::Before,
        nullptr);
}

bool LibraryHierarchyViewModel::moveFolderBefore(int sourceIndex, int targetIndex)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("moveFolderBefore.begin"),
                              QStringLiteral("sourceIndex=%1 targetIndex=%2").arg(sourceIndex).arg(targetIndex));

    FolderMoveOperation operation;
    if (!resolveFolderMoveOperation(
        m_items,
        firstEditableInsertIndex(),
        sourceIndex,
        targetIndex,
        FolderDropPlacement::Before,
        &operation))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("moveFolderBefore.rejected"),
                                  QStringLiteral("sourceIndex=%1 targetIndex=%2").arg(sourceIndex).arg(targetIndex));
        return false;
    }

    QVector<LibraryHierarchyItem> stagedItems = stageFolderMoveItems(m_items, sourceIndex, operation);
    const QHash<QString, QString> movedPathMap = movedFolderPathMapForOperation(
        m_items,
        sourceIndex,
        operation,
        stagedItems);
    if (!commitFolderHierarchyUpdate(std::move(stagedItems), operation.normalizedInsertIndex, movedPathMap))
    {
        return false;
    }

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("moveFolderBefore.success"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2")
                              .arg(operation.normalizedInsertIndex)
                              .arg(m_items.size()));
    return true;
}

bool LibraryHierarchyViewModel::canAcceptFolderDrop(int sourceIndex, int targetIndex, bool asChild) const
{
    return resolveFolderMoveOperation(
        m_items,
        firstEditableInsertIndex(),
        sourceIndex,
        targetIndex,
        asChild ? FolderDropPlacement::Child : FolderDropPlacement::After,
        nullptr);
}

bool LibraryHierarchyViewModel::moveFolder(int sourceIndex, int targetIndex, bool asChild)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("moveFolder.begin"),
                              QStringLiteral("sourceIndex=%1 targetIndex=%2 asChild=%3")
                              .arg(sourceIndex)
                              .arg(targetIndex)
                              .arg(asChild ? QStringLiteral("1") : QStringLiteral("0")));

    FolderMoveOperation operation;
    if (!resolveFolderMoveOperation(
        m_items,
        firstEditableInsertIndex(),
        sourceIndex,
        targetIndex,
        asChild ? FolderDropPlacement::Child : FolderDropPlacement::After,
        &operation))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("moveFolder.rejected"),
                                  QStringLiteral("sourceIndex=%1 targetIndex=%2 asChild=%3")
                                  .arg(sourceIndex)
                                  .arg(targetIndex)
                                  .arg(asChild ? QStringLiteral("1") : QStringLiteral("0")));
        return false;
    }

    QVector<LibraryHierarchyItem> stagedItems = stageFolderMoveItems(m_items, sourceIndex, operation);
    const QHash<QString, QString> movedPathMap = movedFolderPathMapForOperation(
        m_items,
        sourceIndex,
        operation,
        stagedItems);
    if (!commitFolderHierarchyUpdate(std::move(stagedItems), operation.normalizedInsertIndex, movedPathMap))
    {
        return false;
    }

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("moveFolder.success"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2")
                              .arg(operation.normalizedInsertIndex)
                              .arg(m_items.size()));
    return true;
}

bool LibraryHierarchyViewModel::canMoveFolderToRoot(int sourceIndex) const
{
    return resolveFolderMoveOperation(
        m_items,
        firstEditableInsertIndex(),
        sourceIndex,
        -1,
        FolderDropPlacement::RootTop,
        nullptr);
}

bool LibraryHierarchyViewModel::moveFolderToRoot(int sourceIndex)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("moveFolderToRoot.begin"),
                              QStringLiteral("sourceIndex=%1").arg(sourceIndex));
    FolderMoveOperation operation;
    if (!resolveFolderMoveOperation(
        m_items,
        firstEditableInsertIndex(),
        sourceIndex,
        -1,
        FolderDropPlacement::RootTop,
        &operation))
    {
        return false;
    }

    QVector<LibraryHierarchyItem> stagedItems = stageFolderMoveItems(m_items, sourceIndex, operation);
    const QHash<QString, QString> movedPathMap = movedFolderPathMapForOperation(
        m_items,
        sourceIndex,
        operation,
        stagedItems);
    if (!commitFolderHierarchyUpdate(std::move(stagedItems), operation.normalizedInsertIndex, movedPathMap))
    {
        return false;
    }

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("moveFolderToRoot.success"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2")
                              .arg(operation.normalizedInsertIndex)
                              .arg(m_items.size()));
    return true;
}

bool LibraryHierarchyViewModel::canAcceptNoteDrop(int index, const QString& noteId) const
{
    if (!m_runtimeIndexLoaded)
    {
        return false;
    }

    if (index < 0 || index >= m_items.size())
    {
        return false;
    }

    if (isProtectedRootItem(m_items.at(index)))
    {
        return false;
    }

    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    const QString targetFolderPath = normalizeFolderPath(folderPathForIndex(index));
    if (targetFolderPath.isEmpty())
    {
        return false;
    }

    const int noteIndex = indexOfNoteRecordById(m_libraryAll.notes(), normalizedNoteId);
    if (noteIndex < 0)
    {
        return false;
    }

    const LibraryNoteRecord& note = m_libraryAll.notes().at(noteIndex);
    if (resolveNoteHeaderPath(note).isEmpty())
    {
        return false;
    }

    const QString targetFolderKey = normalizeFolderLookupKey(targetFolderPath);
    for (const QString& folder : note.folders)
    {
        if (normalizeFolderLookupKey(folder) == targetFolderKey)
        {
            return false;
        }
    }

    return true;
}

bool LibraryHierarchyViewModel::assignNoteToFolder(int index, const QString& noteId)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("assignNoteToFolder.begin"),
                              QStringLiteral("index=%1 noteId=%2").arg(index).arg(noteId));

    if (!canAcceptNoteDrop(index, noteId))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("assignNoteToFolder.rejected"),
                                  QStringLiteral("index=%1 noteId=%2").arg(index).arg(noteId));
        return false;
    }

    QVector<LibraryNoteRecord> allNotes = m_libraryAll.notes();
    const int noteIndex = indexOfNoteRecordById(allNotes, noteId);
    if (noteIndex < 0)
    {
        return false;
    }

    LibraryNoteRecord& note = allNotes[noteIndex];
    const QString targetFolderPath = normalizeFolderPath(folderPathForIndex(index));
    const QString headerPath = resolveNoteHeaderPath(note);
    if (targetFolderPath.isEmpty() || headerPath.isEmpty())
    {
        return false;
    }

    QString rawHeaderText;
    QString ioError;
    if (!WhatSon::Hierarchy::LibrarySupport::readUtf8File(headerPath, &rawHeaderText, &ioError))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("assignNoteToFolder.failed"),
                                  QStringLiteral("reason=readHeader path=%1 error=%2").arg(headerPath, ioError));
        return false;
    }

    WhatSonNoteHeaderStore headerStore;
    WhatSonNoteHeaderParser headerParser;
    QString parseError;
    if (!headerParser.parse(rawHeaderText, &headerStore, &parseError))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("assignNoteToFolder.failed"),
                                  QStringLiteral("reason=parseHeader path=%1 error=%2").arg(headerPath, parseError));
        return false;
    }

    headerStore.setFolders(folderAssignmentForDrop(targetFolderPath));
    headerStore.setLastModifiedAt(currentNoteTimestamp());

    WhatSonNoteHeaderCreator headerCreator(QFileInfo(headerPath).absolutePath(), QString());
    const QString nextHeaderText = headerCreator.createHeaderText(headerStore);

    QString writeError;
    if (!writeUtf8File(headerPath, nextHeaderText, &writeError))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("assignNoteToFolder.failed"),
                                  QStringLiteral("reason=writeHeader path=%1 error=%2").arg(headerPath, writeError));
        return false;
    }

    note.folders = headerStore.folders();
    note.lastModifiedAt = headerStore.lastModifiedAt();
    note.noteHeaderPath = headerPath;
    if (note.noteDirectoryPath.isEmpty())
    {
        note.noteDirectoryPath = QFileInfo(headerPath).absolutePath();
    }

    m_libraryAll.setIndexedNotes(m_libraryAll.sourceWshubPath(), std::move(allNotes));
    m_libraryDraft.rebuild(m_libraryAll.notes());
    m_libraryToday.rebuild(m_libraryAll.notes());
    refreshNoteListForSelection();

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("assignNoteToFolder.success"),
                              QStringLiteral("index=%1 noteId=%2 folder=%3")
                              .arg(index)
                              .arg(noteId, targetFolderPath));
    return true;
}

bool LibraryHierarchyViewModel::applyHierarchyNodes(const QVariantList& hierarchyNodes, const QString& activeItemKey)
{
    const QVector<WhatSon::Sidebar::Lvrs::FlatNode> flattened =
        WhatSon::Sidebar::Lvrs::flattenHierarchyNodes(hierarchyNodes);
    if (flattened.isEmpty())
    {
        return false;
    }

    QVector<LibraryHierarchyItem> stagedItems;
    stagedItems.reserve(flattened.size());

    int selectedIndex = -1;
    const QString normalizedActiveKey = activeItemKey.trimmed();
    for (int flatIndex = 0; flatIndex < flattened.size(); ++flatIndex)
    {
        const WhatSon::Sidebar::Lvrs::FlatNode& node = flattened.at(flatIndex);
        if (node.key == normalizedActiveKey)
        {
            selectedIndex = flatIndex;
        }

        LibraryHierarchyItem item;
        const bool validSourceIndex = node.sourceIndex >= 0 && node.sourceIndex < m_items.size();
        if (validSourceIndex && m_items.at(node.sourceIndex).systemBucket != LibraryHierarchyItem::SystemBucket::None)
        {
            item = m_items.at(node.sourceIndex);
            item.depth = 0;
            item.accent = true;
            item.expanded = false;
            item.showChevron = false;
            item.folderPath.clear();
        }
        else
        {
            item.depth = std::max(0, node.depth);
            item.label = node.label.trimmed();
            item.accent = false;
            item.expanded = node.expanded;
            item.showChevron = node.showChevron;
            if (validSourceIndex)
            {
                item.folderPath = normalizeFolderPath(m_items.at(node.sourceIndex).folderPath);
            }
            else if (!node.id.isEmpty())
            {
                item.folderPath = normalizeFolderPath(node.id);
            }
            else
            {
                item.folderPath = normalizeFolderPath(node.key);
            }
        }

        stagedItems.push_back(std::move(item));
    }

    finalizeFolderItems(&stagedItems, false);

    QHash<QString, QString> movedFolderPathMap;
    for (int flatIndex = 0; flatIndex < flattened.size() && flatIndex < stagedItems.size(); ++flatIndex)
    {
        const WhatSon::Sidebar::Lvrs::FlatNode& node = flattened.at(flatIndex);
        const LibraryHierarchyItem& stagedItem = stagedItems.at(flatIndex);
        if (stagedItem.systemBucket != LibraryHierarchyItem::SystemBucket::None)
        {
            continue;
        }

        QString previousFolderPath;
        if (node.sourceIndex >= 0 && node.sourceIndex < m_items.size())
        {
            previousFolderPath = normalizeFolderPath(m_items.at(node.sourceIndex).folderPath);
        }
        else if (!node.id.isEmpty())
        {
            previousFolderPath = normalizeFolderPath(node.id);
        }
        else
        {
            previousFolderPath = normalizeFolderPath(node.key);
        }

        const QString nextFolderPath = normalizeFolderPath(stagedItem.folderPath);
        if (!previousFolderPath.isEmpty() && previousFolderPath != nextFolderPath)
        {
            movedFolderPathMap.insert(previousFolderPath, nextFolderPath);
        }
    }

    return commitFolderHierarchyUpdate(std::move(stagedItems), selectedIndex, movedFolderPathMap);
}

bool LibraryHierarchyViewModel::createEmptyNote()
{
    const QString sourceWshubPath = !m_libraryAll.sourceWshubPath().trimmed().isEmpty()
                                        ? m_libraryAll.sourceWshubPath().trimmed()
                                        : m_hubStore.hubPath().trimmed();
    QString libraryPath = WhatSon::Hierarchy::LibrarySupport::normalizePath(m_hubStore.libraryPath());
    QString resolveError;
    if (libraryPath.isEmpty() || !QFileInfo(libraryPath).isDir())
    {
        libraryPath = resolvePrimaryLibraryPathFromWshub(sourceWshubPath, &resolveError);
    }

    if (libraryPath.isEmpty())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("createEmptyNote.failed"),
                                  QStringLiteral("reason=resolveLibrary path=%1 error=%2").arg(
                                      sourceWshubPath, resolveError));
        return false;
    }

    const QString noteId = createUniqueNoteId(libraryPath, m_libraryAll.notes());
    if (noteId.isEmpty())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("createEmptyNote.failed"),
                                  QStringLiteral("reason=generateId library=%1").arg(libraryPath));
        return false;
    }

    const QString profileName = m_hubStore.stat().participants().isEmpty()
                                    ? QString()
                                    : m_hubStore.stat().participants().constFirst().trimmed();
    QStringList assignedFolders;
    if (m_foldersHierarchyLoaded
        && m_selectedIndex >= 0
        && m_selectedIndex < m_items.size()
        && !isProtectedRootItem(m_items.at(m_selectedIndex)))
    {
        const QString selectedFolderPath = normalizeFolderPath(folderPathForIndex(m_selectedIndex));
        if (!selectedFolderPath.isEmpty())
        {
            assignedFolders.push_back(selectedFolderPath);
        }
    }

    WhatSonNoteHeaderCreator headerCreator(libraryPath, QString());
    WhatSonNoteBodyCreator bodyCreator(libraryPath, QString());
    WhatSonNoteAttachManagerCreator attachCreator(libraryPath, QString());
    WhatSonNoteLinkManagerCreator linkCreator(libraryPath, QString());

    const QString headerPath = headerCreator.targetPathForNote(noteId);
    const QString bodyPath = bodyCreator.targetPathForNote(noteId);
    const QString attachmentManifestPath = attachCreator.targetPathForNote(noteId);
    const QString linksPath = linkCreator.targetPathForNote(noteId);
    const QString noteDirectoryPath = QFileInfo(headerPath).absolutePath();
    const QString backlinksPath = QDir(noteDirectoryPath).filePath(linkCreator.backlinksFileName());

    if (QFileInfo(noteDirectoryPath).exists())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("createEmptyNote.failed"),
                                  QStringLiteral("reason=noteDirectoryExists path=%1").arg(noteDirectoryPath));
        return false;
    }

    QString createError;
    if (!ensureDirectoryPath(noteDirectoryPath, &createError))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("createEmptyNote.failed"),
                                  QStringLiteral("reason=createNoteDirectory path=%1 error=%2").arg(
                                      noteDirectoryPath, createError));
        return false;
    }

    auto rollbackNoteDirectory = [&noteDirectoryPath]()
    {
        if (QFileInfo(noteDirectoryPath).exists())
        {
            QDir(noteDirectoryPath).removeRecursively();
        }
    };

    for (const QString& relativePath : headerCreator.requiredRelativePaths())
    {
        if (!ensureDirectoryPath(QDir(noteDirectoryPath).filePath(relativePath), &createError))
        {
            rollbackNoteDirectory();
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("library.viewmodel"),
                                      QStringLiteral("createEmptyNote.failed"),
                                      QStringLiteral("reason=createHeaderSupportDirectory path=%1 error=%2").arg(
                                          relativePath, createError));
            return false;
        }
    }
    for (const QString& relativePath : attachCreator.requiredRelativePaths())
    {
        if (!ensureDirectoryPath(QDir(noteDirectoryPath).filePath(relativePath), &createError))
        {
            rollbackNoteDirectory();
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("library.viewmodel"),
                                      QStringLiteral("createEmptyNote.failed"),
                                      QStringLiteral("reason=createAttachmentDirectory path=%1 error=%2").arg(
                                          relativePath, createError));
            return false;
        }
    }

    const QString createdTimestamp = currentNoteTimestamp();
    WhatSonNoteHeaderStore headerStore;
    headerStore.setNoteId(noteId);
    headerStore.setCreatedAt(createdTimestamp);
    headerStore.setAuthor(profileName);
    headerStore.setLastModifiedAt(createdTimestamp);
    headerStore.setModifiedBy(profileName);
    headerStore.setFolders(assignedFolders);
    headerStore.setProject(QString());
    headerStore.setBookmarked(false);
    headerStore.setBookmarkColors({});
    headerStore.setTags({});
    headerStore.setProgress(0);
    headerStore.setPreset(false);

    if (!writeUtf8File(headerPath, headerCreator.createHeaderText(headerStore), &createError))
    {
        rollbackNoteDirectory();
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("createEmptyNote.failed"),
                                  QStringLiteral("reason=writeHeader path=%1 error=%2").arg(
                                      headerPath, createError));
        return false;
    }

    if (!writeUtf8File(attachmentManifestPath, createAttachmentManifestText(noteId), &createError))
    {
        rollbackNoteDirectory();
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("createEmptyNote.failed"),
                                  QStringLiteral("reason=writeAttachmentManifest path=%1 error=%2").arg(
                                      attachmentManifestPath, createError));
        return false;
    }
    if (!writeUtf8File(
        linksPath,
        createLinkManifestText(noteId, QStringLiteral("whatson.note.links")),
        &createError))
    {
        rollbackNoteDirectory();
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("createEmptyNote.failed"),
                                  QStringLiteral("reason=writeLinks path=%1 error=%2").arg(
                                      linksPath, createError));
        return false;
    }
    if (!writeUtf8File(
        backlinksPath,
        createLinkManifestText(noteId, QStringLiteral("whatson.note.backlinks")),
        &createError))
    {
        rollbackNoteDirectory();
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("createEmptyNote.failed"),
                                  QStringLiteral("reason=writeBacklinks path=%1 error=%2").arg(
                                      backlinksPath, createError));
        return false;
    }

    QString normalizedBodyText;
    QString lastModifiedAt;
    if (!WhatSon::NoteBodyPersistence::persistBodyPlainText(
        noteId,
        noteDirectoryPath,
        headerPath,
        QString(),
        &normalizedBodyText,
        &lastModifiedAt,
        &createError))
    {
        rollbackNoteDirectory();
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("createEmptyNote.failed"),
                                  QStringLiteral("reason=writeBody path=%1 error=%2").arg(bodyPath, createError));
        return false;
    }

    QVector<LibraryNoteRecord> nextAllNotes = m_libraryAll.notes();
    LibraryNoteRecord newNote;
    newNote.noteId = noteId;
    newNote.storageKind = QStringLiteral("wsnote");
    newNote.createdAt = createdTimestamp;
    newNote.lastModifiedAt = lastModifiedAt.isEmpty() ? createdTimestamp : lastModifiedAt;
    newNote.author = profileName;
    newNote.modifiedBy = profileName;
    newNote.folders = assignedFolders;
    newNote.progress = 0;
    newNote.bookmarked = false;
    newNote.preset = false;
    newNote.bodyPlainText = normalizedBodyText;
    newNote.noteDirectoryPath = noteDirectoryPath;
    newNote.noteHeaderPath = headerPath;
    nextAllNotes.push_back(newNote);

    const QString indexPath = QDir(libraryPath).filePath(QStringLiteral("index.wsnindex"));
    QString previousIndexText;
    const bool hadIndexFile = QFileInfo(indexPath).isFile();
    if (hadIndexFile
        && !WhatSon::Hierarchy::LibrarySupport::readUtf8File(indexPath, &previousIndexText, &createError))
    {
        rollbackNoteDirectory();
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("createEmptyNote.failed"),
                                  QStringLiteral("reason=readIndex path=%1 error=%2").arg(indexPath, createError));
        return false;
    }

    WhatSonLibraryHierarchyStore libraryStore;
    libraryStore.setHubPath(sourceWshubPath);
    libraryStore.setNoteIds(noteIdsFromRecords(nextAllNotes));

    WhatSonLibraryHierarchyCreator libraryCreator;
    if (!writeUtf8File(indexPath, libraryCreator.createText(libraryStore), &createError))
    {
        rollbackNoteDirectory();
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("createEmptyNote.failed"),
                                  QStringLiteral("reason=writeIndex path=%1 error=%2").arg(indexPath, createError));
        return false;
    }

    const QString statPath = !m_hubStore.statPath().trimmed().isEmpty()
                                 ? WhatSon::Hierarchy::LibrarySupport::normalizePath(m_hubStore.statPath())
                                 : resolveHubStatPathFromWshub(sourceWshubPath);
    const QString nowUtc = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    QString statCreatedAtUtc = m_hubStore.stat().createdAtUtc().trimmed();
    if (!statPath.isEmpty())
    {
        QJsonObject statRoot;
        if (QFileInfo(statPath).isFile())
        {
            QString rawStatText;
            if (!WhatSon::Hierarchy::LibrarySupport::readUtf8File(statPath, &rawStatText, &createError))
            {
                if (hadIndexFile)
                {
                    writeUtf8File(indexPath, previousIndexText, nullptr);
                }
                else
                {
                    QFile::remove(indexPath);
                }
                rollbackNoteDirectory();
                WhatSon::Debug::traceSelf(this,
                                          QStringLiteral("library.viewmodel"),
                                          QStringLiteral("createEmptyNote.failed"),
                                          QStringLiteral("reason=readStat path=%1 error=%2").arg(
                                              statPath, createError));
                return false;
            }

            QJsonParseError statParseError;
            const QJsonDocument statDocument = QJsonDocument::fromJson(rawStatText.toUtf8(), &statParseError);
            if (statParseError.error == QJsonParseError::NoError && statDocument.isObject())
            {
                statRoot = statDocument.object();
            }
        }

        if (statCreatedAtUtc.isEmpty())
        {
            statCreatedAtUtc = statRoot.value(QStringLiteral("createdAtUtc")).toString().trimmed();
        }
        if (statCreatedAtUtc.isEmpty())
        {
            statCreatedAtUtc = nowUtc;
        }

        if (!statRoot.contains(QStringLiteral("version")))
        {
            statRoot.insert(QStringLiteral("version"), 1);
        }
        if (!statRoot.contains(QStringLiteral("schema")))
        {
            statRoot.insert(QStringLiteral("schema"), QStringLiteral("whatson.hub.stat"));
        }
        if (!statRoot.contains(QStringLiteral("hub")) && !m_hubStore.hubName().trimmed().isEmpty())
        {
            statRoot.insert(QStringLiteral("hub"), m_hubStore.hubName().trimmed());
        }
        if (!statRoot.contains(QStringLiteral("participants")))
        {
            QJsonArray participants;
            for (const QString& participant : m_hubStore.stat().participants())
            {
                participants.push_back(participant);
            }
            statRoot.insert(QStringLiteral("participants"), participants);
        }
        if (!statRoot.contains(QStringLiteral("profileLastModifiedAtUtc")))
        {
            statRoot.insert(
                QStringLiteral("profileLastModifiedAtUtc"),
                QJsonObject::fromVariantMap(m_hubStore.stat().profileLastModifiedAtUtc()));
        }

        statRoot.insert(QStringLiteral("noteCount"), nextAllNotes.size());
        statRoot.insert(QStringLiteral("resourceCount"), m_hubStore.stat().resourceCount());
        statRoot.insert(QStringLiteral("characterCount"), m_hubStore.stat().characterCount());
        statRoot.insert(QStringLiteral("createdAtUtc"), statCreatedAtUtc);
        statRoot.insert(QStringLiteral("lastModifiedAtUtc"), nowUtc);

        if (!writeUtf8File(
            statPath,
            QString::fromUtf8(QJsonDocument(statRoot).toJson(QJsonDocument::Indented)),
            &createError))
        {
            if (hadIndexFile)
            {
                writeUtf8File(indexPath, previousIndexText, nullptr);
            }
            else
            {
                QFile::remove(indexPath);
            }
            rollbackNoteDirectory();
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("library.viewmodel"),
                                      QStringLiteral("createEmptyNote.failed"),
                                      QStringLiteral("reason=writeStat path=%1 error=%2").arg(
                                          statPath, createError));
            return false;
        }
    }

    m_libraryAll.setIndexedNotes(sourceWshubPath, std::move(nextAllNotes));
    m_libraryDraft.rebuild(m_libraryAll.notes());
    m_libraryToday.rebuild(m_libraryAll.notes());

    m_hubStore.setHubPath(sourceWshubPath);
    m_hubStore.setLibraryPath(libraryPath);
    if (!statPath.isEmpty())
    {
        m_hubStore.setStatPath(statPath);
    }
    if (!m_hubStore.hubPath().isEmpty())
    {
        WhatSonHubStat updatedStat = m_hubStore.stat();
        updatedStat.setNoteCount(m_libraryAll.notes().size());
        updatedStat.setResourceCount(updatedStat.resourceCount());
        updatedStat.setCharacterCount(updatedStat.characterCount());
        if (updatedStat.createdAtUtc().trimmed().isEmpty())
        {
            updatedStat.setCreatedAtUtc(statCreatedAtUtc.isEmpty() ? nowUtc : statCreatedAtUtc);
        }
        updatedStat.setLastModifiedAtUtc(nowUtc);
        m_hubStore.setStat(std::move(updatedStat));
    }

    refreshNoteListForSelection();

    int createdNoteIndex = -1;
    const QVector<LibraryNoteListItem>& noteItems = m_noteListModel.items();
    for (int index = 0; index < noteItems.size(); ++index)
    {
        if (noteItems.at(index).id.trimmed() == noteId)
        {
            createdNoteIndex = index;
            break;
        }
    }
    if (createdNoteIndex >= 0)
    {
        m_noteListModel.setCurrentIndex(createdNoteIndex);
    }

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("createEmptyNote.success"),
                              QStringLiteral("id=%1 folderCount=%2 noteCount=%3 path=%4")
                              .arg(noteId)
                              .arg(assignedFolders.size())
                              .arg(m_libraryAll.notes().size())
                              .arg(noteDirectoryPath));
    return true;
}

bool LibraryHierarchyViewModel::deleteNoteById(const QString& noteId)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    const bool removedCurrentVisibleNote = m_noteListModel.currentNoteId().trimmed() == normalizedNoteId;
    const int removedCurrentVisibleIndex = removedCurrentVisibleNote ? m_noteListModel.currentIndex() : -1;
    WhatSonHubNoteDeletionService deletionService;
    WhatSonHubNoteDeletionService::Request request;
    request.wshubPath = !m_libraryAll.sourceWshubPath().trimmed().isEmpty()
                            ? m_libraryAll.sourceWshubPath().trimmed()
                            : m_hubStore.hubPath().trimmed();
    request.libraryPath = m_hubStore.libraryPath();
    request.statPath = m_hubStore.statPath();
    request.hubName = m_hubStore.hubName();
    request.hubStat = m_hubStore.stat();
    request.notes = m_libraryAll.notes();
    request.noteId = normalizedNoteId;

    WhatSonHubNoteDeletionService::Result result;
    QString deleteError;
    if (!deletionService.deleteNote(std::move(request), &result, &deleteError))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("deleteNoteById.failed"),
                                  QStringLiteral("noteId=%1 error=%2").arg(normalizedNoteId, deleteError));
        return false;
    }

    m_libraryAll.setIndexedNotes(result.wshubPath, std::move(result.remainingNotes));
    m_libraryDraft.rebuild(m_libraryAll.notes());
    m_libraryToday.rebuild(m_libraryAll.notes());

    if (!result.wshubPath.isEmpty())
    {
        m_hubStore.setHubPath(result.wshubPath);
    }
    if (!result.libraryPath.isEmpty())
    {
        m_hubStore.setLibraryPath(result.libraryPath);
    }
    if (!result.statPath.isEmpty())
    {
        m_hubStore.setStatPath(result.statPath);
    }
    m_hubStore.setStat(result.hubStat);

    refreshNoteListForSelection();
    if (removedCurrentVisibleNote)
    {
        const int nextIndex = m_noteListModel.items().isEmpty()
                                  ? -1
                                  : std::min(
                                      removedCurrentVisibleIndex,
                                      static_cast<int>(m_noteListModel.items().size()) - 1);
        m_noteListModel.setCurrentIndex(nextIndex);
    }

    emit noteDeleted(result.noteId.isEmpty() ? normalizedNoteId : result.noteId);

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("deleteNoteById.success"),
                              QStringLiteral("noteId=%1 remaining=%2").arg(
                                                                          result.noteId.isEmpty()
                                                                              ? normalizedNoteId
                                                                              : result.noteId)
                                                                      .arg(m_libraryAll.notes().size()));
    return true;
}

bool LibraryHierarchyViewModel::saveBodyTextForNote(const QString& noteId, const QString& text)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    QVector<LibraryNoteRecord> allNotes = m_libraryAll.notes();
    const int noteIndex = indexOfNoteRecordById(allNotes, normalizedNoteId);
    if (noteIndex < 0)
    {
        return false;
    }

    LibraryNoteRecord& note = allNotes[noteIndex];
    QString normalizedBodyText;
    QString lastModifiedAt;
    QString saveError;
    if (!WhatSon::NoteBodyPersistence::persistBodyPlainText(
        note.noteId,
        note.noteDirectoryPath,
        note.noteHeaderPath,
        text,
        &normalizedBodyText,
        &lastModifiedAt,
        &saveError))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.viewmodel"),
                                  QStringLiteral("saveCurrentBodyText.failed"),
                                  QStringLiteral("noteId=%1 error=%2").arg(normalizedNoteId, saveError));
        return false;
    }

    note.bodyPlainText = normalizedBodyText;
    note.bodyFirstLine = WhatSon::NoteBodyPersistence::firstLineFromBodyPlainText(normalizedBodyText);
    if (!lastModifiedAt.isEmpty())
    {
        note.lastModifiedAt = lastModifiedAt;
    }

    m_libraryAll.setIndexedNotes(m_libraryAll.sourceWshubPath(), std::move(allNotes));
    m_libraryDraft.rebuild(m_libraryAll.notes());
    m_libraryToday.rebuild(m_libraryAll.notes());
    refreshNoteListForSelection();
    return true;
}

bool LibraryHierarchyViewModel::saveCurrentBodyText(const QString& text)
{
    return saveBodyTextForNote(m_noteListModel.currentNoteId(), text);
}

void LibraryHierarchyViewModel::setHubStore(WhatSonHubStore store)
{
    const QString nextHubPath = store.hubPath().trimmed();
    const QString nextLibraryPath = store.libraryPath().trimmed();
    if (m_hubStore.hubPath() == nextHubPath
        && m_hubStore.libraryPath() == nextLibraryPath)
    {
        return;
    }

    m_hubStore = std::move(store);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("setHubStore"),
                              QStringLiteral("hub=%1 library=%2")
                              .arg(m_hubStore.hubPath(), m_hubStore.libraryPath()));
}

WhatSonHubStore LibraryHierarchyViewModel::hubStore() const
{
    return m_hubStore;
}

int LibraryHierarchyViewModel::extractDepth(const QVariantMap& entryMap)
{
    QVariant depthValue;
    if (entryMap.contains(QStringLiteral("depth")))
    {
        depthValue = entryMap.value(QStringLiteral("depth"));
    }
    else if (entryMap.contains(QStringLiteral("dpeth")))
    {
        depthValue = entryMap.value(QStringLiteral("dpeth"));
    }
    else if (entryMap.contains(QStringLiteral("indentLevel")))
    {
        depthValue = entryMap.value(QStringLiteral("indentLevel"));
    }

    bool converted = false;
    const int depth = depthValue.toInt(&converted);
    if (!converted)
    {
        return 0;
    }

    return std::max(0, depth);
}

LibraryHierarchyItem LibraryHierarchyViewModel::parseItem(const QVariant& entry, int fallbackOrdinal)
{
    LibraryHierarchyItem parsed;
    Q_UNUSED(fallbackOrdinal);

    if (entry.metaType().id() == QMetaType::QVariantMap)
    {
        const QVariantMap entryMap = entry.toMap();
        parsed.depth = extractDepth(entryMap);
        parsed.label = entryMap.value(QStringLiteral("label")).toString().trimmed();
        parsed.folderPath = normalizeFolderPath(
            entryMap.value(QStringLiteral("id"),
                           entryMap.value(QStringLiteral("path"),
                                          entryMap.value(QStringLiteral("key")))).toString());
        parsed.accent = entryMap.value(QStringLiteral("accent"), false).toBool();
        parsed.expanded = entryMap.value(QStringLiteral("expanded"), false).toBool();
        parsed.showChevron = entryMap.value(QStringLiteral("showChevron"), true).toBool();
    }
    else
    {
        bool converted = false;
        const int depth = entry.toInt(&converted);
        if (converted)
        {
            parsed.depth = std::max(0, depth);
        }
        parsed.label = entry.toString().trimmed();
    }

    if (parsed.label.isEmpty())
    {
        WhatSon::Debug::trace(
            QStringLiteral("library.viewmodel"),
            QStringLiteral("parseItem.emptyLabelKept"));
    }

    return parsed;
}

int LibraryHierarchyViewModel::nextFolderSequence(const QVector<LibraryHierarchyItem>& items)
{
    static const QRegularExpression folderPattern(QStringLiteral("^Folder(\\d+)$"));

    int maxSequence = 0;
    for (const LibraryHierarchyItem& item : items)
    {
        const QRegularExpressionMatch match = folderPattern.match(item.label);
        if (!match.hasMatch())
        {
            continue;
        }

        bool converted = false;
        const int value = match.captured(1).toInt(&converted);
        if (converted)
        {
            maxSequence = std::max(maxSequence, value);
        }
    }

    return maxSequence + 1;
}

QVector<LibraryNoteListItem> LibraryHierarchyViewModel::buildNoteListItems(
    const QVector<LibraryNoteRecord>& notes) const
{
    QVector<LibraryNoteListItem> items;
    items.reserve(notes.size());
    const FolderHierarchyLookup lookup = buildFolderHierarchyLookup(m_items);
    const FolderHierarchyLookup* activeLookup = m_foldersHierarchyLoaded ? &lookup : nullptr;

    for (const LibraryNoteRecord& note : notes)
    {
        const QStringList folderLabels = canonicalNoteFolderLabels(note, activeLookup);
        LibraryNoteListItem item;
        item.id = note.noteId.trimmed();
        item.primaryText = notePrimaryText(note);
        item.searchableText = noteSearchableText(note, folderLabels);
        item.bodyText = note.bodyPlainText;
        item.image = note.bodyHasResource;
        item.imageSource = note.bodyFirstResourceThumbnailUrl;
        item.displayDate = m_systemCalendarStore
                               ? m_systemCalendarStore->formatNoteDate(note.lastModifiedAt, note.createdAt)
                               : SystemCalendarStore::formatNoteDateForSystem(note.lastModifiedAt, note.createdAt);
        item.folders = folderLabels;
        item.tags = noteListTags(note);
        item.bookmarked = note.bookmarked;
        item.bookmarkColor = bookmarkColorHexFromNote(note);
        items.push_back(std::move(item));
    }

    return items;
}

const QVector<LibraryNoteRecord>& LibraryHierarchyViewModel::notesForBucket(IndexedBucket bucket) const
{
    switch (bucket)
    {
    case IndexedBucket::Draft:
        return m_libraryDraft.notes();
    case IndexedBucket::Today:
        return m_libraryToday.notes();
    case IndexedBucket::All:
    default:
        return m_libraryAll.notes();
    }
}

const LibraryHierarchyViewModel::IndexedBucketRange* LibraryHierarchyViewModel::bucketRangeForIndex(int index) const
    noexcept
{
    if (index < 0)
    {
        return nullptr;
    }

    for (const IndexedBucketRange& range : m_bucketRanges)
    {
        if (index >= range.startRow && index <= range.endRow)
        {
            return &range;
        }
    }

    return nullptr;
}

LibraryHierarchyViewModel::IndexedBucket LibraryHierarchyViewModel::selectedBucket() const
{
    if (m_selectedIndex < 0)
    {
        return IndexedBucket::All;
    }

    const IndexedBucketRange* range = bucketRangeForIndex(m_selectedIndex);
    if (range != nullptr)
    {
        return range->bucket;
    }

    return IndexedBucket::All;
}

LibraryHierarchyViewModel::FolderSelectionScope LibraryHierarchyViewModel::selectedFolderScope() const
{
    FolderSelectionScope scope;
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        return scope;
    }

    scope.selectedPathKey = normalizeFolderKey(folderPathForIndex(m_selectedIndex));
    if (scope.selectedPathKey.isEmpty())
    {
        return scope;
    }

    return scope;
}

QString LibraryHierarchyViewModel::normalizeFolderKey(const QString& value)
{
    return normalizeFolderLookupKey(value);
}

QString LibraryHierarchyViewModel::folderPathForIndex(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }
    return normalizeFolderPath(m_items.at(index).folderPath);
}

bool LibraryHierarchyViewModel::commitFolderHierarchyUpdate(
    QVector<LibraryHierarchyItem> stagedItems,
    int selectedIndex,
    const QHash<QString, QString>& movedFolderPathMap)
{
    WhatSonProjectsHierarchyStore stagedStore;
    stagedStore.setFolderEntries(folderEntriesFromItems(stagedItems));

    struct PendingNoteFolderRewrite final
    {
        int noteIndex = -1;
        QString headerPath;
        QString previousText;
        QString nextText;
        QStringList nextFolders;
        QString nextLastModifiedAt;
    };

    QVector<PendingNoteFolderRewrite> pendingNoteWrites;
    QVector<LibraryNoteRecord> stagedAllNotes = m_libraryAll.notes();

    if (m_runtimeIndexLoaded && !movedFolderPathMap.isEmpty())
    {
        const FolderHierarchyLookup originalLookup = buildFolderHierarchyLookup(m_items);
        const QString rewriteTimestamp = currentNoteTimestamp();

        for (int noteIndex = 0; noteIndex < stagedAllNotes.size(); ++noteIndex)
        {
            const LibraryNoteRecord& note = stagedAllNotes.at(noteIndex);
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
                const QString remappedFolder = remapFolderPathForMove(resolvedFolder, movedFolderPathMap);
                const QString remappedKey = normalizeFolderLookupKey(remappedFolder);
                if (remappedKey.isEmpty() || seenFolderKeys.contains(remappedKey))
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

            const QString headerPath = resolveNoteHeaderPath(note);
            if (headerPath.isEmpty())
            {
                WhatSon::Debug::traceSelf(this,
                                          QStringLiteral("library.viewmodel"),
                                          QStringLiteral("commitFolderHierarchyUpdate.failed"),
                                          QStringLiteral("reason=missingHeader noteId=%1").arg(note.noteId));
                return false;
            }

            QString rawHeaderText;
            QString readError;
            if (!WhatSon::Hierarchy::LibrarySupport::readUtf8File(headerPath, &rawHeaderText, &readError))
            {
                WhatSon::Debug::traceSelf(this,
                                          QStringLiteral("library.viewmodel"),
                                          QStringLiteral("commitFolderHierarchyUpdate.failed"),
                                          QStringLiteral("reason=readHeader path=%1 error=%2").arg(
                                              headerPath, readError));
                return false;
            }

            WhatSonNoteHeaderStore headerStore;
            WhatSonNoteHeaderParser headerParser;
            QString parseError;
            if (!headerParser.parse(rawHeaderText, &headerStore, &parseError))
            {
                WhatSon::Debug::traceSelf(this,
                                          QStringLiteral("library.viewmodel"),
                                          QStringLiteral("commitFolderHierarchyUpdate.failed"),
                                          QStringLiteral("reason=parseHeader path=%1 error=%2").arg(
                                              headerPath, parseError));
                return false;
            }

            headerStore.setFolders(remappedFolders);
            headerStore.setLastModifiedAt(rewriteTimestamp);

            WhatSonNoteHeaderCreator headerCreator(QFileInfo(headerPath).absolutePath(), QString());

            PendingNoteFolderRewrite rewrite;
            rewrite.noteIndex = noteIndex;
            rewrite.headerPath = headerPath;
            rewrite.previousText = rawHeaderText;
            rewrite.nextText = headerCreator.createHeaderText(headerStore);
            rewrite.nextFolders = headerStore.folders();
            rewrite.nextLastModifiedAt = headerStore.lastModifiedAt();
            pendingNoteWrites.push_back(std::move(rewrite));
        }
    }

    auto rollbackNoteWrites = [](const QVector<PendingNoteFolderRewrite>& writes)
    {
        for (int index = writes.size() - 1; index >= 0; --index)
        {
            const PendingNoteFolderRewrite& rewrite = writes.at(index);
            writeUtf8File(rewrite.headerPath, rewrite.previousText, nullptr);
        }
    };

    QVector<PendingNoteFolderRewrite> appliedNoteWrites;
    appliedNoteWrites.reserve(pendingNoteWrites.size());
    for (const PendingNoteFolderRewrite& rewrite : pendingNoteWrites)
    {
        QString writeError;
        if (!writeUtf8File(rewrite.headerPath, rewrite.nextText, &writeError))
        {
            rollbackNoteWrites(appliedNoteWrites);
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("library.viewmodel"),
                                      QStringLiteral("commitFolderHierarchyUpdate.failed"),
                                      QStringLiteral("reason=writeHeader path=%1 error=%2").arg(
                                          rewrite.headerPath, writeError));
            return false;
        }
        appliedNoteWrites.push_back(rewrite);
    }

    if (!m_foldersFilePath.trimmed().isEmpty())
    {
        QString writeError;
        if (!stagedStore.writeToFile(m_foldersFilePath, &writeError))
        {
            rollbackNoteWrites(appliedNoteWrites);
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("library.viewmodel"),
                                      QStringLiteral("commitFolderHierarchyUpdate.failed"),
                                      QStringLiteral("reason=writeFolders path=%1 error=%2").arg(
                                          m_foldersFilePath, writeError));
            return false;
        }
    }

    for (const PendingNoteFolderRewrite& rewrite : pendingNoteWrites)
    {
        LibraryNoteRecord& note = stagedAllNotes[rewrite.noteIndex];
        note.folders = rewrite.nextFolders;
        note.lastModifiedAt = rewrite.nextLastModifiedAt;
        note.noteHeaderPath = rewrite.headerPath;
        if (note.noteDirectoryPath.isEmpty())
        {
            note.noteDirectoryPath = QFileInfo(rewrite.headerPath).absolutePath();
        }
    }

    m_items = std::move(stagedItems);
    m_foldersHierarchyLoaded = !folderEntriesFromItems(m_items).isEmpty();

    if (m_runtimeIndexLoaded)
    {
        m_libraryAll.setIndexedNotes(m_libraryAll.sourceWshubPath(), std::move(stagedAllNotes));
        m_libraryDraft.rebuild(m_libraryAll.notes());
        m_libraryToday.rebuild(m_libraryAll.notes());
    }

    rebuildBucketRanges();
    syncModel();
    applySelectedIndex(selectedIndex, true);
    return true;
}

int LibraryHierarchyViewModel::firstEditableInsertIndex() const noexcept
{
    int index = 0;
    while (index < m_items.size() && isProtectedRootItem(m_items.at(index)))
    {
        ++index;
    }
    return index;
}

void LibraryHierarchyViewModel::rebuildBucketRanges()
{
    m_bucketRanges.clear();

    for (int index = 0; index < m_items.size(); ++index)
    {
        const LibraryHierarchyItem& item = m_items.at(index);
        if (!isSystemBucketItem(item))
        {
            continue;
        }

        IndexedBucketRange range;
        if (item.systemBucket == LibraryHierarchyItem::SystemBucket::Draft)
        {
            range.bucket = IndexedBucket::Draft;
        }
        else if (item.systemBucket == LibraryHierarchyItem::SystemBucket::Today)
        {
            range.bucket = IndexedBucket::Today;
        }
        else
        {
            range.bucket = IndexedBucket::All;
        }
        range.startRow = index;
        range.endRow = index;
        m_bucketRanges.push_back(range);
    }
}

void LibraryHierarchyViewModel::refreshNoteListForSelection()
{
    if (!m_runtimeIndexLoaded)
    {
        m_noteListModel.setItems({});
        updateNoteItemCount();
        return;
    }

    if (const IndexedBucketRange* range = bucketRangeForIndex(m_selectedIndex))
    {
        const QVector<LibraryNoteListItem> listItems = buildNoteListItems(notesForBucket(range->bucket));
        m_noteListModel.setItems(listItems);
        updateNoteItemCount();
        return;
    }

    if (m_foldersHierarchyLoaded)
    {
        if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
        {
            m_noteListModel.setItems(buildNoteListItems(m_libraryAll.notes()));
            updateNoteItemCount();
            return;
        }

        const FolderSelectionScope scope = selectedFolderScope();
        const FolderHierarchyLookup lookup = buildFolderHierarchyLookup(m_items);
        QVector<LibraryNoteRecord> filtered;
        filtered.reserve(m_libraryAll.notes().size());

        for (const LibraryNoteRecord& note : m_libraryAll.notes())
        {
            if (noteMatchesFolderScope(note, scope.selectedPathKey, lookup))
            {
                filtered.push_back(note);
            }
        }

        m_noteListModel.setItems(buildNoteListItems(filtered));
        updateNoteItemCount();
        return;
    }

    const IndexedBucket bucket = selectedBucket();
    const QVector<LibraryNoteListItem> listItems = buildNoteListItems(notesForBucket(bucket));
    m_noteListModel.setItems(listItems);
    updateNoteItemCount();
}

void LibraryHierarchyViewModel::applyIndexedBuckets()
{
    m_items = prependSystemBuckets({});
    m_foldersHierarchyLoaded = false;
    rebuildBucketRanges();
    m_createdFolderSequence = nextFolderSequence(m_items);
    syncModel();
    refreshNoteListForSelection();
}

void LibraryHierarchyViewModel::syncModel()
{
    applyChevronByDepth(&m_items);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("syncModel"),
                              QStringLiteral("itemCount=%1").arg(m_items.size()));
    m_itemModel.setItems(m_items);
    updateItemCount();
    emit hierarchyModelChanged();
}

void LibraryHierarchyViewModel::updateItemCount()
{
    const int nextCount = m_itemModel.rowCount();
    if (m_itemCount == nextCount)
    {
        return;
    }
    m_itemCount = nextCount;
    emit itemCountChanged();
}

void LibraryHierarchyViewModel::updateNoteItemCount()
{
    const int nextCount = m_noteListModel.rowCount();
    if (m_noteItemCount == nextCount)
    {
        return;
    }
    m_noteItemCount = nextCount;
    emit noteItemCountChanged();
}

void LibraryHierarchyViewModel::updateLoadState(bool succeeded, QString errorMessage)
{
    errorMessage = errorMessage.trimmed();
    const QString normalizedError = succeeded ? QString() : errorMessage;
    const bool shouldEmit = (m_loadSucceeded != succeeded) || (m_lastLoadError != normalizedError);
    m_loadSucceeded = succeeded;
    m_lastLoadError = normalizedError;
    if (shouldEmit)
    {
        emit loadStateChanged();
    }
}
