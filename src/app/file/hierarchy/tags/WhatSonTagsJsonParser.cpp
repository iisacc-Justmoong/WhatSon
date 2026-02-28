#include "WhatSonTagsJsonParser.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

namespace
{
    bool looksLikeTagNode(const QJsonObject& object)
    {
        return object.contains(QStringLiteral("id"))
            || object.contains(QStringLiteral("key"))
            || object.contains(QStringLiteral("label"))
            || object.contains(QStringLiteral("name"))
            || object.contains(QStringLiteral("title"))
            || object.contains(QStringLiteral("text"))
            || object.contains(QStringLiteral("children"))
            || object.contains(QStringLiteral("items"))
            || object.contains(QStringLiteral("tags"));
    }
} // namespace

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
    if (parseError.error != QJsonParseError::NoError || document.isNull())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Invalid tags JSON: %1").arg(parseError.errorString());
        }
        return false;
    }

    if (document.isArray())
    {
        *outTags = document.array();
        return true;
    }

    if (!document.isObject())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Root JSON must be an object or array.");
        }
        return false;
    }

    const QJsonObject rootObject = document.object();
    const QJsonValue tagsValue = rootObject.value(QStringLiteral("tags"));
    if (tagsValue.isArray())
    {
        *outTags = tagsValue.toArray();
        return true;
    }

    const QJsonValue childrenValue = rootObject.value(QStringLiteral("children"));
    if (childrenValue.isArray())
    {
        *outTags = childrenValue.toArray();
        return true;
    }

    const QJsonValue itemsValue = rootObject.value(QStringLiteral("items"));
    if (itemsValue.isArray())
    {
        *outTags = itemsValue.toArray();
        return true;
    }

    if (looksLikeTagNode(rootObject))
    {
        *outTags = QJsonArray{QJsonValue(rootObject)};
        return true;
    }

    if (errorMessage != nullptr)
    {
        *errorMessage = QStringLiteral(
            "Root JSON must contain tags/children/items array or be a tag node.");
    }
    return false;
}
