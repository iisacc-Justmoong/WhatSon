#include "app/models/editor/gutter/ContentsGutterLineNumberGeometry.hpp"

#include "app/models/editor/resource/ContentsEditorDynamicObjectSupport.hpp"

#include <QPointF>
#include <QRectF>
#include <QVariantMap>

#include <algorithm>
#include <utility>

namespace
{
    qreal realFromMap(const QVariant& value, const QString& key, const qreal fallback)
    {
        if (value.canConvert<QRectF>())
        {
            const QRectF rectangle = value.toRectF();
            if (key == QLatin1String("x"))
            {
                return rectangle.x();
            }
            if (key == QLatin1String("y"))
            {
                return rectangle.y();
            }
            if (key == QLatin1String("width"))
            {
                return rectangle.width();
            }
            if (key == QLatin1String("height"))
            {
                return rectangle.height();
            }
        }

        if (value.canConvert<QPointF>())
        {
            const QPointF point = value.toPointF();
            if (key == QLatin1String("x"))
            {
                return point.x();
            }
            if (key == QLatin1String("y"))
            {
                return point.y();
            }
        }

        const QVariantMap map = value.toMap();
        if (map.isEmpty())
        {
            return fallback;
        }

        bool ok = false;
        const qreal resolvedValue = map.value(key).toReal(&ok);
        return ok ? resolvedValue : fallback;
    }

