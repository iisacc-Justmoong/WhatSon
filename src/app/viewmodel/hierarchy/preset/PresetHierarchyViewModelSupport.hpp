#pragma once

#include "PresetHierarchyModel.hpp"

#include "file/hub/WhatSonHubPathUtils.hpp"
#include "file/WhatSonDebugTrace.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QVariantList>
#include <QVariantMap>

#include <algorithm>

namespace WhatSon::Hierarchy::PresetSupport
{
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
            WhatSon::Debug::trace(
                QStringLiteral("hierarchy.io.support"),
                QStringLiteral("readUtf8File.failed"),
                QStringLiteral("path=%1 reason=outText is null").arg(filePath));
            return false;
        }

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to open file: %1").arg(filePath);
            }
            WhatSon::Debug::trace(
                QStringLiteral("hierarchy.io.support"),
                QStringLiteral("readUtf8File.failed"),
                QStringLiteral("path=%1 reason=open failed").arg(filePath));
            return false;
        }

        const QByteArray bytes = file.readAll();
        *outText = QString::fromUtf8(bytes);
        WhatSon::Debug::trace(
            QStringLiteral("hierarchy.io.support"),
            QStringLiteral("readUtf8File.success"),
            QStringLiteral("path=%1 bytes=%2").arg(filePath).arg(bytes.size()));
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

    inline PresetHierarchyItem parseItemEntry(const QVariant& entry, int fallbackOrdinal, const QString& fallbackPrefix)
    {
        PresetHierarchyItem item;

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

    inline QVector<PresetHierarchyItem> parseDepthItems(
        const QVariantList& depthItems,
        const QString& fallbackPrefix)
    {
        QVector<PresetHierarchyItem> items;
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

    inline QVariantList serializeDepthItems(const QVector<PresetHierarchyItem>& items)
    {
        QVariantList serialized;
        serialized.reserve(items.size());

        for (const PresetHierarchyItem& item : items)
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

    inline QVector<PresetHierarchyItem> buildBucketItems(
        const QString& bucketName,
        const QStringList& values,
        const QString& fallbackPrefix)
    {
        const QStringList sanitized = sanitizeStringList(values);

        QVector<PresetHierarchyItem> items;
        items.reserve(sanitized.size());

        Q_UNUSED(bucketName);
        Q_UNUSED(fallbackPrefix);
        for (const QString& value : sanitized)
        {
            PresetHierarchyItem child;
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

    inline void applyChevronByDepth(QVector<PresetHierarchyItem>* items)
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

    inline int nextGeneratedFolderSequence(const QVector<PresetHierarchyItem>& items)
    {
        static const QRegularExpression folderPattern(QStringLiteral("^Folder(\\d+)$"));

        int maxSequence = 0;
        for (const PresetHierarchyItem& item : items)
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

    inline bool renameHierarchyItem(QVector<PresetHierarchyItem>* items, int index, const QString& displayName)
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

    inline bool isBucketHeaderItem(const PresetHierarchyItem& item)
    {
        return item.depth == 0 && item.accent;
    }

    inline int createHierarchyFolder(QVector<PresetHierarchyItem>* items, int selectedIndex, int* ioFolderSequence)
    {
        if (items == nullptr || ioFolderSequence == nullptr)
        {
            return -1;
        }

        int insertIndex = items->size();
        int folderDepth = 0;

        if (selectedIndex >= 0 && selectedIndex < items->size())
        {
            (*items)[selectedIndex].expanded = true;
            const int selectedDepth = items->at(selectedIndex).depth;
            folderDepth = selectedDepth + 1;

            insertIndex = selectedIndex + 1;
            while (insertIndex < items->size() && items->at(insertIndex).depth > selectedDepth)
            {
                ++insertIndex;
            }
        }

        PresetHierarchyItem newItem;
        newItem.depth = folderDepth;
        newItem.label = QStringLiteral("Untitled");
        ++(*ioFolderSequence);
        newItem.accent = false;
        newItem.expanded = false;
        newItem.showChevron = true;

        items->insert(insertIndex, std::move(newItem));
        applyChevronByDepth(items);
        return insertIndex;
    }

    inline int deleteHierarchySubtree(QVector<PresetHierarchyItem>* items, int selectedIndex)
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

    inline QStringList extractDomainLabelsFromItems(const QVector<PresetHierarchyItem>& items)
    {
        QStringList labels;
        labels.reserve(items.size());

        for (int index = 0; index < items.size(); ++index)
        {
            const PresetHierarchyItem& item = items.at(index);
            if (index == 0 && item.depth == 0 && item.accent)
            {
                continue;
            }

            const QString label = item.label.trimmed();
            if (label.isEmpty() || labels.contains(label))
            {
                continue;
            }
            labels.push_back(label);
        }

        return labels;
    }
} // namespace WhatSon::Hierarchy::PresetSupport
