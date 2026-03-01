#include "WhatSonTagsHierarchyParser.hpp"

#include "WhatSonDebugTrace.hpp"
#include "WhatSonTagsDepthFlattener.hpp"
#include "WhatSonTagsHierarchyStore.hpp"
#include "WhatSonTagsJsonParser.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

namespace
{
    QVector<WhatSonTagDepthEntry> parseTagEntries(const QJsonArray& array)
    {
        QVector<WhatSonTagDepthEntry> entries;
        entries.reserve(array.size());

        for (const QJsonValue& value : array)
        {
            if (!value.isObject())
            {
                continue;
            }

            const QJsonObject object = value.toObject();
            WhatSonTagDepthEntry entry;
            entry.id = object.value(QStringLiteral("id")).toString().trimmed();
            entry.label = object.value(QStringLiteral("label")).toString().trimmed();
            entry.depth = static_cast<int>(object.value(QStringLiteral("depth")).toDouble(0));

            if (entry.label.isEmpty())
            {
                entry.label = entry.id;
            }
            if (!entry.id.isEmpty() || !entry.label.isEmpty())
            {
                entries.push_back(std::move(entry));
            }
        }

        return entries;
    }
} // namespace

WhatSonTagsHierarchyParser::WhatSonTagsHierarchyParser() = default;

WhatSonTagsHierarchyParser::~WhatSonTagsHierarchyParser() = default;

bool WhatSonTagsHierarchyParser::parse(
    const QString& rawText,
    WhatSonTagsHierarchyStore* outStore,
    QString* errorMessage) const
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.tags.parser"),
                              QStringLiteral("parse.begin"),
                              QStringLiteral("bytes=%1").arg(rawText.toUtf8().size()));

    if (outStore == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("outStore must not be null.");
        }
        return false;
    }

    outStore->clear();

    if (rawText.trimmed().isEmpty())
    {
        outStore->setTagEntries({});
        return true;
    }

    WhatSonTagsJsonParser parser;
    QJsonArray rootTags;
    QString parseTreeError;
    if (parser.parseRootTags(rawText, &rootTags, &parseTreeError))
    {
        WhatSonTagsDepthFlattener flattener;
        outStore->setTagEntries(flattener.flatten(rootTags));
        return true;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(rawText.toUtf8(), &parseError);
    if (parseError.error == QJsonParseError::NoError)
    {
        if (document.isArray())
        {
            outStore->setTagEntries(parseTagEntries(document.array()));
            return true;
        }

        if (document.isObject())
        {
            const QJsonObject object = document.object();
            const QJsonValue entriesValue = object.value(QStringLiteral("entries"));
            if (entriesValue.isArray())
            {
                outStore->setTagEntries(parseTagEntries(entriesValue.toArray()));
                return true;
            }
        }
    }

    if (errorMessage != nullptr)
    {
        *errorMessage = parseTreeError.isEmpty()
                            ? QStringLiteral("Unsupported tags format.")
                            : parseTreeError;
    }
    return false;
}
