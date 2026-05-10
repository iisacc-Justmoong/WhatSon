#pragma once

#include "app/models/hierarchy/event/EventHierarchyModel.hpp"
#include "app/models/hierarchy/WhatSonNamedStringHierarchySupport.hpp"

namespace WhatSon::Hierarchy::EventSupport
{
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
        return NamedStringSupport::sanitizeStringList<EventHierarchyItem>(std::move(values));
    }

    inline int clampSelectionIndex(int requested, int itemCount)
    {
        return NamedStringSupport::clampSelectionIndex(requested, itemCount);
    }

    inline EventHierarchyItem parseItemEntry(
        const QVariant& entry,
        int fallbackOrdinal,
        const QString& fallbackPrefix)
    {
        return NamedStringSupport::parseItemEntry<EventHierarchyItem>(entry, fallbackOrdinal, fallbackPrefix);
    }

    inline QVector<EventHierarchyItem> parseDepthItems(
        const QVariantList& depthItems,
        const QString& fallbackPrefix)
    {
        return NamedStringSupport::parseDepthItems<EventHierarchyItem>(depthItems, fallbackPrefix);
    }

    inline QVariantList serializeDepthItems(const QVector<EventHierarchyItem>& items)
    {
        return NamedStringSupport::serializeDepthItems(items);
    }

    inline QVector<EventHierarchyItem> buildBucketItems(
        const QString& bucketName,
        const QStringList& values,
        const QString& fallbackPrefix)
    {
        return NamedStringSupport::buildBucketItems<EventHierarchyItem>(bucketName, values, fallbackPrefix);
    }

    inline int createHierarchyFolder(QVector<EventHierarchyItem>* items, int selectedIndex, int* ioFolderSequence)
    {
        return NamedStringSupport::createHierarchyFolder(items, selectedIndex, ioFolderSequence);
    }

    inline QStringList extractDomainLabelsFromItems(const QVector<EventHierarchyItem>& items)
    {
        return NamedStringSupport::extractDomainLabelsFromItems(items);
    }
} // namespace WhatSon::Hierarchy::EventSupport
