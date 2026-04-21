#include "WhatSonPresetHierarchyCreator.hpp"

#include "WhatSonPresetHierarchyStore.hpp"
#include "WhatSonDebugTrace.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

WhatSonPresetHierarchyCreator::WhatSonPresetHierarchyCreator() = default;

WhatSonPresetHierarchyCreator::~WhatSonPresetHierarchyCreator() = default;

QString WhatSonPresetHierarchyCreator::targetRelativePath() const
{
    return QStringLiteral("Preset.wspreset");
}

QString WhatSonPresetHierarchyCreator::createText(const WhatSonPresetHierarchyStore& store) const
{
    QJsonArray values;
    for (const QString& value : store.presetNames())
    {
        values.push_back(value);
    }

    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("schema"), QStringLiteral("whatson.preset.list"));
    root.insert(QStringLiteral("presets"), values);

    const QString text = QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.preset.creator"),
                              QStringLiteral("createText"),
                              QStringLiteral("count=%1 bytes=%2")
                              .arg(store.presetNames().size())
                              .arg(text.toUtf8().size()));
    return text;
}
