#include "LibraryHierarchyViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/projects/WhatSonProjectsHierarchyParser.hpp"
#include "file/hierarchy/projects/WhatSonProjectsHierarchyStore.hpp"
#include "file/note/WhatSonBookmarkColorPalette.hpp"
#include "file/note/WhatSonNoteHeaderCreator.hpp"
#include "file/note/WhatSonNoteHeaderParser.hpp"
#include "file/note/WhatSonNoteHeaderStore.hpp"
#include "viewmodel/hierarchy/library/LibraryHierarchyViewModelSupport.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
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

    QString noteLabelText(const LibraryNoteRecord& note)
    {
        QString primary = note.bodyFirstLine.trimmed();
        if (primary.isEmpty())
        {
            primary = note.noteId.trimmed();
        }
        if (primary.isEmpty() && !note.noteDirectoryPath.isEmpty())
        {
            primary = QFileInfo(note.noteDirectoryPath).completeBaseName().trimmed();
        }
        return primary;
    }

    QString notePrimaryText(const LibraryNoteRecord& note)
    {
        const QString bodyPlainText = truncateToMaxLines(note.bodyPlainText.trimmed(), kMaxNoteListSummaryLines);
        if (!bodyPlainText.isEmpty())
        {
            return bodyPlainText;
        }
        return noteLabelText(note);
    }

    QString noteSearchableText(const LibraryNoteRecord& note)
    {
        QStringList parts;

        const QString noteId = note.noteId.trimmed();
        if (!noteId.isEmpty())
        {
            parts.push_back(noteId);
        }

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

        for (const QString& folder : note.folders)
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

    QDate parseNoteListDate(const QString& value)
    {
        const QString trimmed = value.trimmed();
        if (trimmed.isEmpty())
        {
            return {};
        }

        const QList<QString> formats{
            QStringLiteral("yyyy-MM-dd-hh-mm-ss"),
            QStringLiteral("yyyy-MM-dd hh:mm:ss"),
            QStringLiteral("yyyy/MM/dd hh:mm:ss"),
            QStringLiteral("yyyy-MM-dd")
        };

        for (const QString& format : formats)
        {
            const QDateTime dateTime = QDateTime::fromString(trimmed, format);
            if (dateTime.isValid())
            {
                return dateTime.date();
            }

            const QDate date = QDate::fromString(trimmed, format);
            if (date.isValid())
            {
                return date;
            }
        }

        const QDateTime isoDateTime = QDateTime::fromString(trimmed, Qt::ISODate);
        if (isoDateTime.isValid())
        {
            return isoDateTime.date();
        }

        const QDateTime isoDateTimeWithMs = QDateTime::fromString(trimmed, Qt::ISODateWithMs);
        if (isoDateTimeWithMs.isValid())
        {
            return isoDateTimeWithMs.date();
        }

        return {};
    }

    QString noteListDisplayDate(const LibraryNoteRecord& note)
    {
        QDate date = parseNoteListDate(note.lastModifiedAt);
        if (!date.isValid())
        {
            date = parseNoteListDate(note.createdAt);
        }
        if (!date.isValid())
        {
            return {};
        }
        return date.toString(QStringLiteral("yyyy-MM-dd"));
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
    };

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

            const QStringList& matches = contextualMatches.isEmpty() ? candidates : contextualMatches;
            for (const QString& candidatePathKey : matches)
            {
                appendResolved(candidatePathKey);
            }
        }

        return resolved;
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
            if (folderPathKey.startsWith(selectedPathKey + QLatin1Char('/')))
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

    QStringList appendFolderAssignment(QStringList folders, const QString& folderPath)
    {
        QStringList merged;
        merged.reserve(folders.size() + 1);
        QSet<QString> seenKeys;

        auto appendUnique = [&merged, &seenKeys](QString rawValue)
        {
            rawValue = normalizeFolderPath(std::move(rawValue));
            const QString key = normalizeFolderLookupKey(rawValue);
            if (rawValue.isEmpty() || key.isEmpty() || seenKeys.contains(key))
            {
                return;
            }

            seenKeys.insert(key);
            merged.push_back(std::move(rawValue));
        };

        for (QString& folder : folders)
        {
            appendUnique(std::move(folder));
        }
        appendUnique(folderPath);

        return merged;
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
        return item.systemBucket != LibraryHierarchyItem::SystemBucket::None;
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
    for (const LibraryHierarchyItem& item : m_items)
    {
        serializedItems.push_back(QVariantMap{
            {"label", item.label},
            {"id", item.folderPath},
            {"depth", item.depth},
            {"accent", item.accent},
            {"expanded", item.expanded},
            {"showChevron", item.showChevron}
        });
    }
    return serializedItems;
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

bool LibraryHierarchyViewModel::canAcceptFolderDrop(int sourceIndex, int targetIndex, bool asChild) const
{
    if (!canMoveFolder(sourceIndex))
    {
        return false;
    }

    if (targetIndex < 0 || targetIndex >= m_items.size())
    {
        return false;
    }

    if (sourceIndex == targetIndex)
    {
        return false;
    }

    const bool targetIsRootInsertion = !asChild && isProtectedRootItem(m_items.at(targetIndex));
    if (!targetIsRootInsertion && !canMoveFolder(targetIndex))
    {
        return false;
    }

    const int sourceEndIndex = subtreeEndIndexExclusive(m_items, sourceIndex);
    if (indexInsideSubtree(targetIndex, sourceIndex, sourceEndIndex))
    {
        return false;
    }

    const int sourceDepth = m_items.at(sourceIndex).depth;
    const int targetDepth = targetIsRootInsertion ? 0 : m_items.at(targetIndex).depth;
    const int targetInsertIndex = targetIsRootInsertion
                                      ? firstEditableInsertIndex()
                                      : subtreeEndIndexExclusive(m_items, targetIndex);
    const int sourceCount = sourceEndIndex - sourceIndex;
    const int normalizedInsertIndex = targetInsertIndex > sourceIndex
                                          ? targetInsertIndex - sourceCount
                                          : targetInsertIndex;
    const int newBaseDepth = asChild ? targetDepth + 1 : targetDepth;

    return normalizedInsertIndex != sourceIndex || newBaseDepth != sourceDepth;
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
    if (!canAcceptFolderDrop(sourceIndex, targetIndex, asChild))
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

    const int sourceEndIndex = subtreeEndIndexExclusive(m_items, sourceIndex);
    const int sourceCount = sourceEndIndex - sourceIndex;
    const bool targetIsRootInsertion = !asChild && isProtectedRootItem(m_items.at(targetIndex));
    const int newBaseDepth = asChild
                                 ? m_items.at(targetIndex).depth + 1
                                 : (targetIsRootInsertion ? 0 : m_items.at(targetIndex).depth);
    const int depthDelta = newBaseDepth - m_items.at(sourceIndex).depth;
    QVector<LibraryHierarchyItem> movedItems;
    movedItems.reserve(sourceCount);
    for (int index = sourceIndex; index < sourceEndIndex; ++index)
    {
        LibraryHierarchyItem item = m_items.at(index);
        item.depth = std::max(0, item.depth + depthDelta);
        movedItems.push_back(std::move(item));
    }

    QVector<LibraryHierarchyItem> stagedItems = m_items;
    stagedItems.remove(sourceIndex, sourceCount);

    int insertIndex = targetIsRootInsertion
                          ? firstEditableInsertIndex()
                          : subtreeEndIndexExclusive(m_items, targetIndex);
    if (insertIndex > sourceIndex)
    {
        insertIndex -= sourceCount;
    }
    insertIndex = std::clamp(insertIndex, 0, static_cast<int>(stagedItems.size()));

    for (int offset = 0; offset < movedItems.size(); ++offset)
    {
        stagedItems.insert(insertIndex + offset, std::move(movedItems[offset]));
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
                                      QStringLiteral("moveFolder.writeFailed"),
                                      QStringLiteral("sourceIndex=%1 targetIndex=%2 path=%3 reason=%4")
                                      .arg(sourceIndex)
                                      .arg(targetIndex)
                                      .arg(m_foldersFilePath, writeError));
            return false;
        }
    }

    m_items = std::move(stagedItems);
    m_foldersHierarchyLoaded = !folderEntriesFromItems(m_items).isEmpty();
    rebuildBucketRanges();
    syncModel();
    applySelectedIndex(insertIndex, true);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("moveFolder.success"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2").arg(insertIndex).arg(m_items.size()));
    return true;
}

