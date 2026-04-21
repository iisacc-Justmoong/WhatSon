#include "app/models/content/structured/ContentsStructuredDocumentCollectionPolicy.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"

#include <algorithm>
#include <cmath>

namespace
{
    QVariantMap normalizedMap(const QVariant& value)
    {
        return value.typeId() == QMetaType::QVariantMap ? value.toMap() : QVariantMap{};
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
    if (rawEntries.typeId() == QMetaType::QVariantList)
    {
        return rawEntries.toList();
    }

    const QVariantMap indexedMap = rawEntries.toMap();
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

QString ContentsStructuredDocumentCollectionPolicy::normalizeSourceText(const QString& value) const
{
    QString normalized = value;
    normalized.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    normalized.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    normalized.replace(QChar(0x2028), QLatin1Char('\n'));
    normalized.replace(QChar(0x2029), QLatin1Char('\n'));
    normalized.replace(QChar::Nbsp, QLatin1Char(' '));
    return normalized;
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
    bool ok = false;
    const double numericValue = value.toDouble(&ok);
    if (!ok)
    {
        return fallbackValue;
    }
    return static_cast<int>(std::floor(numericValue));
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

bool ContentsStructuredDocumentCollectionPolicy::resourceEntryHasResolvedPayload(
    const QVariantMap& entry)
{
    const QString entrySource = entry.value(QStringLiteral("source")).toString().trimmed();
    const QString entryResolvedPath =
        entry.value(QStringLiteral("resolvedPath")).toString().trimmed();
    return !entrySource.isEmpty() || !entryResolvedPath.isEmpty();
}
