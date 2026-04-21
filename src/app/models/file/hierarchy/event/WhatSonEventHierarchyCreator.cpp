#include "WhatSonEventHierarchyCreator.hpp"

#include "WhatSonEventHierarchyStore.hpp"
#include "models/file/WhatSonDebugTrace.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

WhatSonEventHierarchyCreator::WhatSonEventHierarchyCreator() = default;

WhatSonEventHierarchyCreator::~WhatSonEventHierarchyCreator() = default;

QString WhatSonEventHierarchyCreator::targetRelativePath() const
{
    return QStringLiteral("Event.wsevent");
}

QString WhatSonEventHierarchyCreator::createText(const WhatSonEventHierarchyStore& store) const
{
    QJsonArray values;
    for (const QString& value : store.eventNames())
    {
        values.push_back(value);
    }

    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("schema"), QStringLiteral("whatson.event.list"));
    root.insert(QStringLiteral("events"), values);

    const QString text = QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.event.creator"),
                              QStringLiteral("createText"),
                              QStringLiteral("count=%1 bytes=%2")
                              .arg(store.eventNames().size())
                              .arg(text.toUtf8().size()));
    return text;
}
