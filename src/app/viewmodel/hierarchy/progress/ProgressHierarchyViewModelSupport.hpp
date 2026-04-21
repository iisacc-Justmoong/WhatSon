#pragma once

#include "app/viewmodel/hierarchy/progress/ProgressHierarchyModel.hpp"

#include "app/viewmodel/hierarchy/WhatSonHierarchyIoSupport.hpp"
#include "app/viewmodel/hierarchy/WhatSonHierarchyTreeItemSupport.hpp"

#include <QVariantList>
#include <QVariantMap>

#include <algorithm>

namespace WhatSon::Hierarchy::ProgressSupport
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

    inline ProgressHierarchyItem parseItemEntry(const QVariant& entry, int fallbackOrdinal,
                                                const QString& fallbackPrefix)
    {
        ProgressHierarchyItem item;

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

    inline QVector<ProgressHierarchyItem> parseDepthItems(
        const QVariantList& depthItems,
        const QString& fallbackPrefix)
    {
        QVector<ProgressHierarchyItem> items;
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

    inline QVariantList serializeDepthItems(const QVector<ProgressHierarchyItem>& items)
    {
        QVariantList serialized;
        serialized.reserve(items.size());

        for (const ProgressHierarchyItem& item : items)
        {
            serialized.push_back(QVariantMap{
                {QStringLiteral("label"), item.label},
                {QStringLiteral("depth"), item.depth},
                {QStringLiteral("accent"), item.accent},
                {QStringLiteral("expanded"), item.expanded},
                {QStringLiteral("showChevron"), item.showChevron},
                {QStringLiteral("iconName"), progressHierarchyIconName(item)}
            });
        }

        return serialized;
    }

    inline QVector<ProgressHierarchyItem> buildSupportedTypeItems(const QStringList& progressStates)
    {
        struct SupportedProgressItem final
        {
            QString label;
            bool showChevron;
        };

        static const QVector<SupportedProgressItem> kItems = {
            {QStringLiteral("First draft"), true},
            {QStringLiteral("Modified draft"), true},
            {QStringLiteral("In Progress"), true},
            {QStringLiteral("Pending"), true},
            {QStringLiteral("Reviewing"), false},
            {QStringLiteral("Waiting for approval"), false},
            {QStringLiteral("Done"), false},
            {QStringLiteral("Lagacy"), false},
            {QStringLiteral("Archived"), false},
            {QStringLiteral("Delete review"), false}
        };

        Q_UNUSED(progressStates);

        QVector<ProgressHierarchyItem> items;
        items.reserve(kItems.size());

        for (const SupportedProgressItem& supportedItem : kItems)
        {
            ProgressHierarchyItem item;
            item.depth = 0;
            item.accent = false;
            item.expanded = false;
            item.label = supportedItem.label;
            item.showChevron = supportedItem.showChevron;
            items.push_back(std::move(item));
        }

        return items;
    }

    inline QVector<ProgressHierarchyItem> buildBucketItems(
        const QString& bucketName,
        const QStringList& values,
        const QString& fallbackPrefix)
    {
        const QStringList sanitized = sanitizeStringList(values);

        QVector<ProgressHierarchyItem> items;
        items.reserve(sanitized.size() + 1);

        ProgressHierarchyItem bucket;
        bucket.depth = 0;
        bucket.accent = true;
        bucket.expanded = true;
        bucket.label = QStringLiteral("%1 (%2)").arg(bucketName).arg(sanitized.size());
        bucket.showChevron = !sanitized.isEmpty();
        items.push_back(std::move(bucket));

        Q_UNUSED(fallbackPrefix);
        for (const QString& value : sanitized)
        {
            ProgressHierarchyItem child;
            child.depth = 1;
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

    inline int createHierarchyFolder(QVector<ProgressHierarchyItem>* items, int selectedIndex, int* ioFolderSequence)
    {
        return WhatSon::Hierarchy::TreeItemSupport::createNestedHierarchyFolder(
            items,
            selectedIndex,
            ioFolderSequence);
    }

    inline QStringList extractDomainLabelsFromItems(const QVector<ProgressHierarchyItem>& items)
    {
        return extractDistinctLabelsFromItems(
            items,
            [](int index, const ProgressHierarchyItem& item)
            {
                return index == 0 && item.depth == 0 && item.accent;
            });
    }
} // namespace WhatSon::Hierarchy::ProgressSupport
