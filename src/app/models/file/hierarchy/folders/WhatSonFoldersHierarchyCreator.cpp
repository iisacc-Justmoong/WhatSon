#include "WhatSonFoldersHierarchyCreator.hpp"

#include "../WhatSonFolderIdentity.hpp"
#include "models/file/WhatSonDebugTrace.hpp"
#include "WhatSonFoldersHierarchyStore.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>

namespace
{
    struct FolderNode final
    {
        QString id;
        QString label;
        QString uuid;
        QVector<FolderNode> children;
    };

    QJsonObject serializeNode(const FolderNode& node)
    {
        QJsonObject object;
        object.insert(QStringLiteral("id"), node.id);
        object.insert(QStringLiteral("label"), node.label);
        object.insert(QStringLiteral("uuid"), node.uuid);

        if (!node.children.isEmpty())
        {
            QJsonArray childArray;
            for (const FolderNode& child : node.children)
            {
                childArray.push_back(serializeNode(child));
            }
            object.insert(QStringLiteral("children"), childArray);
        }

        return object;
    }
} // namespace

WhatSonFoldersHierarchyCreator::WhatSonFoldersHierarchyCreator() = default;

WhatSonFoldersHierarchyCreator::~WhatSonFoldersHierarchyCreator() = default;

QString WhatSonFoldersHierarchyCreator::targetRelativePath() const
{
    return QStringLiteral("Folders.wsfolders");
}

QString WhatSonFoldersHierarchyCreator::createText(const WhatSonFoldersHierarchyStore& store) const
{
    const QVector<WhatSonFolderDepthEntry> entries = store.folderEntries();
    QVector<FolderNode> roots;
    roots.reserve(entries.size());
    QVector<FolderNode*> stack;
    stack.reserve(entries.size());

    for (const WhatSonFolderDepthEntry& entry : entries)
    {
        const QString id = entry.id.trimmed();
        const QString label = entry.label.trimmed();
        const QString uuid = WhatSon::FolderIdentity::normalizeFolderUuid(entry.uuid);
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

        FolderNode node;
        node.id = id;
        node.label = label;
        node.uuid = uuid.isEmpty() ? WhatSon::FolderIdentity::createFolderUuid() : uuid;

        FolderNode* insertedNode = nullptr;
        if (depth == 0)
        {
            roots.push_back(std::move(node));
            insertedNode = &roots.last();
        }
        else
        {
            FolderNode* parent = stack.at(depth - 1);
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
    for (const FolderNode& node : roots)
    {
        values.push_back(serializeNode(node));
    }

    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("schema"), QStringLiteral("whatson.folders.tree"));
    root.insert(QStringLiteral("folders"), values);

    const QString text = QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.folders.creator"),
                              QStringLiteral("createText"),
                              QStringLiteral("count=%1 bytes=%2")
                              .arg(entries.size())
                              .arg(text.toUtf8().size()));
    return text;
}
