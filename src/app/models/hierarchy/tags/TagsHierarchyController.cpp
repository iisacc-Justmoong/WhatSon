#include "app/models/hierarchy/tags/TagsHierarchyController.hpp"

#include "app/models/calendar/SystemCalendarStore.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/models/hierarchy/WhatSonHierarchyNoteRecordSupport.hpp"
#include "app/models/hierarchy/library/LibraryAll.hpp"
#include "app/models/hierarchy/tags/WhatSonTagsHierarchyParser.hpp"
#include "app/models/hierarchy/tags/WhatSonTagsHierarchyStore.hpp"
#include "app/models/file/note/header/WhatSonBookmarkColorPalette.hpp"
#include "app/models/hierarchy/WhatSonHierarchyTreeItemSupport.hpp"
#include "app/models/hierarchy/tags/TagsHierarchyControllerSupport.hpp"
#include "app/models/sidebar/SidebarHierarchyLvrsSupport.hpp"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <utility>

#include <algorithm>

namespace
{
    constexpr auto kScope = "tags.controller";
    constexpr int kMaxNoteListSummaryLines = 5;

    QString leafTagNameFromPath(const QString& path)
    {
        const QString normalized = path.trimmed();
        if (normalized.isEmpty())
        {
            return {};
        }

        const int slashIndex = normalized.lastIndexOf(QLatin1Char('/'));
        if (slashIndex < 0)
        {
            return normalized;
        }

        return normalized.mid(slashIndex + 1).trimmed();
    }

    QString normalizedTagKeySegment(const QString& label, int index)
    {
        const QString normalizedLabel = label.trimmed();
        if (!normalizedLabel.isEmpty())
        {
            return normalizedLabel;
        }
        return QStringLiteral("item:%1").arg(index);
    }

    QString tagsHierarchyItemKey(const QVector<TagsHierarchyItem>& items, int index)
    {
        if (index < 0 || index >= items.size())
        {
            return {};
        }

        QStringList pathSegments;
        pathSegments.reserve(std::max(1, items.at(index).depth + 1));
        pathSegments.push_front(normalizedTagKeySegment(items.at(index).label, index));

        int expectedDepth = items.at(index).depth;
        for (int cursor = index - 1; cursor >= 0 && expectedDepth > 0; --cursor)
        {
            const TagsHierarchyItem& candidate = items.at(cursor);
            if (candidate.depth != expectedDepth - 1)
            {
                continue;
            }
            pathSegments.push_front(normalizedTagKeySegment(candidate.label, cursor));
            expectedDepth = candidate.depth;
        }

        return pathSegments.join(QLatin1Char('/'));
    }

    void finalizeTagItems(QVector<TagsHierarchyItem>* items)
    {
        if (items == nullptr)
        {
            return;
        }

        int maxNextDepth = 0;
        for (TagsHierarchyItem& item : *items)
        {
            item.label = item.label.trimmed();
            item.depth = std::clamp(std::max(0, item.depth), 0, maxNextDepth);
            maxNextDepth = item.depth + 1;
        }

        WhatSon::Hierarchy::TagsSupport::applyChevronByDepth(items);
    }

    QVector<WhatSonTagDepthEntry> tagEntriesFromItems(const QVector<TagsHierarchyItem>& items)
    {
        QVector<WhatSonTagDepthEntry> entries;
        entries.reserve(items.size());

        for (int index = 0; index < items.size(); ++index)
        {
            const TagsHierarchyItem& item = items.at(index);
            const QString label = item.label.trimmed();
            if (label.isEmpty())
            {
                continue;
            }
            if (item.accent && item.depth == 0)
            {
                continue;
            }

            WhatSonTagDepthEntry entry;
            entry.id = tagsHierarchyItemKey(items, index);
            if (entry.id.trimmed().isEmpty())
            {
                entry.id = label;
            }
            entry.label = label;
            entry.depth = std::max(0, item.depth);
            entries.push_back(std::move(entry));
        }

        return entries;
    }

