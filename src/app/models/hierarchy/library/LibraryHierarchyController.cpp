#include "app/models/hierarchy/library/LibraryHierarchyController.hpp"

#include "app/models/calendar/ISystemCalendarStore.hpp"
#include "app/policy/ArchitecturePolicyLock.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/models/hierarchy/WhatSonFolderIdentity.hpp"
#include "app/models/hierarchy/WhatSonHierarchyNoteRecordSupport.hpp"
#include "app/models/hierarchy/folders/WhatSonFoldersHierarchyParser.hpp"
#include "app/models/hierarchy/folders/WhatSonFoldersHierarchyStore.hpp"
#include "app/models/hierarchy/library/WhatSonLibraryFolderHierarchyMutationService.hpp"
#include "app/models/hierarchy/library/WhatSonLibraryHierarchyCreator.hpp"
#include "app/models/hierarchy/library/WhatSonLibraryHierarchyStore.hpp"
#include "app/models/file/note/folder/WhatSonNoteFolderSemantics.hpp"
#include "app/models/hierarchy/WhatSonHierarchyTreeItemSupport.hpp"
#include "app/models/hierarchy/library/LibraryHierarchyControllerSupport.hpp"
#include "app/models/sidebar/SidebarHierarchyLvrsSupport.hpp"

#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <QDateTime>
#include <QSet>
#include <QVariantMap>

#include <algorithm>
#include <limits>
#include <utility>

namespace
{
    constexpr auto kLibraryDraftLabel = "Drafts";
    constexpr auto kLibraryAllLabel = "All Library";
    constexpr auto kLibraryTodayLabel = "Today";

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

    QString buildFolderPath(const QString& parentPath, const QString& label)
    {
        return WhatSon::NoteFolders::appendFolderPathSegment(parentPath, label);
    }

    bool usesReservedTodayFolderToken(const QString& value)
    {
        return WhatSon::NoteFolders::usesReservedTodayFolderSegment(value);
    }

    bool isProtectedRootItem(const LibraryHierarchyItem& item);

    QStringList canonicalLeafFolderPaths(const QStringList& folderPaths);

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

    QVector<LibraryHierarchyItem> prependInAppLibraryScaffold(QVector<LibraryHierarchyItem> items)
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

    bool hasCompleteInAppLibraryScaffold(const QVector<LibraryHierarchyItem>& items)
    {
        if (items.size() < 3)
        {
            return false;
        }

        return items.at(0).systemBucket == LibraryHierarchyItem::SystemBucket::All
            && items.at(0).label == QLatin1String(kLibraryAllLabel)
            && items.at(1).systemBucket == LibraryHierarchyItem::SystemBucket::Draft
            && items.at(1).label == QLatin1String(kLibraryDraftLabel)
            && items.at(2).systemBucket == LibraryHierarchyItem::SystemBucket::Today
            && items.at(2).label == QLatin1String(kLibraryTodayLabel);
    }

    QString hierarchyItemKey(const LibraryHierarchyItem& item, int fallbackIndex)
    {
        if (item.systemBucket == LibraryHierarchyItem::SystemBucket::All)
        {
            return QStringLiteral("bucket:all");
        }
        if (item.systemBucket == LibraryHierarchyItem::SystemBucket::Draft)
        {
            return QStringLiteral("bucket:draft");
        }
        if (item.systemBucket == LibraryHierarchyItem::SystemBucket::Today)
        {
            return QStringLiteral("bucket:today");
        }

        const QString itemUuid = normalizeFolderUuid(item.folderUuid);
        if (!itemUuid.isEmpty())
        {
            return QStringLiteral("folder:%1").arg(itemUuid);
        }

        const QString itemPath = normalizeFolderPath(item.folderPath);
        if (!itemPath.isEmpty())
        {
            return itemPath;
        }

        return QStringLiteral("item:%1").arg(fallbackIndex);
    }

    bool flatNodeTargetsSystemBucket(
        const WhatSon::Sidebar::Lvrs::FlatNode& node,
        const QVector<LibraryHierarchyItem>& currentItems)
    {
        if (node.sourceIndex >= 0 && node.sourceIndex < currentItems.size())
        {
            return currentItems.at(node.sourceIndex).systemBucket != LibraryHierarchyItem::SystemBucket::None;
        }

        return node.key == QStringLiteral("bucket:all")
            || node.key == QStringLiteral("bucket:draft")
            || node.key == QStringLiteral("bucket:today");
    }

    int selectedHierarchyIndexForKey(
        const QVector<LibraryHierarchyItem>& items,
        const QString& activeItemKey)
    {
        const QString normalizedActiveKey = activeItemKey.trimmed();
        if (normalizedActiveKey.isEmpty())
        {
            return -1;
        }

        for (int index = 0; index < items.size(); ++index)
        {
            if (hierarchyItemKey(items.at(index), index) == normalizedActiveKey)
            {
                return index;
            }
        }

        return -1;
    }

    int selectedHierarchyIndexForFolderPath(
        const QVector<LibraryHierarchyItem>& items,
        const QString& folderPath)
    {
        const QString normalizedFolderPath = normalizeFolderPath(folderPath);
        if (normalizedFolderPath.isEmpty())
        {
            return -1;
        }

        for (int index = 0; index < items.size(); ++index)
        {
            if (normalizeFolderPath(items.at(index).folderPath) == normalizedFolderPath)
            {
                return index;
            }
        }

        return -1;
    }

    QSet<QString> expandedHierarchyItemKeys(const QVector<LibraryHierarchyItem>& items)
    {
        QSet<QString> expandedKeys;
        for (int index = 0; index < items.size(); ++index)
        {
            const LibraryHierarchyItem& item = items.at(index);
            if (!item.expanded)
            {
                continue;
            }
            expandedKeys.insert(hierarchyItemKey(item, index));
        }
        return expandedKeys;
    }

    void restoreExpandedHierarchyItemKeys(QVector<LibraryHierarchyItem>* items, const QSet<QString>& expandedKeys)
    {
        if (items == nullptr)
        {
            return;
        }

        for (int index = 0; index < items->size(); ++index)
        {
            LibraryHierarchyItem& item = (*items)[index];
            item.expanded = expandedKeys.contains(hierarchyItemKey(item, index));
        }
    }

    void finalizeFolderItems(QVector<LibraryHierarchyItem>* items, bool preserveExistingPaths);

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
            item.folderUuid = normalizeFolderUuid(entry.uuid);
            item.showChevron = true;

            if (item.label.isEmpty())
            {
                WhatSon::Debug::trace(
                    QStringLiteral("library.controller"),
                    QStringLiteral("buildFolderItems.emptyLabelKept"));
            }

