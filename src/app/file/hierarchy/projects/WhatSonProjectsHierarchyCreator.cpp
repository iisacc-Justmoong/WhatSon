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
    for (const QString& value : store.projectNames())
    {
        values.push_back(value);
    }

    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("schema"), QStringLiteral("whatson.projects.list"));
    root.insert(QStringLiteral("projects"), values);

    const QString text = QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.projects.creator"),
        QStringLiteral("createText"),
        QStringLiteral("count=%1 bytes=%2")
        .arg(store.projectNames().size())
        .arg(text.toUtf8().size()));
    return text;
}
