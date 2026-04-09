#include "TagsHierarchyViewModel.hpp"

#include "calendar/SystemCalendarStore.hpp"
#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/library/LibraryAll.hpp"
#include "file/hierarchy/tags/WhatSonTagsHierarchyParser.hpp"
#include "file/note/WhatSonBookmarkColorPalette.hpp"
#include "file/note/WhatSonNoteFolderBindingRepository.hpp"
#include "viewmodel/hierarchy/tags/TagsHierarchyViewModelSupport.hpp"

#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <QRegularExpression>
#include <QVariantMap>

#include <algorithm>
#include <utility>

namespace
{
    constexpr auto kScope = "tags.viewmodel";
    constexpr int kMaxNoteListSummaryLines = 5;

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
        const QString firstLine = note.bodyFirstLine.trimmed();
        const QString bodyPlainText = truncateToMaxLines(note.bodyPlainText.trimmed(), kMaxNoteListSummaryLines);
        if (!firstLine.isEmpty())
        {
            if (bodyPlainText.isEmpty())
            {
                return firstLine;
            }

            if (!bodyPlainText.startsWith(firstLine))
            {
                return firstLine + QLatin1Char('\n') + bodyPlainText;
            }
        }

        if (!bodyPlainText.isEmpty())
        {
            return bodyPlainText;
        }

        return note.noteId.trimmed();
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
            folders.push_back(QStringLiteral("Draft"));
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

    QString normalizedTagKeySegment(const WhatSonTagDepthEntry& entry, int index)
    {
        const QString normalizedId = entry.id.trimmed();
        if (!normalizedId.isEmpty())
        {
            return normalizedId;
        }

        const QString normalizedLabel = entry.label.trimmed();
        if (!normalizedLabel.isEmpty())
        {
            return normalizedLabel;
        }

        return QStringLiteral("tag:%1").arg(index);
    }

    QString tagHierarchyItemKey(const QVector<WhatSonTagDepthEntry>& entries, int index)
    {
        if (index < 0 || index >= entries.size())
        {
            return {};
        }

        QStringList pathSegments;
        pathSegments.reserve(std::max(1, entries.at(index).depth + 1));
        pathSegments.push_front(normalizedTagKeySegment(entries.at(index), index));

        int expectedDepth = std::max(0, entries.at(index).depth);
        for (int cursor = index - 1; cursor >= 0 && expectedDepth > 0; --cursor)
        {
            const WhatSonTagDepthEntry& candidate = entries.at(cursor);
            if (std::max(0, candidate.depth) != expectedDepth - 1)
            {
                continue;
            }
            pathSegments.push_front(normalizedTagKeySegment(candidate, cursor));
            expectedDepth = std::max(0, candidate.depth);
        }

        return pathSegments.join(QLatin1Char('/'));
    }

    int selectedTagIndexForKey(const QVector<WhatSonTagDepthEntry>& entries, const QString& activeItemKey)
    {
        const QString normalizedActiveKey = activeItemKey.trimmed();
        if (normalizedActiveKey.isEmpty())
        {
            return -1;
        }

        for (int index = 0; index < entries.size(); ++index)
        {
            if (tagHierarchyItemKey(entries, index) == normalizedActiveKey)
            {
                return index;
            }
        }

        return -1;
    }

    QString normalizeTagLookupKey(QString value)
    {
        return value.trimmed().toCaseFolded();
    }

    void insertTagLookupKey(QSet<QString>* keys, const QString& value)
    {
        if (keys == nullptr)
        {
            return;
        }

        const QString normalizedValue = normalizeTagLookupKey(value);
        if (!normalizedValue.isEmpty())
        {
            keys->insert(normalizedValue);
        }
    }

    int subtreeEndExclusiveForTagIndex(const QVector<WhatSonTagDepthEntry>& entries, int rootIndex)
    {
        if (rootIndex < 0 || rootIndex >= entries.size())
        {
            return entries.size();
        }

        const int rootDepth = std::max(0, entries.at(rootIndex).depth);
        int cursor = rootIndex + 1;
        while (cursor < entries.size() && std::max(0, entries.at(cursor).depth) > rootDepth)
        {
            ++cursor;
        }
        return cursor;
    }

