#include "WhatSonBookmarksHierarchyParser.hpp"

#include "WhatSonBookmarksHierarchyStore.hpp"
#include "models/file/WhatSonDebugTrace.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QRegularExpression>

namespace
{
    QStringList sanitizeLines(const QString& rawText)
    {
        QStringList values;
        const QStringList lines = rawText.split(QRegularExpression(QStringLiteral("[\r\n]+")), Qt::SkipEmptyParts);
        values.reserve(lines.size());

        for (QString line : lines)
        {
            line = line.trimmed();
            if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
            {
                continue;
            }
            values.push_back(line);
        }

        return values;
    }

    QStringList parseArrayValues(const QJsonArray& array)
    {
        QStringList values;
        values.reserve(array.size());

        for (const QJsonValue& value : array)
        {
            if (value.isString())
            {
                const QString text = value.toString().trimmed();
                if (!text.isEmpty())
                {
                    values.push_back(text);
                }
            }
        }

        return values;
    }
} // namespace

WhatSonBookmarksHierarchyParser::WhatSonBookmarksHierarchyParser() = default;

WhatSonBookmarksHierarchyParser::~WhatSonBookmarksHierarchyParser() = default;

bool WhatSonBookmarksHierarchyParser::parse(
    const QString& rawText,
    WhatSonBookmarksHierarchyStore* outStore,
    QString* errorMessage) const
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.bookmarks.parser"),
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
        outStore->setBookmarkIds({});
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("hierarchy.bookmarks.parser"),
                                  QStringLiteral("parse.empty"));
        return true;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(rawText.toUtf8(), &parseError);
    if (parseError.error == QJsonParseError::NoError && !document.isNull())
    {
        if (document.isArray())
        {
            const QStringList parsedValues = parseArrayValues(document.array());
            outStore->setBookmarkIds(parsedValues);
            return true;
        }

        if (document.isObject())
        {
            const QJsonObject object = document.object();
            const QJsonValue listValue = object.value(QStringLiteral("bookmarks"));
            if (listValue.isArray())
            {
                outStore->setBookmarkIds(parseArrayValues(listValue.toArray()));
                return true;
            }
            if (listValue.isString())
            {
                outStore->setBookmarkIds(QStringList{listValue.toString().trimmed()});
                return true;
            }
        }
    }

    outStore->setBookmarkIds(sanitizeLines(rawText));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.bookmarks.parser"),
                              QStringLiteral("parse.fallbackLines"),
                              QStringLiteral("count=%1").arg(outStore->bookmarkIds().size()));
    return true;
}
