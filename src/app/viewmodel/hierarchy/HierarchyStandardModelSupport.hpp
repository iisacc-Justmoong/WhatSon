#pragma once

#include <QVariantList>
#include <QVariantMap>

#include <algorithm>

namespace WhatSon::Hierarchy
{
    inline int standardModelDepth(const QVariantMap& node, const QString& depthRole = QStringLiteral("depth"))
    {
        bool converted = false;
        const int depth = node.value(depthRole).toInt(&converted);
        if (!converted)
        {
            return 0;
        }
        return std::max(0, depth);
    }

    inline QVariantList buildStandardTreeModelLevel(
        const QVariantList& flatNodes,
        int* cursor,
        int expectedDepth,
        const QString& childrenRole = QStringLiteral("children"),
        const QString& depthRole = QStringLiteral("depth"))
    {
        QVariantList result;
        if (cursor == nullptr)
        {
            return result;
        }

        while (*cursor < flatNodes.size())
        {
            QVariantMap node = flatNodes.at(*cursor).toMap();
            if (node.isEmpty())
            {
                ++(*cursor);
                continue;
            }

            const int nodeDepth = standardModelDepth(node, depthRole);
            if (nodeDepth < expectedDepth)
            {
                break;
            }
            if (nodeDepth > expectedDepth)
            {
                break;
            }

            node.remove(childrenRole);
            ++(*cursor);

            const QVariantList childNodes =
                buildStandardTreeModelLevel(flatNodes, cursor, expectedDepth + 1, childrenRole, depthRole);
            if (!childNodes.isEmpty())
            {
                node.insert(childrenRole, childNodes);
            }

            result.push_back(node);
        }

        return result;
    }

    inline QVariantList buildStandardTreeModel(
        const QVariantList& flatNodes,
        const QString& childrenRole = QStringLiteral("children"),
        const QString& depthRole = QStringLiteral("depth"))
    {
        int cursor = 0;
        return buildStandardTreeModelLevel(flatNodes, &cursor, 0, childrenRole, depthRole);
    }
} // namespace WhatSon::Hierarchy
