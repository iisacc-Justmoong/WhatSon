#include "app/models/editor/structure/ContentsStructuredDocumentCollectionPolicy.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"

#include <QJSValue>

#include <algorithm>
#include <cmath>

namespace
{
    QVariantMap normalizedMap(const QVariant& value)
    {
        return value.typeId() == QMetaType::QVariantMap ? value.toMap() : QVariantMap{};
    }

    QVariant normalizedEntriesSource(const QVariant& rawEntries)
    {
        if (rawEntries.metaType().id() != qMetaTypeId<QJSValue>())
        {
            return rawEntries;
        }

        const QJSValue jsValue = rawEntries.value<QJSValue>();
        if (jsValue.isUndefined() || jsValue.isNull())
        {
            return {};
        }

        return jsValue.toVariant();
    }

    int floorNumberOrFallbackValue(const QVariant& value, const int fallbackValue)
    {
        bool ok = false;
        const double numericValue = value.toDouble(&ok);
        if (!ok)
        {
            return fallbackValue;
        }
        return static_cast<int>(std::floor(numericValue));
    }

    QString normalizedSourceTextValue(const QString& value)
    {
        QString normalized = value;
        normalized.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        normalized.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        normalized.replace(QChar(0x2028), QLatin1Char('\n'));
        normalized.replace(QChar(0x2029), QLatin1Char('\n'));
        normalized.replace(QChar::Nbsp, QLatin1Char(' '));
        return normalized;
    }

    QString normalizedBlockType(const QVariantMap& blockEntry)
    {
        return blockEntry.value(QStringLiteral("type")).toString().trimmed().toCaseFolded();
    }

    bool isInteractiveTextFlattenCandidate(const QVariantMap& blockEntry)
    {
        if (blockEntry.value(QStringLiteral("flattenedInteractiveGroup")).toBool())
        {
            return false;
        }
        if (blockEntry.value(QStringLiteral("explicitBlock")).toBool())
        {
            return false;
        }
        if (blockEntry.contains(QStringLiteral("textEditable"))
            && !blockEntry.value(QStringLiteral("textEditable")).toBool())
        {
            return false;
        }
        if (blockEntry.contains(QStringLiteral("atomicBlock"))
            && blockEntry.value(QStringLiteral("atomicBlock")).toBool())
        {
            return false;
        }

        const QString blockType = normalizedBlockType(blockEntry);
        return blockType != QStringLiteral("agenda")
            && blockType != QStringLiteral("callout")
            && blockType != QStringLiteral("resource")
            && blockType != QStringLiteral("break");
    }

    int groupedTextLogicalLineCountHint(const QString& groupSourceText)
    {
        const QString normalizedSourceText = normalizedSourceTextValue(groupSourceText);
        if (normalizedSourceText.isEmpty())
        {
            return 1;
        }
        return std::max(1, static_cast<int>(normalizedSourceText.count(QLatin1Char('\n'))) + 1);
    }

    QVariantMap buildFlattenedInteractiveTextGroup(
        const QVariantList& groupBlocks,
        const QString& normalizedSourceText)
    {
        if (groupBlocks.isEmpty())
        {
            return {};
        }

        const QVariantMap firstBlock = normalizedMap(groupBlocks.constFirst());
        const QVariantMap lastBlock = normalizedMap(groupBlocks.constLast());
        const int sourceStart = std::max(
            0,
            floorNumberOrFallbackValue(firstBlock.value(QStringLiteral("sourceStart")), 0));
        const int sourceEnd = std::max(
            sourceStart,
            floorNumberOrFallbackValue(lastBlock.value(QStringLiteral("sourceEnd")), sourceStart));
        const QString groupedSourceText =
            normalizedSourceText.mid(sourceStart, sourceEnd - sourceStart);

        QVariantMap flattenedGroup;
        flattenedGroup.insert(QStringLiteral("atomicBlock"), false);
        flattenedGroup.insert(
            QStringLiteral("flattenedInteractiveChildCount"),
            static_cast<int>(groupBlocks.size()));
        flattenedGroup.insert(QStringLiteral("flattenedInteractiveGroup"), true);
        flattenedGroup.insert(QStringLiteral("focusSourceOffset"), sourceStart);
        flattenedGroup.insert(QStringLiteral("groupedBlocks"), groupBlocks);
        flattenedGroup.insert(
            QStringLiteral("logicalLineCountHint"),
            groupedTextLogicalLineCountHint(groupedSourceText));
        flattenedGroup.insert(QStringLiteral("minimapRepresentativeCharCount"), 0);
        flattenedGroup.insert(QStringLiteral("minimapVisualKind"), QStringLiteral("text"));
        flattenedGroup.insert(QStringLiteral("plainText"), groupedSourceText);
        flattenedGroup.insert(QStringLiteral("sourceEnd"), sourceEnd);
        flattenedGroup.insert(QStringLiteral("sourceStart"), sourceStart);
        flattenedGroup.insert(QStringLiteral("sourceText"), groupedSourceText);
        flattenedGroup.insert(QStringLiteral("textEditable"), true);
        flattenedGroup.insert(QStringLiteral("type"), QStringLiteral("text-group"));
        return flattenedGroup;
    }

