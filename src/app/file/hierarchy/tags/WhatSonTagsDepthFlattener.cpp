#include "WhatSonTagsDepthFlattener.hpp"

#include <QJsonObject>

WhatSonTagsDepthFlattener::WhatSonTagsDepthFlattener() = default;

WhatSonTagsDepthFlattener::~WhatSonTagsDepthFlattener() = default;

QVector<WhatSonTagDepthEntry> WhatSonTagsDepthFlattener::flatten(const QJsonArray& rootTags) const
{
    QVector<WhatSonTagDepthEntry> entries;
    entries.reserve(rootTags.size());

    for (const QJsonValue& nodeValue : rootTags)
    {
        appendNodeDepth(nodeValue, 0, &entries);
    }
    return entries;
}

void WhatSonTagsDepthFlattener::appendNodeDepth(
    const QJsonValue& nodeValue,
    const int depth,
    QVector<WhatSonTagDepthEntry>* outEntries) const
{
    if (outEntries == nullptr || !nodeValue.isObject())
    {
        return;
    }

    const QJsonObject nodeObject = nodeValue.toObject();

    WhatSonTagDepthEntry entry;
    entry.id = nodeObject.value(QStringLiteral("id")).toString();
    entry.label = nodeObject.value(QStringLiteral("label")).toString();
    entry.depth = depth;
    outEntries->push_back(std::move(entry));

    const QJsonValue childrenValue = nodeObject.value(QStringLiteral("children"));
    if (!childrenValue.isArray())
    {
        return;
    }

    const QJsonArray childrenArray = childrenValue.toArray();
    for (const QJsonValue& childValue : childrenArray)
    {
        appendNodeDepth(childValue, depth + 1, outEntries);
    }
}
