#include "app/models/editor/gutter/ContentsGutterLineNumberGeometry.hpp"

#include "app/models/editor/resource/ContentsEditorDynamicObjectSupport.hpp"

#include <QPointF>
#include <QRectF>
#include <QVariantMap>

#include <algorithm>
#include <utility>

namespace
{
    bool realFromGeometryVariant(const QVariant& value, const QString& key, qreal* resolvedValue)
    {
        if (resolvedValue == nullptr || !value.isValid())
        {
            return false;
        }

        if (value.canConvert<QRectF>())
        {
            const QRectF rectangle = value.toRectF();
            if (key == QLatin1String("x"))
            {
                *resolvedValue = rectangle.x();
                return true;
            }
            if (key == QLatin1String("y"))
            {
                *resolvedValue = rectangle.y();
                return true;
            }
            if (key == QLatin1String("width"))
            {
                *resolvedValue = rectangle.width();
                return true;
            }
            if (key == QLatin1String("height"))
            {
                *resolvedValue = rectangle.height();
                return true;
            }
        }

        if (value.canConvert<QPointF>())
        {
            const QPointF point = value.toPointF();
            if (key == QLatin1String("x"))
            {
                *resolvedValue = point.x();
                return true;
            }
            if (key == QLatin1String("y"))
            {
                *resolvedValue = point.y();
                return true;
            }
        }

        const QVariantMap map = value.toMap();
        if (map.isEmpty())
        {
            return false;
        }

        bool ok = false;
        const qreal valueFromMap = map.value(key).toReal(&ok);
        if (!ok)
        {
            return false;
        }

        *resolvedValue = valueFromMap;
        return true;
    }

    struct BlockKey final
    {
        int blockIndex = -1;
        int sourceEnd = 0;
        int sourceStart = 0;
        QString type;
    };

    int intFromMap(const QVariantMap& map, const QString& key, const int fallback)
    {
        bool ok = false;
        const int value = map.value(key).toInt(&ok);
        return ok ? value : fallback;
    }

    QString normalizedBlockType(const QVariantMap& block)
    {
        const auto normalize = [](const QVariant& value)
        {
            return value.toString().trimmed().toCaseFolded();
        };

        QString type = normalize(block.value(QStringLiteral("type")));
        if (type.isEmpty())
        {
            type = normalize(block.value(QStringLiteral("blockType")));
        }
        if (type.isEmpty())
        {
            type = normalize(block.value(QStringLiteral("renderDelegateType")));
        }
        if (type == QStringLiteral("whatson-resource-block"))
        {
            return QStringLiteral("resource");
        }
        return type;
    }

    bool rawLineLooksLikeResource(const QString& lineText)
    {
        const QString trimmedLine = lineText.trimmed();
        return trimmedLine.startsWith(QStringLiteral("<resource"))
            || trimmedLine.startsWith(QStringLiteral("<whatson-resource"));
    }

    int blockSourceStart(const QVariantMap& block)
    {
        return std::max(
            0,
            intFromMap(
                block,
                QStringLiteral("sourceStart"),
                intFromMap(block, QStringLiteral("blockSourceStart"), 0)));
    }

    int blockSourceEnd(const QVariantMap& block)
    {
        const int start = blockSourceStart(block);
        return std::max(
            start,
            intFromMap(
                block,
                QStringLiteral("sourceEnd"),
                intFromMap(block, QStringLiteral("blockSourceEnd"), start)));
    }

    BlockKey blockKeyForEntry(const QVariantMap& block)
    {
        return BlockKey{
            intFromMap(block, QStringLiteral("blockIndex"), -1),
            blockSourceEnd(block),
            blockSourceStart(block),
            normalizedBlockType(block)};
    }

    bool sameBlockKey(const BlockKey& lhs, const BlockKey& rhs)
    {
        if (lhs.blockIndex >= 0 || rhs.blockIndex >= 0)
        {
            return lhs.blockIndex == rhs.blockIndex;
        }

        return lhs.sourceStart == rhs.sourceStart
            && lhs.sourceEnd == rhs.sourceEnd
            && lhs.type == rhs.type;
    }

