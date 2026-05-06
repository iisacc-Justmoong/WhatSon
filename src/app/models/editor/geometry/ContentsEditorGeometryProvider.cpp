#include "app/models/editor/geometry/ContentsEditorGeometryProvider.hpp"

#include <QMetaObject>
#include <QQuickItem>
#include <QVariant>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace
{
    int boundedOffset(const int offset, const int length) noexcept
    {
        return std::max(0, std::min(offset, std::max(0, length)));
    }

    int integerProperty(QObject* object, const char* propertyName, const int fallback) noexcept
    {
        bool ok = false;
        const int value = object != nullptr ? object->property(propertyName).toInt(&ok) : fallback;
        return ok ? value : fallback;
    }

    qreal numericProperty(QObject* object, const char* propertyName, const qreal fallback) noexcept
    {
        bool ok = false;
        const qreal value = object != nullptr ? object->property(propertyName).toReal(&ok) : fallback;
        return ok && std::isfinite(value) ? value : fallback;
    }

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

    ContentsEditorGeometryMeasurement positionRectangle(
        QObject* object,
        const int position,
        const qreal fallbackLineHeight)
    {
        if (object == nullptr)
        {
            return {
                QRectF(0.0, 0.0, 0.0, std::max<qreal>(1.0, fallbackLineHeight)),
                false,
            };
        }

        QRectF rectangle;
        if (QMetaObject::invokeMethod(
                object,
                "positionToRectangle",
                Q_RETURN_ARG(QRectF, rectangle),
                Q_ARG(int, std::max(0, position))))
        {
            return {rectangle, true};
        }

        QVariant rectangleValue;
        if (QMetaObject::invokeMethod(
                object,
                "positionToRectangle",
                Q_RETURN_ARG(QVariant, rectangleValue),
                Q_ARG(QVariant, QVariant(std::max(0, position)))))
        {
            return {rectangleValue.toRectF(), true};
        }

        return {
            QRectF(0.0, 0.0, 0.0, std::max<qreal>(1.0, fallbackLineHeight)),
            false,
        };
    }

    QPointF mapPoint(QObject* geometryObject, QObject* targetObject, const QPointF& point)
    {
        auto* geometryItem = qobject_cast<QQuickItem*>(geometryObject);
        auto* targetItem = qobject_cast<QQuickItem*>(targetObject);
        if (geometryItem != nullptr)
        {
            return geometryItem->mapToItem(targetItem, point);
        }
        return point;
    }

    qreal nextAvailableRowTop(
        const std::vector<QRectF>& rectangles,
        const std::vector<bool>& geometryAvailable,
        const int rowIndex)
    {
        if (rowIndex < 0 || rowIndex >= static_cast<int>(rectangles.size()))
        {
            return std::numeric_limits<qreal>::quiet_NaN();
        }

        const qreal currentY = rectangles.at(rowIndex).y();
        for (int index = rowIndex + 1; index < static_cast<int>(rectangles.size()); ++index)
        {
            if (!geometryAvailable.at(index))
            {
                continue;
            }

            const qreal candidateY = rectangles.at(index).y();
            if (std::isfinite(candidateY) && candidateY > currentY)
            {
                return candidateY;
            }
        }
        return std::numeric_limits<qreal>::quiet_NaN();
    }
} // namespace

ContentsEditorGeometryProvider::ContentsEditorGeometryProvider(QObject* parent)
    : QObject(parent)
{
}

ContentsEditorGeometryProvider::~ContentsEditorGeometryProvider() = default;

QObject* ContentsEditorGeometryProvider::textItem() const noexcept
{
    return m_textItem.data();
}

void ContentsEditorGeometryProvider::setTextItem(QObject* value)
{
    if (m_textItem == value)
    {
        return;
    }
    m_textItem = value;
    emitGeometryInputChanged(&ContentsEditorGeometryProvider::textItemChanged);
}

QObject* ContentsEditorGeometryProvider::resourceItem() const noexcept
{
    return m_resourceItem.data();
}

void ContentsEditorGeometryProvider::setResourceItem(QObject* value)
{
    if (m_resourceItem == value)
    {
        return;
    }
    m_resourceItem = value;
    emitGeometryInputChanged(&ContentsEditorGeometryProvider::resourceItemChanged);
}

QObject* ContentsEditorGeometryProvider::targetItem() const noexcept
{
    return m_targetItem.data();
}