            items.push_back(std::move(item));
        }

        applyChevronByDepth(&items);
        return items;
    }

    void dropReservedTodayFolderItems(QVector<LibraryHierarchyItem>* items)
    {
        if (items == nullptr || items->isEmpty())
        {
            return;
        }

        bool removedAny = false;
        for (int index = items->size() - 1; index >= 0; --index)
        {
            const LibraryHierarchyItem& item = items->at(index);
            if (item.systemBucket != LibraryHierarchyItem::SystemBucket::None)
            {
                continue;
            }

            const QString reservedCheckValue = item.folderPath.isEmpty() ? item.label : item.folderPath;
            if (!usesReservedTodayFolderToken(reservedCheckValue))
            {
                continue;
            }

            WhatSon::Debug::trace(
                QStringLiteral("library.controller"),
                QStringLiteral("dropReservedTodayFolderItems.remove"),
                QStringLiteral("label=%1 path=%2 depth=%3")
                .arg(item.label)
                .arg(item.folderPath)
                .arg(item.depth));
            items->removeAt(index);
            removedAny = true;
        }

        if (removedAny)
        {
            finalizeFolderItems(items, false);
        }
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
                item.folderUuid.clear();
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
            if (item.folderUuid.isEmpty())
            {
                item.folderUuid = WhatSon::FolderIdentity::createFolderUuid();
            }

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

            const QString reservedCheckValue = item.folderPath.isEmpty() ? label : item.folderPath;
            if (usesReservedTodayFolderToken(reservedCheckValue))
            {
                continue;
            }

            WhatSonFolderDepthEntry entry;
            entry.id = normalizeFolderPath(item.folderPath);
            entry.label = label;
            entry.depth = std::max(0, item.depth);
            entry.uuid = normalizeFolderUuid(item.folderUuid);
            if (entry.id.isEmpty())
            {
                entry.id = label;
            }
            if (entry.uuid.isEmpty())
            {
                entry.uuid = WhatSon::FolderIdentity::createFolderUuid();
            }
            entries.push_back(std::move(entry));
        }

        return entries;
    }

    bool folderDepthEntriesEqual(
        const QVector<WhatSonFolderDepthEntry>& lhs,
        const QVector<WhatSonFolderDepthEntry>& rhs)
    {
        if (lhs.size() != rhs.size())
        {
            return false;
        }

        for (int index = 0; index < lhs.size(); ++index)
        {
            const WhatSonFolderDepthEntry& left = lhs.at(index);
            const WhatSonFolderDepthEntry& right = rhs.at(index);
            if (normalizeFolderPath(left.id) != normalizeFolderPath(right.id)
                || left.label.trimmed() != right.label.trimmed()
                || std::max(0, left.depth) != std::max(0, right.depth)
                || normalizeFolderUuid(left.uuid) != normalizeFolderUuid(right.uuid))
            {
                return false;
            }
        }

        return true;
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

    bool resolveFolderMoveOperationFromLvrsMoveEvent(
        const QVector<LibraryHierarchyItem>& items,
        int firstEditableInsertIndex,
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

        QVector<LibraryHierarchyItem> remainingItems = items;
        remainingItems.remove(sourceIndex, sourceCount);

        const int maxInsertIndex = std::max(0, static_cast<int>(remainingItems.size()));
        int normalizedInsertIndex = std::clamp(targetIndex, 0, maxInsertIndex);
        normalizedInsertIndex = std::max(normalizedInsertIndex, firstEditableInsertIndex);

        int minDepth = normalizedInsertIndex < remainingItems.size()
            ? std::max(0, remainingItems.at(normalizedInsertIndex).depth)
            : 0;
        int maxDepth = normalizedInsertIndex > 0
            ? std::max(0, remainingItems.at(normalizedInsertIndex - 1).depth + 1)
            : 0;
        if (normalizedInsertIndex <= firstEditableInsertIndex
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

    QHash<QString, QString> movedFolderPathMapForUpdatedSubtree(
        const QVector<LibraryHierarchyItem>& originalItems,
        int subtreeStartIndex,
        const QVector<LibraryHierarchyItem>& stagedItems)
    {
        QHash<QString, QString> movedPathMap;
        if (subtreeStartIndex < 0
            || subtreeStartIndex >= originalItems.size()
            || originalItems.size() != stagedItems.size())
        {
            return movedPathMap;
        }

        const int subtreeEndIndex = subtreeEndIndexExclusive(originalItems, subtreeStartIndex);
        for (int index = subtreeStartIndex; index < subtreeEndIndex && index < stagedItems.size(); ++index)
        {
            const QString sourcePath = normalizeFolderPath(originalItems.at(index).folderPath);
            const QString targetPath = normalizeFolderPath(stagedItems.at(index).folderPath);
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

LibraryHierarchyController::LibraryHierarchyController(QObject* parent)
    : IHierarchyController(parent)
      , m_itemModel(this)
      , m_noteListModel(this)
{
    WhatSon::Debug::traceSelf(this, QStringLiteral("library.controller"), QStringLiteral("ctor"));
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
    QObject::connect(
        &m_noteListModel,
        &LibraryNoteListModel::itemCountChanged,
        this,
        [this](int)
        {
            updateNoteItemCount();
        });
    applyInAppLibraryScaffold();
    setSelectedIndex(-1);
}

LibraryHierarchyController::~LibraryHierarchyController() = default;

WhatSonHierarchyModel* LibraryHierarchyController::itemModel() noexcept
{
    return &m_itemModel;
}

LibraryNoteListModel* LibraryHierarchyController::noteListModel() noexcept
{
    return &m_noteListModel;
}

bool LibraryHierarchyController::supportsHierarchyNodeReorder() const noexcept
{
    return true;
}

bool LibraryHierarchyController::supportsHierarchyNoteDrop() const noexcept
{
    return true;
}

void LibraryHierarchyController::setSystemCalendarStore(ISystemCalendarStore* store)
{
    if (store != nullptr
        && !WhatSon::Policy::verifyMutableDependencyAllowed(
            WhatSon::Policy::Layer::Controller,
            WhatSon::Policy::Layer::Store,
            QStringLiteral("LibraryHierarchyController::setSystemCalendarStore")))
    {
        return;
    }

    if (store == nullptr
        && !WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("LibraryHierarchyController::setSystemCalendarStore")))
    {
        return;
    }

    if (m_noteListProjection.systemCalendarStore() == store)
    {
        return;
    }

    if (m_systemCalendarStoreChangedConnection)
    {
        QObject::disconnect(m_systemCalendarStoreChangedConnection);
    }

    m_noteListProjection.setSystemCalendarStore(store);
    if (m_noteListProjection.systemCalendarStore())
    {
        m_systemCalendarStoreChangedConnection = QObject::connect(
            m_noteListProjection.systemCalendarStore(),
            &ISystemCalendarStore::systemInfoChanged,
            this,
            [this]()
            {
                invalidateNoteListItemCache();
                refreshNoteListForSelection();
            });
    }
    else
    {
        m_systemCalendarStoreChangedConnection = {};
    }

    refreshNoteListForSelection();
}

ISystemCalendarStore* LibraryHierarchyController::systemCalendarStore() const noexcept
{
    return m_noteListProjection.systemCalendarStore();
}

int LibraryHierarchyController::selectedIndex() const noexcept
{
    return m_selectedIndex;
}

int LibraryHierarchyController::itemCount() const noexcept
{
    return m_itemCount;
}

int LibraryHierarchyController::noteItemCount() const noexcept
{
    return m_noteItemCount;
}

bool LibraryHierarchyController::loadSucceeded() const noexcept
{
    return m_loadSucceeded;
}

QString LibraryHierarchyController::lastLoadError() const
{
    return m_lastLoadError;
}

void LibraryHierarchyController::setSelectedIndex(int index)
{
    applySelectedIndex(index, false);
}

void LibraryHierarchyController::applySelectedIndex(int index, bool forceReapply)
{
    const int clamped = WhatSon::Hierarchy::TreeItemSupport::clampSelectionIndexToVisibleDefault(
        index,
        m_itemModel.rowCount());

    if (m_selectedIndex == clamped)
    {
        if (!forceReapply)
        {
            return;
        }

        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.controller"),
                                  QStringLiteral("reapplySelectedIndex"),
                                  QStringLiteral("value=%1").arg(m_selectedIndex));
        refreshNoteListForSelection();
        emit selectedIndexChanged();
        return;
    }

    m_selectedIndex = clamped;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
                              QStringLiteral("setSelectedIndex"),
                              QStringLiteral("value=%1").arg(m_selectedIndex));
    refreshNoteListForSelection();
    emit selectedIndexChanged();
}

void LibraryHierarchyController::clearSelectedIndex(bool forceReapply)
{
    if (m_selectedIndex == -1)
    {
        if (!forceReapply)
        {
            return;
        }

        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.controller"),
                                  QStringLiteral("reapplyClearedSelectedIndex"),
                                  QStringLiteral("value=-1"));
        refreshNoteListForSelection();
        emit selectedIndexChanged();
        return;
    }

    m_selectedIndex = -1;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
                              QStringLiteral("clearSelectedIndex"),
                              QStringLiteral("value=-1"));
    refreshNoteListForSelection();
    emit selectedIndexChanged();
}