    bool containsBlockKey(const QList<BlockKey>& keys, const BlockKey& key)
    {
        return std::any_of(keys.cbegin(), keys.cend(), [&key](const BlockKey& existing)
        {
            return sameBlockKey(existing, key);
        });
    }

    QVariantList uniqueBlockEntries(const QVariantList& entries)
    {
        QVariantList uniqueEntries;
        uniqueEntries.reserve(entries.size());

        QList<BlockKey> keys;
        keys.reserve(entries.size());
        for (const QVariant& entryValue : entries)
        {
            const QVariantMap block = entryValue.toMap();
            if (block.isEmpty())
            {
                continue;
            }

            const BlockKey key = blockKeyForEntry(block);
            if (containsBlockKey(keys, key))
            {
                continue;
            }

            keys.append(key);
            uniqueEntries.append(block);
        }
        return uniqueEntries;
    }

    bool blockCoversCandidate(const QVariantMap& existing, const QVariantMap& candidate)
    {
        if (sameBlockKey(blockKeyForEntry(existing), blockKeyForEntry(candidate)))
        {
            return true;
        }

        const QString existingType = normalizedBlockType(existing);
        const QString candidateType = normalizedBlockType(candidate);
        if (existingType == QStringLiteral("resource") || candidateType == QStringLiteral("resource"))
        {
            return false;
        }

        return blockSourceStart(existing) <= blockSourceStart(candidate)
            && blockSourceEnd(existing) >= blockSourceEnd(candidate);
    }

    bool containsCoveredBlock(const QVariantList& entries, const QVariantMap& candidate)
    {
        return std::any_of(entries.cbegin(), entries.cend(), [&candidate](const QVariant& entryValue)
        {
            return blockCoversCandidate(entryValue.toMap(), candidate);
        });
    }

    QVariantList mergeBlockStreams(const QVariantList& primary, const QVariantList& secondary)
    {
        QVariantList merged = uniqueBlockEntries(primary);
        for (const QVariant& entryValue : uniqueBlockEntries(secondary))
        {
            const QVariantMap candidate = entryValue.toMap();
            if (!containsCoveredBlock(merged, candidate))
            {
                merged.append(candidate);
            }
        }

        std::stable_sort(merged.begin(), merged.end(), [](const QVariant& lhs, const QVariant& rhs)
        {
            const QVariantMap leftBlock = lhs.toMap();
            const QVariantMap rightBlock = rhs.toMap();
            const int leftStart = blockSourceStart(leftBlock);
            const int rightStart = blockSourceStart(rightBlock);
            if (leftStart != rightStart)
            {
                return leftStart < rightStart;
            }

            const int leftEnd = blockSourceEnd(leftBlock);
            const int rightEnd = blockSourceEnd(rightBlock);
            if (leftEnd != rightEnd)
            {
                return leftEnd < rightEnd;
            }

            return normalizedBlockType(leftBlock) < normalizedBlockType(rightBlock);
        });
        return merged;
    }

}

namespace
{
    int lineIndexForSourceSpan(
        const QList<ContentsGutterLineNumberGeometry::LineSlot>& lineSlots,
        const int sourceLength,
        const int sourceStart,
        const int sourceEnd)
    {
        const int blockStart = std::clamp(sourceStart, 0, sourceLength);
        const int blockEnd = std::clamp(std::max(blockStart, sourceEnd), blockStart, sourceLength);
        for (int index = 0; index < lineSlots.size(); ++index)
        {
            const ContentsGutterLineNumberGeometry::LineSlot& slot = lineSlots.at(index);
            const int lineStart = std::clamp(slot.sourceStart, 0, sourceLength);
            const int nextLineStart = std::clamp(
                std::max(slot.sourceEnd, lineStart + 1),
                lineStart,
                sourceLength + 1);
            const bool startsInsideLine = blockStart >= lineStart && blockStart < nextLineStart;
            const bool coversLineStart = blockStart <= lineStart && blockEnd > lineStart;
            if (startsInsideLine || coversLineStart)
            {
                return index;
            }
        }
        return -1;
    }
}

