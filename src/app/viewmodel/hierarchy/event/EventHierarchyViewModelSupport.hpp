#pragma once

#include "EventHierarchyModel.hpp"

#include "viewmodel/hierarchy/WhatSonHierarchyIoSupport.hpp"
#include "viewmodel/hierarchy/WhatSonHierarchyTreeItemSupport.hpp"

#include <QVariantList>
#include <QVariantMap>

#include <algorithm>

namespace WhatSon::Hierarchy::EventSupport
{
    using WhatSon::Hierarchy::IoSupport::deduplicateStringsPreservingOrder;
    using WhatSon::Hierarchy::IoSupport::extractDistinctLabelsFromItems;
    using WhatSon::Hierarchy::IoSupport::normalizePath;
    using WhatSon::Hierarchy::IoSupport::readUtf8File;
    using WhatSon::Hierarchy::IoSupport::resolveContentsDirectories;
    using WhatSon::Hierarchy::TreeItemSupport::applyChevronByDepth;
    using WhatSon::Hierarchy::TreeItemSupport::deleteHierarchySubtree;
    using WhatSon::Hierarchy::TreeItemSupport::isBucketHeaderItem;
    using WhatSon::Hierarchy::TreeItemSupport::nextGeneratedFolderSequence;
    using WhatSon::Hierarchy::TreeItemSupport::renameHierarchyItem;

    inline QStringList sanitizeStringList(QStringList values)
    {
        return deduplicateStringsPreservingOrder(
            values,
            [](QString& value)
            {
                return value.trimmed();
            });
    }

    inline int clampSelectionIndex(int requested, int itemCount)
    {
        if (itemCount <= 0)
        {
            return -1;
        }

        return std::clamp(requested, -1, itemCount - 1);
    }

    inline EventHierarchyItem parseItemEntry(const QVariant& entry, int fallbackOrdinal, const QString& fallbackPrefix)
    {
        EventHierarchyItem item;

        if (entry.metaType().id() == QMetaType::QVariantMap)
        {
            const QVariantMap map = entry.toMap();

            QVariant depthValue;
            if (map.contains(QStringLiteral("depth")))
            {
                depthValue = map.value(QStringLiteral("depth"));
            }
            else if (map.contains(QStringLiteral("dpeth")))
            {
                depthValue = map.value(QStringLiteral("dpeth"));
            }
            else if (map.contains(QStringLiteral("indentLevel")))
            {
                depthValue = map.value(QStringLiteral("indentLevel"));
            }

            bool converted = false;
            const int parsedDepth = depthValue.toInt(&converted);
            item.depth = converted ? std::max(0, parsedDepth) : 0;
            item.label = map.value(QStringLiteral("label")).toString().trimmed();
            item.accent = map.value(QStringLiteral("accent"), false).toBool();
            item.expanded = map.value(QStringLiteral("expanded"), false).toBool();
            item.showChevron = map.value(QStringLiteral("showChevron"), true).toBool();
        }
        else
        {
            bool converted = false;
            const int depth = entry.toInt(&converted);
            if (converted)
            {
                item.depth = std::max(0, depth);
            }
            item.label = entry.toString().trimmed();
        }

        if (item.label.isEmpty())
        {
            Q_UNUSED(fallbackPrefix);
            Q_UNUSED(fallbackOrdinal);
            WhatSon::Debug::trace(
                QStringLiteral("hierarchy.io.support"),
                QStringLiteral("parseItemEntry.emptyLabel"),
                QStringLiteral("depth=%1").arg(item.depth));
        }

        return item;
    }

    inline QVector<EventHierarchyItem> parseDepthItems(
        const QVariantList& depthItems,
        const QString& fallbackPrefix)
    {
        QVector<EventHierarchyItem> items;
        items.reserve(depthItems.size());

        int ordinal = 1;
        for (const QVariant& entry : depthItems)
        {
            items.push_back(parseItemEntry(entry, ordinal, fallbackPrefix));
            ++ordinal;
        }

        for (int index = 0; index < items.size(); ++index)
        {
            const int nextIndex = index + 1;
            const bool hasChild = nextIndex < items.size() && items.at(nextIndex).depth > items.at(index).depth;
            items[index].showChevron = hasChild;
        }

        return items;
    }

    inline QVariantList serializeDepthItems(const QVector<EventHierarchyItem>& items)
    {
        QVariantList serialized;
        serialized.reserve(items.size());

        for (const EventHierarchyItem& item : items)
        {
            serialized.push_back(QVariantMap{
                {QStringLiteral("label"), item.label},
                {QStringLiteral("depth"), item.depth},
                {QStringLiteral("accent"), item.accent},
                {QStringLiteral("expanded"), item.expanded},
                {QStringLiteral("showChevron"), item.showChevron}
            });
        }

        return serialized;
    }

    inline QVector<EventHierarchyItem> buildBucketItems(
        const QString& bucketName,
        const QStringList& values,
        const QString& fallbackPrefix)
    {
        const QStringList sanitized = sanitizeStringList(values);

        QVector<EventHierarchyItem> items;
        items.reserve(sanitized.size());

        Q_UNUSED(bucketName);
        Q_UNUSED(fallbackPrefix);
        for (const QString& value : sanitized)
        {
            EventHierarchyItem child;
            child.depth = 0;
            child.accent = false;
            child.expanded = false;
            child.label = value;
            child.showChevron = false;
            items.push_back(std::move(child));
        }

        for (int index = 0; index < items.size(); ++index)
        {
            const int nextIndex = index + 1;
            const bool hasChild = nextIndex < items.size() && items.at(nextIndex).depth > items.at(index).depth;
            items[index].showChevron = hasChild;
        }

        return items;
    }

    inline int createHierarchyFolder(QVector<EventHierarchyItem>* items, int selectedIndex, int* ioFolderSequence)
    {
        return WhatSon::Hierarchy::TreeItemSupport::createNestedHierarchyFolder(
            items,
            selectedIndex,
            ioFolderSequence,
            true);
    }

    inline QStringList extractDomainLabelsFromItems(const QVector<EventHierarchyItem>& items)
    {
        return extractDistinctLabelsFromItems(
            items,
            [](int index, const EventHierarchyItem& item)
            {
                return index == 0 && item.depth == 0 && item.accent;
            });
    }
} // namespace WhatSon::Hierarchy::EventSupport
