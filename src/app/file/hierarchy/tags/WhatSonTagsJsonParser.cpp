#include "WhatSonTagsJsonParser.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

WhatSonTagsJsonParser::WhatSonTagsJsonParser() = default;

WhatSonTagsJsonParser::~WhatSonTagsJsonParser() = default;

bool WhatSonTagsJsonParser::parseRootTags(
    const QString& rawJson,
    QJsonArray* outTags,
    QString* errorMessage) const
{
    if (outTags == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("outTags must not be null.");
        }
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document =
        QJsonDocument::fromJson(rawJson.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Invalid tags JSON: %1").arg(parseError.errorString());
        }
        return false;
    }

    const QJsonObject rootObject = document.object();
    if (!rootObject.contains(QStringLiteral("tags")) || !rootObject.value(QStringLiteral("tags")).isArray())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Root JSON must contain an array field named 'tags'.");
        }
        return false;
    }

    *outTags = rootObject.value(QStringLiteral("tags")).toArray();
    return true;
}