    QVector<TagsHierarchyItem> itemsFromTagEntries(const QVector<WhatSonTagDepthEntry>& entries)
    {
        QVector<TagsHierarchyItem> items;
        items.reserve(entries.size());

        int maxNextDepth = 0;
        for (const WhatSonTagDepthEntry& entry : entries)
        {
            QString label = entry.label.trimmed();
            if (label.isEmpty())
            {
                label = leafTagNameFromPath(entry.id);
            }
            if (label.isEmpty())
            {
                continue;
            }

            TagsHierarchyItem item;
            item.depth = std::clamp(std::max(0, entry.depth), 0, maxNextDepth);
            item.label = label;
            item.accent = false;
            item.expanded = false;
            item.showChevron = false;
            items.push_back(std::move(item));
            maxNextDepth = items.constLast().depth + 1;
        }

        finalizeTagItems(&items);
        return items;
    }

    int selectedTagIndexForKey(const QVector<TagsHierarchyItem>& items, const QString& key)
    {
        const QString normalizedKey = key.trimmed();
        if (normalizedKey.isEmpty())
        {
            return -1;
        }

        for (int index = 0; index < items.size(); ++index)
        {
            if (tagsHierarchyItemKey(items, index) == normalizedKey)
            {
                return index;
            }
        }

        return -1;
    }

