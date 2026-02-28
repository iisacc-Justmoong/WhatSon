#pragma once

#include "WhatSonTagDepthEntry.hpp"

#include <QJsonArray>
#include <QVector>

class WhatSonTagsDepthFlattener
{
public:
    WhatSonTagsDepthFlattener();
    ~WhatSonTagsDepthFlattener();

    QVector<WhatSonTagDepthEntry> flatten(const QJsonArray& rootTags) const;

private:
    void appendNodeDepth(
        const QJsonValue& nodeValue,
        int depth,
        QVector<WhatSonTagDepthEntry>* outEntries) const;
};