void LibraryHierarchyController::setDepthItems(const QVariantList& depthItems)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
                              QStringLiteral("setDepthItems.begin"),
                              QStringLiteral("count=%1").arg(depthItems.size()));
    const QSet<QString> preservedExpandedKeys = expandedHierarchyItemKeys(m_items);

    if (depthItems.isEmpty())
    {
        if (m_runtimeIndexLoaded && m_foldersHierarchyLoaded)
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("library.controller"),
                                      QStringLiteral("setDepthItems.keepFoldersHierarchy"),
                                      QStringLiteral("folderCount=%1").arg(m_items.size()));
            refreshNoteListForSelection();
            return;
        }

        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.controller"),
                                  QStringLiteral("setDepthItems.useInAppLibraryScaffold"),
                                  QStringLiteral("all=%1 draft=%2 today=%3")
                                  .arg(m_indexedState.allNotes().size())
                                  .arg(m_indexedState.draftNotes().size())
                                  .arg(m_indexedState.todayNotes().size()));
        applyInAppLibraryScaffold();
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

    m_items = prependInAppLibraryScaffold(std::move(parsedItems));
    finalizeFolderItems(&m_items, true);
    dropReservedTodayFolderItems(&m_items);
    restoreExpandedHierarchyItemKeys(&m_items, preservedExpandedKeys);
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
                              QStringLiteral("library.controller"),
                              QStringLiteral("setDepthItems.success"),
                              QStringLiteral("itemCount=%1 nextFolderSeq=%2 foldersLoaded=%3")
                              .arg(m_items.size())
                              .arg(m_createdFolderSequence)
                              .arg(m_foldersHierarchyLoaded ? QStringLiteral("1") : QStringLiteral("0")));
}

bool LibraryHierarchyController::loadFromWshub(const QString& wshubPath, QString* errorMessage)
{
    m_foldersFilePath.clear();

    QString indexError;
    if (!loadIndexedStateFromWshub(wshubPath, &indexError))
    {
        m_runtimeIndexLoaded = false;
        m_foldersHierarchyLoaded = false;
        m_indexedState.clear();
        emit indexedNotesSnapshotChanged();
        if (errorMessage != nullptr)
        {
            *errorMessage = indexError;
        }
        applyInAppLibraryScaffold();
        setSelectedIndex(-1);
        updateLoadState(false, indexError);
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.controller"),
                                  QStringLiteral("loadFromWshub.failed"),
                                  QStringLiteral("path=%1 reason=%2").arg(wshubPath, indexError));
        return false;
    }

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

    WhatSonFoldersHierarchyParser foldersParser;
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

        WhatSonFoldersHierarchyStore foldersStore;
        QString parseError;
        bool folderUuidMigrationRequired = false;
        if (!foldersParser.parse(rawText, &foldersStore, &parseError, &folderUuidMigrationRequired))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = parseError;
            }
            updateLoadState(false, parseError);
            return false;
        }

        if (folderUuidMigrationRequired)
        {
            QString writeError;
            if (!foldersStore.writeToFile(filePath, &writeError))
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = writeError;
                }
                updateLoadState(false, writeError);
                return false;
            }
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
        m_items = prependInAppLibraryScaffold(buildFolderItems(folderEntries));
        finalizeFolderItems(&m_items, true);
        dropReservedTodayFolderItems(&m_items);
        m_foldersHierarchyLoaded = true;
        rebuildBucketRanges();
        m_createdFolderSequence = nextFolderSequence(m_items);
        syncModel();
        setSelectedIndex(-1);
        refreshNoteListForSelection();

        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.controller"),
                                  QStringLiteral("loadFromWshub.folderHierarchy"),
                                  QStringLiteral("path=%1 folderCount=%2 all=%3 draft=%4 today=%5")
                                  .arg(wshubPath)
                                  .arg(m_items.size())
                                  .arg(m_indexedState.allNotes().size())
                                  .arg(m_indexedState.draftNotes().size())
                                  .arg(m_indexedState.todayNotes().size()));
        updateLoadState(true);
        return true;
    }

    applyInAppLibraryScaffold();
    setSelectedIndex(-1);

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
                              QStringLiteral("loadFromWshub.success"),
                              QStringLiteral("foldersFileFound=%1 all=%2 draft=%3 today=%4")
                              .arg(foldersFileFound ? QStringLiteral("1") : QStringLiteral("0"))
                              .arg(m_indexedState.allNotes().size())
                              .arg(m_indexedState.draftNotes().size())
                              .arg(m_indexedState.todayNotes().size()));
    updateLoadState(true);
    return true;
}

void LibraryHierarchyController::applyRuntimeSnapshot(
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
        m_indexedState.clear();
        emit indexedNotesSnapshotChanged();
        m_runtimeIndexLoaded = false;
        m_foldersHierarchyLoaded = false;
        applyInAppLibraryScaffold();
        setSelectedIndex(-1);
        updateLoadState(false, errorMessage);
        return;
    }

    const QString preservedSelectionKey =
        (m_selectedIndex >= 0 && m_selectedIndex < m_items.size())
            ? hierarchyItemKey(m_items.at(m_selectedIndex), m_selectedIndex)
            : QString();
    const QString preservedSelectionFolderPath = folderPathForIndex(m_selectedIndex);
    const QSet<QString> preservedExpandedKeys = expandedHierarchyItemKeys(m_items);

    applyIndexedStateSnapshot(wshubPath, std::move(allNotes), std::move(draftNotes), std::move(todayNotes));
    m_runtimeIndexLoaded = true;

    const QVector<WhatSonFolderDepthEntry> currentFolderEntries = folderEntriesFromItems(m_items);
    const bool hierarchySourceChanged =
        !hasCompleteInAppLibraryScaffold(m_items) || !folderDepthEntriesEqual(currentFolderEntries, folderEntries);

    if (!hierarchySourceChanged)
    {
        refreshNoteListForSelectionAndNotifyHierarchyModel();
        updateLoadState(true);
        return;
    }

    if (!folderEntries.isEmpty())
    {
        m_items = prependInAppLibraryScaffold(buildFolderItems(folderEntries));
        finalizeFolderItems(&m_items, true);
        dropReservedTodayFolderItems(&m_items);
        restoreExpandedHierarchyItemKeys(&m_items, preservedExpandedKeys);
        m_foldersHierarchyLoaded = true;
        rebuildBucketRanges();
        m_createdFolderSequence = nextFolderSequence(m_items);
        syncModel();
    }
    else
    {
        applyInAppLibraryScaffold();
    }

    int restoredSelectionIndex = selectedHierarchyIndexForKey(m_items, preservedSelectionKey);
    if (restoredSelectionIndex < 0 && !preservedSelectionFolderPath.isEmpty())
    {
        restoredSelectionIndex = selectedHierarchyIndexForFolderPath(m_items, preservedSelectionFolderPath);
    }
    applySelectedIndex(restoredSelectionIndex, true);

    updateLoadState(true);
}