ContentsGutterLineNumberGeometry::ContentsGutterLineNumberGeometry(QObject* parent)
    : QObject(parent)
{
    rebuildLineNumberEntries();
}

ContentsGutterLineNumberGeometry::~ContentsGutterLineNumberGeometry() = default;

QObject* ContentsGutterLineNumberGeometry::editorGeometryHost() const noexcept
{
    return m_editorGeometryHost;
}

void ContentsGutterLineNumberGeometry::setEditorGeometryHost(QObject* value)
{
    if (m_editorGeometryHost == value)
    {
        return;
    }

    m_editorGeometryHost = value;
    emit editorGeometryHostChanged();
    rebuildLineNumberEntries();
}

QObject* ContentsGutterLineNumberGeometry::mapTarget() const noexcept
{
    return m_mapTarget;
}

void ContentsGutterLineNumberGeometry::setMapTarget(QObject* value)
{
    if (m_mapTarget == value)
    {
        return;
    }

    m_mapTarget = value;
    emit mapTargetChanged();
    rebuildLineNumberEntries();
}

QString ContentsGutterLineNumberGeometry::sourceText() const
{
    return m_sourceText;
}

void ContentsGutterLineNumberGeometry::setSourceText(const QString& value)
{
    if (m_sourceText == value)
    {
        return;
    }

    m_sourceText = value;
    emit sourceTextChanged();
    rebuildLineNumberEntries();
}

QVariantList ContentsGutterLineNumberGeometry::displayBlocks() const
{
    return m_displayBlocks;
}

void ContentsGutterLineNumberGeometry::setDisplayBlocks(const QVariantList& value)
{
    if (m_displayBlocks == value)
    {
        return;
    }

    m_displayBlocks = value;
    emit displayBlocksChanged();
    rebuildLineNumberEntries();
}

QVariantList ContentsGutterLineNumberGeometry::documentBlocks() const
{
    return m_documentBlocks;
}

void ContentsGutterLineNumberGeometry::setDocumentBlocks(const QVariantList& value)
{
    if (m_documentBlocks == value)
    {
        return;
    }

    m_documentBlocks = value;
    emit documentBlocksChanged();
    rebuildLineNumberEntries();
}

QVariantList ContentsGutterLineNumberGeometry::renderedResources() const
{
    return m_renderedResources;
}

void ContentsGutterLineNumberGeometry::setRenderedResources(const QVariantList& value)
{
    if (m_renderedResources == value)
    {
        return;
    }

    m_renderedResources = value;
    emit renderedResourcesChanged();
    rebuildLineNumberEntries();
}

int ContentsGutterLineNumberGeometry::lineNumberCount() const noexcept
{
    return m_lineNumberCount;
}

QVariantList ContentsGutterLineNumberGeometry::logicalLineStartOffsets() const
{
    return m_logicalLineStartOffsets;
}

void ContentsGutterLineNumberGeometry::setLogicalLineStartOffsets(const QVariantList& value)
{
    if (m_logicalLineStartOffsets == value)
    {
        return;
    }

    m_logicalLineStartOffsets = value;
    emit logicalLineStartOffsetsChanged();
    rebuildLineNumberEntries();
}

QVariantList ContentsGutterLineNumberGeometry::logicalToSourceOffsets() const
{
    return m_logicalToSourceOffsets;
}

void ContentsGutterLineNumberGeometry::setLogicalToSourceOffsets(const QVariantList& value)
{
    if (m_logicalToSourceOffsets == value)
    {
        return;
    }

    m_logicalToSourceOffsets = value;
    emit logicalToSourceOffsetsChanged();
    rebuildLineNumberEntries();
}