bool LibraryHierarchyViewModel::canMoveFolderToRoot(int sourceIndex) const
{
    return canMoveFolder(sourceIndex);
}

bool LibraryHierarchyViewModel::moveFolderToRoot(int sourceIndex)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("moveFolderToRoot.begin"),
                              QStringLiteral("sourceIndex=%1").arg(sourceIndex));
    if (!canMoveFolderToRoot(sourceIndex))
    {
        return false;
    }

    const int sourceEndIndex = subtreeEndIndexExclusive(m_items, sourceIndex);
    const int sourceCount = sourceEndIndex - sourceIndex;
    const int newBaseDepth = 0;
    const int depthDelta = newBaseDepth - m_items.at(sourceIndex).depth;
    QVector<LibraryHierarchyItem> movedItems;
    movedItems.reserve(sourceCount);
    for (int index = sourceIndex; index < sourceEndIndex; ++index)
    {
        LibraryHierarchyItem item = m_items.at(index);
        item.depth = std::max(0, item.depth + depthDelta);
        movedItems.push_back(std::move(item));
    }

    QVector<LibraryHierarchyItem> stagedItems = m_items;
    stagedItems.remove(sourceIndex, sourceCount);

    int insertIndex = firstEditableInsertIndex();
    if (insertIndex > sourceIndex)
    {
        insertIndex -= sourceCount;
    }
    insertIndex = std::clamp(insertIndex, 0, static_cast<int>(stagedItems.size()));
    if (insertIndex == sourceIndex && depthDelta == 0)
    {
        return false;
    }

    for (int offset = 0; offset < movedItems.size(); ++offset)
    {
        stagedItems.insert(insertIndex + offset, std::move(movedItems[offset]));
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
                                      QStringLiteral("moveFolderToRoot.writeFailed"),
                                      QStringLiteral("sourceIndex=%1 path=%2 reason=%3").arg(sourceIndex).arg(
                                          m_foldersFilePath, writeError));
            return false;
        }
    }

    m_items = std::move(stagedItems);
    m_foldersHierarchyLoaded = !folderEntriesFromItems(m_items).isEmpty();
    rebuildBucketRanges();
    syncModel();
    applySelectedIndex(insertIndex, true);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.viewmodel"),
                              QStringLiteral("moveFolderToRoot.success"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2").arg(insertIndex).arg(m_items.size()));
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

    headerStore.setFolders(appendFolderAssignment(headerStore.folders(), targetFolderPath));
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
    const QVector<LibraryNoteRecord>& notes)
{
    QVector<LibraryNoteListItem> items;
    items.reserve(notes.size());

    for (const LibraryNoteRecord& note : notes)
    {
        LibraryNoteListItem item;
        item.id = note.noteId.trimmed();
        item.primaryText = notePrimaryText(note);
        item.searchableText = noteSearchableText(note);
        item.bodyText = note.bodyPlainText.trimmed();
        item.displayDate = noteListDisplayDate(note);
        item.folders = noteListFolders(note);
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