void ContentsEditorGeometryProvider::setTargetItem(QObject* value)
{
    if (m_targetItem == value)
    {
        return;
    }
    m_targetItem = value;
    emitGeometryInputChanged(&ContentsEditorGeometryProvider::targetItemChanged);
}

QObject* ContentsEditorGeometryProvider::visualItem() const noexcept
{
    return m_visualItem.data();
}

void ContentsEditorGeometryProvider::setVisualItem(QObject* value)
{
    if (m_visualItem == value)
    {
        return;
    }
    m_visualItem = value;
    emitGeometryInputChanged(&ContentsEditorGeometryProvider::visualItemChanged);
}

QVariantList ContentsEditorGeometryProvider::lineNumberRanges() const
{
    return m_lineNumberRanges;
}

void ContentsEditorGeometryProvider::setLineNumberRanges(const QVariantList& value)
{
    if (m_lineNumberRanges == value)
    {
        return;
    }
    m_lineNumberRanges = value;
    emitGeometryInputChanged(&ContentsEditorGeometryProvider::lineNumberRangesChanged);
}

int ContentsEditorGeometryProvider::logicalLength() const noexcept
{
    return m_logicalLength;
}

void ContentsEditorGeometryProvider::setLogicalLength(const int value)
{
    const int nextValue = std::max(0, value);
    if (m_logicalLength == nextValue)
    {
        return;
    }
    m_logicalLength = nextValue;
    emitGeometryInputChanged(&ContentsEditorGeometryProvider::logicalLengthChanged);
}

qreal ContentsEditorGeometryProvider::fallbackLineHeight() const noexcept
{
    return m_fallbackLineHeight;
}

void ContentsEditorGeometryProvider::setFallbackLineHeight(const qreal value)
{
    const qreal nextValue = std::max<qreal>(1.0, finiteOrFallback(value, 1.0));
    if (qrealEquals(m_fallbackLineHeight, nextValue))
    {
        return;
    }
    m_fallbackLineHeight = nextValue;
    emitGeometryInputChanged(&ContentsEditorGeometryProvider::fallbackLineHeightChanged);
}

qreal ContentsEditorGeometryProvider::fallbackWidth() const noexcept
{
    return m_fallbackWidth;
}

void ContentsEditorGeometryProvider::setFallbackWidth(const qreal value)
{
    const qreal nextValue = std::max<qreal>(0.0, finiteOrFallback(value, 0.0));
    if (qrealEquals(m_fallbackWidth, nextValue))
    {
        return;
    }
    m_fallbackWidth = nextValue;
    emitGeometryInputChanged(&ContentsEditorGeometryProvider::fallbackWidthChanged);
}

int ContentsEditorGeometryProvider::visualTextLineCount() const noexcept
{
    return m_visualTextLineCount;
}

void ContentsEditorGeometryProvider::setVisualTextLineCount(const int value)
{
    const int nextValue = std::max(0, value);
    if (m_visualTextLineCount == nextValue)
    {
        return;
    }
    m_visualTextLineCount = nextValue;
    emitGeometryInputChanged(&ContentsEditorGeometryProvider::visualTextLineCountChanged);
}

qreal ContentsEditorGeometryProvider::visualTextContentHeight() const noexcept
{
    return m_visualTextContentHeight;
}

void ContentsEditorGeometryProvider::setVisualTextContentHeight(const qreal value)
{
    const qreal nextValue = std::max<qreal>(0.0, finiteOrFallback(value, 0.0));
    if (qrealEquals(m_visualTextContentHeight, nextValue))
    {
        return;
    }
    m_visualTextContentHeight = nextValue;
    emitGeometryInputChanged(&ContentsEditorGeometryProvider::visualTextContentHeightChanged);
}

qreal ContentsEditorGeometryProvider::visualTextWidth() const noexcept
{
    return m_visualTextWidth;
}

void ContentsEditorGeometryProvider::setVisualTextWidth(const qreal value)
{
    const qreal nextValue = std::max<qreal>(0.0, finiteOrFallback(value, 0.0));
    if (qrealEquals(m_visualTextWidth, nextValue))
    {
        return;
    }
    m_visualTextWidth = nextValue;
    emitGeometryInputChanged(&ContentsEditorGeometryProvider::visualTextWidthChanged);
}

qreal ContentsEditorGeometryProvider::visualLineHeight() const noexcept
{
    return m_visualLineHeight;
}

void ContentsEditorGeometryProvider::setVisualLineHeight(const qreal value)
{
    const qreal nextValue = std::max<qreal>(1.0, finiteOrFallback(value, 1.0));
    if (qrealEquals(m_visualLineHeight, nextValue))
    {
        return;
    }
    m_visualLineHeight = nextValue;
    emitGeometryInputChanged(&ContentsEditorGeometryProvider::visualLineHeightChanged);
}

