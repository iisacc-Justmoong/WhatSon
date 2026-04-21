#include "WhatSonProgressHierarchyCreator.hpp"

#include "models/file/WhatSonDebugTrace.hpp"
#include "WhatSonProgressHierarchyStore.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

WhatSonProgressHierarchyCreator::WhatSonProgressHierarchyCreator() = default;

WhatSonProgressHierarchyCreator::~WhatSonProgressHierarchyCreator() = default;

QString WhatSonProgressHierarchyCreator::targetRelativePath() const
{
    return QStringLiteral("Progress.wsprogress");
}

QString WhatSonProgressHierarchyCreator::createText(const WhatSonProgressHierarchyStore& store) const
{
    QJsonArray states;
    for (const QString& state : store.progressStates())
    {
        states.push_back(state);
    }

    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("schema"), QStringLiteral("whatson.progress.state"));
    root.insert(QStringLiteral("progress"), store.progressValue());
    root.insert(QStringLiteral("states"), states);

    const QString text = QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.progress.creator"),
                              QStringLiteral("createText"),
                              QStringLiteral("value=%1 states=%2 bytes=%3")
                              .arg(store.progressValue())
                              .arg(store.progressStates().size())
                              .arg(text.toUtf8().size()));
    return text;
}
