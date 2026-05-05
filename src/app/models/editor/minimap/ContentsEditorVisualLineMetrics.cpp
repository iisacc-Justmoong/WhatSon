#include "app/models/editor/minimap/ContentsEditorVisualLineMetrics.hpp"

#include <QMetaObject>
#include <QRectF>
#include <QVariant>

#include <algorithm>
#include <cmath>

namespace
{
    qreal finiteOrFallback(const qreal value, const qreal fallback) noexcept
    {
        return std::isfinite(value) ? value : fallback;
    }

    bool qrealEquals(const qreal left, const qreal right) noexcept
    {
        return qFuzzyCompare(left + 1.0, right + 1.0);
    }

    int invokePositionAt(QObject* object, const qreal x, const qreal y)
    {
        if (object == nullptr)
        {
            return 0;
        }

        int position = 0;
        if (QMetaObject::invokeMethod(
                object,
                "positionAt",
                Q_RETURN_ARG(int, position),
                Q_ARG(qreal, x),
                Q_ARG(qreal, y)))
        {
            return std::max(0, position);
        }

        QVariant positionValue;
        if (QMetaObject::invokeMethod(
                object,
                "positionAt",
                Q_RETURN_ARG(QVariant, positionValue),
                Q_ARG(QVariant, QVariant(x)),
                Q_ARG(QVariant, QVariant(y))))
        {
            return std::max(0, positionValue.toInt());
        }

        return 0;
    }

    QRectF invokePositionToRectangle(QObject* object, const int position, const qreal lineHeight)
    {
        if (object == nullptr)
        {
            return QRectF(0.0, 0.0, 0.0, std::max<qreal>(1.0, lineHeight));
        }

        QRectF rectangle;
        if (QMetaObject::invokeMethod(
                object,
                "positionToRectangle",
                Q_RETURN_ARG(QRectF, rectangle),
                Q_ARG(int, std::max(0, position))))
        {
            return rectangle;
        }

        QVariant rectangleValue;
        if (QMetaObject::invokeMethod(
                object,
                "positionToRectangle",
                Q_RETURN_ARG(QVariant, rectangleValue),
                Q_ARG(QVariant, QVariant(std::max(0, position)))))
        {
            return rectangleValue.toRectF();
        }

        return QRectF(0.0, 0.0, 0.0, std::max<qreal>(1.0, lineHeight));
    }
} // namespace

ContentsEditorVisualLineMetrics::ContentsEditorVisualLineMetrics(QObject* parent)
    : QObject(parent)
{
}

ContentsEditorVisualLineMetrics::~ContentsEditorVisualLineMetrics() = default;

QObject* ContentsEditorVisualLineMetrics::textItem() const noexcept
{
    return m_textItem.data();
}

void ContentsEditorVisualLineMetrics::setTextItem(QObject* value)
{
    if (m_textItem == value)
    {
        return;
    }
    m_textItem = value;
    emitInputChanged(&ContentsEditorVisualLineMetrics::textItemChanged);
}

int ContentsEditorVisualLineMetrics::textLineCount() const noexcept
{
    return m_textLineCount;
}

void ContentsEditorVisualLineMetrics::setTextLineCount(const int value)
{
    const int nextValue = std::max(0, value);
    if (m_textLineCount == nextValue)
    {
        return;
    }
    m_textLineCount = nextValue;
    emitInputChanged(&ContentsEditorVisualLineMetrics::textLineCountChanged);
}

qreal ContentsEditorVisualLineMetrics::textContentHeight() const noexcept
{
    return m_textContentHeight;
}

void ContentsEditorVisualLineMetrics::setTextContentHeight(const qreal value)
{
    const qreal nextValue = std::max<qreal>(0.0, finiteOrFallback(value, 0.0));
    if (qrealEquals(m_textContentHeight, nextValue))
    {
        return;
    }
    m_textContentHeight = nextValue;
    emitInputChanged(&ContentsEditorVisualLineMetrics::textContentHeightChanged);
}

qreal ContentsEditorVisualLineMetrics::textWidth() const noexcept
{
    return m_textWidth;
}

void ContentsEditorVisualLineMetrics::setTextWidth(const qreal value)
{
    const qreal nextValue = std::max<qreal>(0.0, finiteOrFallback(value, 0.0));
    if (qrealEquals(m_textWidth, nextValue))
    {
        return;
    }
    m_textWidth = nextValue;
    emitInputChanged(&ContentsEditorVisualLineMetrics::textWidthChanged);
}

qreal ContentsEditorVisualLineMetrics::fallbackWidth() const noexcept
{
    return m_fallbackWidth;
}