    QSet<QString> tagProjectionKeysForSelection(
        const QVector<WhatSonTagDepthEntry>& entries,
        int selectedIndex)
    {
        QSet<QString> projectionKeys;
        if (entries.isEmpty())
        {
            return projectionKeys;
        }

        const bool hasSelection = selectedIndex >= 0 && selectedIndex < entries.size();
        const int beginIndex = hasSelection ? selectedIndex : 0;
        const int endIndex = hasSelection
            ? subtreeEndExclusiveForTagIndex(entries, selectedIndex)
            : entries.size();

        for (int index = beginIndex; index < endIndex; ++index)
        {
            insertTagLookupKey(&projectionKeys, entries.at(index).id);
            insertTagLookupKey(&projectionKeys, entries.at(index).label);
            insertTagLookupKey(&projectionKeys, tagHierarchyItemKey(entries, index));
        }
        return projectionKeys;
    }

    bool noteMatchesTagProjection(const LibraryNoteRecord& note, const QSet<QString>& projectionKeys)
    {
        if (projectionKeys.isEmpty())
        {
            return false;
        }

        for (const QString& tag : note.tags)
        {
            if (projectionKeys.contains(normalizeTagLookupKey(tag)))
            {
                return true;
            }
        }
        return false;
    }

    QSet<QString> expandedTagItemKeys(
        const QVector<WhatSonTagDepthEntry>& entries,
        const QVector<TagsHierarchyItem>& items)
    {
        QSet<QString> expandedKeys;
        const int count = std::min(entries.size(), items.size());
        for (int index = 0; index < count; ++index)
        {
            if (!items.at(index).expanded)
            {
                continue;
            }
            expandedKeys.insert(tagHierarchyItemKey(entries, index));
        }
        return expandedKeys;
    }

    void restoreExpandedTagItemKeys(
        QVector<TagsHierarchyItem>* items,
        const QVector<WhatSonTagDepthEntry>& entries,
        const QSet<QString>& expandedKeys)
    {
        if (items == nullptr)
        {
            return;
        }

        const int count = std::min(items->size(), entries.size());
        for (int index = 0; index < count; ++index)
        {
            (*items)[index].expanded = expandedKeys.contains(tagHierarchyItemKey(entries, index));
        }
    }