    bool folderDepthEntriesEqual(
        const QVector<WhatSonTagDepthEntry>& lhs,
        const QVector<WhatSonTagDepthEntry>& rhs)
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
                || std::max(0, left.depth) != std::max(0, right.depth)
                || left.uuid.trimmed() != right.uuid.trimmed())
            {
                return false;
            }
        }

        return true;
    }

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
        Q_UNUSED(kMaxNoteListSummaryLines)
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

        const QString tag = note.tags.join(QStringLiteral(" ")).trimmed();
        if (!tag.isEmpty())
        {
            parts.push_back(tag);
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

    bool tagValueMatchesHierarchyItem(
        const QString& tagValue,
        const QVector<TagsHierarchyItem>& items,
        int itemIndex)
    {
        if (itemIndex < 0 || itemIndex >= items.size())
        {
            return false;
        }

        const QString normalizedTagValue = tagValue.trimmed();
        if (normalizedTagValue.isEmpty())
        {
            return false;
        }

        const TagsHierarchyItem& item = items.at(itemIndex);
        if (normalizedTagValue.compare(item.label.trimmed(), Qt::CaseInsensitive) == 0)
        {
            return true;
        }

        return normalizedTagValue.compare(
            tagsHierarchyItemKey(items, itemIndex),
            Qt::CaseInsensitive) == 0;
    }

    int noteCountForTagItem(
        const QVector<LibraryNoteRecord>& notes,
        const QVector<TagsHierarchyItem>& items,
        int itemIndex)
    {
        if (itemIndex < 0 || itemIndex >= items.size())
        {
            return 0;
        }

        int noteCount = 0;
        for (const LibraryNoteRecord& note : notes)
        {
            if (std::any_of(note.tags.cbegin(), note.tags.cend(), [&](const auto &tag) { return tagValueMatchesHierarchyItem(tag, items, itemIndex); }))
            {
                ++noteCount;
            }
        }
        return noteCount;
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

    bool isProtectedRootItem(const TagsHierarchyItem& item)
    {
        return item.accent && item.depth == 0;
    }

    int firstEditableInsertIndex(const QVector<TagsHierarchyItem>& items)
    {
        int index = 0;
        while (index < items.size() && isProtectedRootItem(items.at(index)))
        {
            ++index;
        }
        return index;
    }

    bool isEditableFolderItem(const QVector<TagsHierarchyItem>& items, int index)
    {
        return index >= 0 && index < items.size() && !isProtectedRootItem(items.at(index));
    }

    int subtreeEndIndexExclusive(const QVector<TagsHierarchyItem>& items, int startIndex)
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

    bool resolveFolderMoveOperation(
        const QVector<TagsHierarchyItem>& items,
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

        int rawInsertIndex = firstEditableInsertIndex(items);
        int newBaseDepth = 0;

        switch (placement)
        {
        case FolderDropPlacement::RootTop:
            rawInsertIndex = firstEditableInsertIndex(items);
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

    QVector<TagsHierarchyItem> stageFolderMoveItems(
        const QVector<TagsHierarchyItem>& items,
        int sourceIndex,
        const FolderMoveOperation& operation)
    {
        const int depthDelta = operation.newBaseDepth - items.at(sourceIndex).depth;

        QVector<TagsHierarchyItem> movedItems;
        movedItems.reserve(operation.sourceCount);
        for (int index = sourceIndex; index < operation.sourceEndIndex; ++index)
        {
            TagsHierarchyItem item = items.at(index);
            item.depth = std::max(0, item.depth + depthDelta);
            movedItems.push_back(std::move(item));
        }

        QVector<TagsHierarchyItem> stagedItems = items;
        stagedItems.remove(sourceIndex, operation.sourceCount);
        for (int offset = 0; offset < movedItems.size(); ++offset)
        {
            stagedItems.insert(operation.normalizedInsertIndex + offset, std::move(movedItems[offset]));
        }

        finalizeTagItems(&stagedItems);
        return stagedItems;
    }

    bool resolveFolderMoveOperationFromLvrsMoveEvent(
        const QVector<TagsHierarchyItem>& items,
        int sourceIndex,
        int targetIndex,
        int targetDepth,
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

        QVector<TagsHierarchyItem> remainingItems = items;
        remainingItems.remove(sourceIndex, sourceCount);

        const int firstEditableIndex = firstEditableInsertIndex(items);
        const int maxInsertIndex = std::max(0, static_cast<int>(remainingItems.size()));
        int normalizedInsertIndex = std::clamp(targetIndex, 0, maxInsertIndex);
        normalizedInsertIndex = std::max(normalizedInsertIndex, firstEditableIndex);

        int minDepth = normalizedInsertIndex < remainingItems.size()
            ? std::max(0, remainingItems.at(normalizedInsertIndex).depth)
            : 0;
        int maxDepth = normalizedInsertIndex > 0
            ? std::max(0, remainingItems.at(normalizedInsertIndex - 1).depth + 1)
            : 0;
        if (normalizedInsertIndex <= firstEditableIndex
            || (normalizedInsertIndex > 0 && isProtectedRootItem(remainingItems.at(normalizedInsertIndex - 1))))
        {
            minDepth = 0;
            maxDepth = 0;
        }
        if (maxDepth < minDepth)
        {
            maxDepth = minDepth;
        }

        const int newBaseDepth = std::clamp(std::max(0, targetDepth), minDepth, maxDepth);
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
}

TagsHierarchyController::TagsHierarchyController(QObject* parent)
    : IHierarchyController(parent)
      , m_itemModel(this)
{
    WhatSon::Debug::traceSelf(this, QString::fromLatin1(kScope), QStringLiteral("ctor"));
    initializeHierarchyInterfaceSignalBridge();
    QObject::connect(
        &m_itemModel,
        &WhatSonHierarchyModel::itemCountChanged,
        this,
        [this](int)
        {
            updateItemCount();
            setSelectedIndex(m_selectedIndex);
        });
    setTagNames({});
}

TagsHierarchyController::~TagsHierarchyController() = default;

WhatSonHierarchyModel* TagsHierarchyController::itemModel() noexcept
{
    return &m_itemModel;
}

LibraryNoteListModel* TagsHierarchyController::noteListModel() noexcept
{
    return &m_noteListModel;
}

bool TagsHierarchyController::supportsHierarchyNodeReorder() const noexcept
{
    return true;
}

QString TagsHierarchyController::noteDirectoryPathForNoteId(const QString& noteId) const
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return {};
    }

    return WhatSon::Hierarchy::NoteRecordSupport::directoryPathForNoteId(m_allNotes, normalizedNoteId);
}

int TagsHierarchyController::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

int TagsHierarchyController::itemCount() const noexcept
{
    return m_itemCount;
}

bool TagsHierarchyController::loadSucceeded() const noexcept
{
    return m_loadSucceeded;
}

QString TagsHierarchyController::lastLoadError() const
{
    return m_lastLoadError;
}

void TagsHierarchyController::setSelectedIndex(int index)
{
    const int clamped = WhatSon::Hierarchy::TreeItemSupport::clampSelectionIndexToVisibleDefault(
        index,
        m_itemModel.rowCount());
    if (m_selectedIndex == clamped)
    {
        return;
    }

    m_selectedIndex = clamped;
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setSelectedIndex"),
                              QStringLiteral("value=%1").arg(m_selectedIndex));
    refreshNoteListForSelection();
    emit selectedIndexChanged();
}

void TagsHierarchyController::setDepthItems(const QVariantList& depthItems)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setDepthItems.begin"),
                              QStringLiteral("count=%1").arg(depthItems.size()));
    m_items = itemsFromTagEntries(
        tagEntriesFromItems(
            WhatSon::Hierarchy::TagsSupport::parseDepthItems(depthItems, QStringLiteral("Tag"))));
    syncDomainStoreFromItems();
    syncModel();
    setSelectedIndex(-1);
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setDepthItems.success"),
                              QStringLiteral("itemCount=%1").arg(m_items.size()));
}