    int intFromMap(const QVariantMap& map, const QString& key, const int fallback)
    {
        bool ok = false;
        const int value = map.value(key).toInt(&ok);
        return ok ? value : fallback;
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

    int lineIndexForSourceSpan(
        const QList<int>& sourceLineStartOffsets,
        const int sourceLength,
        const int sourceStart,
        const int sourceEnd)
    {
        const int blockStart = std::clamp(sourceStart, 0, sourceLength);
        const int blockEnd = std::clamp(std::max(blockStart, sourceEnd), blockStart, sourceLength);
        for (int index = 0; index < sourceLineStartOffsets.size(); ++index)
        {
            const int lineStart = std::clamp(sourceLineStartOffsets.at(index), 0, sourceLength);
            const int nextLineStart = index + 1 < sourceLineStartOffsets.size()
                ? std::clamp(sourceLineStartOffsets.at(index + 1), lineStart, sourceLength)
                : sourceLength + 1;
            const bool startsInsideLine = blockStart >= lineStart && blockStart < nextLineStart;
            const bool coversLineStart = blockStart <= lineStart && blockEnd > lineStart;
            if (startsInsideLine || coversLineStart)
            {
                return index;
            }
        }
        return -1;
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

int ContentsGutterLineNumberGeometry::effectiveLineNumberCount() const noexcept
{
    return std::max(1, m_lineNumberCount);
}

QList<int> ContentsGutterLineNumberGeometry::displayLineStartOffsets() const
{
    if (!m_logicalLineStartOffsets.isEmpty())
    {
        QList<int> offsets;
        offsets.reserve(m_logicalLineStartOffsets.size());
        for (const QVariant& displayOffsetValue : m_logicalLineStartOffsets)
        {
            bool ok = false;
            const int displayOffset = displayOffsetValue.toInt(&ok);
            offsets.append(std::max(0, ok ? displayOffset : 0));
        }
        return offsets;
    }

    QList<int> offsets;
    offsets.reserve(effectiveLineNumberCount());
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

int ContentsGutterLineNumberGeometry::sourceOffsetForLogicalOffset(const int logicalOffset) const noexcept
{
    const int sourceLength = static_cast<int>(m_sourceText.size());
    if (logicalOffset >= 0 && logicalOffset < m_logicalToSourceOffsets.size())
    {
        bool ok = false;
        const int sourceOffset = m_logicalToSourceOffsets.at(logicalOffset).toInt(&ok);
        if (ok)
        {
            return std::clamp(sourceOffset, 0, sourceLength);
        }
    }

    return std::clamp(logicalOffset, 0, sourceLength);
}

qreal ContentsGutterLineNumberGeometry::fallbackYForIndex(const int index) const noexcept
{
    return m_fallbackTopInset + (static_cast<qreal>(std::max(0, index)) * m_fallbackLineHeight);
}

QList<int> ContentsGutterLineNumberGeometry::resourceLineIndices(
    const QList<int>& sourceLineStartOffsets) const
{
    QList<int> lineIndices;
    if (m_documentBlocks.isEmpty() || sourceLineStartOffsets.isEmpty())
    {
        return lineIndices;
    }

    const int sourceLength = static_cast<int>(m_sourceText.size());
    for (const QVariant& blockValue : m_documentBlocks)
    {
        const QVariantMap block = blockValue.toMap();
        const QString blockType = block.value(QStringLiteral("type")).toString().trimmed().toCaseFolded();
        if (blockType != QStringLiteral("resource"))
        {
            continue;
        }

        appendUniqueLineIndex(
            &lineIndices,
            lineIndexForSourceSpan(
                sourceLineStartOffsets,
                sourceLength,
                block.value(QStringLiteral("sourceStart")).toInt(),
                block.value(QStringLiteral("sourceEnd")).toInt()));
    }

    return lineIndices;
}

QList<int> ContentsGutterLineNumberGeometry::renderedResourceLineIndices(
    const QList<int>& sourceLineStartOffsets) const
{
    QList<int> lineIndices;
    if (m_renderedResources.isEmpty() || sourceLineStartOffsets.isEmpty())
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
            for (const QVariant& blockValue : m_documentBlocks)
            {
                const QVariantMap block = blockValue.toMap();
                const QString blockType =
                    block.value(QStringLiteral("type")).toString().trimmed().toCaseFolded();
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
                    sourceLineStartOffsets,
                    sourceLength,
                    sourceStart,
                    sourceEnd)
                : -1);
    }

    return lineIndices;
}

QList<int> ContentsGutterLineNumberGeometry::extraHeightTargetLineIndices(
    const QList<int>& sourceLineStartOffsets) const
{
    QList<int> targetLineIndices = renderedResourceLineIndices(sourceLineStartOffsets);
    if (!targetLineIndices.isEmpty())
    {
        return targetLineIndices;
    }

    if (!m_renderedResources.isEmpty())
    {
        return targetLineIndices;
    }

    return resourceLineIndices(sourceLineStartOffsets);
}

qreal ContentsGutterLineNumberGeometry::editorLineYForOffset(
    const int displayOffset,
    const int sourceOffset,
    const int fallbackIndex) const
{
    if (!m_editorGeometryHost)
    {
        return fallbackYForIndex(fallbackIndex);
    }

    QVariant rawRectangle = WhatSon::Editor::DynamicObjectSupport::invokeVariant(
        m_editorGeometryHost,
        "lineStartRectangle",
        {QVariant(displayOffset), QVariant(sourceOffset)});
    if (!rawRectangle.isValid())
    {
        rawRectangle = WhatSon::Editor::DynamicObjectSupport::invokeVariant(
            m_editorGeometryHost,
            "lineStartRectangle",
            {QVariant(displayOffset)});
    }
    const qreal rawY = realFromMap(rawRectangle, QStringLiteral("y"), fallbackYForIndex(fallbackIndex));

    if (!m_mapTarget)
    {
        return rawY;
    }

    const QVariant mappedPoint = WhatSon::Editor::DynamicObjectSupport::invokeVariant(
        m_editorGeometryHost,
        "mapEditorPointToItem",
        {QVariant::fromValue(m_mapTarget), QVariant(0.0), QVariant(rawY)});
    return realFromMap(mappedPoint, QStringLiteral("y"), rawY);
}

void ContentsGutterLineNumberGeometry::rebuildLineNumberEntries()
{
    const QList<int> lineStartOffsets = displayLineStartOffsets();
    const int terminalDisplayOffset = lineStartOffsets.isEmpty() ? 0 : lineStartOffsets.last();
    const int count = effectiveLineNumberCount();

    QList<qreal> lineYs;
    lineYs.reserve(count);
    QList<int> sourceLineStartOffsets;
    sourceLineStartOffsets.reserve(count);
    qreal previousY = 0.0;
    bool hasPreviousY = false;
    for (int index = 0; index < count; ++index)
    {
        const int displayOffset = index < lineStartOffsets.size()
            ? lineStartOffsets.at(index)
            : terminalDisplayOffset;
        const int sourceOffset = sourceOffsetForLogicalOffset(displayOffset);
        sourceLineStartOffsets.append(sourceOffset);
        qreal y = editorLineYForOffset(displayOffset, sourceOffset, index);
        if (hasPreviousY && y <= previousY)
        {
            y = previousY + m_fallbackLineHeight;
        }

        lineYs.append(y);
        previousY = y;
        hasPreviousY = true;
    }

    QList<qreal> lineHeights;
    lineHeights.reserve(count);
    for (int index = 0; index < count; ++index)
    {
        const qreal height = index + 1 < lineYs.size()
            ? std::max<qreal>(1.0, lineYs.at(index + 1) - lineYs.at(index))
            : m_fallbackLineHeight;
        lineHeights.append(height);
    }

    if (!lineYs.isEmpty() && m_editorContentHeight > 0.0)
    {
        qreal baseContentHeight = 0.0;
        for (const qreal height : std::as_const(lineHeights))
        {
            baseContentHeight += height;
        }

        const qreal extraContentHeight = m_editorContentHeight - baseContentHeight;
        if (extraContentHeight > 1.0)
        {
            QList<int> targetLineIndices = extraHeightTargetLineIndices(sourceLineStartOffsets);
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

            for (int index = 1; index < lineYs.size(); ++index)
            {
                lineYs[index] = lineYs.at(index - 1) + lineHeights.at(index - 1);
            }
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
        nextEntries.append(entry);
    }

    if (m_lineNumberEntries == nextEntries)
    {
        return;
    }

    m_lineNumberEntries = nextEntries;
    emit lineNumberEntriesChanged();
}