    bool tagDepthEntriesEqual(const QVector<WhatSonTagDepthEntry>& lhs, const QVector<WhatSonTagDepthEntry>& rhs)
    {
        if (lhs.size() != rhs.size())
        {
            return false;
        }

        for (int index = 0; index < lhs.size(); ++index)
        {
            const WhatSonTagDepthEntry& left = lhs.at(index);
            const WhatSonTagDepthEntry& right = rhs.at(index);
            if (left.id.trimmed() != right.id.trimmed()
                || left.label.trimmed() != right.label.trimmed()
                || std::max(0, left.depth) != std::max(0, right.depth))
            {
                return false;
            }
        }

        return true;
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

    QString resolveWshubPathFromTagsFile(const QString& tagsFilePath)
    {
        QFileInfo info(tagsFilePath.trimmed());
        QString currentPath = info.isDir() ? info.absoluteFilePath() : info.absolutePath();
        while (!currentPath.isEmpty())
        {
            const QFileInfo currentInfo(currentPath);
            if (currentInfo.fileName().endsWith(QStringLiteral(".wshub")) && currentInfo.isDir())
            {
                return currentInfo.absoluteFilePath();
            }

            const QDir dir(currentPath);
            const QString parentPath = dir.absolutePath() == dir.rootPath() ? QString() : dir.filePath(QStringLiteral(".."));
            const QString normalizedParentPath = QFileInfo(parentPath).absoluteFilePath();
            if (normalizedParentPath.isEmpty() || normalizedParentPath == currentPath)
            {
                break;
            }
            currentPath = normalizedParentPath;
        }

        return {};
    }

    void syncNoteRecordFromDocument(LibraryNoteRecord* note, const WhatSonLocalNoteDocument& document)
    {
        if (note == nullptr)
        {
            return;
        }

        const LibraryNoteRecord updatedRecord = document.toLibraryNoteRecord();
        if (!updatedRecord.noteId.trimmed().isEmpty())
        {
            note->noteId = updatedRecord.noteId;
        }
        note->storageKind = updatedRecord.storageKind;
        note->bodyPlainText = updatedRecord.bodyPlainText;
        note->bodySourceText = updatedRecord.bodySourceText;
        note->bodyFirstLine = updatedRecord.bodyFirstLine;
        note->bodyHasResource = updatedRecord.bodyHasResource;
        note->bodyFirstResourceThumbnailUrl = updatedRecord.bodyFirstResourceThumbnailUrl;
        note->createdAt = updatedRecord.createdAt;
        note->lastModifiedAt = updatedRecord.lastModifiedAt;
        note->author = updatedRecord.author;
        note->modifiedBy = updatedRecord.modifiedBy;
        note->project = updatedRecord.project;
        note->folders = updatedRecord.folders;
        note->folderUuids = updatedRecord.folderUuids;
        note->bookmarkColors = updatedRecord.bookmarkColors;
        note->tags = updatedRecord.tags;
        note->progress = updatedRecord.progress;
        note->bookmarked = updatedRecord.bookmarked;
        note->preset = updatedRecord.preset;
        if (!updatedRecord.noteDirectoryPath.isEmpty())
        {
            note->noteDirectoryPath = updatedRecord.noteDirectoryPath;
        }
        note->noteHeaderPath = updatedRecord.noteHeaderPath;
    }
}

TagsHierarchyViewModel::TagsHierarchyViewModel(QObject* parent)
    : IHierarchyViewModel(parent)
      , m_itemModel(this)
{
    WhatSon::Debug::traceSelf(this, QStringLiteral("tags.viewmodel"), QStringLiteral("ctor"));
    initializeHierarchyInterfaceSignalBridge();
    QObject::connect(
        &m_itemModel,
        &TagsHierarchyModel::itemCountChanged,
        this,
        [this](int)
        {
            updateItemCount();
            setSelectedIndex(m_selectedIndex);
        });
    syncModel();
}

TagsHierarchyViewModel::~TagsHierarchyViewModel() = default;

TagsHierarchyModel* TagsHierarchyViewModel::itemModel() noexcept
{
    return &m_itemModel;
}

LibraryNoteListModel* TagsHierarchyViewModel::noteListModel() noexcept
{
    return &m_noteListModel;
}

int TagsHierarchyViewModel::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

int TagsHierarchyViewModel::itemCount() const noexcept
{
    return m_itemCount;
}

bool TagsHierarchyViewModel::loadSucceeded() const noexcept
{
    return m_loadSucceeded;
}

QString TagsHierarchyViewModel::lastLoadError() const
{
    return m_lastLoadError;
}

void TagsHierarchyViewModel::setSelectedIndex(int index)
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
        return;
    }

    m_selectedIndex = clamped;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("setSelectedIndex"),
                              QStringLiteral("value=%1").arg(m_selectedIndex));
    refreshNoteListForSelection();
    emit selectedIndexChanged();
}

void TagsHierarchyViewModel::setDepthItems(const QVariantList& depthItems)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("setDepthItems.begin"),
                              QStringLiteral("count=%1").arg(depthItems.size()));
    QVector<WhatSonTagDepthEntry> parsedEntries;
    parsedEntries.reserve(depthItems.size());

    for (const QVariant& entry : depthItems)
    {
        WhatSonTagDepthEntry parsedEntry;

        if (entry.metaType().id() == QMetaType::QVariantMap)
        {
            const QVariantMap entryMap = entry.toMap();
            parsedEntry.id = entryMap.value(QStringLiteral("id")).toString().trimmed();
            parsedEntry.label = entryMap.value(QStringLiteral("label")).toString().trimmed();
            parsedEntry.depth = extractDepth(entryMap);
        }
        else
        {
            parsedEntry.depth = 0;
            parsedEntry.label = entry.toString().trimmed();
        }

        if (parsedEntry.id.isEmpty())
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("tags.viewmodel"),
                                      QStringLiteral("setDepthItems.emptyIdKept"),
                                      QStringLiteral("label=%1").arg(parsedEntry.label));
        }
        if (parsedEntry.label.isEmpty())
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("tags.viewmodel"),
                                      QStringLiteral("setDepthItems.emptyLabelKept"));
        }

        parsedEntries.push_back(std::move(parsedEntry));
    }

    setTagDepthEntries(std::move(parsedEntries));
}

