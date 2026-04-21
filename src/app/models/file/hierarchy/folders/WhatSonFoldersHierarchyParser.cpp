#include "WhatSonFoldersHierarchyParser.hpp"

#include "../WhatSonFolderIdentity.hpp"
#include "WhatSonDebugTrace.hpp"
#include "WhatSonFoldersHierarchyStore.hpp"
#include "models/file/note/WhatSonNoteFolderSemantics.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QRegularExpression>

#include <algorithm>

namespace
{
    QString normalizedOrGeneratedFolderUuid(QString value, bool* outUuidMigrationRequired)
    {
        const QString normalizedValue = WhatSon::FolderIdentity::normalizeFolderUuid(std::move(value));
        if (!normalizedValue.isEmpty())
        {
            return normalizedValue;
        }

        if (outUuidMigrationRequired != nullptr)
        {
            *outUuidMigrationRequired = true;
        }
        return WhatSon::FolderIdentity::createFolderUuid();
    }

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

    QString leafNameFromPath(const QString& path)
    {
        return WhatSon::NoteFolders::leafFolderName(path);
    }

    void normalizeEntriesByDepthAndPath(
        QVector<WhatSonFolderDepthEntry>* entries,
        bool* outUuidMigrationRequired)
    {
        if (entries == nullptr)
        {
            return;
        }

        QVector<WhatSonFolderDepthEntry> normalized;
        normalized.reserve(entries->size());
        QStringList pathStack;

        for (WhatSonFolderDepthEntry entry : *entries)
        {
            entry.id = WhatSon::NoteFolders::normalizeFolderPath(std::move(entry.id));
            entry.label = entry.label.trimmed();
            entry.uuid = normalizedOrGeneratedFolderUuid(std::move(entry.uuid), outUuidMigrationRequired);
            if (entry.label.isEmpty() && !entry.id.isEmpty())
            {
                entry.label = leafNameFromPath(entry.id);
            }
            if (entry.label.isEmpty())
            {
                continue;
            }

            int depth = std::max(0, entry.depth);
            if (depth > pathStack.size())
            {
                depth = pathStack.size();
            }
            while (pathStack.size() > depth)
            {
                pathStack.removeLast();
            }
            entry.depth = depth;

            const QString parentPath = (depth > 0 && !pathStack.isEmpty()) ? pathStack.constLast() : QString();
            entry.id = WhatSon::NoteFolders::appendFolderPathSegment(parentPath, entry.label);
            if (entry.id.isEmpty())
            {
                continue;
            }

            normalized.push_back(entry);

            if (pathStack.size() <= depth)
            {
                pathStack.push_back(entry.id);
            }
            else
            {
                pathStack[depth] = entry.id;
                pathStack = pathStack.mid(0, depth + 1);
            }
        }

        *entries = std::move(normalized);
    }

    QVector<WhatSonFolderDepthEntry> buildFlatEntries(
        const QStringList& values,
        bool* outUuidMigrationRequired)
    {
        QVector<WhatSonFolderDepthEntry> entries;
        entries.reserve(values.size());

        for (const QString& value : values)
        {
            const QString trimmedValue = value.trimmed();
            if (trimmedValue.isEmpty())
            {
                continue;
            }

            WhatSonFolderDepthEntry entry;
            entry.id = WhatSon::NoteFolders::appendFolderPathSegment({}, trimmedValue);
            entry.label = trimmedValue;
            entry.depth = 0;
            entry.uuid = normalizedOrGeneratedFolderUuid({}, outUuidMigrationRequired);
            entries.push_back(std::move(entry));
        }

        return entries;
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
        QVector<WhatSonFolderDepthEntry>* outEntries,
        bool* outUuidMigrationRequired);