qreal ContentsEditorGeometryProvider::visualStrokeThin() const noexcept
{
    return m_visualStrokeThin;
}

void ContentsEditorGeometryProvider::setVisualStrokeThin(const qreal value)
{
    const qreal nextValue = std::max<qreal>(1.0, finiteOrFallback(value, 1.0));
    if (qrealEquals(m_visualStrokeThin, nextValue))
    {
        return;
    }
    m_visualStrokeThin = nextValue;
    emitGeometryInputChanged(&ContentsEditorGeometryProvider::visualStrokeThinChanged);
}

int ContentsEditorGeometryProvider::visualLineCount() const
{
    const int lineCountRows =
        m_visualTextLineCount > 0 ? std::max(1, m_visualTextLineCount) : 1;
    const qreal heightRows =
        m_visualTextContentHeight > 0.0
            ? std::ceil(m_visualTextContentHeight / std::max<qreal>(1.0, m_visualLineHeight))
            : 1.0;
    return std::max(lineCountRows, static_cast<int>(std::max<qreal>(1.0, heightRows)));
}

QVariantList ContentsEditorGeometryProvider::visualLineWidthRatios() const
{
    const int rowCount = visualLineCount();
    QVariantList ratios;
    ratios.reserve(rowCount);

    const qreal itemWidth = std::max<qreal>(1.0, m_visualTextWidth > 0.0 ? m_visualTextWidth : m_fallbackWidth);
    const qreal lineHeight = std::max<qreal>(1.0, m_visualLineHeight);
    const qreal contentHeight =
        std::max(lineHeight, m_visualTextContentHeight > 0.0 ? m_visualTextContentHeight : lineHeight);
    const int measuredLineRows =
        m_visualTextLineCount > 0 ? std::max(1, m_visualTextLineCount) : rowCount;
    const qreal minimumRatio = std::min<qreal>(1.0, std::max<qreal>(0.0, m_visualStrokeThin / itemWidth));

    for (int rowIndex = 0; rowIndex < rowCount; ++rowIndex)
    {
        if (m_visualItem == nullptr || rowIndex >= measuredLineRows)
        {
            ratios.push_back(1.0);
            continue;
        }

        const qreal probeY =
            std::max<qreal>(0.0, std::min(contentHeight - 1.0, rowIndex * lineHeight + lineHeight / 2.0));
        const int startOffset = invokePositionAt(m_visualItem.data(), 0.0, probeY);
        const int endOffset = invokePositionAt(m_visualItem.data(), std::max<qreal>(0.0, itemWidth - 1.0), probeY);
        const ContentsEditorGeometryMeasurement startMeasurement =
            positionRectangle(m_visualItem.data(), startOffset, lineHeight);
        const ContentsEditorGeometryMeasurement endMeasurement =
            positionRectangle(m_visualItem.data(), endOffset, lineHeight);
        const qreal rawWidth =
            std::abs(endMeasurement.rectangle.x() - startMeasurement.rectangle.x());
        ratios.push_back(std::max(minimumRatio, std::min<qreal>(1.0, rawWidth / itemWidth)));
    }

    return ratios;
}

