#pragma once

#include "FlatHierarchyModel.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QVariantList>
#include <QVariantMap>

#include <algorithm>

namespace WhatSon::Hierarchy::Support
{
    inline QString normalizePath(const QString& input)
    {
        const QString trimmed = input.trimmed();
        if (trimmed.isEmpty())
        {
            return {};
        }
        return QDir::cleanPath(trimmed);
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
            outContentsDirectories->push_back(QDir::cleanPath(fixedInternalPath));
        }

        const QStringList dynamicContentsDirectories = hubDir.entryList(
            QStringList{QStringLiteral("*.wscontents")},
            QDir::Dirs | QDir::NoDotAndDotDot,
            QDir::Name);
        for (const QString& directoryName : dynamicContentsDirectories)
        {
            outContentsDirectories->push_back(QDir::cleanPath(hubDir.filePath(directoryName)));
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
            value = value.trimmed();
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

    inline FlatHierarchyItem parseFlatItem(const QVariant& entry, int fallbackOrdinal, const QString& fallbackPrefix)
    {
        FlatHierarchyItem item;

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
            item.label = QStringLiteral("%1%2").arg(fallbackPrefix).arg(fallbackOrdinal);
        }

        return item;
    }

    inline QVector<FlatHierarchyItem> parseDepthItems(
        const QVariantList& depthItems,
        const QString& fallbackPrefix)
    {
        QVector<FlatHierarchyItem> items;
        items.reserve(depthItems.size());

        int ordinal = 1;
        for (const QVariant& entry : depthItems)
        {
            items.push_back(parseFlatItem(entry, ordinal, fallbackPrefix));
            ++ordinal;
        }

        return items;
    }

    inline QVariantList serializeDepthItems(const QVector<FlatHierarchyItem>& items)
    {
        QVariantList serialized;
        serialized.reserve(items.size());

        for (const FlatHierarchyItem& item : items)
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

    inline QVector<FlatHierarchyItem> buildBucketItems(
        const QString& bucketName,
        const QStringList& values,
        const QString& fallbackPrefix)
    {
        const QStringList sanitized = sanitizeStringList(values);

        QVector<FlatHierarchyItem> items;
        items.reserve(sanitized.size() + 1);

        FlatHierarchyItem bucket;
        bucket.depth = 0;
        bucket.accent = true;
        bucket.expanded = true;
        bucket.label = QStringLiteral("%1 (%2)").arg(bucketName).arg(sanitized.size());
        bucket.showChevron = !sanitized.isEmpty();
        items.push_back(std::move(bucket));

        int ordinal = 1;
        for (const QString& value : sanitized)
        {
            FlatHierarchyItem child;
            child.depth = 1;
            child.accent = false;
            child.expanded = false;
            child.label = value.trimmed().isEmpty() ? QStringLiteral("%1%2").arg(fallbackPrefix).arg(ordinal) : value;
            child.showChevron = false;
            items.push_back(std::move(child));
            ++ordinal;
        }

        return items;
    }
} // namespace WhatSon::Hierarchy::Support