    QVariantMap emptyInteractiveTextGroup()
    {
        return QVariantMap{
            {QStringLiteral("atomicBlock"), false},
            {QStringLiteral("flattenedInteractiveChildCount"), 0},
            {QStringLiteral("flattenedInteractiveGroup"), true},
            {QStringLiteral("focusSourceOffset"), 0},
            {QStringLiteral("groupedBlocks"), QVariantList{}},
            {QStringLiteral("logicalLineCountHint"), 1},
            {QStringLiteral("minimapRepresentativeCharCount"), 0},
            {QStringLiteral("minimapVisualKind"), QStringLiteral("text")},
            {QStringLiteral("plainText"), QString()},
            {QStringLiteral("sourceEnd"), 0},
            {QStringLiteral("sourceStart"), 0},
            {QStringLiteral("sourceText"), QString()},
            {QStringLiteral("textEditable"), true},
            {QStringLiteral("type"), QStringLiteral("text-group")},
        };
    }
}

ContentsStructuredDocumentCollectionPolicy::ContentsStructuredDocumentCollectionPolicy(QObject* parent)
    : QObject(parent)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentCollectionPolicy"),
        QStringLiteral("ctor"));
}

ContentsStructuredDocumentCollectionPolicy::~ContentsStructuredDocumentCollectionPolicy()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredDocumentCollectionPolicy"),
        QStringLiteral("dtor"));
}

QVariantList ContentsStructuredDocumentCollectionPolicy::normalizeEntries(
    const QVariant& rawEntries) const
{
    const QVariant normalizedSource = normalizedEntriesSource(rawEntries);
    if (normalizedSource.typeId() == QMetaType::QVariantList)
    {
        return normalizedSource.toList();
    }

    const QVariantMap indexedMap = normalizedSource.toMap();
    if (!indexedMap.isEmpty())
    {
        QList<int> keys;
        keys.reserve(indexedMap.size());
        for (auto it = indexedMap.constBegin(); it != indexedMap.constEnd(); ++it)
        {
            bool ok = false;
            const int index = it.key().toInt(&ok);
            if (ok)
            {
                keys.push_back(index);
            }
        }

        std::sort(keys.begin(), keys.end());

        QVariantList normalizedEntries;
        normalizedEntries.reserve(keys.size());
        for (const int index : keys)
        {
            normalizedEntries.push_back(indexedMap.value(QString::number(index)));
        }
        if (!normalizedEntries.isEmpty())
        {
            return normalizedEntries;
        }
    }

    return {};
}

QVariantList ContentsStructuredDocumentCollectionPolicy::normalizeInteractiveDocumentBlocks(
    const QVariant& rawEntries,
    const QString& sourceText) const
{
    return normalizeInteractiveDocumentBlockEntries(normalizeEntries(rawEntries), sourceText);
}

QString ContentsStructuredDocumentCollectionPolicy::normalizeSourceText(const QString& value) const
{
    return normalizedSourceTextValue(value);
}

QString ContentsStructuredDocumentCollectionPolicy::spliceSourceRange(
    const QString& sourceText,
    const int start,
    const int end,
    const QString& replacementText) const
{
    const QString normalizedSourceText = normalizeSourceText(sourceText);
    const int normalizedSourceLength = static_cast<int>(normalizedSourceText.size());
    const int safeStart = std::clamp(start, 0, normalizedSourceLength);
    const int safeEnd = std::clamp(end, safeStart, normalizedSourceLength);
    return normalizedSourceText.left(safeStart)
        + QString(replacementText)
        + normalizedSourceText.mid(safeEnd);
}

int ContentsStructuredDocumentCollectionPolicy::floorNumberOrFallback(
    const QVariant& value,
    const int fallbackValue) const
{
    return floorNumberOrFallbackValue(value, fallbackValue);
}