QVariantList TagsHierarchyController::hierarchyModel() const
{
    return depthItems();
}

QVariantList TagsHierarchyController::depthItems() const
{
    QVariantList serialized = WhatSon::Hierarchy::TagsSupport::serializeDepthItems(m_items);
    for (int index = 0; index < serialized.size() && index < m_items.size(); ++index)
    {
        QVariantMap entry = serialized.at(index).toMap();
        const int noteCount = std::max(0, noteCountForTagItem(m_allNotes, m_items, index));
        entry.insert(QStringLiteral("draggable"), canMoveFolder(index));
        entry.insert(QStringLiteral("itemId"), index);
        entry.insert(QStringLiteral("key"), tagsHierarchyItemKey(m_items, index));
        entry.insert(QStringLiteral("count"), noteCount);
        serialized[index] = entry;
    }
    return serialized;
}

QString TagsHierarchyController::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    return m_items.at(index).label;
}

bool TagsHierarchyController::canRenameItem(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return false;
    }

    const TagsHierarchyItem& item = m_items.at(index);
    return !(item.accent && item.depth == 0);
}

bool TagsHierarchyController::renameItem(int index, const QString& displayName)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("renameItem.begin"),
                              QStringLiteral("index=%1 label=%2").arg(index).arg(displayName));
    if (!canRenameItem(index))
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("renameItem.rejected"),
                                  QStringLiteral("reason=canRenameItem false index=%1").arg(index));
        return false;
    }

    QVector<TagsHierarchyItem> stagedItems = m_items;
    if (!WhatSon::Hierarchy::TagsSupport::renameHierarchyItem(&stagedItems, index, displayName))
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("renameItem.rejected"),
                                  QStringLiteral("reason=support rejected index=%1 label=%2").arg(index).arg(
                                      displayName));
        return false;
    }
    finalizeTagItems(&stagedItems);

    WhatSonTagsHierarchyStore stagedStore = m_store;
    stagedStore.setTagEntries(tagEntriesFromItems(stagedItems));

    if (!m_tagsFilePath.trimmed().isEmpty())
    {
        QString writeError;
        if (!stagedStore.writeToFile(m_tagsFilePath, &writeError))
        {
            WhatSon::Debug::traceSelf(this,
                                      QString::fromLatin1(kScope),
                                      QStringLiteral("renameItem.writeFailed"),
                                      QStringLiteral("index=%1 path=%2 reason=%3").arg(index).arg(
                                          m_tagsFilePath, writeError));
            return false;
        }
    }

    m_items = std::move(stagedItems);
    m_store = std::move(stagedStore);
    m_tagNames = m_store.tagNames();
    syncModel();
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("renameItem.success"),
                              QStringLiteral("index=%1 label=%2 itemCount=%3").arg(index).arg(displayName).arg(
                                  m_items.size()));
    return true;
}

bool TagsHierarchyController::setItemExpanded(int index, bool expanded)
{
    return setHierarchyItemExpanded(
        &m_items,
        index,
        expanded,
        [this](int changedIndex, bool changedExpanded)
        {
            m_itemModel.setItemExpanded(changedIndex, changedExpanded);
            emit hierarchyModelChanged();
        });
}

bool TagsHierarchyController::setAllItemsExpanded(bool expanded)
{
    return setAllHierarchyItemsExpanded(
        &m_items,
        expanded,
        [this]()
        {
            syncModel();
        });
}

void TagsHierarchyController::createFolder()
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("createFolder.begin"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2").arg(m_selectedIndex).arg(m_items.size()));
    if (!createFolderEnabled())
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("createFolder.rejected"),
                                  QStringLiteral("reason=createFolderEnabled false"));
        return;
    }

    QVector<TagsHierarchyItem> stagedItems = m_items;
    const int insertIndex = WhatSon::Hierarchy::TagsSupport::createHierarchyFolder(
        &stagedItems, m_selectedIndex, &m_createdFolderSequence);
    if (insertIndex < 0)
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("createFolder.rejected"),
                                  QStringLiteral("reason=insertIndex invalid"));
        return;
    }

    if (!commitHierarchyUpdate(std::move(stagedItems), insertIndex))
    {
        return;
    }
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("createFolder.success"),
                              QStringLiteral("insertIndex=%1 itemCount=%2").arg(insertIndex).arg(m_items.size()));
}

