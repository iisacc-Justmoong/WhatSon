#include "WhatSonProjectsHierarchyParser.hpp"

#include "WhatSonProjectsHierarchyStore.hpp"
#include "WhatSonDebugTrace.hpp"

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

    QString firstNonEmptyText(const QJsonObject& object, const QStringList& keys)
    {
        for (const QString& key : keys)
        {
            const QJsonValue value = object.value(key);
            if (value.isString())
            {
                const QString text = value.toString().trimmed();
                if (!text.isEmpty())
                {
                    return text;
                }
            }
        }
        return {};
    }

    int parseDepthValue(const QJsonObject& object, int fallbackDepth)
    {
        auto parseIntField = [&object](const QString& key, int* outValue) -> bool
        {
            if (outValue == nullptr || !object.contains(key))
            {
                return false;
            }

            const QJsonValue value = object.value(key);
            if (value.isDouble())
            {
                *outValue = value.toInt();
                return true;
            }
            if (value.isString())
            {
                bool converted = false;
                const int parsed = value.toString().trimmed().toInt(&converted);
                if (converted)
                {
                    *outValue = parsed;
                    return true;
                }
            }
            return false;
        };

        int parsedDepth = fallbackDepth;
        if (parseIntField(QStringLiteral("depth"), &parsedDepth)
            || parseIntField(QStringLiteral("dpeth"), &parsedDepth)
            || parseIntField(QStringLiteral("indentLevel"), &parsedDepth))
        {
            return parsedDepth < 0 ? 0 : parsedDepth;
        }
        return fallbackDepth < 0 ? 0 : fallbackDepth;
    }

    QJsonArray childNodes(const QJsonObject& object)
    {
        const QStringList keys{
            QStringLiteral("children"),
            QStringLiteral("items"),
            QStringLiteral("folders"),
            QStringLiteral("projects")
        };
        for (const QString& key : keys)
        {
            const QJsonValue value = object.value(key);
            if (value.isArray())
            {
                return value.toArray();
            }
        }
        return {};
    }

    void appendNodeRecursive(
        const QJsonValue& nodeValue,
        int fallbackDepth,
        QVector<WhatSonFolderDepthEntry>* outEntries);

    void appendObjectMap(
        const QJsonObject& objectMap,
        int fallbackDepth,
        QVector<WhatSonFolderDepthEntry>* outEntries)
    {
        if (outEntries == nullptr)
        {
            return;
        }

        for (auto it = objectMap.constBegin(); it != objectMap.constEnd(); ++it)
        {
            const QString key = it.key().trimmed();
            if (it.value().isObject())
            {
                QJsonObject nodeObject = it.value().toObject();
                if (!nodeObject.contains(QStringLiteral("id"))
                    && !nodeObject.contains(QStringLiteral("path")))
                {
                    nodeObject.insert(QStringLiteral("id"), key);
                }
                if (!nodeObject.contains(QStringLiteral("label"))
                    && !nodeObject.contains(QStringLiteral("name"))
                    && !nodeObject.contains(QStringLiteral("title")))
                {
                    nodeObject.insert(QStringLiteral("label"), key);
                }
                appendNodeRecursive(nodeObject, fallbackDepth, outEntries);
                continue;
            }

            if (it.value().isString())
            {
                WhatSonFolderDepthEntry entry;
                entry.id = key;
                entry.label = it.value().toString().trimmed();
                if (entry.label.isEmpty())
                {
                    entry.label = key;
                }
                entry.depth = fallbackDepth < 0 ? 0 : fallbackDepth;
                if (!entry.id.isEmpty() && !entry.label.isEmpty())
                {
                    outEntries->push_back(std::move(entry));
                }
                continue;
            }

            if (it.value().isArray())
            {
                const QJsonArray array = it.value().toArray();
                for (const QJsonValue& childValue : array)
                {
                    appendNodeRecursive(childValue, fallbackDepth, outEntries);
                }
            }
        }
    }

    void appendNodeRecursive(
        const QJsonValue& nodeValue,
        int fallbackDepth,
        QVector<WhatSonFolderDepthEntry>* outEntries)
    {
        if (outEntries == nullptr)
        {
            return;
        }

        if (nodeValue.isString())
        {
            const QString text = nodeValue.toString().trimmed();
            if (text.isEmpty())
            {
                return;
            }
            WhatSonFolderDepthEntry entry;
            entry.id = text;
            entry.label = text;
            entry.depth = fallbackDepth < 0 ? 0 : fallbackDepth;
            outEntries->push_back(std::move(entry));
            return;
        }

        if (!nodeValue.isObject())
        {
            return;
        }

        const QJsonObject object = nodeValue.toObject();
        WhatSonFolderDepthEntry entry;
        entry.depth = parseDepthValue(object, fallbackDepth);
        entry.id = firstNonEmptyText(object, {
                                         QStringLiteral("id"),
                                         QStringLiteral("path"),
                                         QStringLiteral("key"),
                                         QStringLiteral("name"),
                                         QStringLiteral("label"),
                                         QStringLiteral("title")
                                     });
        entry.label = firstNonEmptyText(object, {
                                            QStringLiteral("label"),
                                            QStringLiteral("name"),
                                            QStringLiteral("title"),
                                            QStringLiteral("id"),
                                            QStringLiteral("path"),
                                            QStringLiteral("key")
                                        });
        if (entry.label.isEmpty() && !entry.id.isEmpty())
        {
            entry.label = entry.id;
        }
        if (entry.id.isEmpty() && !entry.label.isEmpty())
        {
            entry.id = entry.label;
        }

        bool pushed = false;
        if (!entry.id.isEmpty() && !entry.label.isEmpty())
        {
            outEntries->push_back(std::move(entry));
            pushed = true;
        }

        const QJsonArray children = childNodes(object);
        if (children.isEmpty())
        {
            return;
        }

        const int childDepth = pushed ? parseDepthValue(object, fallbackDepth) + 1 : fallbackDepth + 1;
        for (const QJsonValue& childNode : children)
        {
            appendNodeRecursive(childNode, childDepth, outEntries);
        }
    }

    bool parseRootObject(
        const QJsonObject& object,
        QVector<WhatSonFolderDepthEntry>* outEntries)
    {
        if (outEntries == nullptr)
        {
            return false;
        }

        outEntries->clear();

        const QStringList listKeys{
            QStringLiteral("folders"),
            QStringLiteral("projects")
        };
        for (const QString& key : listKeys)
        {
            if (!object.contains(key))
            {
                continue;
            }

            const QJsonValue listValue = object.value(key);
            if (listValue.isArray())
            {
                const QJsonArray array = listValue.toArray();
                for (const QJsonValue& value : array)
                {
                    appendNodeRecursive(value, 0, outEntries);
                }
                return true;
            }
            if (listValue.isObject())
            {
                appendObjectMap(listValue.toObject(), 0, outEntries);
                return true;
            }
            if (listValue.isString())
            {
                appendNodeRecursive(listValue, 0, outEntries);
                return true;
            }
        }

        const bool looksLikeFolderNode = object.contains(QStringLiteral("id"))
            || object.contains(QStringLiteral("label"))
            || object.contains(QStringLiteral("name"))
            || object.contains(QStringLiteral("title"))
            || object.contains(QStringLiteral("children"))
            || object.contains(QStringLiteral("items"));
        if (looksLikeFolderNode)
        {
            appendNodeRecursive(object, 0, outEntries);
            return true;
        }

        appendObjectMap(object, 0, outEntries);
        return !outEntries->isEmpty();
    }
} // namespace