QVariantList TagsHierarchyViewModel::hierarchyModel() const
{
    return depthItems();
}

QVariantList TagsHierarchyViewModel::depthItems() const
{
    QVariantList serialized;
    serialized.reserve(m_items.size());

    for (int index = 0; index < m_items.size() && index < m_entries.size(); ++index)
    {
        const WhatSonTagDepthEntry& entry = m_entries.at(index);
        const TagsHierarchyItem& item = m_items.at(index);
        QString itemKey = entry.id.trimmed();
        if (itemKey.isEmpty())
        {
            itemKey = QStringLiteral("tag:%1").arg(index);
        }
        serialized.push_back(QVariantMap{
            {QStringLiteral("itemId"), index},
            {QStringLiteral("key"), itemKey},
            {QStringLiteral("id"), entry.id},
            {QStringLiteral("label"), item.label},
            {QStringLiteral("depth"), item.depth},
            {QStringLiteral("accent"), item.accent},
            {QStringLiteral("expanded"), item.expanded},
            {QStringLiteral("showChevron"), item.showChevron},
            {QStringLiteral("count"), 0}
        });
    }

    return serialized;
}

QString TagsHierarchyViewModel::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }
    return m_items.at(index).label;
}

bool TagsHierarchyViewModel::canRenameItem(int index) const
{
    return index >= 0 && index < m_entries.size();
}

bool TagsHierarchyViewModel::renameItem(int index, const QString& displayName)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("renameItem.begin"),
                              QStringLiteral("index=%1 label=%2").arg(index).arg(displayName));
    if (!canRenameItem(index))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("tags.viewmodel"),
                                  QStringLiteral("renameItem.rejected"),
                                  QStringLiteral("reason=canRenameItem false index=%1").arg(index));
        return false;
    }

    const QString trimmed = displayName.trimmed();
    if (trimmed.isEmpty())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("tags.viewmodel"),
                                  QStringLiteral("renameItem.rejected"),
                                  QStringLiteral("reason=empty label index=%1").arg(index));
        return false;
    }

    QVector<WhatSonTagDepthEntry> stagedEntries = m_entries;
    stagedEntries[index].label = trimmed;
    if (stagedEntries[index].id.trimmed().isEmpty())
    {
        stagedEntries[index].id = trimmed;
    }

    WhatSonTagsHierarchyStore stagedStore = m_store;
    stagedStore.setTagEntries(stagedEntries);

    if (!m_tagsFilePath.trimmed().isEmpty())
    {
        QString writeError;
        if (!stagedStore.writeToFile(m_tagsFilePath, &writeError))
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("tags.viewmodel"),
                                      QStringLiteral("renameItem.writeFailed"),
                                      QStringLiteral("index=%1 path=%2 reason=%3").arg(index).arg(
                                          m_tagsFilePath, writeError));
            return false;
        }
    }

    m_entries = std::move(stagedEntries);
    m_store = std::move(stagedStore);
    m_items = buildItems(m_entries);
    syncModel();
    refreshNoteListForSelection();
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("renameItem.success"),
                              QStringLiteral("index=%1 label=%2 itemCount=%3").arg(index).arg(trimmed).arg(
                                  m_items.size()));
    return true;
}

bool TagsHierarchyViewModel::setItemExpanded(int index, bool expanded)
{
    if (index < 0 || index >= m_items.size())
    {
        return false;
    }

    if (!m_items.at(index).showChevron)
    {
        return false;
    }

    if (m_items.at(index).expanded == expanded)
    {
        return true;
    }

    m_items[index].expanded = expanded;
    syncModel();
    return true;
}

