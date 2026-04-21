#include "WhatSonLibraryHierarchyCreator.hpp"

#include "WhatSonLibraryHierarchyStore.hpp"
#include "models/file/WhatSonDebugTrace.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

WhatSonLibraryHierarchyCreator::WhatSonLibraryHierarchyCreator() = default;

WhatSonLibraryHierarchyCreator::~WhatSonLibraryHierarchyCreator() = default;

QString WhatSonLibraryHierarchyCreator::targetRelativePath() const
{
    return QStringLiteral("Library.wslibrary/index.wsnindex");
}

QString WhatSonLibraryHierarchyCreator::createText(const WhatSonLibraryHierarchyStore& store) const
{
    QJsonArray values;
    for (const QString& value : store.noteIds())
    {
        values.push_back(value);
    }

    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("schema"), QStringLiteral("whatson.library.index"));
    root.insert(QStringLiteral("notes"), values);

    const QString text = QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.library.creator"),
                              QStringLiteral("createText"),
                              QStringLiteral("count=%1 bytes=%2")
                              .arg(store.noteIds().size())
                              .arg(text.toUtf8().size()));
    return text;
}
