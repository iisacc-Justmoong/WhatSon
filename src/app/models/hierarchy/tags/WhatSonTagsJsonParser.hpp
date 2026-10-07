#pragma once

#include <QJsonArray>
#include <QString>

class WhatSonTagsJsonParser
{
public:
    WhatSonTagsJsonParser();
    ~WhatSonTagsJsonParser();

    bool parseRootTags(
        const QString& rawJson,
        QJsonArray* outTags,
        QString* errorMessage = nullptr) const;
};