QVariantList LibraryHierarchyController::depthItems() const
{
    QVariantList serializedItems;
    serializedItems.reserve(m_items.size());

    QHash<QString, int> folderNoteCountByFolderUuid;
    if (m_runtimeIndexLoaded && m_foldersHierarchyLoaded)
    {
        folderNoteCountByFolderUuid = m_noteListProjection.folderNoteCountByFolderUuid(
            m_items,
            m_indexedState.allNotes(),
            m_foldersHierarchyLoaded);
    }

    const auto noteCountForItem = [this, &folderNoteCountByFolderUuid](const LibraryHierarchyItem& item) -> int
    {
        if (!m_runtimeIndexLoaded)
        {
            return 0;
        }

        switch (item.systemBucket)
        {
        case LibraryHierarchyItem::SystemBucket::All:
            return m_indexedState.allNotes().size();
        case LibraryHierarchyItem::SystemBucket::Draft:
            return m_indexedState.draftNotes().size();
        case LibraryHierarchyItem::SystemBucket::Today:
            return m_indexedState.todayNotes().size();
        case LibraryHierarchyItem::SystemBucket::None:
            break;
        }

        const QString folderUuid = normalizeFolderUuid(item.folderUuid);
        return folderUuid.isEmpty() ? 0 : folderNoteCountByFolderUuid.value(folderUuid, 0);
    };

    for (int index = 0; index < m_items.size(); ++index)
    {
        const LibraryHierarchyItem& item = m_items.at(index);
        const bool movable = canMoveFolder(index);
        const int noteCount = std::max(0, noteCountForItem(item));

        serializedItems.push_back(QVariantMap{
            {"label", item.label},
            {"id", item.folderPath},
            {"uuid", item.folderUuid},
            {"itemId", index},
            {"key", hierarchyItemKey(item, index)},
            {"iconName", libraryHierarchyIconName(item)},
            {"depth", item.depth},
            {"accent", item.accent},
            {"expanded", item.expanded},
            {"draggable", movable},
            {"dragAllowed", movable},
            {"movable", movable},
            {"dragLocked", !movable},
            {"showChevron", item.showChevron},
            {"count", noteCount}
        });
    }
    return serializedItems;
}

QVariantList LibraryHierarchyController::hierarchyModel() const
{
    return depthItems();
}

QString LibraryHierarchyController::itemLabel(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }

    return m_items.at(index).label;
}

bool LibraryHierarchyController::canRenameItem(int index) const
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

bool LibraryHierarchyController::renameItem(int index, const QString& displayName)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
                              QStringLiteral("renameItem.begin"),
                              QStringLiteral("index=%1 label=%2").arg(index).arg(displayName));
    if (!canRenameItem(index))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.controller"),
                                  QStringLiteral("renameItem.rejected"),
                                  QStringLiteral("reason=canRenameItem false index=%1").arg(index));
        return false;
    }

    const QString trimmedName = displayName.trimmed();
    if (trimmedName.isEmpty())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.controller"),
                                  QStringLiteral("renameItem.rejected"),
                                  QStringLiteral("reason=empty label index=%1").arg(index));
        return false;
    }

    if (usesReservedTodayFolderToken(trimmedName))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.controller"),
                                  QStringLiteral("renameItem.rejected"),
                                  QStringLiteral("reason=reserved today token index=%1 label=%2")
                                  .arg(index)
                                  .arg(trimmedName));
        return false;
    }

    if (m_items.at(index).label == trimmedName)
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.controller"),
                                  QStringLiteral("renameItem.skipped"),
                                  QStringLiteral("reason=same label index=%1").arg(index));
        return true;
    }

    QVector<LibraryHierarchyItem> stagedItems = m_items;
    stagedItems[index].label = trimmedName;
    finalizeFolderItems(&stagedItems, false);
    const QHash<QString, QString> movedPathMap = movedFolderPathMapForUpdatedSubtree(m_items, index, stagedItems);
    if (!commitFolderHierarchyUpdate(std::move(stagedItems), m_selectedIndex, movedPathMap))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.controller"),
                                  QStringLiteral("renameItem.writeFailed"),
                                  QStringLiteral("index=%1 path=%2").arg(index).arg(m_foldersFilePath));
        return false;
    }
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
                              QStringLiteral("renameItem"),
                              QStringLiteral("index=%1 label=%2").arg(index).arg(trimmedName));
    return true;
}

bool LibraryHierarchyController::setItemExpanded(int index, bool expanded)
{
    return setHierarchyItemExpanded(
        &m_items,
        index,
        expanded,
        [this](int changedIndex, bool changedExpanded)
        {
            m_itemModel.setItemExpanded(changedIndex, changedExpanded);
            emit hierarchyModelChanged();
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("library.controller"),
                                      QStringLiteral("setItemExpanded"),
                                      QStringLiteral("index=%1 expanded=%2")
                                      .arg(changedIndex)
                                      .arg(changedExpanded ? QStringLiteral("true") : QStringLiteral("false")));
        });
}

bool LibraryHierarchyController::renameEnabled() const noexcept
{
    return true;
}

bool LibraryHierarchyController::createFolderEnabled() const noexcept
{
    return true;
}

bool LibraryHierarchyController::deleteFolderEnabled() const noexcept
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        return false;
    }

    return !isProtectedRootItem(m_items.at(m_selectedIndex));
}

void LibraryHierarchyController::createFolder()
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
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
    const QString insertedSelectionKey =
        (insertIndex >= 0 && insertIndex < stagedItems.size())
            ? hierarchyItemKey(stagedItems.at(insertIndex), insertIndex)
            : QString();
    const QString insertedSelectionFolderPath =
        (insertIndex >= 0 && insertIndex < stagedItems.size())
            ? normalizeFolderPath(stagedItems.at(insertIndex).folderPath)
            : QString();
    const QSet<QString> stagedExpandedKeys = expandedHierarchyItemKeys(stagedItems);

    WhatSonFoldersHierarchyStore stagedStore;
    stagedStore.setFolderEntries(folderEntriesFromItems(stagedItems));
    if (!m_foldersFilePath.trimmed().isEmpty())
    {
        QString writeError;
        if (!stagedStore.writeToFile(m_foldersFilePath, &writeError))
        {
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("library.controller"),
                                      QStringLiteral("createFolder.writeFailed"),
                                      QStringLiteral("insertIndex=%1 path=%2 reason=%3").arg(insertIndex).arg(
                                          m_foldersFilePath, writeError));
            return;
        }
    }

    QString reloadError;
    if (!reloadFolderHierarchyFromFoldersFile(stagedExpandedKeys, &reloadError))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.controller"),
                                  QStringLiteral("createFolder.reloadFallback"),
                                  QStringLiteral("insertIndex=%1 error=%2").arg(insertIndex).arg(reloadError));
        m_items = std::move(stagedItems);
        m_foldersHierarchyLoaded = true;
        m_createdFolderSequence = nextFolderSequence(m_items);
    }

    int mirroredInsertIndex = selectedHierarchyIndexForKey(m_items, insertedSelectionKey);
    if (mirroredInsertIndex < 0 && !insertedSelectionFolderPath.isEmpty())
    {
        mirroredInsertIndex = selectedHierarchyIndexForFolderPath(m_items, insertedSelectionFolderPath);
    }
    if (mirroredInsertIndex < 0)
    {
        mirroredInsertIndex = insertIndex;
    }

    rebuildBucketRanges();
    syncModel();
    setSelectedIndex(mirroredInsertIndex);
    emit hubFilesystemMutated();
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
                              QStringLiteral("createFolder"),
                              QStringLiteral("insertIndex=%1 depth=%2 itemCount=%3")
                              .arg(mirroredInsertIndex)
                              .arg(folderDepth)
                              .arg(m_items.size()));
}

void LibraryHierarchyController::deleteSelectedFolder()
{
    const int startIndex = m_selectedIndex;
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
                              QStringLiteral("deleteSelectedFolder.begin"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2").arg(startIndex).arg(m_items.size()));
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.controller"),
                                  QStringLiteral("deleteSelectedFolder.rejected"),
                                  QStringLiteral("reason=selection out of range selectedIndex=%1").arg(startIndex));
        return;
    }

    if (isProtectedRootItem(m_items.at(m_selectedIndex)))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.controller"),
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
    if (!commitFolderHierarchyUpdate(std::move(stagedItems), -1, {}, true))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.controller"),
                                  QStringLiteral("deleteSelectedFolder.writeFailed"),
                                  QStringLiteral("startIndex=%1 path=%2").arg(startIndex).arg(m_foldersFilePath));
        return;
    }

    emit hubFilesystemMutated();
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
                              QStringLiteral("deleteSelectedFolder"),
                              QStringLiteral("startIndex=%1 removeCount=%2 remaining=%3")
                              .arg(startIndex)
                              .arg(removeCount)
                              .arg(m_items.size()));
}