void TagsHierarchyViewModel::createFolder()
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("createFolder.begin"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2").arg(m_selectedIndex).
                                                                              arg(m_entries.size()));
    if (!createFolderEnabled())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("tags.viewmodel"),
                                  QStringLiteral("createFolder.rejected"),
                                  QStringLiteral("reason=createFolderEnabled false"));
        return;
    }

    int insertIndex = m_entries.size();
    int depth = 0;
    int expandedParentIndex = -1;

    if (m_selectedIndex >= 0 && m_selectedIndex < m_entries.size())
    {
        const int selectedDepth = std::max(0, m_entries.at(m_selectedIndex).depth);
        depth = selectedDepth + 1;
        expandedParentIndex = m_selectedIndex;

        insertIndex = m_selectedIndex + 1;
        while (insertIndex < m_entries.size() && m_entries.at(insertIndex).depth > selectedDepth)
        {
            ++insertIndex;
        }
    }

    WhatSonTagDepthEntry entry;
    ++m_createdFolderSequence;
    entry.id.clear();
    entry.label = QStringLiteral("Untitled");
    entry.depth = depth;
    m_entries.insert(insertIndex, std::move(entry));

    m_items = buildItems(m_entries);
    if (expandedParentIndex >= 0 && expandedParentIndex < m_items.size())
    {
        m_items[expandedParentIndex].expanded = true;
    }
    syncStore();
    syncModel();
    setSelectedIndex(insertIndex);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("createFolder.success"),
                              QStringLiteral("insertIndex=%1 label=<empty> depth=%2 itemCount=%3").arg(insertIndex).
                              arg(depth).arg(m_entries.size()));
}

void TagsHierarchyViewModel::deleteSelectedFolder()
{
    const int startIndex = m_selectedIndex;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("deleteSelectedFolder.begin"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2").arg(startIndex).arg(m_entries.size()));
    if (!deleteFolderEnabled())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("tags.viewmodel"),
                                  QStringLiteral("deleteSelectedFolder.rejected"),
                                  QStringLiteral("reason=deleteFolderEnabled false selectedIndex=%1").arg(startIndex));
        return;
    }

    const int baseDepth = m_entries.at(startIndex).depth;

    int removeCount = 1;
    while (startIndex + removeCount < m_entries.size() && m_entries.at(startIndex + removeCount).depth > baseDepth)
    {
        ++removeCount;
    }

    m_entries.remove(startIndex, removeCount);
    m_items = buildItems(m_entries);
    syncStore();
    syncModel();
    setSelectedIndex(std::min(startIndex, static_cast<int>(m_entries.size() - 1)));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("deleteSelectedFolder.success"),
                              QStringLiteral("startIndex=%1 removeCount=%2 itemCount=%3").arg(startIndex).
                              arg(removeCount).arg(m_entries.size()));
}

QString TagsHierarchyViewModel::noteDirectoryPathForNoteId(const QString& noteId) const
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return {};
    }

    const int noteIndex = indexOfNoteRecordById(m_allNotes, normalizedNoteId);
    if (noteIndex < 0 || noteIndex >= m_allNotes.size())
    {
        return {};
    }

    return m_allNotes.at(noteIndex).noteDirectoryPath.trimmed();
}

bool TagsHierarchyViewModel::reloadNoteMetadataForNoteId(const QString& noteId)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    const int noteIndex = indexOfNoteRecordById(m_allNotes, normalizedNoteId);
    if (noteIndex < 0 || noteIndex >= m_allNotes.size())
    {
        return false;
    }

    WhatSonNoteFolderBindingRepository noteRepository;
    WhatSonLocalNoteDocument noteDocument;
    QString ioError;
    if (!noteRepository.readDocument(m_allNotes.at(noteIndex), &noteDocument, &ioError))
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("reloadNoteMetadataForNoteId.failed"),
                                  QStringLiteral("noteId=%1 error=%2").arg(normalizedNoteId, ioError));
        return false;
    }

    syncNoteRecordFromDocument(&m_allNotes[noteIndex], noteDocument);
    refreshNoteListForSelection();
    emit hierarchyModelChanged();
    return true;
}

