#include "WhatSonTagsDepthFlattener.hpp"

#include "WhatSonDebugTrace.hpp"

#include <QJsonObject>

namespace
{
    int parseDepth(const QJsonObject& object, int fallbackDepth)
    {
        auto parseField = [&object](const QString& key, int* outValue) -> bool
        {
            const QJsonValue value = object.value(key);
            if (!value.isDouble())
            {
                return false;
            }
            *outValue = static_cast<int>(value.toDouble());
            return true;
        };

        int depth = fallbackDepth;
        if (parseField(QStringLiteral("depth"), &depth)
            || parseField(QStringLiteral("dpeth"), &depth)
            || parseField(QStringLiteral("indentLevel"), &depth))
        {
            return depth < 0 ? 0 : depth;
        }

        return fallbackDepth < 0 ? 0 : fallbackDepth;
    }

    QString parseLabel(const QJsonObject& object)
    {
        auto parseField = [&object](const QString& key) -> QString
        {
            return object.value(key).toString().trimmed();
        };

        const QString label = parseField(QStringLiteral("label"));
        if (!label.isEmpty())
        {
            return label;
        }

        const QString name = parseField(QStringLiteral("name"));
        if (!name.isEmpty())
        {
            return name;
        }

        const QString title = parseField(QStringLiteral("title"));
        if (!title.isEmpty())
        {
            return title;
        }

        const QString text = parseField(QStringLiteral("text"));
        if (!text.isEmpty())
        {
            return text;
        }

        return {};
    }

    QString parseId(const QJsonObject& object)
    {
        const QString id = object.value(QStringLiteral("id")).toString().trimmed();
        if (!id.isEmpty())
        {
            return id;
        }
        return object.value(QStringLiteral("key")).toString().trimmed();
    }

    QJsonArray parseChildren(const QJsonObject& object)
    {
        const QJsonValue childrenValue = object.value(QStringLiteral("children"));
        if (childrenValue.isArray())
        {
            return childrenValue.toArray();
        }

        const QJsonValue tagsValue = object.value(QStringLiteral("tags"));
        if (tagsValue.isArray())
        {
            return tagsValue.toArray();
        }

        const QJsonValue itemsValue = object.value(QStringLiteral("items"));
        if (itemsValue.isArray())
        {
            return itemsValue.toArray();
        }

        return {};
    }
} // namespace

WhatSonTagsDepthFlattener::WhatSonTagsDepthFlattener() = default;

WhatSonTagsDepthFlattener::~WhatSonTagsDepthFlattener() = default;

QVector<WhatSonTagDepthEntry> WhatSonTagsDepthFlattener::flatten(const QJsonArray& rootTags) const
{
    WhatSon::Debug::trace(
        QStringLiteral("hub.tags.flattener"),
        QStringLiteral("flatten.begin"),
        QStringLiteral("rootCount=%1").arg(rootTags.size()));
    QVector<WhatSonTagDepthEntry> entries;
    entries.reserve(rootTags.size());

    for (const QJsonValue& nodeValue : rootTags)
    {
        appendNodeDepth(nodeValue, 0, &entries);
    }
    WhatSon::Debug::trace(
        QStringLiteral("hub.tags.flattener"),
        QStringLiteral("flatten.success"),
        QStringLiteral("entryCount=%1").arg(entries.size()));
    return entries;
}

void WhatSonTagsDepthFlattener::appendNodeDepth(
    const QJsonValue& nodeValue,
    const int depth,
    QVector<WhatSonTagDepthEntry>* outEntries) const
{
    if (outEntries == nullptr || !nodeValue.isObject())
    {
        WhatSon::Debug::trace(
            QStringLiteral("hub.tags.flattener"),
            QStringLiteral("append.skip"),
            QStringLiteral("reason=%1").arg(outEntries == nullptr
                                                ? QStringLiteral("outEntries-null")
                                                : QStringLiteral("node-not-object")));
        return;
    }

    const QJsonObject nodeObject = nodeValue.toObject();
    const int effectiveDepth = parseDepth(nodeObject, depth);

    WhatSonTagDepthEntry entry;
    entry.id = parseId(nodeObject);
    entry.label = parseLabel(nodeObject);
    entry.depth = effectiveDepth;
    if (entry.label.isEmpty())
    {
        entry.label = entry.id;
    }
    if (!entry.id.isEmpty() || !entry.label.isEmpty())
    {
        WhatSon::Debug::trace(
            QStringLiteral("hub.tags.flattener"),
            QStringLiteral("append.entry"),
            QStringLiteral("id=%1 label=%2 depth=%3").arg(entry.id, entry.label).arg(entry.depth));
        outEntries->push_back(std::move(entry));
    }

    const QJsonArray childrenArray = parseChildren(nodeObject);
    for (const QJsonValue& childValue : childrenArray)
    {
        appendNodeDepth(childValue, effectiveDepth + 1, outEntries);
    }
}