bool LibraryHierarchyController::canMoveFolder(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return false;
    }

    return !isProtectedRootItem(m_items.at(index));
}

bool LibraryHierarchyController::canAcceptFolderDropBefore(int sourceIndex, int targetIndex) const
{
    return resolveFolderMoveOperation(
        m_items,
        firstEditableInsertIndex(),
        sourceIndex,
        targetIndex,
        FolderDropPlacement::Before,
        nullptr);
}

bool LibraryHierarchyController::moveFolderBefore(int sourceIndex, int targetIndex)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
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
                                  QStringLiteral("library.controller"),
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
                              QStringLiteral("library.controller"),
                              QStringLiteral("moveFolderBefore.success"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2")
                              .arg(operation.normalizedInsertIndex)
                              .arg(m_items.size()));
    emit hubFilesystemMutated();
    return true;
}

bool LibraryHierarchyController::canAcceptFolderDrop(int sourceIndex, int targetIndex, bool asChild) const
{
    return resolveFolderMoveOperation(
        m_items,
        firstEditableInsertIndex(),
        sourceIndex,
        targetIndex,
        asChild ? FolderDropPlacement::Child : FolderDropPlacement::After,
        nullptr);
}

bool LibraryHierarchyController::moveFolder(int sourceIndex, int targetIndex, bool asChild)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
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
                                  QStringLiteral("library.controller"),
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
                              QStringLiteral("library.controller"),
                              QStringLiteral("moveFolder.success"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2")
                              .arg(operation.normalizedInsertIndex)
                              .arg(m_items.size()));
    emit hubFilesystemMutated();
    return true;
}

bool LibraryHierarchyController::canMoveFolderToRoot(int sourceIndex) const
{
    return resolveFolderMoveOperation(
        m_items,
        firstEditableInsertIndex(),
        sourceIndex,
        -1,
        FolderDropPlacement::RootTop,
        nullptr);
}

bool LibraryHierarchyController::moveFolderToRoot(int sourceIndex)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
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
                              QStringLiteral("library.controller"),
                              QStringLiteral("moveFolderToRoot.success"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2")
                              .arg(operation.normalizedInsertIndex)
                              .arg(m_items.size()));
    emit hubFilesystemMutated();
    return true;
}

bool LibraryHierarchyController::canAcceptNoteDrop(int index, const QString& noteId) const
{
    Q_UNUSED(index)
    Q_UNUSED(noteId)
    WhatSon::Debug::traceSelf(const_cast<LibraryHierarchyController*>(this),
                              QStringLiteral("library.controller"),
                              QStringLiteral("canAcceptNoteDrop.notePackagesDisabled"));
    return false;
}

bool LibraryHierarchyController::assignNoteToFolder(int index, const QString& noteId)
{
    Q_UNUSED(index)
    Q_UNUSED(noteId)
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
                              QStringLiteral("assignNoteToFolder.notePackagesDisabled"));
    return false;
}

bool LibraryHierarchyController::applyHierarchyNodes(const QVariantList& hierarchyNodes, const QString& activeItemKey)
{
    const QVector<WhatSon::Sidebar::Lvrs::FlatNode> flattened =
        WhatSon::Sidebar::Lvrs::flattenHierarchyNodes(hierarchyNodes);
    if (flattened.isEmpty())
    {
        return false;
    }

    QVector<LibraryHierarchyItem> stagedItems;
    stagedItems.reserve(flattened.size());
    QVector<int> stagedIndexByFlatIndex(flattened.size(), -1);
    const bool preserveSystemBucketPrefix = std::any_of(
        m_items.cbegin(),
        m_items.cend(),
        [](const LibraryHierarchyItem& item)
        {
            return item.systemBucket != LibraryHierarchyItem::SystemBucket::None;
        });

    const QString normalizedActiveKey = activeItemKey.trimmed();
    int selectedIndex = -1;
    for (int flatIndex = 0; flatIndex < flattened.size(); ++flatIndex)
    {
        const WhatSon::Sidebar::Lvrs::FlatNode& node = flattened.at(flatIndex);
        if (preserveSystemBucketPrefix && flatNodeTargetsSystemBucket(node, m_items))
        {
            continue;
        }

        LibraryHierarchyItem item;
        const bool validSourceIndex = node.sourceIndex >= 0 && node.sourceIndex < m_items.size();
        item.depth = std::max(0, node.depth);
        item.label = node.label.trimmed();
        item.accent = false;
        item.expanded = node.expanded;
        item.showChevron = node.showChevron;
        if (validSourceIndex)
        {
            item.folderPath = normalizeFolderPath(m_items.at(node.sourceIndex).folderPath);
            item.folderUuid = folderUuidForIndex(node.sourceIndex);
        }
        else if (!node.id.isEmpty())
        {
            item.folderPath = normalizeFolderPath(node.id);
            item.folderUuid = node.key.startsWith(QStringLiteral("folder:"))
                                  ? normalizeFolderUuid(node.key.mid(QStringLiteral("folder:").size()))
                                  : QString();
        }
        else
        {
            item.folderPath = normalizeFolderPath(node.key);
            item.folderUuid = node.key.startsWith(QStringLiteral("folder:"))
                                  ? normalizeFolderUuid(node.key.mid(QStringLiteral("folder:").size()))
                                  : QString();
        }

        stagedIndexByFlatIndex[flatIndex] = stagedItems.size();
        stagedItems.push_back(std::move(item));
        if (node.key == normalizedActiveKey)
        {
            selectedIndex = stagedIndexByFlatIndex.at(flatIndex);
        }
    }

    finalizeFolderItems(&stagedItems, false);
    dropReservedTodayFolderItems(&stagedItems);
    if (preserveSystemBucketPrefix)
    {
        stagedItems = prependInAppLibraryScaffold(std::move(stagedItems));
        if (selectedIndex >= 0)
        {
            selectedIndex += 3;
        }
    }

    QHash<QString, QString> movedFolderPathMap;
    for (int flatIndex = 0; flatIndex < flattened.size() && flatIndex < stagedItems.size(); ++flatIndex)
    {
        const WhatSon::Sidebar::Lvrs::FlatNode& node = flattened.at(flatIndex);
        if (preserveSystemBucketPrefix && flatNodeTargetsSystemBucket(node, m_items))
        {
            continue;
        }

        int stagedIndex = stagedIndexByFlatIndex.at(flatIndex);
        if (stagedIndex >= 0 && preserveSystemBucketPrefix)
        {
            stagedIndex += 3;
        }
        if (stagedIndex < 0 || stagedIndex >= stagedItems.size())
        {
            continue;
        }

        const LibraryHierarchyItem& stagedItem = stagedItems.at(stagedIndex);

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

    if (selectedIndex < 0)
    {
        selectedIndex = selectedHierarchyIndexForKey(stagedItems, normalizedActiveKey);
    }
    return commitFolderHierarchyUpdate(std::move(stagedItems), selectedIndex, movedFolderPathMap);
}

bool LibraryHierarchyController::applyHierarchyMove(
    const int sourceIndex,
    const int targetIndex,
    const int targetDepth,
    const QString& activeItemKey)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
                              QStringLiteral("applyHierarchyMove.begin"),
                              QStringLiteral("sourceIndex=%1 targetIndex=%2 targetDepth=%3")
                              .arg(sourceIndex)
                              .arg(targetIndex)
                              .arg(targetDepth));

    FolderMoveOperation operation;
    if (!resolveFolderMoveOperationFromLvrsMoveEvent(
        m_items,
        firstEditableInsertIndex(),
        sourceIndex,
        targetIndex,
        targetDepth,
        &operation))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.controller"),
                                  QStringLiteral("applyHierarchyMove.rejected"),
                                  QStringLiteral("sourceIndex=%1 targetIndex=%2 targetDepth=%3")
                                  .arg(sourceIndex)
                                  .arg(targetIndex)
                                  .arg(targetDepth));
        return false;
    }

    QVector<LibraryHierarchyItem> stagedItems = stageFolderMoveItems(m_items, sourceIndex, operation);
    const QHash<QString, QString> movedPathMap = movedFolderPathMapForOperation(
        m_items,
        sourceIndex,
        operation,
        stagedItems);
    int selectedIndex = selectedHierarchyIndexForKey(stagedItems, activeItemKey);
    if (selectedIndex < 0)
    {
        selectedIndex = operation.normalizedInsertIndex;
    }
    if (!commitFolderHierarchyUpdate(std::move(stagedItems), selectedIndex, movedPathMap))
    {
        return false;
    }

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
                              QStringLiteral("applyHierarchyMove.success"),
                              QStringLiteral("selectedIndex=%1 itemCount=%2")
                              .arg(selectedIndex)
                              .arg(m_items.size()));
    emit hubFilesystemMutated();
    return true;
}

