#pragma once

#include "app/models/hierarchy/preset/PresetHierarchyModel.hpp"
#include "app/models/hierarchy/WhatSonNamedStringHierarchySupport.hpp"

namespace WhatSon::Hierarchy::PresetSupport
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
        return NamedStringSupport::sanitizeStringList<PresetHierarchyItem>(std::move(values));
    }

    inline int clampSelectionIndex(int requested, int itemCount)
    {
        return NamedStringSupport::clampSelectionIndex(requested, itemCount);
    }

    inline PresetHierarchyItem parseItemEntry(
        const QVariant& entry,
        int fallbackOrdinal,
        const QString& fallbackPrefix)
    {
        return NamedStringSupport::parseItemEntry<PresetHierarchyItem>(entry, fallbackOrdinal, fallbackPrefix);
    }

    inline QVector<PresetHierarchyItem> parseDepthItems(
        const QVariantList& depthItems,
        const QString& fallbackPrefix)
    {
        return NamedStringSupport::parseDepthItems<PresetHierarchyItem>(depthItems, fallbackPrefix);
    }

    inline QVariantList serializeDepthItems(const QVector<PresetHierarchyItem>& items)
    {
        return NamedStringSupport::serializeDepthItems(items);
    }

    inline QVector<PresetHierarchyItem> buildBucketItems(
        const QString& bucketName,
        const QStringList& values,
        const QString& fallbackPrefix)
    {
        return NamedStringSupport::buildBucketItems<PresetHierarchyItem>(bucketName, values, fallbackPrefix);
    }

    inline int createHierarchyFolder(QVector<PresetHierarchyItem>* items, int selectedIndex, int* ioFolderSequence)
    {
        return NamedStringSupport::createHierarchyFolder(items, selectedIndex, ioFolderSequence);
    }

    inline QStringList extractDomainLabelsFromItems(const QVector<PresetHierarchyItem>& items)
    {
        return NamedStringSupport::extractDomainLabelsFromItems(items);
    }
} // namespace WhatSon::Hierarchy::PresetSupport