void ContentsGutterLineNumberGeometry::setLineNumberCount(const int value)
{
    const int normalizedValue = std::max(1, value);
    if (m_lineNumberCount == normalizedValue)
    {
        return;
    }

    m_lineNumberCount = normalizedValue;
    emit lineNumberCountChanged();
    rebuildLineNumberEntries();
}

int ContentsGutterLineNumberGeometry::lineNumberBaseOffset() const noexcept
{
    return m_lineNumberBaseOffset;
}

void ContentsGutterLineNumberGeometry::setLineNumberBaseOffset(const int value)
{
    if (m_lineNumberBaseOffset == value)
    {
        return;
    }

    m_lineNumberBaseOffset = value;
    emit lineNumberBaseOffsetChanged();
    rebuildLineNumberEntries();
}

qreal ContentsGutterLineNumberGeometry::fallbackLineHeight() const noexcept
{
    return m_fallbackLineHeight;
}

void ContentsGutterLineNumberGeometry::setFallbackLineHeight(const qreal value)
{
    const qreal normalizedValue = std::max<qreal>(1.0, value);
    if (qFuzzyCompare(m_fallbackLineHeight, normalizedValue))
    {
        return;
    }

    m_fallbackLineHeight = normalizedValue;
    emit fallbackLineHeightChanged();
    rebuildLineNumberEntries();
}

qreal ContentsGutterLineNumberGeometry::fallbackTopInset() const noexcept
{
    return m_fallbackTopInset;
}

void ContentsGutterLineNumberGeometry::setFallbackTopInset(const qreal value)
{
    if (qFuzzyCompare(m_fallbackTopInset, value))
    {
        return;
    }

    m_fallbackTopInset = value;
    emit fallbackTopInsetChanged();
    rebuildLineNumberEntries();
}

qreal ContentsGutterLineNumberGeometry::editorContentHeight() const noexcept
{
    return m_editorContentHeight;
}

void ContentsGutterLineNumberGeometry::setEditorContentHeight(const qreal value)
{
    if (qFuzzyCompare(m_editorContentHeight, value))
    {
        return;
    }

    m_editorContentHeight = value;
    emit editorContentHeightChanged();
    rebuildLineNumberEntries();
}

QVariantList ContentsGutterLineNumberGeometry::lineNumberEntries() const
{
    return m_lineNumberEntries;
}

void ContentsGutterLineNumberGeometry::refresh()
{
    rebuildLineNumberEntries();
}

QList<int> ContentsGutterLineNumberGeometry::sourceLineStartOffsets() const
{
    QList<int> offsets;
    offsets.append(0);

    for (int index = 0; index < m_sourceText.size(); ++index)
    {
        if (m_sourceText.at(index) == QLatin1Char('\n'))
        {
            offsets.append(index + 1);
        }
    }

    return offsets;
}

qreal ContentsGutterLineNumberGeometry::fallbackYForIndex(const int index) const noexcept
{
    return m_fallbackTopInset + (static_cast<qreal>(std::max(0, index)) * m_fallbackLineHeight);
}

int ContentsGutterLineNumberGeometry::logicalOffsetForSourceOffset(const int sourceOffset) const noexcept
{
    const int sourceLength = static_cast<int>(m_sourceText.size());
    const int boundedSourceOffset = std::clamp(sourceOffset, 0, sourceLength);
    if (m_logicalToSourceOffsets.isEmpty())
    {
        return boundedSourceOffset;
    }

    int nearestLogicalOffset = 0;
    for (int index = 0; index < m_logicalToSourceOffsets.size(); ++index)
    {
        bool ok = false;
        const int candidateSourceOffset = m_logicalToSourceOffsets.at(index).toInt(&ok);
        if (!ok)
        {
            continue;
        }

        nearestLogicalOffset = index;
        if (candidateSourceOffset >= boundedSourceOffset)
        {
            return index;
        }
    }

    return nearestLogicalOffset;
}