bool LibraryHierarchyController::createEmptyNote()
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
                              QStringLiteral("createEmptyNote.notePackagesDisabled"));
    return false;
}

bool LibraryHierarchyController::deleteNoteById(const QString& noteId)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
                              QStringLiteral("deleteNoteById.notePackagesDisabled"),
                              QStringLiteral("noteId=%1").arg(normalizedNoteId));
    return false;
}

bool LibraryHierarchyController::clearNoteFoldersById(const QString& noteId)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
                              QStringLiteral("clearNoteFoldersById.notePackagesDisabled"),
                              QStringLiteral("noteId=%1").arg(normalizedNoteId));
    return false;
}

QString LibraryHierarchyController::noteDirectoryPathForNoteId(const QString& noteId) const
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return {};
    }

    return WhatSon::Hierarchy::NoteRecordSupport::directoryPathForNoteId(
        m_indexedState.allNotes(),
        normalizedNoteId);
}

bool LibraryHierarchyController::shouldAutoActivateMostRecentNote() const noexcept
{
    const bool shouldActivate = m_startupAutoActivationPending && m_noteListModel.currentNoteId().trimmed().isEmpty();
    WhatSon::Debug::traceSelf(const_cast<LibraryHierarchyController*>(this),
                              QStringLiteral("library.controller"),
                              QStringLiteral("shouldAutoActivateMostRecentNote"),
                              QStringLiteral("pending=%1 currentNoteId=%2 shouldActivate=%3 noteItemCount=%4")
                                  .arg(m_startupAutoActivationPending)
                                  .arg(m_noteListModel.currentNoteId())
                                  .arg(shouldActivate)
                                  .arg(m_noteListModel.itemCount()));
    return shouldActivate;
}

QString LibraryHierarchyController::mostRecentIndexedNoteIdByHeader() const
{
    const QVector<LibraryNoteRecord> allNotes = m_indexedState.allNotes();
    WhatSon::Debug::traceSelf(const_cast<LibraryHierarchyController*>(this),
                              QStringLiteral("library.controller"),
                              QStringLiteral("mostRecentIndexedNoteIdByHeader.begin"),
                              QStringLiteral("allNotes=%1")
                                  .arg(allNotes.size()));
    if (allNotes.isEmpty())
    {
        WhatSon::Debug::traceSelf(const_cast<LibraryHierarchyController*>(this),
                                  QStringLiteral("library.controller"),
                                  QStringLiteral("mostRecentIndexedNoteIdByHeader.empty"),
                                  QStringLiteral("reason=no-indexed-notes"));
        return {};
    }

    auto parseTimestamp = [](const QString& value) -> QDateTime
    {
        const QString trimmed = value.trimmed();
        if (trimmed.isEmpty())
        {
            return {};
        }
        QDateTime dt = QDateTime::fromString(trimmed, Qt::ISODateWithMs);
        if (!dt.isValid())
            dt = QDateTime::fromString(trimmed, Qt::ISODate);
        if (!dt.isValid())
            dt = QDateTime::fromString(trimmed, QStringLiteral("yyyy-MM-dd-HH-mm-ss"));
        if (!dt.isValid())
            dt = QDateTime::fromString(trimmed, QStringLiteral("yyyy-MM-dd-hh-mm-ss"));
        return dt;
    };

    QString bestNoteId;
    qint64 bestTimestamp = std::numeric_limits<qint64>::min();

    for (const LibraryNoteRecord& note : allNotes)
    {
        const QString noteId = note.noteId.trimmed();
        if (noteId.isEmpty())
        {
            continue;
        }

        const QDateTime createdAtDateTime = parseTimestamp(note.createdAt);
        const QDateTime lastModifiedAtDateTime = parseTimestamp(note.lastModifiedAt);
        const qint64 createdAt = createdAtDateTime.isValid()
            ? createdAtDateTime.toMSecsSinceEpoch()
            : std::numeric_limits<qint64>::min();
        const qint64 lastModifiedAt = lastModifiedAtDateTime.isValid()
            ? lastModifiedAtDateTime.toMSecsSinceEpoch()
            : std::numeric_limits<qint64>::min();
        const qint64 effectiveTimestamp = std::max(createdAt, lastModifiedAt);
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.controller"),
                              QStringLiteral("autoActivateMostRecentNote.candidate"),
                              QStringLiteral("noteId=%1 createdAt=%2 lastModifiedAt=%3 effectiveTimestamp=%4")
                                  .arg(noteId)
                                      .arg(note.createdAt)
                                      .arg(note.lastModifiedAt)
                                      .arg(effectiveTimestamp));
        if (effectiveTimestamp > bestTimestamp)
        {
            bestTimestamp = effectiveTimestamp;
            bestNoteId = noteId;
        }
    }

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
                              QStringLiteral("autoActivateMostRecentNote.selectedCandidate"),
                              QStringLiteral("noteId=%1 timestamp=%2")
                                  .arg(bestNoteId)
                                  .arg(bestTimestamp));
    return bestNoteId;
}

bool LibraryHierarchyController::autoActivateMostRecentNote()
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
                              QStringLiteral("autoActivateMostRecentNote.begin"),
                              QStringLiteral("pending=%1 currentNoteId=%2 noteItemCount=%3")
                                  .arg(m_startupAutoActivationPending)
                                  .arg(m_noteListModel.currentNoteId())
                                  .arg(m_noteListModel.itemCount()));
    const QString noteId = mostRecentIndexedNoteIdByHeader();
    if (noteId.trimmed().isEmpty())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.controller"),
                                  QStringLiteral("autoActivateMostRecentNote.noCandidate"),
                                  QStringLiteral("reason=mostRecentIndexedNoteIdByHeader returned empty"));
        return false;
    }
    const bool activated = activateNoteById(noteId);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
                              QStringLiteral("autoActivateMostRecentNote"),
                              QStringLiteral("noteId=%1 activated=%2")
                                  .arg(noteId)
                                  .arg(activated));
    if (activated)
    {
        m_startupAutoActivationPending = false;
    }
    return activated;
}

bool LibraryHierarchyController::activateNoteById(const QString& noteId)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    auto visibleNoteIndexForId = [this, &normalizedNoteId]() -> int
    {
        const QVector<LibraryNoteListItem>& noteItems = m_noteListModel.items();
        for (int index = 0; index < noteItems.size(); ++index)
        {
            if (noteItems.at(index).id.trimmed() == normalizedNoteId)
            {
                return index;
            }
        }
        return -1;
    };

    int noteIndex = visibleNoteIndexForId();
    if (noteIndex < 0 && !m_noteListModel.searchText().trimmed().isEmpty())
    {
        m_noteListModel.setSearchText(QString());
        noteIndex = visibleNoteIndexForId();
    }
    if (noteIndex < 0 && m_selectedIndex != -1)
    {
        setSelectedIndex(-1);
        noteIndex = visibleNoteIndexForId();
    }
    if (noteIndex < 0)
    {
        WhatSon::Debug::traceSelf(
            this,
            QStringLiteral("library.controller"),
            QStringLiteral("activateNoteById.missing"),
            QStringLiteral("noteId=%1 selectedIndex=%2 noteCount=%3")
                .arg(normalizedNoteId)
                .arg(m_selectedIndex)
                .arg(m_noteListModel.items().size()));
        return false;
    }

    m_noteListModel.setCurrentIndex(noteIndex);
    return m_noteListModel.currentNoteId().trimmed() == normalizedNoteId;
}

