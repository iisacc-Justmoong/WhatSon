#include "WhatSonTagsHierarchyCreator.hpp"

#include "WhatSonDebugTrace.hpp"
#include "WhatSonTagsHierarchyStore.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

WhatSonTagsHierarchyCreator::WhatSonTagsHierarchyCreator() = default;

WhatSonTagsHierarchyCreator::~WhatSonTagsHierarchyCreator() = default;

QString WhatSonTagsHierarchyCreator::targetRelativePath() const
{
    return QStringLiteral("Tags.wstags");
}

QString WhatSonTagsHierarchyCreator::createText(const WhatSonTagsHierarchyStore& store) const
{
    QJsonArray entries;
    for (const WhatSonTagDepthEntry& entry : store.tagEntries())
    {
        QJsonObject row;
        row.insert(QStringLiteral("id"), entry.id);
        row.insert(QStringLiteral("label"), entry.label);
        row.insert(QStringLiteral("depth"), entry.depth);
        entries.push_back(row);
    }

    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("schema"), QStringLiteral("whatson.tags.flat-v1"));
    root.insert(QStringLiteral("entries"), entries);

    const QString text = QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.tags.creator"),
        QStringLiteral("createText"),
        QStringLiteral("count=%1 bytes=%2")
        .arg(store.tagEntries().size())
        .arg(text.toUtf8().size()));
    return text;
}