void TagsHierarchyController::deleteSelectedFolder()
{
    const int startIndex = m_selectedIndex;
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("deleteSelectedFolder.begin"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2").arg(startIndex).arg(m_items.size()));
    if (!deleteFolderEnabled())
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("deleteSelectedFolder.rejected"),
                                  QStringLiteral("reason=deleteFolderEnabled false selectedIndex=%1").arg(startIndex));
        return;
    }

    QVector<TagsHierarchyItem> stagedItems = m_items;
    const int nextSelectedIndex =
        WhatSon::Hierarchy::TagsSupport::deleteHierarchySubtree(&stagedItems, m_selectedIndex);
    if (!commitHierarchyUpdate(std::move(stagedItems), nextSelectedIndex))
    {
        return;
    }
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("deleteSelectedFolder.success"),
                              QStringLiteral("startIndex=%1 nextIndex=%2 itemCount=%3").arg(startIndex).arg(
                                  nextSelectedIndex).arg(m_items.size()));
}

bool TagsHierarchyController::canMoveFolder(int index) const
{
    return isEditableFolderItem(m_items, index);
}

bool TagsHierarchyController::canAcceptFolderDropBefore(int sourceIndex, int targetIndex) const
{
    return resolveFolderMoveOperation(
        m_items,
        sourceIndex,
        targetIndex,
        FolderDropPlacement::Before,
        nullptr);
}

bool TagsHierarchyController::moveFolderBefore(int sourceIndex, int targetIndex)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("moveFolderBefore.begin"),
                              QStringLiteral("sourceIndex=%1 targetIndex=%2").arg(sourceIndex).arg(targetIndex));

    FolderMoveOperation operation;
    if (!resolveFolderMoveOperation(
        m_items,
        sourceIndex,
        targetIndex,
        FolderDropPlacement::Before,
        &operation))
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("moveFolderBefore.rejected"),
                                  QStringLiteral("sourceIndex=%1 targetIndex=%2").arg(sourceIndex).arg(targetIndex));
        return false;
    }

    return commitHierarchyUpdate(
        stageFolderMoveItems(m_items, sourceIndex, operation),
        operation.normalizedInsertIndex);
}

bool TagsHierarchyController::canAcceptFolderDrop(int sourceIndex, int targetIndex, bool asChild) const
{
    return resolveFolderMoveOperation(
        m_items,
        sourceIndex,
        targetIndex,
        asChild ? FolderDropPlacement::Child : FolderDropPlacement::After,
        nullptr);
}

bool TagsHierarchyController::moveFolder(int sourceIndex, int targetIndex, bool asChild)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("moveFolder.begin"),
                              QStringLiteral("sourceIndex=%1 targetIndex=%2 asChild=%3")
                              .arg(sourceIndex)
                              .arg(targetIndex)
                              .arg(asChild ? QStringLiteral("1") : QStringLiteral("0")));
    FolderMoveOperation operation;
    if (!resolveFolderMoveOperation(
        m_items,
        sourceIndex,
        targetIndex,
        asChild ? FolderDropPlacement::Child : FolderDropPlacement::After,
        &operation))
    {
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("moveFolder.rejected"),
                                  QStringLiteral("sourceIndex=%1 targetIndex=%2 asChild=%3")
                                  .arg(sourceIndex)
                                  .arg(targetIndex)
                                  .arg(asChild ? QStringLiteral("1") : QStringLiteral("0")));
        return false;
    }

    return commitHierarchyUpdate(
        stageFolderMoveItems(m_items, sourceIndex, operation),
        operation.normalizedInsertIndex);
}

bool TagsHierarchyController::canMoveFolderToRoot(int sourceIndex) const
{
    return resolveFolderMoveOperation(
        m_items,
        sourceIndex,
        -1,
        FolderDropPlacement::RootTop,
        nullptr);
}

bool TagsHierarchyController::moveFolderToRoot(int sourceIndex)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("moveFolderToRoot.begin"),
                              QStringLiteral("sourceIndex=%1").arg(sourceIndex));

    FolderMoveOperation operation;
    if (!resolveFolderMoveOperation(
        m_items,
        sourceIndex,
        -1,
        FolderDropPlacement::RootTop,
        &operation))
    {
        return false;
    }

    return commitHierarchyUpdate(
        stageFolderMoveItems(m_items, sourceIndex, operation),
        operation.normalizedInsertIndex);
}

