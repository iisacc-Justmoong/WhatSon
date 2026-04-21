#pragma once

#include "ResourcesHierarchyModel.hpp"

#include "models/file/hierarchy/resources/WhatSonResourcePackageSupport.hpp"
#include "viewmodel/hierarchy/WhatSonHierarchyIoSupport.hpp"

#include <QHash>
#include <QMetaType>
#include <QSet>
#include <QVariantList>
#include <QVariantMap>

#include <algorithm>

namespace WhatSon::Hierarchy::ResourcesSupport
{
    using WhatSon::Hierarchy::IoSupport::deduplicateStringsPreservingOrder;
    using WhatSon::Hierarchy::IoSupport::normalizePath;
    using WhatSon::Hierarchy::IoSupport::readUtf8File;
    using WhatSon::Hierarchy::IoSupport::resolveContentsDirectories;

    struct MaterializedResourceEntry final
    {
        WhatSon::Resources::ResourcePackageMetadata metadata;
        QString resolvedAssetPath;
    };

    inline QStringList sanitizeStringList(QStringList values)
    {
        return deduplicateStringsPreservingOrder(
            values,
            [](QString& value)
            {
                return normalizePath(value.trimmed());
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

    inline ResourcesHierarchyItem parseItemEntry(const QVariant& entry, int fallbackOrdinal, const QString&)
    {
        ResourcesHierarchyItem item;

        if (entry.metaType().id() == QMetaType::QVariantMap)
        {
            const QVariantMap map = entry.toMap();
            item.depth = std::max(0, map.value(QStringLiteral("depth")).toInt());
            item.count = std::max(0, map.value(QStringLiteral("count")).toInt());
            item.label = map.value(QStringLiteral("label")).toString().trimmed();
            item.accent = map.value(QStringLiteral("accent"), false).toBool();
            item.expanded = map.value(QStringLiteral("expanded"), false).toBool();
            item.showChevron = map.value(QStringLiteral("showChevron"), false).toBool();
            item.key = map.value(QStringLiteral("key")).toString().trimmed();
            item.kind = map.value(QStringLiteral("kind")).toString().trimmed();
            item.bucket = map.value(QStringLiteral("bucket")).toString().trimmed();
            item.type = map.value(QStringLiteral("type")).toString().trimmed();
            item.format = map.value(QStringLiteral("format")).toString().trimmed();
            item.resourceId = map.value(QStringLiteral("resourceId")).toString().trimmed();
            item.resourcePath = normalizePath(map.value(QStringLiteral("resourcePath")).toString());
            item.assetPath = normalizePath(map.value(QStringLiteral("assetPath")).toString());
        }
        else
        {
            item.label = entry.toString().trimmed();
            item.key = QStringLiteral("resources:%1").arg(fallbackOrdinal);
        }

        if (item.key.isEmpty())
        {
            item.key = QStringLiteral("resources:%1").arg(fallbackOrdinal);
        }
        return item;
    }

    inline QVector<ResourcesHierarchyItem> parseDepthItems(const QVariantList& depthItems, const QString& fallbackPrefix)
    {
        QVector<ResourcesHierarchyItem> items;
        items.reserve(depthItems.size());

        int ordinal = 1;
        for (const QVariant& entry : depthItems)
        {
            items.push_back(parseItemEntry(entry, ordinal, fallbackPrefix));
            ++ordinal;
        }
        return items;
    }

    inline QVariantList serializeDepthItems(const QVector<ResourcesHierarchyItem>& items)
    {
        QVariantList serialized;
        serialized.reserve(items.size());

        for (const ResourcesHierarchyItem& item : items)
        {
            serialized.push_back(QVariantMap{
                {QStringLiteral("label"), item.label},
                {QStringLiteral("count"), item.count},
                {QStringLiteral("depth"), item.depth},
                {QStringLiteral("accent"), item.accent},
                {QStringLiteral("expanded"), item.expanded},
                {QStringLiteral("showChevron"), item.showChevron},
                {QStringLiteral("iconName"), resourcesHierarchyIconName(item)},
                {QStringLiteral("key"), item.key},
                {QStringLiteral("kind"), item.kind},
                {QStringLiteral("bucket"), item.bucket},
                {QStringLiteral("type"), item.type},
                {QStringLiteral("format"), item.format},
                {QStringLiteral("resourceId"), item.resourceId},
                {QStringLiteral("resourcePath"), item.resourcePath},
                {QStringLiteral("assetPath"), item.assetPath}
            });
        }

        return serialized;
    }

    inline QHash<QString, bool> expansionStateByKey(const QVector<ResourcesHierarchyItem>& items)
    {
        QHash<QString, bool> states;
        states.reserve(items.size());
        for (const ResourcesHierarchyItem& item : items)
        {
            if (!item.key.isEmpty() && item.showChevron)
            {
                states.insert(item.key, item.expanded);
            }
        }
        return states;
    }

    inline QString itemKeyForType(const QString& type)
    {
        QString normalizedTypeKey = WhatSon::Resources::normalizedType(type);
        if (normalizedTypeKey.isEmpty())
        {
            normalizedTypeKey = QStringLiteral("other");
        }
        return QStringLiteral("type:%1").arg(normalizedTypeKey);
    }

    inline QString itemKeyForFormat(const QString& type, const QString& format)
    {
        QString normalizedTypeKey = WhatSon::Resources::normalizedType(type);
        if (normalizedTypeKey.isEmpty())
        {
            normalizedTypeKey = QStringLiteral("other");
        }
        const QString normalizedFormat = WhatSon::Resources::normalizedFormatLookupKey(format);
        return QStringLiteral("format:%1:%2").arg(
            normalizedTypeKey,
            normalizedFormat.isEmpty() ? QStringLiteral(".bin") : normalizedFormat);
    }

    inline QString itemKeyForAsset(const WhatSon::Resources::ResourcePackageMetadata& metadata)
    {
        const QString normalizedPath = normalizePath(metadata.resourcePath);
        if (!normalizedPath.isEmpty())
        {
            return QStringLiteral("asset:%1").arg(normalizedPath);
        }
        return QStringLiteral("asset:%1").arg(metadata.resourceId.trimmed());
    }

    inline QString typeKeyForMetadata(const WhatSon::Resources::ResourcePackageMetadata& metadata)
    {
        QString typeKey = WhatSon::Resources::normalizedTypeFromBucketAndFormat(
            metadata.type,
            metadata.bucket,
            metadata.format);
        typeKey = WhatSon::Resources::normalizedType(typeKey);
        if (typeKey == QStringLiteral("image")
            || typeKey == QStringLiteral("video")
            || typeKey == QStringLiteral("document")
            || typeKey == QStringLiteral("model")
            || typeKey == QStringLiteral("link")
            || typeKey == QStringLiteral("music")
            || typeKey == QStringLiteral("audio")
            || typeKey == QStringLiteral("archive"))
        {
            return typeKey;
        }
        return QStringLiteral("other");
    }

    inline QString displayLabelForTypeKey(const QString& typeKey)
    {
        const QString normalizedTypeKey = WhatSon::Resources::normalizedType(typeKey);
        if (normalizedTypeKey == QStringLiteral("image"))
        {
            return QStringLiteral("Image");
        }
        if (normalizedTypeKey == QStringLiteral("video"))
        {
            return QStringLiteral("Video");
        }
        if (normalizedTypeKey == QStringLiteral("document"))
        {
            return QStringLiteral("Document");
        }
        if (normalizedTypeKey == QStringLiteral("model"))
        {
            return QStringLiteral("3D Model");
        }
        if (normalizedTypeKey == QStringLiteral("link"))
        {
            return QStringLiteral("Web page");
        }
        if (normalizedTypeKey == QStringLiteral("music"))
        {
            return QStringLiteral("Music");
        }
        if (normalizedTypeKey == QStringLiteral("audio"))
        {
            return QStringLiteral("Audio");
        }
        if (normalizedTypeKey == QStringLiteral("archive"))
        {
            return QStringLiteral("ZIP");
        }

        return QStringLiteral("Other");
    }

    inline QStringList defaultTypeKeys()
    {
        return {
            QStringLiteral("image"),
            QStringLiteral("video"),
            QStringLiteral("document"),
            QStringLiteral("model"),
            QStringLiteral("link"),
            QStringLiteral("music"),
            QStringLiteral("audio"),
            QStringLiteral("archive"),
            QStringLiteral("other")
        };
    }

    inline QSet<QString> defaultFormatsForTypeKey(const QString& typeKey)
    {
        const QString normalizedTypeKey = WhatSon::Resources::normalizedType(typeKey);
        if (normalizedTypeKey == QStringLiteral("image"))
        {
            return {
                QStringLiteral(".png"),
                QStringLiteral(".jpeg"),
                QStringLiteral(".jpg"),
                QStringLiteral(".tiff"),
                QStringLiteral(".webp"),
                QStringLiteral(".gif"),
                QStringLiteral(".bmp"),
                QStringLiteral(".svg"),
                QStringLiteral(".heic"),
                QStringLiteral(".avif")
            };
        }
        if (normalizedTypeKey == QStringLiteral("video"))
        {
            return {
                QStringLiteral(".mp4"),
                QStringLiteral(".mov"),
                QStringLiteral(".avi"),
                QStringLiteral(".mkv"),
                QStringLiteral(".webm"),
                QStringLiteral(".m4v")
            };
        }
        if (normalizedTypeKey == QStringLiteral("document"))
        {
            return {
                QStringLiteral(".pdf"),
                QStringLiteral(".doc"),
                QStringLiteral(".docx"),
                QStringLiteral(".txt"),
                QStringLiteral(".md"),
                QStringLiteral(".rtf"),
                QStringLiteral(".ppt"),
                QStringLiteral(".pptx"),
                QStringLiteral(".xls"),
                QStringLiteral(".xlsx"),
                QStringLiteral(".csv")
            };
        }
        if (normalizedTypeKey == QStringLiteral("model"))
        {
            return {
                QStringLiteral(".fbx"),
                QStringLiteral(".obj"),
                QStringLiteral(".gltf"),
                QStringLiteral(".glb"),
                QStringLiteral(".usd"),
                QStringLiteral(".usdz"),
                QStringLiteral(".blend"),
                QStringLiteral(".stl")
            };
        }
        if (normalizedTypeKey == QStringLiteral("link"))
        {
            return {
                QStringLiteral(".html"),
                QStringLiteral(".htm"),
                QStringLiteral(".mhtml"),
                QStringLiteral(".webloc"),
                QStringLiteral(".url")
            };
        }
        if (normalizedTypeKey == QStringLiteral("music"))
        {
            return {
                QStringLiteral(".mp3"),
                QStringLiteral(".aac"),
                QStringLiteral(".m4a"),
                QStringLiteral(".flac"),
                QStringLiteral(".alac"),
                QStringLiteral(".aiff")
            };
        }
        if (normalizedTypeKey == QStringLiteral("audio"))
        {
            return {
                QStringLiteral(".wav"),
                QStringLiteral(".ogg"),
                QStringLiteral(".opus"),
                QStringLiteral(".caf"),
                QStringLiteral(".wma"),
                QStringLiteral(".amr")
            };
        }
        if (normalizedTypeKey == QStringLiteral("archive"))
        {
            return {
                QStringLiteral(".zip"),
                QStringLiteral(".rar"),
                QStringLiteral(".7z"),
                QStringLiteral(".tar"),
                QStringLiteral(".gz"),
                QStringLiteral(".bz2"),
                QStringLiteral(".xz")
            };
        }

        return {
            QStringLiteral(".bin")
        };
    }

    inline MaterializedResourceEntry materializeResourceEntry(
        const QString& resourcePath,
        const QStringList& resolutionBasePaths)
    {
        MaterializedResourceEntry entry;
        entry.resolvedAssetPath = WhatSon::Resources::resolveAssetLocationFromReference(resourcePath, resolutionBasePaths);

        const QString resolvedPackagePath = WhatSon::Resources::resolvePackageDirectoryFromReference(
            resourcePath,
            resolutionBasePaths);
        if (!resolvedPackagePath.isEmpty())
        {
            WhatSon::Resources::ResourcePackageMetadata metadata;
            QString errorMessage;
            if (WhatSon::Resources::loadResourcePackageMetadata(resolvedPackagePath, &metadata, &errorMessage))
            {
                if (metadata.resourcePath.trimmed().isEmpty())
                {
                    metadata.resourcePath = normalizePath(resourcePath);
                }
                entry.metadata = metadata;
                return entry;
            }

            WhatSon::Debug::trace(
                QStringLiteral("resources.viewmodel"),
                QStringLiteral("materializeResourceEntry.packageMetadataFailed"),
                QStringLiteral("path=%1 reason=%2").arg(resourcePath, errorMessage));
        }

        entry.metadata = WhatSon::Resources::buildFallbackMetadataFromResourcePath(resourcePath, entry.resolvedAssetPath);
        return entry;
    }

    inline QVector<ResourcesHierarchyItem> buildHierarchyItems(
        const QStringList& resourcePaths,
        const QStringList& resolutionBasePaths,
        const QVector<ResourcesHierarchyItem>& previousItems = {})
    {
        const QHash<QString, bool> previousExpansionStates = expansionStateByKey(previousItems);

        const QStringList orderedTypes = defaultTypeKeys();
        QHash<QString, QSet<QString>> typeFormats;
        typeFormats.reserve(orderedTypes.size());
        QHash<QString, int> typeCounts;
        typeCounts.reserve(orderedTypes.size());
        QHash<QString, QHash<QString, int>> formatCountsByType;
        formatCountsByType.reserve(orderedTypes.size());
        for (const QString& resourcePath : resourcePaths)
        {
            const MaterializedResourceEntry materialized = materializeResourceEntry(resourcePath, resolutionBasePaths);
            const QString typeKey = typeKeyForMetadata(materialized.metadata);
            QString format = materialized.metadata.format.trimmed().isEmpty()
                                 ? QStringLiteral(".bin")
                                 : materialized.metadata.format.trimmed();
            format = WhatSon::Resources::normalizedFormatLookupKey(format);
            if (format.isEmpty())
            {
                format = QStringLiteral(".bin");
            }
            typeFormats[typeKey].insert(format);
            typeCounts[typeKey] = typeCounts.value(typeKey) + 1;
            QHash<QString, int>& formatCounts = formatCountsByType[typeKey];
            formatCounts[format] = formatCounts.value(format) + 1;
        }

        QVector<ResourcesHierarchyItem> items;
        items.reserve(orderedTypes.size() * 2);
        for (const QString& typeKey : orderedTypes)
        {
            QSet<QString> formats = defaultFormatsForTypeKey(typeKey);
            formats.unite(typeFormats.value(typeKey));
            QStringList sortedFormats = QStringList(formats.begin(), formats.end());
            std::sort(
                sortedFormats.begin(),
                sortedFormats.end(),
                [](const QString& left, const QString& right)
                {
                    return left.localeAwareCompare(right) < 0;
                });

            ResourcesHierarchyItem typeItem;
            typeItem.depth = 0;
            typeItem.label = displayLabelForTypeKey(typeKey);
            typeItem.key = itemKeyForType(typeKey);
            typeItem.kind = QStringLiteral("type");
            typeItem.count = typeCounts.value(typeKey);
            typeItem.type = typeKey;
            typeItem.bucket = WhatSon::Resources::inferBucket(typeKey, QString());
            typeItem.showChevron = !sortedFormats.isEmpty();
            typeItem.expanded = previousExpansionStates.value(typeItem.key, false);
            items.push_back(typeItem);

            const auto formatCountsIt = formatCountsByType.constFind(typeKey);
            const QHash<QString, int>* formatCounts = formatCountsIt == formatCountsByType.cend()
                                                         ? nullptr
                                                         : &formatCountsIt.value();
            for (const QString& format : sortedFormats)
            {
                ResourcesHierarchyItem formatItem;
                formatItem.depth = 1;
                formatItem.label = format;
                formatItem.key = itemKeyForFormat(typeKey, format);
                formatItem.kind = QStringLiteral("format");
                formatItem.count = formatCounts == nullptr ? 0 : formatCounts->value(format);
                formatItem.bucket = WhatSon::Resources::inferBucket(typeKey, format);
                formatItem.type = typeKey;
                formatItem.format = format;
                formatItem.showChevron = false;
                formatItem.expanded = false;
                items.push_back(formatItem);
            }
        }

        return items;
    }

    inline QStringList extractResourcePathsFromItems(const QVector<ResourcesHierarchyItem>& items)
    {
        QStringList resourcePaths;
        resourcePaths.reserve(items.size());

        QSet<QString> seenPaths;
        seenPaths.reserve(items.size());
        for (const ResourcesHierarchyItem& item : items)
        {
            if (item.kind != QStringLiteral("asset"))
            {
                continue;
            }

            const QString resourcePath = normalizePath(item.resourcePath);
            if (resourcePath.isEmpty() || seenPaths.contains(resourcePath))
            {
                continue;
            }

            seenPaths.insert(resourcePath);
            resourcePaths.push_back(resourcePath);
        }
        return resourcePaths;
    }

    inline bool hierarchyItemsEqual(
        const QVector<ResourcesHierarchyItem>& lhs,
        const QVector<ResourcesHierarchyItem>& rhs)
    {
        if (lhs.size() != rhs.size())
        {
            return false;
        }

        for (int index = 0; index < lhs.size(); ++index)
        {
            const ResourcesHierarchyItem& left = lhs.at(index);
            const ResourcesHierarchyItem& right = rhs.at(index);
            if (left.depth != right.depth
                || left.count != right.count
                || left.accent != right.accent
                || left.expanded != right.expanded
                || left.label != right.label
                || left.showChevron != right.showChevron
                || left.key != right.key
                || left.kind != right.kind
                || left.bucket != right.bucket
                || left.type != right.type
                || left.format != right.format
                || left.resourceId != right.resourceId
                || left.resourcePath != right.resourcePath
                || left.assetPath != right.assetPath)
            {
                return false;
            }
        }

        return true;
    }
} // namespace WhatSon::Hierarchy::ResourcesSupport
