#pragma once

#include "app/models/hierarchy/WhatSonHierarchyIoSupport.hpp"
#include "app/models/hierarchy/WhatSonHierarchyTreeItemSupport.hpp"

#include <QSet>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

#include <algorithm>
#include <utility>

namespace WhatSon::Hierarchy::NamedStringSupport
{
    template <typename Item>
    inline QStringList sanitizeStringList(QStringList values)
    {
        return IoSupport::deduplicateStringsPreservingOrder(
            std::move(values),
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

    template <typename Item>
    inline Item parseItemEntry(const QVariant& entry, int fallbackOrdinal, const QString& fallbackPrefix)
    {
        Item item;

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

    template <typename Item>
    inline QVector<Item> parseDepthItems(const QVariantList& depthItems, const QString& fallbackPrefix)
    {
        QVector<Item> items;
        items.reserve(depthItems.size());

        int ordinal = 1;
        for (const QVariant& entry : depthItems)
        {
            items.push_back(parseItemEntry<Item>(entry, ordinal, fallbackPrefix));
            ++ordinal;
        }

        TreeItemSupport::applyChevronByDepth(&items);
        return items;
    }

    template <typename Item>
    inline QVariantList serializeDepthItems(const QVector<Item>& items)
    {
        QVariantList serialized;
        serialized.reserve(items.size());

        for (const Item& item : items)
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

    template <typename Item>
    inline QVector<Item> buildBucketItems(
        const QString& bucketName,
        const QStringList& values,
        const QString& fallbackPrefix)
    {
        const QStringList sanitized = sanitizeStringList<Item>(values);

        QVector<Item> items;
        items.reserve(sanitized.size());

        Q_UNUSED(bucketName);
        Q_UNUSED(fallbackPrefix);
        for (const QString& value : sanitized)
        {
            Item child;
            child.depth = 0;
            child.accent = false;
            child.expanded = false;
            child.label = value;
            child.showChevron = false;
            items.push_back(std::move(child));
        }

        TreeItemSupport::applyChevronByDepth(&items);
        return items;
    }

    template <typename Item>
    inline int createHierarchyFolder(QVector<Item>* items, int selectedIndex, int* ioFolderSequence)
    {
        return TreeItemSupport::createNestedHierarchyFolder(items, selectedIndex, ioFolderSequence);
    }

    template <typename Item>
    inline QStringList extractDomainLabelsFromItems(const QVector<Item>& items)
    {
        return IoSupport::extractDistinctLabelsFromItems(
            items,
            [](int index, const Item& item)
            {
                return index == 0 && item.depth == 0 && item.accent;
            });
    }

    template <typename Item>
    inline QString itemKey(const QVector<Item>& items, int index, const QString& keyPrefix)
    {
        if (index < 0 || index >= items.size())
        {
            return {};
        }

        const auto normalizedSegment = [&keyPrefix](const Item& item, int fallbackIndex)
        {
            const QString normalizedLabel = item.label.trimmed();
            if (!normalizedLabel.isEmpty())
            {
                return normalizedLabel;
            }
            return QStringLiteral("%1:%2").arg(keyPrefix).arg(fallbackIndex);
        };

        QStringList pathSegments;
        pathSegments.reserve(std::max(1, items.at(index).depth + 1));
        pathSegments.push_front(normalizedSegment(items.at(index), index));

        int expectedDepth = std::max(0, items.at(index).depth);
        for (int cursor = index - 1; cursor >= 0 && expectedDepth > 0; --cursor)
        {
            const Item& candidate = items.at(cursor);
            if (std::max(0, candidate.depth) != expectedDepth - 1)
            {
                continue;
            }
            pathSegments.push_front(normalizedSegment(candidate, cursor));
            expectedDepth = std::max(0, candidate.depth);
        }

        return pathSegments.join(QLatin1Char('/'));
    }

    template <typename Item>
    inline QSet<QString> expandedItemKeys(const QVector<Item>& items, const QString& keyPrefix)
    {
        QSet<QString> expandedKeys;
        for (int index = 0; index < items.size(); ++index)
        {
            if (!items.at(index).expanded)
            {
                continue;
            }
            expandedKeys.insert(itemKey(items, index, keyPrefix));
        }
        return expandedKeys;
    }

    template <typename Item>
    inline void restoreExpandedItemKeys(QVector<Item>* items, const QString& keyPrefix, const QSet<QString>& expandedKeys)
    {
        if (items == nullptr)
        {
            return;
        }

        for (int index = 0; index < items->size(); ++index)
        {
            (*items)[index].expanded = expandedKeys.contains(itemKey(*items, index, keyPrefix));
        }
    }

    template <typename Item>
    inline int selectedIndexForKey(const QVector<Item>& items, const QString& keyPrefix, const QString& key)
    {
        const QString normalizedKey = key.trimmed();
        if (normalizedKey.isEmpty())
        {
            return -1;
        }

        for (int index = 0; index < items.size(); ++index)
        {
            if (itemKey(items, index, keyPrefix) == normalizedKey)
            {
                return index;
            }
        }

        return -1;
    }

    template <typename Item>
    inline QVariantList lvrsDepthItems(const QVector<Item>& items, const QString& keyPrefix)
    {
        QVariantList serialized = serializeDepthItems(items);
        for (int index = 0; index < serialized.size(); ++index)
        {
            QVariantMap entry = serialized.at(index).toMap();
            entry.insert(QStringLiteral("itemId"), index);
            entry.insert(QStringLiteral("key"), QStringLiteral("%1:%2").arg(keyPrefix).arg(index));
            entry.insert(QStringLiteral("count"), 0);
            serialized[index] = entry;
        }
        return serialized;
    }

    template <typename Item>
    inline bool setItemExpanded(QVector<Item>* items, int index, bool expanded, bool* changed = nullptr)
    {
        if (changed != nullptr)
        {
            *changed = false;
        }
        if (items == nullptr || index < 0 || index >= items->size())
        {
            return false;
        }
        if (!items->at(index).showChevron)
        {
            return false;
        }
        if (items->at(index).expanded == expanded)
        {
            return true;
        }

        (*items)[index].expanded = expanded;
        if (changed != nullptr)
        {
            *changed = true;
        }
        return true;
    }
} // namespace WhatSon::Hierarchy::NamedStringSupport