bool TagsHierarchyController::applyHierarchyNodes(const QVariantList& hierarchyNodes, const QString& activeItemKey)
{
    const QVector<WhatSon::Sidebar::Lvrs::FlatNode> flattened =
        WhatSon::Sidebar::Lvrs::flattenHierarchyNodes(hierarchyNodes);
    if (flattened.isEmpty())
    {
        return false;
    }

    QVector<TagsHierarchyItem> stagedItems;
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

        TagsHierarchyItem item;
        item.depth = std::max(0, node.depth);
        item.label = node.label.trimmed();
        item.accent = node.accent;
        item.expanded = node.expanded;
        item.showChevron = node.showChevron;
        stagedItems.push_back(std::move(item));
    }

    finalizeTagItems(&stagedItems);
    return commitHierarchyUpdate(std::move(stagedItems), selectedIndex);
}

bool TagsHierarchyController::applyHierarchyMove(
    const int sourceIndex,
    const int targetIndex,
    const int targetDepth,
    const QString& activeItemKey)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("applyHierarchyMove.begin"),
                              QStringLiteral("sourceIndex=%1 targetIndex=%2 targetDepth=%3")
                              .arg(sourceIndex)
                              .arg(targetIndex)
                              .arg(targetDepth));

    FolderMoveOperation operation;
    if (!resolveFolderMoveOperationFromLvrsMoveEvent(
        m_items,
        sourceIndex,
        targetIndex,
        targetDepth,
        &operation))
    {
        return false;
    }

    QVector<TagsHierarchyItem> stagedItems = stageFolderMoveItems(m_items, sourceIndex, operation);
    int selectedIndex = selectedTagIndexForKey(stagedItems, activeItemKey);
    if (selectedIndex < 0)
    {
        selectedIndex = operation.normalizedInsertIndex;
    }
    return commitHierarchyUpdate(std::move(stagedItems), selectedIndex);
}

void TagsHierarchyController::setTagNames(QStringList tagNames)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setTagNames.begin"),
                              QStringLiteral("rawCount=%1").arg(tagNames.size()));
    m_tagNames = WhatSon::Hierarchy::TagsSupport::sanitizeStringList(std::move(tagNames));
    m_store.setTagNames(m_tagNames);
    m_items = WhatSon::Hierarchy::TagsSupport::buildBucketItems(
        QStringLiteral("Tags"),
        m_tagNames,
        QStringLiteral("Tag"));
    m_createdFolderSequence = WhatSon::Hierarchy::TagsSupport::nextGeneratedFolderSequence(m_items);
    syncModel();
    setSelectedIndex(-1);
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("setTagNames.success"),
                              QStringLiteral("sanitizedCount=%1 itemCount=%2").arg(m_tagNames.size()).arg(
                                  m_items.size()));
}

QStringList TagsHierarchyController::tagNames() const
{
    return m_tagNames;
}

bool TagsHierarchyController::renameEnabled() const noexcept
{
    return true;
}

bool TagsHierarchyController::createFolderEnabled() const noexcept
{
    return true;
}

bool TagsHierarchyController::deleteFolderEnabled() const noexcept
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        return false;
    }

    const TagsHierarchyItem& selectedItem = m_items.at(m_selectedIndex);
    return !(selectedItem.accent && selectedItem.depth == 0);
}