QVariantList ContentsEditorGeometryProvider::lineNumberGeometryRows() const
{
    std::vector<QRectF> rectangles;
    std::vector<bool> geometryAvailable;
    std::vector<bool> resourceRanges;
    rectangles.reserve(static_cast<std::size_t>(m_lineNumberRanges.size()));
    geometryAvailable.reserve(static_cast<std::size_t>(m_lineNumberRanges.size()));
    resourceRanges.reserve(static_cast<std::size_t>(m_lineNumberRanges.size()));

    for (const QVariant& rangeValue : m_lineNumberRanges)
    {
        const QVariantMap range = rangeValue.toMap();
        const bool resourceRange = range.value(QStringLiteral("resourceRange"), false).toBool();
        const int logicalStart = range.value(QStringLiteral("logicalStart")).toInt();
        const int logicalEnd = range.value(QStringLiteral("logicalEnd")).toInt();
        ContentsEditorGeometryMeasurement measurement = resourceRange
            ? measureResourceRange(logicalStart, logicalEnd, m_logicalLength, m_fallbackLineHeight, m_fallbackWidth)
            : measureTextRange(logicalStart, logicalEnd, m_logicalLength, m_fallbackLineHeight, m_fallbackWidth);
        rectangles.push_back(measurement.rectangle);
        geometryAvailable.push_back(measurement.geometryAvailable);
        resourceRanges.push_back(resourceRange);
    }

    QVariantList rows;
    rows.reserve(m_lineNumberRanges.size());
    for (int index = 0; index < static_cast<int>(rectangles.size()); ++index)
    {
        QRectF rectangle = rectangles.at(index);
        if (resourceRanges.at(index))
        {
            qreal resourceBottom = resourceContentHeight(rectangle.bottom());
            const qreal followingRowTop = nextAvailableRowTop(rectangles, geometryAvailable, index);
            if (std::isfinite(followingRowTop) && followingRowTop > rectangle.y())
            {
                resourceBottom = std::min(resourceBottom, followingRowTop);
            }
            if (rectangle.height() <= m_fallbackLineHeight && resourceBottom > rectangle.y())
            {
                rectangle.setHeight(std::max(m_fallbackLineHeight, resourceBottom - rectangle.y()));
            }
        }
        rows.push_back(QVariantMap{
            {QStringLiteral("geometryAvailable"), static_cast<bool>(geometryAvailable.at(index))},
            {QStringLiteral("height"), rectangle.height()},
            {QStringLiteral("width"), rectangle.width()},
            {QStringLiteral("x"), rectangle.x()},
            {QStringLiteral("y"), rectangle.y()},
        });
    }

    return rows;
}

ContentsEditorGeometryMeasurement ContentsEditorGeometryProvider::measureTextRange(
    const int logicalStart,
    const int logicalEnd,
    const int logicalLength,
    const qreal fallbackLineHeight,
    const qreal fallbackWidth) const
{
    return measureRange(
        m_textItem.data(),
        logicalStart,
        logicalEnd,
        logicalLength,
        fallbackLineHeight,
        fallbackWidth);
}

ContentsEditorGeometryMeasurement ContentsEditorGeometryProvider::measureResourceRange(
    const int logicalStart,
    const int logicalEnd,
    const int logicalLength,
    const qreal fallbackLineHeight,
    const qreal fallbackWidth) const
{
    return measureRange(
        m_resourceItem.data(),
        logicalStart,
        logicalEnd,
        logicalLength,
        fallbackLineHeight,
        fallbackWidth);
}

qreal ContentsEditorGeometryProvider::resourceContentHeight(const qreal fallback) const
{
    return numericProperty(m_resourceItem.data(), "contentHeight", fallback);
}

ContentsEditorGeometryMeasurement ContentsEditorGeometryProvider::measureRange(
    QObject* geometryObject,
    const int logicalStart,
    const int logicalEnd,
    const int logicalLength,
    const qreal fallbackLineHeight,
    const qreal fallbackWidth) const
{
    const qreal lineHeight = std::max<qreal>(1.0, std::isfinite(fallbackLineHeight) ? fallbackLineHeight : 1.0);
    const qreal width = std::max<qreal>(0.0, std::isfinite(fallbackWidth) ? fallbackWidth : 0.0);
    const int itemLength =
        std::max(0, integerProperty(geometryObject, "length", std::max(0, logicalLength)));
    const int safeStart = boundedOffset(logicalStart, itemLength);
    const int safeEnd = std::max(safeStart, boundedOffset(logicalEnd, itemLength));
    const ContentsEditorGeometryMeasurement startMeasurement =
        positionRectangle(geometryObject, safeStart, lineHeight);
    const ContentsEditorGeometryMeasurement endMeasurement =
        positionRectangle(
            geometryObject,
            std::max(safeStart, std::min(std::max(0, safeEnd - 1), itemLength)),
            lineHeight);

    const QPointF startPoint =
        mapPoint(geometryObject, m_targetItem.data(), startMeasurement.rectangle.topLeft());
    const QPointF endPoint =
        mapPoint(geometryObject, m_targetItem.data(), endMeasurement.rectangle.topLeft());
    const qreal height =
        std::max(lineHeight,
                 endPoint.y() + std::max(lineHeight, endMeasurement.rectangle.height()) - startPoint.y());
    return {
        QRectF(startPoint.x(), startPoint.y(), width, height),
        startMeasurement.geometryAvailable || endMeasurement.geometryAvailable,
    };
}

void ContentsEditorGeometryProvider::emitGeometryInputChanged(void (ContentsEditorGeometryProvider::*signal)())
{
    emit (this->*signal)();
    emit geometryChanged();
}