void TagsHierarchyViewModel::setTagDepthEntries(QVector<WhatSonTagDepthEntry> entries)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("setTagDepthEntries.begin"),
                              QStringLiteral("count=%1").arg(entries.size()));

    QVector<WhatSonTagDepthEntry> sanitized;
    sanitized.reserve(entries.size());

    for (WhatSonTagDepthEntry& entry : entries)
    {
        entry.id = entry.id.trimmed();
        entry.label = entry.label.trimmed();
        entry.depth = std::max(0, entry.depth);

        if (entry.label.isEmpty())
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("tags.viewmodel"),
                                      QStringLiteral("setTagDepthEntries.emptyLabelKept"));
        }
        if (entry.id.isEmpty())
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("tags.viewmodel"),
                                      QStringLiteral("setTagDepthEntries.emptyIdKept"),
                                      QStringLiteral("label=%1").arg(entry.label));
        }

        sanitized.push_back(std::move(entry));
    }

    m_entries = std::move(sanitized);
    m_items = buildItems(m_entries);
    syncStore();
    m_createdFolderSequence = nextFolderSequence(m_entries);
    syncModel();
    setSelectedIndex(-1);
    refreshNoteListForSelection();
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("setTagDepthEntries.success"),
                              QStringLiteral("itemCount=%1").arg(m_items.size()));
}

QVector<WhatSonTagDepthEntry> TagsHierarchyViewModel::tagDepthEntries() const
{
    return m_entries;
}

bool TagsHierarchyViewModel::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("loadFromWshub.begin"),
                              QStringLiteral("path=%1").arg(wshubPath));
    m_tagsFilePath.clear();

    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::TagsSupport::resolveContentsDirectories(wshubPath, &contentsDirectories, &resolveError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = resolveError;
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("tags.viewmodel"),
                                  QStringLiteral("loadFromWshub.failed.resolve"),
                                  QStringLiteral("path=%1 reason=%2").arg(wshubPath, resolveError));
        updateLoadState(false, resolveError);
        return false;
    }

    QVector<WhatSonTagDepthEntry> aggregated;
    bool fileFound = false;
    WhatSonTagsHierarchyParser parser;

    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Tags.wstags"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }
        fileFound = true;

        if (m_tagsFilePath.isEmpty())
        {
            m_tagsFilePath = filePath;
        }

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::TagsSupport::readUtf8File(filePath, &rawText, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("tags.viewmodel"),
                                      QStringLiteral("loadFromWshub.failed.read"),
                                      QStringLiteral("path=%1 reason=%2").arg(filePath, readError));
            updateLoadState(false, readError);
            return false;
        }

        QString parseError;
        if (!parser.parse(rawText, &m_store, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("tags.viewmodel"),
                                      QStringLiteral("loadFromWshub.failed.parse"),
                                      QStringLiteral("path=%1 reason=%2").arg(filePath, parseError));
            updateLoadState(false, parseError);
            return false;
        }

        const QVector<WhatSonTagDepthEntry> parsed = m_store.tagEntries();
        for (const WhatSonTagDepthEntry& entry : parsed)
        {
            aggregated.push_back(entry);
        }
    }

    if (m_tagsFilePath.isEmpty() && !contentsDirectories.isEmpty())
    {
        m_tagsFilePath = QDir(contentsDirectories.first()).filePath(QStringLiteral("Tags.wstags"));
    }

    setTagDepthEntries(std::move(aggregated));
    QString noteLoadError;
    if (!refreshIndexedNotesFromWshub(wshubPath, &noteLoadError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = noteLoadError;
        }
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("loadFromWshub.failed.index"),
                                  QStringLiteral("path=%1 reason=%2").arg(wshubPath, noteLoadError));
        updateLoadState(false, noteLoadError);
        return false;
    }
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("loadFromWshub.success"),
                              QStringLiteral("path=%1 fileFound=%2 count=%3").arg(wshubPath).arg(
                                  fileFound ? QStringLiteral("1") : QStringLiteral("0")).arg(m_entries.size()));
    updateLoadState(true);
    return true;
}