bool TagsHierarchyController::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    WhatSon::Debug::traceSelf(this,
                              QString::fromLatin1(kScope),
                              QStringLiteral("loadFromWshub.begin"),
                              QStringLiteral("path=%1").arg(wshubPath));
    m_tagsFilePath.clear();

    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::TagsSupport::resolveContentsDirectories(
        wshubPath, &contentsDirectories, &resolveError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = resolveError;
        }
        WhatSon::Debug::traceSelf(this,
                                  QString::fromLatin1(kScope),
                                  QStringLiteral("loadFromWshub.failed.resolve"),
                                  QStringLiteral("path=%1 reason=%2").arg(wshubPath, resolveError));
        updateLoadState(false, resolveError);
        return false;
    }

    QVector<WhatSonTagDepthEntry> aggregatedEntries;
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
                                      QString::fromLatin1(kScope),
                                      QStringLiteral("loadFromWshub.failed.read"),
                                      QStringLiteral("path=%1 reason=%2").arg(filePath, readError));
            updateLoadState(false, readError);
            return false;
        }

        QString parseError;
        WhatSonTagsHierarchyStore parsedStore;
        if (!parser.parse(rawText, &parsedStore, &parseError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            WhatSon::Debug::traceSelf(this,
                                      QString::fromLatin1(kScope),
                                      QStringLiteral("loadFromWshub.failed.parse"),
                                      QStringLiteral("path=%1 reason=%2").arg(filePath, parseError));
            updateLoadState(false, parseError);
            return false;
        }

        const QVector<WhatSonTagDepthEntry> parsedEntries = parsedStore.tagEntries();
        for (const WhatSonTagDepthEntry& entry : parsedEntries)
        {
            aggregatedEntries.push_back(entry);
        }
    }

    if (m_tagsFilePath.isEmpty() && !contentsDirectories.isEmpty())
    {
        m_tagsFilePath = QDir(contentsDirectories.first()).filePath(QStringLiteral("Tags.wstags"));
    }

    if (aggregatedEntries.isEmpty())
    {
        setTagNames({});
    }
    else
    {
        m_store.setTagEntries(std::move(aggregatedEntries));
        m_tagNames = m_store.tagNames();
        m_items = itemsFromTagEntries(m_store.tagEntries());
        m_createdFolderSequence = WhatSon::Hierarchy::TagsSupport::nextGeneratedFolderSequence(m_items);
        syncModel();
        setSelectedIndex(-1);
    }

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
                              QString::fromLatin1(kScope),
                              QStringLiteral("loadFromWshub"),
                              QStringLiteral("path=%1 fileFound=%2 count=%3 entryCount=%4")
                              .arg(wshubPath)
                              .arg(fileFound ? QStringLiteral("1") : QStringLiteral("0"))
                              .arg(m_tagNames.size())
                              .arg(m_store.tagEntries().size()));

    if (WhatSon::Debug::isEnabled())
    {
        qWarning().noquote()
            << QStringLiteral("[tags:index] path=%1 count=%2 values=[%3]")
               .arg(wshubPath)
               .arg(m_tagNames.size())
               .arg(m_tagNames.join(QStringLiteral(", ")));
    }

    updateLoadState(true);
    return true;
}

void TagsHierarchyController::applyRuntimeSnapshot(
    QVector<WhatSonTagDepthEntry> tagEntries,
    QString tagsFilePath,
    bool loadSucceeded,
    QString errorMessage)
{
    const QString preservedSelectionKey =
        (m_selectedIndex >= 0 && m_selectedIndex < m_items.size())
            ? tagsHierarchyItemKey(m_items, m_selectedIndex)
            : QString();
    m_tagsFilePath = tagsFilePath.trimmed();

    if (!loadSucceeded)
    {
        updateLoadState(false, std::move(errorMessage));
        return;
    }

    if (!folderDepthEntriesEqual(tagEntriesFromItems(m_items), tagEntries))
    {
        if (tagEntries.isEmpty())
        {
            m_tagNames.clear();
            m_store.setTagEntries({});
            m_items.clear();
            m_createdFolderSequence = 1;
            syncModel();
            setSelectedIndex(-1);
        }
        else
        {
            m_store.setTagEntries(tagEntries);
            m_tagNames = m_store.tagNames();
            m_items = itemsFromTagEntries(m_store.tagEntries());
            m_createdFolderSequence = WhatSon::Hierarchy::TagsSupport::nextGeneratedFolderSequence(m_items);
            syncModel();
            setSelectedIndex(selectedTagIndexForKey(m_items, preservedSelectionKey));
        }
    }

    QString noteLoadError;
    if (!refreshIndexedNotesFromTagsFilePath(&noteLoadError))
    {
        updateLoadState(false, noteLoadError);
        return;
    }

    updateLoadState(true);
}

void TagsHierarchyController::requestControllerHook()
{
    if (m_tagsFilePath.trimmed().isEmpty())
    {
        emit controllerHookRequested();
        return;
    }

    QString noteLoadError;
    if (!refreshIndexedNotesFromTagsFilePath(&noteLoadError))
    {
        updateLoadState(false, noteLoadError);
        emit controllerHookRequested();
        return;
    }

    updateLoadState(true);
    emit controllerHookRequested();
}

