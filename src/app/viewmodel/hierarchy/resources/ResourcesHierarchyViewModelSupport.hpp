#pragma once

#include "ResourcesHierarchyModel.hpp"

#include "file/WhatSonDebugTrace.hpp"
#include "file/hierarchy/resources/WhatSonResourcePackageSupport.hpp"
#include "file/hub/WhatSonHubPathUtils.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QMetaType>
#include <QSet>
#include <QVariantList>
#include <QVariantMap>

#include <algorithm>

namespace WhatSon::Hierarchy::ResourcesSupport
{
    struct MaterializedResourceEntry final
    {
        WhatSon::Resources::ResourcePackageMetadata metadata;
        QString resolvedAssetPath;
    };

    inline QString normalizePath(const QString& input)
    {
        return WhatSon::HubPath::normalizePath(input);
    }

    inline bool resolveContentsDirectories(
        const QString& wshubPath,
        QStringList* outContentsDirectories,
        QString* errorMessage = nullptr)
    {
        if (outContentsDirectories == nullptr)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("outContentsDirectories must not be null.");
            }
            return false;
        }

        outContentsDirectories->clear();

        const QString hubRootPath = normalizePath(wshubPath);
        if (hubRootPath.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("wshubPath must not be empty.");
            }
            return false;
        }

        const QFileInfo hubInfo(hubRootPath);
        if (!hubInfo.exists())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("wshubPath does not exist: %1").arg(hubRootPath);
            }
            return false;
        }

        if (!hubInfo.fileName().endsWith(QStringLiteral(".wshub")) || !hubInfo.isDir())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("wshubPath must be an unpacked .wshub directory: %1").arg(hubRootPath);
            }
            return false;
        }

        const QDir hubDir(hubRootPath);

        const QString fixedInternalPath = hubDir.filePath(QStringLiteral(".wscontents"));
        if (QFileInfo(fixedInternalPath).isDir())
        {
            outContentsDirectories->push_back(WhatSon::HubPath::normalizePath(fixedInternalPath));
        }

        const QStringList dynamicContentsDirectories = hubDir.entryList(
            QStringList{QStringLiteral("*.wscontents")},
            QDir::Dirs | QDir::NoDotAndDotDot,
            QDir::Name);
        for (const QString& directoryName : dynamicContentsDirectories)
        {
            outContentsDirectories->push_back(WhatSon::HubPath::joinPath(hubDir.path(), directoryName));
        }

        outContentsDirectories->removeDuplicates();

        if (outContentsDirectories->isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("No *.wscontents directory was found inside .wshub: %1").
                    arg(hubRootPath);
            }
            return false;
        }

        return true;
    }

    inline bool readUtf8File(const QString& filePath, QString* outText, QString* errorMessage = nullptr)
    {
        WhatSon::Debug::trace(
            QStringLiteral("hierarchy.io.support"),
            QStringLiteral("readUtf8File.begin"),
            QStringLiteral("path=%1").arg(filePath));

        if (outText == nullptr)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("outText must not be null.");
            }
            return false;
        }

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to open file: %1").arg(filePath);
            }
            return false;
        }

        *outText = QString::fromUtf8(file.readAll());
        return true;
    }

    inline QStringList sanitizeStringList(QStringList values)
    {
        QStringList sanitized;
        sanitized.reserve(values.size());

        for (QString& value : values)
        {
            value = normalizePath(value.trimmed());
            if (value.isEmpty())
            {
                continue;
            }
            if (!sanitized.contains(value))
            {
                sanitized.push_back(value);
            }
        }

        return sanitized;
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

        QHash<QString, QSet<QString>> typeFormats;
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
        }

        QVector<ResourcesHierarchyItem> items;
        const QStringList orderedTypes = defaultTypeKeys();
        for (const QString& typeKey : orderedTypes)
        {
            const QSet<QString> formats = typeFormats.value(typeKey);
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
            typeItem.type = typeKey;
            typeItem.bucket = WhatSon::Resources::inferBucket(typeKey, QString());
            typeItem.showChevron = !sortedFormats.isEmpty();
            typeItem.expanded = previousExpansionStates.value(typeItem.key, false);
            items.push_back(typeItem);

            for (const QString& format : sortedFormats)
            {
                ResourcesHierarchyItem formatItem;
                formatItem.depth = 1;
                formatItem.label = format;
                formatItem.key = itemKeyForFormat(typeKey, format);
                formatItem.kind = QStringLiteral("format");
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
        for (const ResourcesHierarchyItem& item : items)
        {
            if (item.kind != QStringLiteral("asset"))
            {
                continue;
            }

            const QString resourcePath = normalizePath(item.resourcePath);
            if (resourcePath.isEmpty() || resourcePaths.contains(resourcePath))
            {
                continue;
            }
            resourcePaths.push_back(resourcePath);
        }
        return resourcePaths;
    }
} // namespace WhatSon::Hierarchy::ResourcesSupport
