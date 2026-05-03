#include "app/models/editor/gutter/ContentsGutterLineNumberGeometry.hpp"

#include <QStringList>
#include <QVariantMap>

#include <algorithm>
#include <utility>

namespace
{
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

    int logicalLineCountHintForBlock(const QVariantMap& block)
    {
        const int explicitHint = intFromMap(block, QStringLiteral("logicalLineCountHint"), 0);
        if (explicitHint > 0)
        {
            return explicitHint;
        }

        const QString sourceText = block.value(QStringLiteral("sourceText")).toString();
        return std::max(1, static_cast<int>(sourceText.count(QLatin1Char('\n'))) + 1);
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

    int expandedBlockRowCount(const QVariantMap& block)
    {
        const QVariantList groupedBlocks = block.value(QStringLiteral("groupedBlocks")).toList();
        if (block.value(QStringLiteral("flattenedInteractiveGroup")).toBool() && !groupedBlocks.isEmpty())
        {
            return groupedBlocks.size();
        }

        return std::max(1, logicalLineCountHintForBlock(block));
    }

    int blockStreamRowCount(const QVariantList& entries)
    {
        int count = 0;
        for (const QVariant& entryValue : uniqueBlockEntries(entries))
        {
            count += expandedBlockRowCount(entryValue.toMap());
        }
        return count;
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
            && blockSourceEnd(existing) >= blockSourceEnd(candidate)
            && expandedBlockRowCount(existing) >= expandedBlockRowCount(candidate);
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

    bool resourceEntryRendersAsTallImageFrame(const QVariantMap& entry)
    {
        const QString renderMode =
            entry.value(QStringLiteral("renderMode")).toString().trimmed().toCaseFolded();
        if (renderMode == QStringLiteral("image"))
        {
            return true;
        }

        return entry.value(QStringLiteral("imageWidth")).toInt() > 0
            && entry.value(QStringLiteral("imageHeight")).toInt() > 0;
    }

    void appendUniqueLineIndex(QList<int>* lineIndices, const int lineIndex)
    {
        if (lineIndices == nullptr || lineIndex < 0 || lineIndices->contains(lineIndex))
        {
            return;
        }
        lineIndices->append(lineIndex);
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
            const ContentsGutterLineNumberGeometry::LineSlot slot = lineSlots.at(index);
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

QVariantList ContentsGutterLineNumberGeometry::effectiveBlockStream() const
{
    const QVariantList uniqueDocumentBlocks = uniqueBlockEntries(m_documentBlocks);
    const QVariantList uniqueDisplayBlocks = uniqueBlockEntries(m_displayBlocks);
    if (uniqueDocumentBlocks.isEmpty())
    {
        return uniqueDisplayBlocks;
    }
    if (uniqueDisplayBlocks.isEmpty())
    {
        return uniqueDocumentBlocks;
    }

    return blockStreamRowCount(uniqueDisplayBlocks) >= blockStreamRowCount(uniqueDocumentBlocks)
        ? mergeBlockStreams(uniqueDisplayBlocks, uniqueDocumentBlocks)
        : mergeBlockStreams(uniqueDocumentBlocks, uniqueDisplayBlocks);
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
        lineSlots.append(LineSlot{QStringLiteral("raw"), lineEnd, lineStart, false});
    }

    return lineSlots.isEmpty() ? QList<LineSlot>{LineSlot{QStringLiteral("raw"), 0, 0, false}} : lineSlots;
}

QList<ContentsGutterLineNumberGeometry::LineSlot> ContentsGutterLineNumberGeometry::blockLineSlots() const
{
    QList<LineSlot> lineSlots;
    const QVariantList blocks = effectiveBlockStream();
    if (blocks.isEmpty())
    {
        return rawLineSlots();
    }

    const auto appendBlock = [this, &lineSlots](const QVariantMap& block)
    {
        const QString type = normalizedBlockType(block);
        const bool resourceBlock = type == QStringLiteral("resource");
        const int sourceStart = std::clamp(blockSourceStart(block), 0, static_cast<int>(m_sourceText.size()));
        const int sourceEnd = std::clamp(blockSourceEnd(block), sourceStart, static_cast<int>(m_sourceText.size()));
        const QString blockText = block.value(QStringLiteral("sourceText")).toString();
        const int logicalLineCount = std::max(1, logicalLineCountHintForBlock(block));

        if (resourceBlock || logicalLineCount == 1)
        {
            lineSlots.append(LineSlot{type, sourceEnd, sourceStart, resourceBlock});
            return;
        }

        int segmentStart = sourceStart;
        const QString sourceTextForSplit = blockText.isEmpty()
            ? m_sourceText.mid(sourceStart, sourceEnd - sourceStart)
            : blockText;
        const QStringList segments = sourceTextForSplit.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
        const int segmentCount = std::max(logicalLineCount, static_cast<int>(segments.size()));
        for (int segmentIndex = 0; segmentIndex < segmentCount; ++segmentIndex)
        {
            const int segmentLength = segmentIndex < segments.size()
                ? static_cast<int>(segments.at(segmentIndex).size())
                : 0;
            const int segmentEnd = std::clamp(segmentStart + segmentLength, segmentStart, sourceEnd);
            lineSlots.append(LineSlot{type, segmentEnd, segmentStart, false});
            segmentStart = std::clamp(segmentEnd + 1, segmentEnd, sourceEnd);
        }
    };

    for (const QVariant& blockValue : blocks)
    {
        const QVariantMap block = blockValue.toMap();
        if (block.isEmpty())
        {
            continue;
        }

        const QVariantList groupedBlocks = block.value(QStringLiteral("groupedBlocks")).toList();
        if (block.value(QStringLiteral("flattenedInteractiveGroup")).toBool() && !groupedBlocks.isEmpty())
        {
            for (const QVariant& groupedValue : groupedBlocks)
            {
                const QVariantMap groupedBlock = groupedValue.toMap();
                if (!groupedBlock.isEmpty())
                {
                    appendBlock(groupedBlock);
                }
            }
            continue;
        }

        appendBlock(block);
    }

    return lineSlots.isEmpty() ? rawLineSlots() : lineSlots;
}

QList<int> ContentsGutterLineNumberGeometry::resourceLineIndices(
    const QList<LineSlot>& lineSlots) const
{
    QList<int> lineIndices;
    for (int index = 0; index < lineSlots.size(); ++index)
    {
        if (lineSlots.at(index).resource)
        {
            lineIndices.append(index);
        }
    }
    return lineIndices;
}

QList<int> ContentsGutterLineNumberGeometry::renderedResourceLineIndices(
    const QList<LineSlot>& lineSlots) const
{
    QList<int> lineIndices;
    if (m_renderedResources.isEmpty() || lineSlots.isEmpty())
    {
        return lineIndices;
    }

    const int sourceLength = static_cast<int>(m_sourceText.size());
    for (const QVariant& resourceValue : m_renderedResources)
    {
        const QVariantMap resource = resourceValue.toMap();
        if (!resourceEntryRendersAsTallImageFrame(resource))
        {
            continue;
        }

        int sourceStart = intFromMap(resource, QStringLiteral("sourceStart"), -1);
        int sourceEnd = intFromMap(resource, QStringLiteral("sourceEnd"), -1);
        if (sourceStart < 0 || sourceEnd <= sourceStart)
        {
            const int resourceIndex = intFromMap(resource, QStringLiteral("index"), -1);
            for (const QVariant& blockValue : effectiveBlockStream())
            {
                const QVariantMap block = blockValue.toMap();
                const QString blockType = normalizedBlockType(block);
                if (blockType != QStringLiteral("resource")
                    || intFromMap(block, QStringLiteral("resourceIndex"), -1) != resourceIndex)
                {
                    continue;
                }

                sourceStart = intFromMap(block, QStringLiteral("sourceStart"), -1);
                sourceEnd = intFromMap(block, QStringLiteral("sourceEnd"), -1);
                break;
            }
        }

        appendUniqueLineIndex(
            &lineIndices,
            sourceStart >= 0 && sourceEnd > sourceStart
                ? lineIndexForSourceSpan(
                    lineSlots,
                    sourceLength,
                    sourceStart,
                    sourceEnd)
                : -1);
    }

    return lineIndices;
}

QList<int> ContentsGutterLineNumberGeometry::extraHeightTargetLineIndices(
    const QList<LineSlot>& lineSlots) const
{
    QList<int> targetLineIndices = renderedResourceLineIndices(lineSlots);
    if (!targetLineIndices.isEmpty())
    {
        return targetLineIndices;
    }

    if (!m_renderedResources.isEmpty())
    {
        return targetLineIndices;
    }

    return resourceLineIndices(lineSlots);
}

void ContentsGutterLineNumberGeometry::rebuildLineNumberEntries()
{
    const QList<LineSlot> lineSlots = blockLineSlots();
    const int count = std::max(1, static_cast<int>(lineSlots.size()));

    QList<qreal> lineHeights;
    lineHeights.reserve(count);
    for (int index = 0; index < count; ++index)
    {
        lineHeights.append(m_fallbackLineHeight);
    }

    if (!lineHeights.isEmpty() && m_editorContentHeight > 0.0)
    {
        qreal baseContentHeight = 0.0;
        for (const qreal height : std::as_const(lineHeights))
        {
            baseContentHeight += height;
        }

        const qreal extraContentHeight = m_editorContentHeight - baseContentHeight;
        if (extraContentHeight > 1.0)
        {
            QList<int> targetLineIndices = extraHeightTargetLineIndices(lineSlots);
            if (targetLineIndices.isEmpty() && !lineHeights.isEmpty())
            {
                targetLineIndices.append(lineHeights.size() - 1);
            }

            const int targetLineCount = std::max(1, static_cast<int>(targetLineIndices.size()));
            const qreal extraPerLine =
                extraContentHeight / static_cast<qreal>(targetLineCount);
            for (const int lineIndex : std::as_const(targetLineIndices))
            {
                if (lineIndex >= 0 && lineIndex < lineHeights.size())
                {
                    lineHeights[lineIndex] += extraPerLine;
                }
            }
        }
    }

    QList<qreal> lineYs;
    lineYs.reserve(count);
    qreal cursorY = m_fallbackTopInset;
    for (int index = 0; index < count; ++index)
    {
        lineYs.append(cursorY);
        cursorY += lineHeights.value(index, m_fallbackLineHeight);
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
