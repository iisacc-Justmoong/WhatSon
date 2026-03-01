#include "WhatSonProjectsHierarchyCreator.hpp"

#include "WhatSonProjectsHierarchyStore.hpp"
#include "WhatSonDebugTrace.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

WhatSonProjectsHierarchyCreator::WhatSonProjectsHierarchyCreator() = default;

WhatSonProjectsHierarchyCreator::~WhatSonProjectsHierarchyCreator() = default;

QString WhatSonProjectsHierarchyCreator::targetRelativePath() const
{
    return QStringLiteral("Folders.wsfolders");
}

QString WhatSonProjectsHierarchyCreator::createText(const WhatSonProjectsHierarchyStore& store) const
{
    QJsonArray values;
    const QVector<WhatSonFolderDepthEntry> entries = store.folderEntries();
    for (const WhatSonFolderDepthEntry& entry : entries)
    {
        QJsonObject row;
        row.insert(QStringLiteral("id"), entry.id);
        row.insert(QStringLiteral("label"), entry.label);
        row.insert(QStringLiteral("depth"), entry.depth);
        values.push_back(row);
    }

    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("schema"), QStringLiteral("whatson.folders.tree"));
    root.insert(QStringLiteral("folders"), values);

    const QString text = QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.projects.creator"),
        QStringLiteral("createText"),
        QStringLiteral("count=%1 bytes=%2")
        .arg(entries.size())
        .arg(text.toUtf8().size()));
    return text;
}
