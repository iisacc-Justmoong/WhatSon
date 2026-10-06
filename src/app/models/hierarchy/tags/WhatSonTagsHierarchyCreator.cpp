#include "app/models/hierarchy/tags/WhatSonTagsHierarchyCreator.hpp"

#include "app/models/hierarchy/tags/WhatSonTagsHierarchyStore.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>

namespace
{
    struct TagNode final
    {
        QString id;
        QString label;
        QVector<TagNode> children;
    };

    QJsonObject serializeNode(const TagNode& node)
    {
        QJsonObject object;
        object.insert(QStringLiteral("id"), node.id);
        object.insert(QStringLiteral("label"), node.label);

        if (!node.children.isEmpty())
        {
            QJsonArray childArray;
            for (const TagNode& child : node.children)
            {
                childArray.push_back(serializeNode(child));
            }
            object.insert(QStringLiteral("children"), childArray);
        }

        return object;
    }
} // namespace

WhatSonTagsHierarchyCreator::WhatSonTagsHierarchyCreator() = default;

WhatSonTagsHierarchyCreator::~WhatSonTagsHierarchyCreator() = default;

QString WhatSonTagsHierarchyCreator::targetRelativePath() const
{
    return QStringLiteral("Tags.wstags");
}

QString WhatSonTagsHierarchyCreator::createText(const WhatSonTagsHierarchyStore& store) const
{
    const QVector<WhatSonTagDepthEntry> entries = store.tagEntries();
    QVector<TagNode> roots;
    roots.reserve(entries.size());
    QVector<TagNode*> stack;
    stack.reserve(entries.size());

    for (const WhatSonTagDepthEntry& entry : entries)
    {
        const QString id = entry.id.trimmed();
        const QString label = entry.label.trimmed();
        if (id.isEmpty() || label.isEmpty())
        {
            continue;
        }

        int depth = std::max(0, entry.depth);
        if (depth > stack.size())
        {
            depth = stack.size();
        }
        while (stack.size() > depth)
        {
            stack.removeLast();
        }

        TagNode node;
        node.id = id;
        node.label = label;

        TagNode* insertedNode = nullptr;
        if (depth == 0)
        {
            roots.push_back(std::move(node));
            insertedNode = &roots.last();
        }
        else
        {
            TagNode* parent = stack.at(depth - 1);
            parent->children.push_back(std::move(node));
            insertedNode = &parent->children.last();
        }

        if (stack.size() <= depth)
        {
            stack.push_back(insertedNode);
        }
        else
        {
            stack[depth] = insertedNode;
            stack.resize(depth + 1);
        }
    }

    QJsonArray values;
    for (const TagNode& node : roots)
    {
        values.push_back(serializeNode(node));
    }

    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("schema"), QStringLiteral("whatson.tags.list"));
    root.insert(QStringLiteral("tags"), values);

    const QString text = QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.tags.creator"),
                              QStringLiteral("createText"),
                              QStringLiteral("count=%1 bytes=%2")
                              .arg(entries.size())
                              .arg(text.toUtf8().size()));
    return text;
}
