#pragma once

#include <QRegularExpression>
#include <QVector>

#include <algorithm>

namespace WhatSon::Hierarchy::TreeItemSupport
{
    inline int clampSelectionIndexToVisibleDefault(int requested, int itemCount)
    {
        if (itemCount <= 0)
        {
            return -1;
        }

        const int clamped = std::clamp(requested, -1, itemCount - 1);
        return clamped >= 0 ? clamped : 0;
    }

    template <typename Item>
    inline void applyChevronByDepth(QVector<Item>* items)
    {
        if (items == nullptr)
        {
            return;
        }

        for (int index = 0; index < items->size(); ++index)
        {
            const int nextIndex = index + 1;
            const bool hasChild = nextIndex < items->size() && items->at(nextIndex).depth > items->at(index).depth;
            (*items)[index].showChevron = hasChild;
        }
    }

    template <typename Item>
    inline int nextGeneratedFolderSequence(const QVector<Item>& items)
    {
        static const QRegularExpression folderPattern(QStringLiteral("^Folder(\\d+)$"));

        int maxSequence = 0;
        for (const Item& item : items)
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

    template <typename Item>
    inline bool renameHierarchyItem(QVector<Item>* items, int index, const QString& displayName)
    {
        if (items == nullptr || index < 0 || index >= items->size())
        {
            return false;
        }

        const QString trimmedName = displayName.trimmed();
        if (trimmedName.isEmpty())
        {
            return false;
        }

        if (items->at(index).label == trimmedName)
        {
            return true;
        }

        (*items)[index].label = trimmedName;
        return true;
    }

    template <typename Item>
    inline bool isBucketHeaderItem(const Item& item)
    {
        return item.depth == 0 && item.accent;
    }

    template <typename Item>
    inline int createFlatHierarchyFolder(QVector<Item>* items, int selectedIndex, int* ioFolderSequence)
    {
        if (items == nullptr || ioFolderSequence == nullptr)
        {
            return -1;
        }

        int insertIndex = items->size();
        if (selectedIndex >= 0 && selectedIndex < items->size())
        {
            insertIndex = selectedIndex + 1;
        }

        Item newItem;
        newItem.depth = 0;
        newItem.label = QStringLiteral("Untitled");
        ++(*ioFolderSequence);
        newItem.accent = false;
        newItem.expanded = false;
        newItem.showChevron = false;

        items->insert(insertIndex, newItem);
        applyChevronByDepth(items);
        return insertIndex;
    }

    template <typename Item>
    inline int createNestedHierarchyFolder(
        QVector<Item>* items,
        int selectedIndex,
        int* ioFolderSequence,
        bool expandSelectedParent = false)
    {
        if (items == nullptr || ioFolderSequence == nullptr)
        {
            return -1;
        }

        int insertIndex = items->size();
        int folderDepth = 0;

        if (selectedIndex >= 0 && selectedIndex < items->size())
        {
            if (expandSelectedParent)
            {
                (*items)[selectedIndex].expanded = true;
            }

            const int selectedDepth = items->at(selectedIndex).depth;
            folderDepth = selectedDepth + 1;

            insertIndex = selectedIndex + 1;
            while (insertIndex < items->size() && items->at(insertIndex).depth > selectedDepth)
            {
                ++insertIndex;
            }
        }

        Item newItem;
        newItem.depth = folderDepth;
        newItem.label = QStringLiteral("Untitled");
        ++(*ioFolderSequence);
        newItem.accent = false;
        newItem.expanded = false;
        newItem.showChevron = true;

        items->insert(insertIndex, newItem);
        applyChevronByDepth(items);
        return insertIndex;
    }

    template <typename Item>
    inline int deleteHierarchySubtree(QVector<Item>* items, int selectedIndex)
    {
        if (items == nullptr || selectedIndex < 0 || selectedIndex >= items->size())
        {
            return -1;
        }

        const int startIndex = selectedIndex;
        const int baseDepth = items->at(startIndex).depth;

        int removeCount = 1;
        while (startIndex + removeCount < items->size() && items->at(startIndex + removeCount).depth > baseDepth)
        {
            ++removeCount;
        }

        items->remove(startIndex, removeCount);
        applyChevronByDepth(items);

        if (items->isEmpty())
        {
            return -1;
        }
        return std::min(startIndex, static_cast<int>(items->size() - 1));
    }
} // namespace WhatSon::Hierarchy::TreeItemSupport
