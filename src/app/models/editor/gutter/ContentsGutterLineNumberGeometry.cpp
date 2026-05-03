#include "app/models/editor/gutter/ContentsGutterLineNumberGeometry.hpp"

#include "app/models/editor/resource/ContentsEditorDynamicObjectSupport.hpp"

#include <QPointF>
#include <QRectF>
#include <QVariantMap>

#include <algorithm>

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

    QVariantList nextEntries;
    nextEntries.reserve(count);
    for (int index = 0; index < count; ++index)
    {
        const int displayOffset = index < lineStartOffsets.size()
            ? lineStartOffsets.at(index)
            : terminalDisplayOffset;
        const int sourceOffset = sourceOffsetForLogicalOffset(displayOffset);
        QVariantMap entry;
        entry.insert(QStringLiteral("lineNumber"), index + m_lineNumberBaseOffset);
        entry.insert(QStringLiteral("y"), editorLineYForOffset(displayOffset, sourceOffset, index));
        nextEntries.append(entry);
    }

    if (m_lineNumberEntries == nextEntries)
    {
        return;
    }

    m_lineNumberEntries = nextEntries;
    emit lineNumberEntriesChanged();
}