void ContentsEditorVisualLineMetrics::setFallbackWidth(const qreal value)
{
    const qreal nextValue = std::max<qreal>(1.0, finiteOrFallback(value, 1.0));
    if (qrealEquals(m_fallbackWidth, nextValue))
    {
        return;
    }
    m_fallbackWidth = nextValue;
    emitInputChanged(&ContentsEditorVisualLineMetrics::fallbackWidthChanged);
}

qreal ContentsEditorVisualLineMetrics::lineHeight() const noexcept
{
    return m_lineHeight;
}

void ContentsEditorVisualLineMetrics::setLineHeight(const qreal value)
{
    const qreal nextValue = std::max<qreal>(1.0, finiteOrFallback(value, 1.0));
    if (qrealEquals(m_lineHeight, nextValue))
    {
        return;
    }
    m_lineHeight = nextValue;
    emitInputChanged(&ContentsEditorVisualLineMetrics::lineHeightChanged);
}

qreal ContentsEditorVisualLineMetrics::strokeThin() const noexcept
{
    return m_strokeThin;
}

void ContentsEditorVisualLineMetrics::setStrokeThin(const qreal value)
{
    const qreal nextValue = std::max<qreal>(1.0, finiteOrFallback(value, 1.0));
    if (qrealEquals(m_strokeThin, nextValue))
    {
        return;
    }
    m_strokeThin = nextValue;
    emitInputChanged(&ContentsEditorVisualLineMetrics::strokeThinChanged);
}

int ContentsEditorVisualLineMetrics::visualLineCount() const
{
    const int lineCountRows =
        m_textLineCount > 0 ? std::max(1, m_textLineCount) : 1;
    const qreal resolvedHeight = std::max<qreal>(0.0, m_textContentHeight);
    const qreal resolvedHeightRows =
        resolvedHeight > 0.0
            ? std::ceil(resolvedHeight / resolvedLineHeight())
            : 1.0;
    return std::max(lineCountRows, static_cast<int>(std::max<qreal>(1.0, resolvedHeightRows)));
}

QVariantList ContentsEditorVisualLineMetrics::visualLineWidthRatios() const
{
    const int rowCount = visualLineCount();
    QVariantList ratios;
    ratios.reserve(rowCount);

    const qreal itemWidth = resolvedTextWidth();
    const qreal lineHeight = resolvedLineHeight();
    const int measuredLineRows =
        m_textLineCount > 0 ? std::max(1, m_textLineCount) : rowCount;
    const qreal contentHeight = std::max(lineHeight, m_textContentHeight > 0.0 ? m_textContentHeight : lineHeight);
    const qreal minimumRatio = std::min<qreal>(1.0, std::max<qreal>(0.0, m_strokeThin / itemWidth));

    for (int rowIndex = 0; rowIndex < rowCount; ++rowIndex)
    {
        if (m_textItem == nullptr || rowIndex >= measuredLineRows)
        {
            ratios.push_back(1.0);
            continue;
        }

        const qreal probeY =
            std::max<qreal>(0.0, std::min(contentHeight - 1.0, rowIndex * lineHeight + lineHeight / 2.0));
        const int startOffset = invokePositionAt(m_textItem.data(), 0.0, probeY);
        const int endOffset = invokePositionAt(m_textItem.data(), std::max<qreal>(0.0, itemWidth - 1.0), probeY);
        const QRectF startRectangle = invokePositionToRectangle(m_textItem.data(), startOffset, lineHeight);
        const QRectF endRectangle = invokePositionToRectangle(m_textItem.data(), endOffset, lineHeight);
        const qreal rawWidth = std::abs(endRectangle.x() - startRectangle.x());
        ratios.push_back(std::max(minimumRatio, std::min<qreal>(1.0, rawWidth / itemWidth)));
    }

    return ratios;
}

void ContentsEditorVisualLineMetrics::requestMetricsRefresh()
{
    emit metricsChanged();
}

void ContentsEditorVisualLineMetrics::emitInputChanged(void (ContentsEditorVisualLineMetrics::*signal)())
{
    emit (this->*signal)();
    emit metricsChanged();
}

qreal ContentsEditorVisualLineMetrics::resolvedTextWidth() const noexcept
{
    if (m_textWidth > 0.0)
    {
        return m_textWidth;
    }
    return std::max<qreal>(1.0, m_fallbackWidth);
}

qreal ContentsEditorVisualLineMetrics::resolvedLineHeight() const noexcept
{
    return std::max<qreal>(1.0, m_lineHeight);
}