WhatSonProjectsHierarchyParser::WhatSonProjectsHierarchyParser() = default;

WhatSonProjectsHierarchyParser::~WhatSonProjectsHierarchyParser() = default;

bool WhatSonProjectsHierarchyParser::parse(
    const QString& rawText,
    WhatSonProjectsHierarchyStore* outStore,
    QString* errorMessage) const
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.projects.parser"),
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
        outStore->setFolderEntries({});
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("hierarchy.projects.parser"),
                                  QStringLiteral("parse.empty"));
        return true;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(rawText.toUtf8(), &parseError);
    if (parseError.error == QJsonParseError::NoError && !document.isNull())
    {
        QVector<WhatSonFolderDepthEntry> parsedEntries;
        if (document.isArray())
        {
            const QJsonArray array = document.array();
            parsedEntries.reserve(array.size());
            for (const QJsonValue& value : array)
            {
                appendNodeRecursive(value, 0, &parsedEntries);
            }
            outStore->setFolderEntries(std::move(parsedEntries));
            return true;
        }

        if (document.isObject())
        {
            const QJsonObject object = document.object();
            if (parseRootObject(object, &parsedEntries))
            {
                outStore->setFolderEntries(std::move(parsedEntries));
                return true;
            }
        }
    }

    outStore->setProjectNames(sanitizeLines(rawText));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.projects.parser"),
                              QStringLiteral("parse.fallbackLines"),
                              QStringLiteral("count=%1").arg(outStore->projectNames().size()));
    return true;
}