void TagsHierarchyViewModel::applyRuntimeSnapshot(
    QVector<WhatSonTagDepthEntry> entries,
    QString tagsFilePath,
    bool loadSucceeded,
    QString errorMessage)
{
    const QString preservedSelectionKey =
        (m_selectedIndex >= 0 && m_selectedIndex < m_entries.size())
            ? tagHierarchyItemKey(m_entries, m_selectedIndex)
            : QString();
    const QSet<QString> preservedExpandedKeys = expandedTagItemKeys(m_entries, m_items);
    m_tagsFilePath = tagsFilePath.trimmed();
    if (!loadSucceeded)
    {
        updateLoadState(false, errorMessage);
        return;
    }

    if (!tagDepthEntriesEqual(m_entries, entries))
    {
        QVector<WhatSonTagDepthEntry> sanitized;
        sanitized.reserve(entries.size());
        for (WhatSonTagDepthEntry& entry : entries)
        {
            entry.id = entry.id.trimmed();
            entry.label = entry.label.trimmed();
            entry.depth = std::max(0, entry.depth);
            sanitized.push_back(std::move(entry));
        }

        m_entries = std::move(sanitized);
        m_items = buildItems(m_entries);
        restoreExpandedTagItemKeys(&m_items, m_entries, preservedExpandedKeys);
        syncStore();
        m_createdFolderSequence = nextFolderSequence(m_entries);
        syncModel();
        setSelectedIndex(selectedTagIndexForKey(m_entries, preservedSelectionKey));
    }

    QString noteLoadError;
    if (!refreshIndexedNotesFromTagsFilePath(&noteLoadError))
    {
        updateLoadState(false, noteLoadError);
        return;
    }
    updateLoadState(true);
}

void TagsHierarchyViewModel::requestViewModelHook()
{
    if (m_tagsFilePath.trimmed().isEmpty())
    {
        emit viewModelHookRequested();
        return;
    }

    QString reloadError;
    if (!reloadFromTagsFilePath(&reloadError))
    {
        updateLoadState(false, reloadError);
        emit viewModelHookRequested();
        return;
    }

    updateLoadState(true);
    emit viewModelHookRequested();
}

bool TagsHierarchyViewModel::reloadFromTagsFilePath(QString* errorMessage)
{
    const QString normalizedFilePath = m_tagsFilePath.trimmed();
    if (normalizedFilePath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            errorMessage->clear();
        }
        return true;
    }

    QVector<WhatSonTagDepthEntry> refreshedEntries;
    if (QFileInfo(normalizedFilePath).isFile())
    {
        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::TagsSupport::readUtf8File(normalizedFilePath, &rawText, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            return false;
        }

        WhatSonTagsHierarchyStore refreshedStore;
        WhatSonTagsHierarchyParser parser;
        QString parseError;
        if (!parser.parse(rawText, &refreshedStore, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            return false;
        }

        refreshedEntries = refreshedStore.tagEntries();
    }

    applyRuntimeSnapshot(std::move(refreshedEntries), normalizedFilePath, true);
    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }
    return true;
}

bool TagsHierarchyViewModel::renameEnabled() const noexcept
{
    return true;
}

bool TagsHierarchyViewModel::createFolderEnabled() const noexcept
{
    return true;
}

bool TagsHierarchyViewModel::deleteFolderEnabled() const noexcept
{
    return m_selectedIndex >= 0;
}

int TagsHierarchyViewModel::extractDepth(const QVariantMap& entryMap)
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
    return converted ? std::max(0, depth) : 0;
}

QVector<TagsHierarchyItem> TagsHierarchyViewModel::buildItems(const QVector<WhatSonTagDepthEntry>& entries)
{
    QVector<TagsHierarchyItem> items;
    items.reserve(entries.size());

    for (const WhatSonTagDepthEntry& entry : entries)
    {
        TagsHierarchyItem item;
        item.id = entry.id.trimmed();
        item.label = entry.label.trimmed();
        item.depth = std::max(0, entry.depth);

        if (item.label.isEmpty())
        {
            WhatSon::Debug::trace(
                QStringLiteral("tags.viewmodel"),
                QStringLiteral("buildItems.emptyLabelKept"),
                QStringLiteral("id=%1").arg(item.id));
        }

        items.push_back(std::move(item));
    }

    for (int index = 0; index < items.size(); ++index)
    {
        const int nextIndex = index + 1;
        const bool hasChild = nextIndex < items.size() && items.at(nextIndex).depth > items.at(index).depth;
        items[index].showChevron = hasChild;
    }

    return items;
}

