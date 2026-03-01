#include "WhatSonBookmarksHierarchyCreator.hpp"

#include "WhatSonBookmarksHierarchyStore.hpp"
#include "WhatSonDebugTrace.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

WhatSonBookmarksHierarchyCreator::WhatSonBookmarksHierarchyCreator() = default;

WhatSonBookmarksHierarchyCreator::~WhatSonBookmarksHierarchyCreator() = default;

QString WhatSonBookmarksHierarchyCreator::targetRelativePath() const
{
    return QStringLiteral("Bookmarks.wsbookmarks");
}

QString WhatSonBookmarksHierarchyCreator::createText(const WhatSonBookmarksHierarchyStore& store) const
{
    QJsonArray values;
    for (const QString& value : store.bookmarkIds())
    {
        values.push_back(value);
    }

    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("schema"), QStringLiteral("whatson.bookmarks.list"));
    root.insert(QStringLiteral("bookmarks"), values);

    const QString text = QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.bookmarks.creator"),
                              QStringLiteral("createText"),
                              QStringLiteral("count=%1 bytes=%2")
                              .arg(store.bookmarkIds().size())
                              .arg(text.toUtf8().size()));
    return text;
}