bool ContentsGutterLineNumberGeometry::editorLineGeometryForSlot(
    const LineSlot& slot,
    const int fallbackIndex,
    qreal* resolvedY,
    qreal* resolvedHeight) const
{
    if (m_editorGeometryHost == nullptr || resolvedY == nullptr || resolvedHeight == nullptr)
    {
        return false;
    }

    const int sourceLength = static_cast<int>(m_sourceText.size());
    const int sourceOffset = std::clamp(slot.sourceStart, 0, sourceLength);
    const int logicalOffset = logicalOffsetForSourceOffset(sourceOffset);
    const QVariant rawRectangle = WhatSon::Editor::DynamicObjectSupport::invokeVariant(
        m_editorGeometryHost,
        "lineStartRectangle",
        {QVariant(logicalOffset), QVariant(sourceOffset)});

    qreal rawX = 0.0;
    realFromGeometryVariant(rawRectangle, QStringLiteral("x"), &rawX);

    qreal rawY = fallbackYForIndex(fallbackIndex);
    if (!realFromGeometryVariant(rawRectangle, QStringLiteral("y"), &rawY))
    {
        return false;
    }

    qreal rawHeight = m_fallbackLineHeight;
    realFromGeometryVariant(rawRectangle, QStringLiteral("height"), &rawHeight);

    if (m_mapTarget != nullptr)
    {
        const QVariant mappedPoint = WhatSon::Editor::DynamicObjectSupport::invokeVariant(
            m_editorGeometryHost,
            "mapEditorPointToItem",
            {QVariant::fromValue(m_mapTarget), QVariant(rawX), QVariant(rawY)});
        realFromGeometryVariant(mappedPoint, QStringLiteral("y"), &rawY);
    }

    *resolvedY = rawY;
    *resolvedHeight = std::max<qreal>(1.0, rawHeight);
    return true;
}

QVariantList ContentsGutterLineNumberGeometry::effectiveBlockStream() const
{
    QVariantList uniqueDocumentBlocks = uniqueBlockEntries(m_documentBlocks);
    QVariantList uniqueDisplayBlocks = uniqueBlockEntries(m_displayBlocks);
    if (uniqueDocumentBlocks.isEmpty())
    {
        return uniqueDisplayBlocks;
    }
    if (uniqueDisplayBlocks.isEmpty())
    {
        return uniqueDocumentBlocks;
    }

    return mergeBlockStreams(uniqueDocumentBlocks, uniqueDisplayBlocks);
}

QList<ContentsGutterLineNumberGeometry::LineSlot> ContentsGutterLineNumberGeometry::rawLineSlots() const
{
    const QList<int> starts = sourceLineStartOffsets();
    QList<LineSlot> lineSlots;
    lineSlots.reserve(starts.size());

    const int sourceLength = static_cast<int>(m_sourceText.size());
    for (int index = 0; index < starts.size(); ++index)
    {
        const int lineStart = std::clamp(starts.at(index), 0, sourceLength);
        const int lineEnd = index + 1 < starts.size()
            ? std::clamp(starts.at(index + 1) - 1, lineStart, sourceLength)
            : sourceLength;
        const QString lineText = m_sourceText.mid(lineStart, lineEnd - lineStart);
        const bool resourceLine = rawLineLooksLikeResource(lineText);
        lineSlots.append(LineSlot{
            resourceLine ? QStringLiteral("resource") : QStringLiteral("raw"),
            lineEnd,
            lineStart,
            resourceLine});
    }

    return lineSlots.isEmpty() ? QList<LineSlot>{LineSlot{QStringLiteral("raw"), 0, 0, false}} : lineSlots;
}