    void appendObjectMap(
        const QJsonObject& objectMap,
        int fallbackDepth,
        QVector<WhatSonFolderDepthEntry>* outEntries,
        bool* outUuidMigrationRequired)
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
                appendNodeRecursive(nodeObject, fallbackDepth, outEntries, outUuidMigrationRequired);
                continue;
            }

            if (it.value().isString())
            {
                WhatSonFolderDepthEntry entry;
                entry.id = key;
                entry.label = it.value().toString().trimmed();
                entry.uuid = normalizedOrGeneratedFolderUuid({}, outUuidMigrationRequired);
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
                    appendNodeRecursive(childValue, fallbackDepth, outEntries, outUuidMigrationRequired);
                }
            }
        }
    }

    void appendNodeRecursive(
        const QJsonValue& nodeValue,
        int fallbackDepth,
        QVector<WhatSonFolderDepthEntry>* outEntries,
        bool* outUuidMigrationRequired)
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
            entry.id = WhatSon::NoteFolders::appendFolderPathSegment({}, text);
            entry.label = text;
            entry.depth = fallbackDepth < 0 ? 0 : fallbackDepth;
            entry.uuid = normalizedOrGeneratedFolderUuid({}, outUuidMigrationRequired);
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
        entry.uuid = firstNonEmptyText(object, {
                                           QStringLiteral("uuid"),
                                           QStringLiteral("UUID"),
                                           QStringLiteral("folderUuid")
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
            entry.uuid = normalizedOrGeneratedFolderUuid(std::move(entry.uuid), outUuidMigrationRequired);
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
            appendNodeRecursive(childNode, childDepth, outEntries, outUuidMigrationRequired);
        }
    }

    bool parseRootObject(
        const QJsonObject& object,
        QVector<WhatSonFolderDepthEntry>* outEntries,
        bool* outUuidMigrationRequired)
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
                    appendNodeRecursive(value, 0, outEntries, outUuidMigrationRequired);
                }
                return true;
            }
            if (listValue.isObject())
            {
                appendObjectMap(listValue.toObject(), 0, outEntries, outUuidMigrationRequired);
                return true;
            }
            if (listValue.isString())
            {
                appendNodeRecursive(listValue, 0, outEntries, outUuidMigrationRequired);
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
            appendNodeRecursive(object, 0, outEntries, outUuidMigrationRequired);
            return true;
        }

        appendObjectMap(object, 0, outEntries, outUuidMigrationRequired);
        return !outEntries->isEmpty();
    }
} // namespace

WhatSonFoldersHierarchyParser::WhatSonFoldersHierarchyParser() = default;

WhatSonFoldersHierarchyParser::~WhatSonFoldersHierarchyParser() = default;

bool WhatSonFoldersHierarchyParser::parse(
    const QString& rawText,
    WhatSonFoldersHierarchyStore* outStore,
    QString* errorMessage,
    bool* outUuidMigrationRequired) const
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.folders.parser"),
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
    if (outUuidMigrationRequired != nullptr)
    {
        *outUuidMigrationRequired = false;
    }

    if (rawText.trimmed().isEmpty())
    {
        outStore->setFolderEntries({});
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("hierarchy.folders.parser"),
                                  QStringLiteral("parse.empty"));
        return true;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(rawText.toUtf8(), &parseError);
    if (parseError.error == QJsonParseError::NoError && !document.isNull())
    {
        QVector<WhatSonFolderDepthEntry> parsedEntries;
        bool uuidMigrationRequired = false;
        if (document.isArray())
        {
            const QJsonArray array = document.array();
            parsedEntries.reserve(array.size());
            for (const QJsonValue& value : array)
            {
                appendNodeRecursive(value, 0, &parsedEntries, &uuidMigrationRequired);
            }
            normalizeEntriesByDepthAndPath(&parsedEntries, &uuidMigrationRequired);
            outStore->setFolderEntries(std::move(parsedEntries));
            if (outUuidMigrationRequired != nullptr)
            {
                *outUuidMigrationRequired = uuidMigrationRequired;
            }
            return true;
        }

        if (document.isObject())
        {
            const QJsonObject object = document.object();
            if (parseRootObject(object, &parsedEntries, &uuidMigrationRequired))
            {
                normalizeEntriesByDepthAndPath(&parsedEntries, &uuidMigrationRequired);
                outStore->setFolderEntries(std::move(parsedEntries));
                if (outUuidMigrationRequired != nullptr)
                {
                    *outUuidMigrationRequired = uuidMigrationRequired;
                }
                return true;
            }
        }
    }

    bool uuidMigrationRequired = false;
    outStore->setFolderEntries(buildFlatEntries(sanitizeLines(rawText), &uuidMigrationRequired));
    if (outUuidMigrationRequired != nullptr)
    {
        *outUuidMigrationRequired = uuidMigrationRequired;
    }
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hierarchy.folders.parser"),
                              QStringLiteral("parse.fallbackLines"),
                              QStringLiteral("count=%1").arg(outStore->folderEntries().size()));
    return true;
}