QVector<LibraryNoteRecord> LibraryHierarchyController::indexedNotesSnapshot() const
{
    return m_indexedState.allNotes();
}

bool LibraryHierarchyController::indexedNoteRecordById(const QString& noteId, LibraryNoteRecord* outNote) const
{
    return m_indexedState.noteById(noteId, outNote);
}

void LibraryHierarchyController::setHubStore(WhatSonHubStore store)
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
                              QStringLiteral("library.controller"),
                              QStringLiteral("setHubStore"),
                              QStringLiteral("hub=%1 library=%2")
                              .arg(m_hubStore.hubPath(), m_hubStore.libraryPath()));
}

WhatSonHubStore LibraryHierarchyController::hubStore() const
{
    return m_hubStore;
}

int LibraryHierarchyController::extractDepth(const QVariantMap& entryMap)
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

LibraryHierarchyItem LibraryHierarchyController::parseItem(const QVariant& entry, int fallbackOrdinal)
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
        parsed.folderUuid = normalizeFolderUuid(
            entryMap.value(QStringLiteral("uuid"),
                           entryMap.value(QStringLiteral("folderUuid"))).toString());
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
            QStringLiteral("library.controller"),
            QStringLiteral("parseItem.emptyLabelKept"));
    }

    return parsed;
}

int LibraryHierarchyController::nextFolderSequence(const QVector<LibraryHierarchyItem>& items)
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

QVector<LibraryNoteListItem> LibraryHierarchyController::buildNoteListItems(
    const QVector<LibraryNoteRecord>& notes) const
{
    return m_noteListProjection.buildNoteListItems(m_items, notes, m_foldersHierarchyLoaded);
}

QVector<LibraryNoteListItem> LibraryHierarchyController::buildFolderScopedNoteListItems(
    const FolderSelectionScope& scope) const
{
    return m_noteListProjection.buildFolderScopedNoteListItems(
        m_items,
        m_indexedState.allNotes(),
        scope.selectedFolderUuid);
}

const QVector<LibraryNoteRecord>& LibraryHierarchyController::notesForBucket(IndexedBucket bucket) const
{
    switch (bucket)
    {
    case IndexedBucket::Draft:
        return m_indexedState.draftNotes();
    case IndexedBucket::Today:
        return m_indexedState.todayNotes();
    case IndexedBucket::All:
    default:
        return m_indexedState.allNotes();
    }
}

const LibraryHierarchyController::IndexedBucketRange* LibraryHierarchyController::bucketRangeForIndex(int index) const
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

LibraryHierarchyController::IndexedBucket LibraryHierarchyController::selectedBucket() const
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

LibraryHierarchyController::FolderSelectionScope LibraryHierarchyController::selectedFolderScope() const
{
    FolderSelectionScope scope;
    if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
    {
        return scope;
    }

    scope.selectedFolderUuid = folderUuidForIndex(m_selectedIndex);
    if (scope.selectedFolderUuid.isEmpty())
    {
        return scope;
    }

    return scope;
}

QString LibraryHierarchyController::normalizeFolderKey(const QString& value)
{
    return normalizeFolderLookupKey(value);
}

QString LibraryHierarchyController::folderPathForIndex(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }
    return normalizeFolderPath(m_items.at(index).folderPath);
}

QString LibraryHierarchyController::folderUuidForIndex(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return {};
    }
    return normalizeFolderUuid(m_items.at(index).folderUuid);
}

bool LibraryHierarchyController::upsertIndexedNote(const LibraryNoteRecord& note)
{
    const QString normalizedNoteId = note.noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    const bool changed = m_indexedState.upsertNote(note);
    invalidateNoteListItemCacheForNoteId(normalizedNoteId);
    if (changed)
    {
        emit indexedNoteUpserted(normalizedNoteId);
    }
    return changed;
}

bool LibraryHierarchyController::removeIndexedNoteById(const QString& noteId)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    const bool changed = m_indexedState.removeNoteById(normalizedNoteId);
    invalidateNoteListItemCacheForNoteId(normalizedNoteId);
    return changed;
}

void LibraryHierarchyController::invalidateNoteListItemCache() const
{
    m_noteListProjection.invalidate();
}

void LibraryHierarchyController::invalidateNoteListItemCacheForNoteId(const QString& noteId) const
{
    m_noteListProjection.invalidateForNoteId(noteId);
}

void LibraryHierarchyController::setIndexedStateNotes(QString sourceWshubPath, QVector<LibraryNoteRecord> notes)
{
    m_indexedState.setIndexedNotes(std::move(sourceWshubPath), std::move(notes));
    invalidateNoteListItemCache();
    emit indexedNotesSnapshotChanged();
}

void LibraryHierarchyController::applyIndexedStateSnapshot(
    QString wshubPath,
    QVector<LibraryNoteRecord> allNotes,
    QVector<LibraryNoteRecord> draftNotes,
    QVector<LibraryNoteRecord> todayNotes)
{
    m_indexedState.applySnapshot(
        std::move(wshubPath),
        std::move(allNotes),
        std::move(draftNotes),
        std::move(todayNotes));
    invalidateNoteListItemCache();
    emit indexedNotesSnapshotChanged();
}

bool LibraryHierarchyController::loadIndexedStateFromWshub(const QString& wshubPath, QString* errorMessage)
{
    const bool succeeded = m_indexedState.indexFromWshub(wshubPath, errorMessage);
    invalidateNoteListItemCache();
    if (succeeded)
    {
        emit indexedNotesSnapshotChanged();
    }
    return succeeded;
}

bool LibraryHierarchyController::commitFolderHierarchyUpdate(
    QVector<LibraryHierarchyItem> stagedItems,
    int selectedIndex,
    const QHash<QString, QString>& movedFolderPathMap,
    bool preserveClearedSelection)
{
    const QVector<WhatSonFolderDepthEntry> currentFolderEntries = folderEntriesFromItems(m_items);
    const QVector<WhatSonFolderDepthEntry> stagedFolderEntries = folderEntriesFromItems(stagedItems);
    const QString stagedSelectionKey =
        (selectedIndex >= 0 && selectedIndex < stagedItems.size())
            ? hierarchyItemKey(stagedItems.at(selectedIndex), selectedIndex)
            : QString();
    const QString stagedSelectionFolderPath =
        (selectedIndex >= 0 && selectedIndex < stagedItems.size())
            ? normalizeFolderPath(stagedItems.at(selectedIndex).folderPath)
            : QString();
    const QSet<QString> stagedExpandedKeys = expandedHierarchyItemKeys(stagedItems);

    WhatSonLibraryFolderHierarchyMutationService mutationService;
    WhatSonLibraryFolderHierarchyMutationService::Request request;
    request.foldersFilePath = m_foldersFilePath;
    request.runtimeIndexLoaded = m_runtimeIndexLoaded;
    request.currentFolderEntries = currentFolderEntries;
    request.stagedFolderEntries = stagedFolderEntries;
    request.notes = m_indexedState.allNotes();
    request.movedFolderPathMap = movedFolderPathMap;

    WhatSonLibraryFolderHierarchyMutationService::Result result;
    QString mutationError;
    if (!mutationService.commitMutation(std::move(request), &result, &mutationError))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.controller"),
                                  QStringLiteral("commitFolderHierarchyUpdate.failed"),
                                  QStringLiteral("error=%1").arg(mutationError));
        return false;
    }

    if (m_runtimeIndexLoaded)
    {
        setIndexedStateNotes(m_indexedState.sourceWshubPath(), std::move(result.notes));
    }

    QString reloadError;
    if (!reloadFolderHierarchyFromFoldersFile(stagedExpandedKeys, &reloadError))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.controller"),
                                  QStringLiteral("commitFolderHierarchyUpdate.reloadFallback"),
                                  QStringLiteral("error=%1").arg(reloadError));
        m_items = std::move(stagedItems);
        m_foldersHierarchyLoaded = !stagedFolderEntries.isEmpty();
        m_createdFolderSequence = nextFolderSequence(m_items);
    }

    int mirroredSelectedIndex = selectedHierarchyIndexForKey(m_items, stagedSelectionKey);
    if (mirroredSelectedIndex < 0 && !stagedSelectionFolderPath.isEmpty())
    {
        mirroredSelectedIndex = selectedHierarchyIndexForFolderPath(m_items, stagedSelectionFolderPath);
    }
    if (mirroredSelectedIndex < 0)
    {
        mirroredSelectedIndex = selectedIndex;
    }

    rebuildBucketRanges();
    syncModel();
    if (preserveClearedSelection && selectedIndex < 0)
    {
        clearSelectedIndex(true);
    }
    else
    {
        applySelectedIndex(mirroredSelectedIndex, true);
    }
    return true;
}