int TagsHierarchyViewModel::nextFolderSequence(const QVector<WhatSonTagDepthEntry>& entries)
{
    static const QRegularExpression folderPattern(QStringLiteral("^Folder(\\d+)$"));

    int maxSequence = 0;
    for (const WhatSonTagDepthEntry& entry : entries)
    {
        const QRegularExpressionMatch match = folderPattern.match(entry.label.trimmed());
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

void TagsHierarchyViewModel::updateItemCount()
{
    const int nextCount = m_itemModel.rowCount();
    if (m_itemCount == nextCount)
    {
        return;
    }
    m_itemCount = nextCount;
    emit itemCountChanged();
}

void TagsHierarchyViewModel::updateLoadState(bool succeeded, QString errorMessage)
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

LibraryNoteListItem TagsHierarchyViewModel::buildNoteListItem(const LibraryNoteRecord& note) const
{
    const QStringList folderLabels = noteListFolders(note);

    LibraryNoteListItem item;
    item.id = note.noteId.trimmed();
    item.primaryText = notePrimaryText(note);
    item.searchableText = noteSearchableText(note, folderLabels);
    item.bodyText = note.bodySourceText.isEmpty() ? note.bodyPlainText : note.bodySourceText;
    item.createdAt = note.createdAt;
    item.lastModifiedAt = note.lastModifiedAt;
    item.image = note.bodyHasResource;
    item.imageSource = note.bodyFirstResourceThumbnailUrl;
    item.displayDate = SystemCalendarStore::formatNoteDateForSystem(note.lastModifiedAt, note.createdAt);
    item.folders = folderLabels;
    item.tags = noteListTags(note);
    item.bookmarked = note.bookmarked;
    item.bookmarkColor = bookmarkColorHexFromNote(note);
    return item;
}

void TagsHierarchyViewModel::refreshNoteListForSelection()
{
    const QSet<QString> projectionKeys = tagProjectionKeysForSelection(m_entries, m_selectedIndex);

    QVector<LibraryNoteListItem> items;
    items.reserve(m_allNotes.size());
    for (const LibraryNoteRecord& note : std::as_const(m_allNotes))
    {
        if (!noteMatchesTagProjection(note, projectionKeys))
        {
            continue;
        }

        items.push_back(buildNoteListItem(note));
    }

    m_noteListModel.setItems(std::move(items));
}

bool TagsHierarchyViewModel::refreshIndexedNotesFromWshub(const QString& wshubPath, QString* errorMessage)
{
    LibraryAll libraryAll;
    if (!libraryAll.indexFromWshub(wshubPath, errorMessage))
    {
        m_allNotes.clear();
        m_noteListModel.setItems({});
        emit hierarchyModelChanged();
        return false;
    }

    m_allNotes = libraryAll.notes();
    refreshNoteListForSelection();
    emit hierarchyModelChanged();
    return true;
}

bool TagsHierarchyViewModel::refreshIndexedNotesFromTagsFilePath(QString* errorMessage)
{
    const QString wshubPath = resolveWshubPathFromTagsFile(m_tagsFilePath);
    if (wshubPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to resolve .wshub path from Tags.wstags.");
        }
        m_allNotes.clear();
        m_noteListModel.setItems({});
        emit hierarchyModelChanged();
        return false;
    }

    return refreshIndexedNotesFromWshub(wshubPath, errorMessage);
}

void TagsHierarchyViewModel::syncStore()
{
    m_store.setTagEntries(m_entries);
}

void TagsHierarchyViewModel::syncModel()
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("tags.viewmodel"),
                              QStringLiteral("syncModel"),
                              QStringLiteral("itemCount=%1").arg(m_items.size()));
    m_itemModel.setItems(m_items);
    updateItemCount();
    emit hierarchyModelChanged();
}
