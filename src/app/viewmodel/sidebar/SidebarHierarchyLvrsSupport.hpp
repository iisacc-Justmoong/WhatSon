#pragma once

#include <QHash>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

#include <algorithm>

namespace WhatSon::Sidebar::Lvrs
{
    struct FlatNode final
    {
        QString key;
        QString id;
        QString label;
        int depth = 0;
        bool accent = false;
        bool expanded = false;
        bool showChevron = false;
        bool enabled = true;
        bool dragLocked = false;
        int sourceIndex = -1;
    };

    inline QString normalizedStringValue(const QVariant& value)
    {
        return value.toString().trimmed();
    }

    inline bool normalizedBoolValue(const QVariant& value, bool fallbackValue = false)
    {
        if (!value.isValid() || value.isNull())
        {
            return fallbackValue;
        }
        return value.toBool();
    }

    inline int normalizedDepthValue(const QVariant& value, int fallbackDepth = 0)
    {
        bool converted = false;
        const int parsedDepth = value.toInt(&converted);
        if (converted)
        {
            return std::max(0, parsedDepth);
        }
        return std::max(0, fallbackDepth);
    }

    inline QString normalizedKeySegment(const QString& label, int sourceIndex)
    {
        QString segment = label.trimmed();
        if (segment.isEmpty())
        {
            segment = QStringLiteral("item");
        }

        segment.replace(QLatin1Char('/'), QLatin1Char('_'));
        segment.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral("_"));
        return QStringLiteral("%1@%2").arg(segment, QString::number(std::max(0, sourceIndex)));
    }

    inline QString fallbackKeyForNode(const QString& parentKey, const QString& label, int sourceIndex)
    {
        const QString segment = normalizedKeySegment(label, sourceIndex);
        if (parentKey.trimmed().isEmpty())
        {
            return segment;
        }
        return parentKey + QLatin1Char('/') + segment;
    }

    inline QVariantList childNodesForMap(const QVariantMap& map)
    {
        const QVariant childrenVariant = map.value(QStringLiteral("children"));
        if (childrenVariant.canConvert<QVariantList>())
        {
            return childrenVariant.toList();
        }

        const QVariant itemsVariant = map.value(QStringLiteral("items"));
        if (itemsVariant.canConvert<QVariantList>())
        {
            return itemsVariant.toList();
        }

        const QVariant nodesVariant = map.value(QStringLiteral("nodes"));
        if (nodesVariant.canConvert<QVariantList>())
        {
            return nodesVariant.toList();
        }

        return {};
    }

    inline void flattenHierarchyNodesRecursive(
        const QVariantList& nodes,
        int fallbackDepth,
        const QString& parentKey,
        int* visitIndex,
        QVector<FlatNode>* sink)
    {
        if (visitIndex == nullptr || sink == nullptr)
        {
            return;
        }

        for (const QVariant& nodeValue : nodes)
        {
            QVariantMap map = nodeValue.toMap();
            if (map.isEmpty())
            {
                const QString primitiveLabel = normalizedStringValue(nodeValue);
                if (primitiveLabel.isEmpty())
                {
                    ++(*visitIndex);
                    continue;
                }
                map.insert(QStringLiteral("label"), primitiveLabel);
            }

            const int sourceIndex = map.value(QStringLiteral("sourceIndex"), *visitIndex).toInt();
            const QString label = normalizedStringValue(map.value(QStringLiteral("label")));
            const QString explicitKey = normalizedStringValue(map.value(QStringLiteral("key")));
            const QString id = normalizedStringValue(map.value(QStringLiteral("id")));

            FlatNode node;
            node.sourceIndex = sourceIndex;
            node.label = label;
            node.id = id;
            node.depth = normalizedDepthValue(map.value(QStringLiteral("depth")), fallbackDepth);
            node.accent = normalizedBoolValue(map.value(QStringLiteral("accent")), false);
            node.expanded = normalizedBoolValue(map.value(QStringLiteral("expanded")), false);
            node.showChevron = normalizedBoolValue(map.value(QStringLiteral("showChevron")), false);
            node.enabled = normalizedBoolValue(map.value(QStringLiteral("enabled")), true);
            node.dragLocked = normalizedBoolValue(map.value(QStringLiteral("dragLocked")), false);
            node.key = !explicitKey.isEmpty()
                           ? explicitKey
                           : (!id.isEmpty() ? id : fallbackKeyForNode(parentKey, label, sourceIndex));
            sink->push_back(node);

            ++(*visitIndex);

            const QVariantList childNodes = childNodesForMap(map);
            if (!childNodes.isEmpty())
            {
                flattenHierarchyNodesRecursive(childNodes, node.depth + 1, node.key, visitIndex, sink);
            }
        }
    }

    inline QVector<FlatNode> flattenHierarchyNodes(const QVariantList& nodes)
    {
        QVector<FlatNode> flattened;
        flattened.reserve(nodes.size());
        int visitIndex = 0;
        flattenHierarchyNodesRecursive(nodes, 0, QString(), &visitIndex, &flattened);
        return flattened;
    }

    inline QVector<FlatNode> filterFlatNodes(const QVector<FlatNode>& nodes, const QString& rawQuery)
    {
        const QString query = rawQuery.trimmed().toLower();
        if (query.isEmpty())
        {
            return nodes;
        }

        QVector<bool> keep(nodes.size(), false);
        QVector<int> ancestorIndexByDepth;
        ancestorIndexByDepth.reserve(nodes.size());

        for (int index = 0; index < nodes.size(); ++index)
        {
            const FlatNode& node = nodes.at(index);
            while (ancestorIndexByDepth.size() > node.depth)
            {
                ancestorIndexByDepth.removeLast();
            }
            if (ancestorIndexByDepth.size() == node.depth)
            {
                ancestorIndexByDepth.push_back(index);
            }
            else if (node.depth < ancestorIndexByDepth.size())
            {
                ancestorIndexByDepth[node.depth] = index;
            }

            const QString label = node.label.toLower();
            if (!label.contains(query))
            {
                continue;
            }

            keep[index] = true;
            for (int depthIndex = 0; depthIndex < node.depth && depthIndex < ancestorIndexByDepth.size(); ++depthIndex)
            {
                const int ancestorIndex = ancestorIndexByDepth.at(depthIndex);
                if (ancestorIndex >= 0 && ancestorIndex < keep.size())
                {
                    keep[ancestorIndex] = true;
                }
            }
        }

        QVector<FlatNode> filtered;
        filtered.reserve(nodes.size());
        for (int index = 0; index < nodes.size(); ++index)
        {
            if (keep.at(index))
            {
                filtered.push_back(nodes.at(index));
            }
        }
        return filtered;
    }

    inline QVariantMap flatNodeToVariantMap(const FlatNode& node)
    {
        QVariantMap map;
        map.insert(QStringLiteral("key"), node.key);
        map.insert(QStringLiteral("label"), node.label);
        map.insert(QStringLiteral("depth"), node.depth);
        map.insert(QStringLiteral("accent"), node.accent);
        map.insert(QStringLiteral("expanded"), node.expanded);
        map.insert(QStringLiteral("showChevron"), node.showChevron);
        map.insert(QStringLiteral("enabled"), node.enabled);
        map.insert(QStringLiteral("dragLocked"), node.dragLocked);
        map.insert(QStringLiteral("sourceIndex"), node.sourceIndex);
        map.insert(QStringLiteral("itemId"), node.sourceIndex);
        if (!node.id.isEmpty())
        {
            map.insert(QStringLiteral("id"), node.id);
        }
        return map;
    }

    inline QVariantList flatNodesToVariantList(const QVector<FlatNode>& nodes)
    {
        QVariantList list;
        list.reserve(nodes.size());
        for (const FlatNode& node : nodes)
        {
            list.push_back(flatNodeToVariantMap(node));
        }
        return list;
    }
} // namespace WhatSon::Sidebar::Lvrs
