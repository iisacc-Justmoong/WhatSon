#include "WhatSonResourcesHierarchyCreator.hpp"

#include "WhatSonResourcesHierarchyStore.hpp"
#include "WhatSonDebugTrace.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

WhatSonResourcesHierarchyCreator::WhatSonResourcesHierarchyCreator() = default;

WhatSonResourcesHierarchyCreator::~WhatSonResourcesHierarchyCreator() = default;

QString WhatSonResourcesHierarchyCreator::targetRelativePath() const
{
    return QStringLiteral("Resources.wsresources");
}

QString WhatSonResourcesHierarchyCreator::createText(const WhatSonResourcesHierarchyStore& store) const
{
    QJsonArray values;
    for (const QString& value : store.resourcePaths())
    {
        QJsonObject entry;
        entry.insert(QStringLiteral("resourcePath"), value);
        values.push_back(entry);
    }

    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("schema"), QStringLiteral("whatson.resources.list"));
    root.insert(QStringLiteral("resources"), values);

    const QString text = QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.resources.creator"),
                              QStringLiteral("createText"),
                              QStringLiteral("count=%1 bytes=%2")
                              .arg(store.resourcePaths().size())
                              .arg(text.toUtf8().size()));
    return text;
}