bool LibraryHierarchyController::reloadFolderHierarchyFromFoldersFile(
    const QSet<QString>& preservedExpandedKeys,
    QString* errorMessage)
{
    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }

    const QString foldersFilePath = m_foldersFilePath.trimmed();
    if (foldersFilePath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Folders.wsfolders path is empty.");
        }
        return false;
    }

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

    WhatSonFoldersHierarchyParser foldersParser;
    WhatSonFoldersHierarchyStore foldersStore;
    QString parseError;
    bool folderUuidMigrationRequired = false;
    if (!foldersParser.parse(rawText, &foldersStore, &parseError, &folderUuidMigrationRequired))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = parseError;
        }
        return false;
    }

    if (folderUuidMigrationRequired)
    {
        QString writeError;
        if (!foldersStore.writeToFile(foldersFilePath, &writeError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = writeError;
            }
            return false;
        }
    }

    QVector<LibraryHierarchyItem> mirroredItems = prependInAppLibraryScaffold(
        buildFolderItems(foldersStore.folderEntries()));
    finalizeFolderItems(&mirroredItems, true);
    dropReservedTodayFolderItems(&mirroredItems);
    restoreExpandedHierarchyItemKeys(&mirroredItems, preservedExpandedKeys);

    m_items = std::move(mirroredItems);
    m_foldersHierarchyLoaded = !foldersStore.folderEntries().isEmpty();
    m_createdFolderSequence = nextFolderSequence(m_items);

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
                              QStringLiteral("reloadFolderHierarchyFromFoldersFile"),
                              QStringLiteral("path=%1 itemCount=%2 foldersLoaded=%3")
                              .arg(foldersFilePath)
                              .arg(m_items.size())
                              .arg(m_foldersHierarchyLoaded ? QStringLiteral("1") : QStringLiteral("0")));
    return true;
}

int LibraryHierarchyController::firstEditableInsertIndex() const noexcept
{
    int index = 0;
    while (index < m_items.size() && isProtectedRootItem(m_items.at(index)))
    {
        ++index;
    }
    return index;
}

void LibraryHierarchyController::rebuildBucketRanges()
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

void LibraryHierarchyController::refreshNoteListForSelection()
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
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.controller"),
                                  QStringLiteral("refreshNoteListModel.bucketRange"),
                                  QStringLiteral("bucket=%1 count=%2 firstItemId=%3 firstItemDirectoryPath=%4")
                                      .arg(static_cast<int>(range->bucket))
                                      .arg(listItems.size())
                                      .arg(listItems.isEmpty() ? QString() : listItems.constFirst().id)
                                      .arg(listItems.isEmpty() ? QString() : listItems.constFirst().noteDirectoryPath));
        m_noteListModel.setItems(listItems);
        updateNoteItemCount();
        if (shouldAutoActivateMostRecentNote())
            autoActivateMostRecentNote();
        return;
    }

    if (m_foldersHierarchyLoaded)
    {
        if (m_selectedIndex < 0 || m_selectedIndex >= m_items.size())
        {
            const QVector<LibraryNoteListItem> listItems = buildNoteListItems(m_indexedState.allNotes());
            WhatSon::Debug::traceSelf(this,
                                      QStringLiteral("library.controller"),
                                      QStringLiteral("refreshNoteListModel.allNotes"),
                                      QStringLiteral("count=%1 firstItemId=%2 firstItemDirectoryPath=%3")
                                          .arg(listItems.size())
                                          .arg(listItems.isEmpty() ? QString() : listItems.constFirst().id)
                                          .arg(listItems.isEmpty() ? QString() : listItems.constFirst().noteDirectoryPath));
            m_noteListModel.setItems(listItems);
            updateNoteItemCount();
            if (shouldAutoActivateMostRecentNote())
                autoActivateMostRecentNote();
            return;
        }

        const FolderSelectionScope scope = selectedFolderScope();
        const QVector<LibraryNoteListItem> listItems = buildFolderScopedNoteListItems(scope);
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("library.controller"),
                                  QStringLiteral("refreshNoteListModel.folderScope"),
                                  QStringLiteral("folderUuid=%1 count=%2 firstItemId=%3 firstItemDirectoryPath=%4")
                                      .arg(scope.selectedFolderUuid)
                                      .arg(listItems.size())
                                      .arg(listItems.isEmpty() ? QString() : listItems.constFirst().id)
                                      .arg(listItems.isEmpty() ? QString() : listItems.constFirst().noteDirectoryPath));
        m_noteListModel.setItems(listItems);
        updateNoteItemCount();
        if (shouldAutoActivateMostRecentNote())
            autoActivateMostRecentNote();
        return;
    }

    const IndexedBucket bucket = selectedBucket();
    const QVector<LibraryNoteListItem> listItems = buildNoteListItems(notesForBucket(bucket));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
                              QStringLiteral("refreshNoteListModel.bucket"),
                              QStringLiteral("bucket=%1 count=%2 firstItemId=%3 firstItemDirectoryPath=%4")
                                  .arg(static_cast<int>(bucket))
                                  .arg(listItems.size())
                                  .arg(listItems.isEmpty() ? QString() : listItems.constFirst().id)
                                  .arg(listItems.isEmpty() ? QString() : listItems.constFirst().noteDirectoryPath));
    m_noteListModel.setItems(listItems);
    updateNoteItemCount();
    if (shouldAutoActivateMostRecentNote())
        autoActivateMostRecentNote();
}

void LibraryHierarchyController::refreshNoteListForSelectionAndNotifyHierarchyModel()
{
    refreshNoteListForSelection();
    emit hierarchyModelChanged();
}

void LibraryHierarchyController::applyInAppLibraryScaffold()
{
    m_items = prependInAppLibraryScaffold({});
    m_foldersHierarchyLoaded = false;
    rebuildBucketRanges();
    m_createdFolderSequence = nextFolderSequence(m_items);
    syncModel();
    refreshNoteListForSelection();
}

void LibraryHierarchyController::syncModel()
{
    invalidateNoteListItemCache();
    applyChevronByDepth(&m_items);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("library.controller"),
                              QStringLiteral("syncModel"),
                              QStringLiteral("itemCount=%1").arg(m_items.size()));
    m_itemModel.setItems(depthItems());
    updateItemCount();
    emit hierarchyModelChanged();
}

void LibraryHierarchyController::updateItemCount()
{
    const int nextCount = m_itemModel.rowCount();
    if (m_itemCount == nextCount)
    {
        return;
    }
    m_itemCount = nextCount;
    emit itemCountChanged();
}

void LibraryHierarchyController::updateNoteItemCount()
{
    const int nextCount = m_noteListModel.rowCount();
    if (m_noteItemCount == nextCount)
    {
        return;
    }
    m_noteItemCount = nextCount;
    emit noteItemCountChanged();
}

void LibraryHierarchyController::updateLoadState(bool succeeded, QString errorMessage)
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
