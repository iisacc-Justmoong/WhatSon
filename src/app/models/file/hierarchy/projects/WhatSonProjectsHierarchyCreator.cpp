#include "WhatSonProjectsHierarchyCreator.hpp"

#include "WhatSonProjectsHierarchyStore.hpp"
#include "models/file/WhatSonDebugTrace.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>

namespace
{
    struct ProjectNode final
    {
        QString id;
        QString label;
        QVector<ProjectNode> children;
    };

    QJsonObject serializeNode(const ProjectNode& node)
    {
        QJsonObject object;
        object.insert(QStringLiteral("id"), node.id);
        object.insert(QStringLiteral("label"), node.label);

        if (!node.children.isEmpty())
        {
            QJsonArray childArray;
            for (const ProjectNode& child : node.children)
            {
                childArray.push_back(serializeNode(child));
            }
            object.insert(QStringLiteral("children"), childArray);
        }

        return object;
    }
} // namespace

WhatSonProjectsHierarchyCreator::WhatSonProjectsHierarchyCreator() = default;

WhatSonProjectsHierarchyCreator::~WhatSonProjectsHierarchyCreator() = default;

QString WhatSonProjectsHierarchyCreator::targetRelativePath() const
{
    return QStringLiteral("ProjectLists.wsproj");
}

QString WhatSonProjectsHierarchyCreator::createText(const WhatSonProjectsHierarchyStore& store) const
{
    const QVector<WhatSonFolderDepthEntry> entries = store.folderEntries();
    QVector<ProjectNode> roots;
    roots.reserve(entries.size());
    QVector<ProjectNode*> stack;
    stack.reserve(entries.size());

    for (const WhatSonFolderDepthEntry& entry : entries)
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

        ProjectNode node;
        node.id = id;
        node.label = label;

        ProjectNode* insertedNode = nullptr;
        if (depth == 0)
        {
            roots.push_back(std::move(node));
            insertedNode = &roots.last();
        }
        else
        {
            ProjectNode* parent = stack.at(depth - 1);
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
    for (const ProjectNode& node : roots)
    {
        values.push_back(serializeNode(node));
    }

    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("schema"), QStringLiteral("whatson.projects.list"));
    root.insert(QStringLiteral("projects"), values);

    const QString text = QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.projects.creator"),
                              QStringLiteral("createText"),
                              QStringLiteral("count=%1 bytes=%2")
                              .arg(entries.size())
                              .arg(text.toUtf8().size()));
    return text;
}