void TagsHierarchyController::updateItemCount()
{
    const int nextCount = m_itemModel.rowCount();
    if (m_itemCount == nextCount)
    {
        return;
    }
    m_itemCount = nextCount;
    emit itemCountChanged();
}

void TagsHierarchyController::updateLoadState(bool succeeded, QString errorMessage)
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

void TagsHierarchyController::syncModel()
{
    m_itemModel.setItems(depthItems());
    updateItemCount();
    emit hierarchyModelChanged();
    refreshNoteListForSelection();
}

bool TagsHierarchyController::commitHierarchyUpdate(QVector<TagsHierarchyItem> stagedItems, int selectedIndex)
{
    finalizeTagItems(&stagedItems);

    WhatSonTagsHierarchyStore stagedStore = m_store;
    stagedStore.setTagEntries(tagEntriesFromItems(stagedItems));

    if (!m_tagsFilePath.trimmed().isEmpty())
    {
        QString writeError;
        if (!stagedStore.writeToFile(m_tagsFilePath, &writeError))
        {
            WhatSon::Debug::traceSelf(this,
                                      QString::fromLatin1(kScope),
                                      QStringLiteral("commitHierarchyUpdate.writeFailed"),
                                      QStringLiteral("path=%1 reason=%2").arg(m_tagsFilePath, writeError));
            return false;
        }
    }

    m_items = std::move(stagedItems);
    m_store = std::move(stagedStore);
    m_tagNames = m_store.tagNames();
    m_createdFolderSequence = WhatSon::Hierarchy::TagsSupport::nextGeneratedFolderSequence(m_items);
    syncModel();
    setSelectedIndex(selectedIndex);
    return true;
}

void TagsHierarchyController::syncDomainStoreFromItems()
{
    finalizeTagItems(&m_items);
    m_store.setTagEntries(tagEntriesFromItems(m_items));
    m_tagNames = m_store.tagNames();
}

LibraryNoteListItem TagsHierarchyController::buildNoteListItem(const LibraryNoteRecord& note) const
{
    const QStringList folderLabels = noteListFolders(note);

    LibraryNoteListItem item;
    item.id = note.noteId.trimmed();
    item.primaryText = notePrimaryText(note);
    item.searchableText = noteSearchableText(note, folderLabels);
    item.bodyText.clear();
    item.createdAt = note.createdAt;
    item.lastModifiedAt = note.lastModifiedAt;
    item.image = false;
    item.imageSource.clear();
    item.displayDate = SystemCalendarStore::formatNoteDateForSystem(note.lastModifiedAt, note.createdAt);
    item.folders = folderLabels;
    item.tags = noteListTags(note);
    item.bookmarked = note.bookmarked;
    item.bookmarkColor = bookmarkColorHexFromNote(note);
    return item;
}

void TagsHierarchyController::refreshNoteListForSelection(const bool synchronizeTagHeaders)
{
    Q_UNUSED(synchronizeTagHeaders)

    QSet<QString> availableTagKeys;
    availableTagKeys.reserve(m_items.size() * 2);
    for (int index = 0; index < m_items.size(); ++index)
    {
        const TagsHierarchyItem& item = m_items.at(index);
        if (item.accent && item.depth == 0)
        {
            continue;
        }

        const QString label = item.label.trimmed();
        if (label.isEmpty())
        {
            continue;
        }

        availableTagKeys.insert(label.toCaseFolded());
        availableTagKeys.insert(tagsHierarchyItemKey(m_items, index).toCaseFolded());
    }

    QVector<LibraryNoteListItem> items;
    items.reserve(m_allNotes.size());
    for (const LibraryNoteRecord& note : std::as_const(m_allNotes))
    {
        const bool matches = std::any_of(note.tags.cbegin(), note.tags.cend(), [&](const QString& tag) {
            const auto label = tag.trimmed();
            return availableTagKeys.contains(label.toCaseFolded())
                && (m_selectedIndex < 0 || m_selectedIndex >= m_items.size()
                    || tagValueMatchesHierarchyItem(label, m_items, m_selectedIndex));
        });
        if (!matches) continue;

        items.push_back(buildNoteListItem(note));
    }

    m_noteListModel.setItems(std::move(items));
}

bool TagsHierarchyController::refreshIndexedNotesFromWshub(const QString& wshubPath, QString* errorMessage)
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

bool TagsHierarchyController::refreshIndexedNotesFromTagsFilePath(QString* errorMessage)
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