QVariantMap ContentsStructuredDocumentCollectionPolicy::resourceEntryForBlock(
    const QVariantMap& blockEntry,
    const QVariant& renderedResources) const
{
    const QVariantList resourceEntries = normalizeEntries(renderedResources);
    const int blockResourceIndex = floorNumberOrFallback(
        blockEntry.value(QStringLiteral("resourceIndex")),
        -1);
    const int blockSourceStart = std::max(
        0,
        floorNumberOrFallback(blockEntry.value(QStringLiteral("sourceStart")), 0));
    const int blockSourceEnd = std::max(
        blockSourceStart,
        floorNumberOrFallback(blockEntry.value(QStringLiteral("sourceEnd")), blockSourceStart));
    const QString blockResourceId =
        blockEntry.value(QStringLiteral("resourceId")).toString().trimmed();
    const QString blockResourcePath =
        blockEntry.value(QStringLiteral("resourcePath")).toString().trimmed();

    QVariantMap fallbackMatch;
    for (const QVariant& entryValue : resourceEntries)
    {
        const QVariantMap entry = normalizedMap(entryValue);
        const int entryIndex = floorNumberOrFallback(entry.value(QStringLiteral("index")), -1);
        if (blockResourceIndex < 0 || entryIndex != blockResourceIndex)
        {
            continue;
        }
        if (resourceEntryHasResolvedPayload(entry))
        {
            return entry;
        }
        if (fallbackMatch.isEmpty())
        {
            fallbackMatch = entry;
        }
    }

    for (const QVariant& entryValue : resourceEntries)
    {
        const QVariantMap entry = normalizedMap(entryValue);
        const int entrySourceStart = std::max(
            0,
            floorNumberOrFallback(entry.value(QStringLiteral("sourceStart")), 0));
        const int entrySourceEnd = std::max(
            entrySourceStart,
            floorNumberOrFallback(entry.value(QStringLiteral("sourceEnd")), entrySourceStart));
        if (entrySourceStart != blockSourceStart || entrySourceEnd != blockSourceEnd)
        {
            continue;
        }
        if (resourceEntryHasResolvedPayload(entry))
        {
            return entry;
        }
        if (fallbackMatch.isEmpty())
        {
            fallbackMatch = entry;
        }
    }

    for (const QVariant& entryValue : resourceEntries)
    {
        const QVariantMap entry = normalizedMap(entryValue);
        const QString entryResourceId =
            entry.value(QStringLiteral("resourceId")).toString().trimmed();
        const QString entryResourcePath =
            entry.value(QStringLiteral("resourcePath")).toString().trimmed();
        const bool idMatched = !blockResourceId.isEmpty() && entryResourceId == blockResourceId;
        const bool pathMatched =
            !blockResourcePath.isEmpty() && entryResourcePath == blockResourcePath;
        if (!idMatched && !pathMatched)
        {
            continue;
        }
        if (resourceEntryHasResolvedPayload(entry))
        {
            return entry;
        }
        if (fallbackMatch.isEmpty())
        {
            fallbackMatch = entry;
        }
    }

    return fallbackMatch;
}

QVariantList ContentsStructuredDocumentCollectionPolicy::normalizeInteractiveDocumentBlockEntries(
    const QVariantList& entries,
    const QString& sourceText)
{
    const QString normalizedSourceText = normalizedSourceTextValue(sourceText);
    if (entries.isEmpty())
    {
        return normalizedSourceText.isEmpty()
            ? QVariantList{emptyInteractiveTextGroup()}
            : QVariantList{};
    }

    QVariantList normalizedBlocks;
    normalizedBlocks.reserve(entries.size());

    QVariantList pendingTextBlocks;
    pendingTextBlocks.reserve(entries.size());

    const auto flushPendingTextBlocks = [&normalizedBlocks, &pendingTextBlocks, &normalizedSourceText]()
    {
        if (pendingTextBlocks.isEmpty())
        {
            return;
        }

        normalizedBlocks.push_back(
            buildFlattenedInteractiveTextGroup(pendingTextBlocks, normalizedSourceText));
        pendingTextBlocks.clear();
    };

    for (const QVariant& entryValue : entries)
    {
        const QVariantMap blockEntry = normalizedMap(entryValue);
        if (isInteractiveTextFlattenCandidate(blockEntry))
        {
            pendingTextBlocks.push_back(blockEntry);
            continue;
        }

        flushPendingTextBlocks();
        normalizedBlocks.push_back(blockEntry);
    }

    flushPendingTextBlocks();
    return normalizedBlocks;
}

bool ContentsStructuredDocumentCollectionPolicy::resourceEntryHasResolvedPayload(
    const QVariantMap& entry)
{
    const QString entrySource = entry.value(QStringLiteral("source")).toString().trimmed();
    const QString entryResolvedPath =
        entry.value(QStringLiteral("resolvedPath")).toString().trimmed();
    return !entrySource.isEmpty() || !entryResolvedPath.isEmpty();
}