QList<ContentsGutterLineNumberGeometry::LineSlot> ContentsGutterLineNumberGeometry::editorLineSlots() const
{
    QList<LineSlot> lineSlots = rawLineSlots();
    const QVariantList blocks = effectiveBlockStream();
    if (blocks.isEmpty() || lineSlots.isEmpty())
    {
        return lineSlots;
    }

    const auto appendBlock = [this, &lineSlots](const QVariantMap& block)
    {
        const QString type = normalizedBlockType(block);
        const bool resourceBlock = type == QStringLiteral("resource");
        const int sourceStart = std::clamp(blockSourceStart(block), 0, static_cast<int>(m_sourceText.size()));
        const int sourceEnd = std::clamp(blockSourceEnd(block), sourceStart, static_cast<int>(m_sourceText.size()));
        const int lineIndex = lineIndexForSourceSpan(
            lineSlots,
            static_cast<int>(m_sourceText.size()),
            sourceStart,
            sourceEnd);
        if (lineIndex < 0 || lineIndex >= lineSlots.size())
        {
            return;
        }

        LineSlot& slot = lineSlots[lineIndex];
        if (slot.blockType == QStringLiteral("raw") || resourceBlock)
        {
            slot.blockType = type;
        }
        slot.resource = slot.resource || resourceBlock;
    };

    for (const QVariant& blockValue : blocks)
    {
        const QVariantMap block = blockValue.toMap();
        if (block.isEmpty())
        {
            continue;
        }

        appendBlock(block);
    }

    return lineSlots;
}

void ContentsGutterLineNumberGeometry::rebuildLineNumberEntries()
{
    const QList<LineSlot> lineSlots = editorLineSlots();
    const int count = std::max(1, static_cast<int>(lineSlots.size()));

    QList<qreal> lineYs;
    lineYs.reserve(count);
    QList<qreal> lineHeights;
    lineHeights.reserve(count);
    QList<bool> sampledLineGeometry;
    sampledLineGeometry.reserve(count);
    bool sampledAnyLine = false;
    for (int index = 0; index < count; ++index)
    {
        qreal sampledY = fallbackYForIndex(index);
        qreal sampledHeight = m_fallbackLineHeight;
        const bool sampled = editorLineGeometryForSlot(
            lineSlots.value(index),
            index,
            &sampledY,
            &sampledHeight);
        sampledAnyLine = sampledAnyLine || sampled;
        lineYs.append(sampled ? sampledY : fallbackYForIndex(index));
        lineHeights.append(sampled ? sampledHeight : m_fallbackLineHeight);
        sampledLineGeometry.append(sampled);
    }

    if (sampledAnyLine)
    {
        for (int index = 1; index < count; ++index)
        {
            if (!sampledLineGeometry.value(index)
                || lineYs.value(index) <= lineYs.value(index - 1))
            {
                lineYs[index] = lineYs.value(index - 1)
                    + std::max<qreal>(1.0, lineHeights.value(index - 1, m_fallbackLineHeight));
            }
        }

        for (int index = 0; index + 1 < count; ++index)
        {
            const qreal nextY = lineYs.value(index + 1);
            const qreal currentY = lineYs.value(index);
            if (nextY > currentY)
            {
                lineHeights[index] = std::max<qreal>(1.0, nextY - currentY);
            }
        }
    }
    else
    {
        qreal cursorY = m_fallbackTopInset;
        for (int index = 0; index < count; ++index)
        {
            lineYs[index] = cursorY;
            cursorY += lineHeights.value(index, m_fallbackLineHeight);
        }
    }

    QVariantList nextEntries;
    nextEntries.reserve(count);
    for (int index = 0; index < count; ++index)
    {
        QVariantMap entry;
        entry.insert(QStringLiteral("lineNumber"), index + m_lineNumberBaseOffset);
        entry.insert(QStringLiteral("y"), lineYs.value(index, fallbackYForIndex(index)));
        entry.insert(QStringLiteral("height"), lineHeights.value(index, m_fallbackLineHeight));
        const LineSlot slot = lineSlots.value(index);
        entry.insert(QStringLiteral("sourceStart"), slot.sourceStart);
        entry.insert(QStringLiteral("sourceEnd"), slot.sourceEnd);
        entry.insert(QStringLiteral("blockType"), slot.blockType);
        nextEntries.append(entry);
    }

    if (m_lineNumberEntries == nextEntries)
    {
        return;
    }

    m_lineNumberEntries = nextEntries;
    emit lineNumberEntriesChanged();
}
