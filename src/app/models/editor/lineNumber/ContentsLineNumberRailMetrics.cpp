#include "app/models/editor/lineNumber/ContentsLineNumberRailMetrics.hpp"

#include <QMetaObject>
#include <QQuickItem>
#include <QRectF>
#include <QSet>
#include <QVariantMap>

#include <algorithm>
#include <cmath>

namespace
{
    struct MeasuredRectangle
    {
        QRectF rectangle;
        bool geometryAvailable = false;
    };

    int boundedOffset(const int offset, const int length) noexcept
    {
        return std::max(0, std::min(offset, std::max(0, length)));
    }

    int floorNumberOrFallback(const QVariant& value, const int fallback) noexcept
    {
        bool ok = false;
        const double number = value.toDouble(&ok);
        if (!ok || !std::isfinite(number))
        {
            return fallback;
        }
        return static_cast<int>(std::floor(number));
    }

    int htmlBlockSourceStart(const QVariantMap& block) noexcept
    {
        return std::max(
            0,
            floorNumberOrFallback(block.value(QStringLiteral("sourceStart")), -1));
    }

    int htmlBlockSourceEnd(const QVariantMap& block) noexcept
    {
        return std::max(
            0,
            floorNumberOrFallback(block.value(QStringLiteral("sourceEnd")), -1));
    }

    QString blockStableKey(const QVariantMap& block, const int fallbackIndex)
    {
        const int tokenIndex =
            floorNumberOrFallback(block.value(QStringLiteral("htmlTokenStartIndex")), -1);
        if (tokenIndex >= 0)
        {
            return QStringLiteral("token:%1").arg(tokenIndex);
        }

        const int blockStart = htmlBlockSourceStart(block);
        const int blockEnd = htmlBlockSourceEnd(block);
        if (blockEnd >= blockStart)
        {
            return QStringLiteral("source:%1:%2").arg(blockStart).arg(blockEnd);
        }

        return QStringLiteral("index:%1").arg(fallbackIndex);
    }

    QString sourceTextForBlock(const QVariantMap& block, const QString& sourceText)
    {
        if (block.contains(QStringLiteral("sourceText")))
        {
            return block.value(QStringLiteral("sourceText")).toString();
        }
        if (block.contains(QStringLiteral("plainText")))
        {
            return block.value(QStringLiteral("plainText")).toString();
        }

        const int blockStart = htmlBlockSourceStart(block);
        const int blockEnd = htmlBlockSourceEnd(block);
        if (blockStart >= 0 && blockEnd >= blockStart)
        {
            return sourceText.mid(
                boundedOffset(blockStart, sourceText.size()),
                std::max(0, boundedOffset(blockEnd, sourceText.size()) - boundedOffset(blockStart, sourceText.size())));
        }
        return {};
    }

    bool isIiHtmlResourceBlock(const QVariantMap& block)
    {
        const QString kind =
            block.value(QStringLiteral("renderDelegateType"),
                        block.value(QStringLiteral("blockType"),
                                    block.value(QStringLiteral("type"))))
                .toString()
                .toLower();
        return kind == QStringLiteral("resource")
            && block.value(QStringLiteral("htmlBlockObjectSource")).toString() == QStringLiteral("iiHtmlBlock")
            && block.value(QStringLiteral("htmlBlockIsDisplayBlock"), true).toBool();
    }

    QVariant propertyValue(QObject* object, const char* propertyName)
    {
        return object != nullptr ? object->property(propertyName) : QVariant();
    }

    qreal numericProperty(QObject* object, const char* propertyName, const qreal fallback)
    {
        bool ok = false;
        const qreal value = propertyValue(object, propertyName).toReal(&ok);
        return ok && std::isfinite(value) ? value : fallback;
    }

    MeasuredRectangle positionRectangle(QObject* object, const int position, const qreal lineHeight)
    {
        if (object == nullptr)
        {
            return {
                QRectF(0.0, 0.0, 0.0, std::max<qreal>(1.0, lineHeight)),
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
            QRectF(0.0, 0.0, 0.0, std::max<qreal>(1.0, lineHeight)),
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

    bool rectangleYCollapsed(const QRectF& rectangle, const qreal fallbackY, const qreal lineHeight, const bool hasPreviousRows) noexcept
    {
        return hasPreviousRows && fallbackY > 0.0 && rectangle.y() < fallbackY - std::max<qreal>(1.0, lineHeight) * 0.25;
    }
} // namespace

ContentsLineNumberRailMetrics::ContentsLineNumberRailMetrics(QObject* parent)
    : QObject(parent)
{
}

ContentsLineNumberRailMetrics::~ContentsLineNumberRailMetrics() = default;

QString ContentsLineNumberRailMetrics::sourceText() const
{
    return m_sourceText;
}

void ContentsLineNumberRailMetrics::setSourceText(const QString& value)
{
    if (m_sourceText == value)
    {
        return;
    }
    m_sourceText = value;
    emitInputChanged(&ContentsLineNumberRailMetrics::sourceTextChanged);
}

QString ContentsLineNumberRailMetrics::logicalText() const
{
    return m_logicalText;
}

void ContentsLineNumberRailMetrics::setLogicalText(const QString& value)
{
    if (m_logicalText == value)
    {
        return;
    }
    m_logicalText = value;
    emitInputChanged(&ContentsLineNumberRailMetrics::logicalTextChanged);
}

QVariantList ContentsLineNumberRailMetrics::normalizedHtmlBlocks() const
{
    return m_normalizedHtmlBlocks;
}

void ContentsLineNumberRailMetrics::setNormalizedHtmlBlocks(const QVariantList& value)
{
    if (m_normalizedHtmlBlocks == value)
    {
        return;
    }
    m_normalizedHtmlBlocks = value;
    emitInputChanged(&ContentsLineNumberRailMetrics::normalizedHtmlBlocksChanged);
}

QVariantList ContentsLineNumberRailMetrics::logicalToSourceOffsets() const
{
    return m_logicalToSourceOffsets;
}

void ContentsLineNumberRailMetrics::setLogicalToSourceOffsets(const QVariantList& value)
{
    if (m_logicalToSourceOffsets == value)
    {
        return;
    }
    m_logicalToSourceOffsets = value;
    emitInputChanged(&ContentsLineNumberRailMetrics::logicalToSourceOffsetsChanged);
}

QObject* ContentsLineNumberRailMetrics::textGeometryItem() const noexcept
{
    return m_textGeometryItem.data();
}

void ContentsLineNumberRailMetrics::setTextGeometryItem(QObject* value)
{
    if (m_textGeometryItem == value)
    {
        return;
    }
    m_textGeometryItem = value;
    emitInputChanged(&ContentsLineNumberRailMetrics::textGeometryItemChanged);
}

QObject* ContentsLineNumberRailMetrics::resourceGeometryItem() const noexcept
{
    return m_resourceGeometryItem.data();
}

void ContentsLineNumberRailMetrics::setResourceGeometryItem(QObject* value)
{
    if (m_resourceGeometryItem == value)
    {
        return;
    }
    m_resourceGeometryItem = value;
    emitInputChanged(&ContentsLineNumberRailMetrics::resourceGeometryItemChanged);
}

QObject* ContentsLineNumberRailMetrics::targetItem() const noexcept
{
    return m_targetItem.data();
}

void ContentsLineNumberRailMetrics::setTargetItem(QObject* value)
{
    if (m_targetItem == value)
    {
        return;
    }
    m_targetItem = value;
    emitInputChanged(&ContentsLineNumberRailMetrics::targetItemChanged);
}

qreal ContentsLineNumberRailMetrics::textLineHeight() const noexcept
{
    return m_textLineHeight;
}

void ContentsLineNumberRailMetrics::setTextLineHeight(const qreal value)
{
    const qreal nextValue = std::max<qreal>(1.0, std::isfinite(value) ? value : 1.0);
    if (qFuzzyCompare(m_textLineHeight, nextValue))
    {
        return;
    }
    m_textLineHeight = nextValue;
    emitInputChanged(&ContentsLineNumberRailMetrics::textLineHeightChanged);
}

qreal ContentsLineNumberRailMetrics::geometryWidth() const noexcept
{
    return m_geometryWidth;
}

void ContentsLineNumberRailMetrics::setGeometryWidth(const qreal value)
{
    const qreal nextValue = std::max<qreal>(0.0, std::isfinite(value) ? value : 0.0);
    if (qFuzzyCompare(m_geometryWidth, nextValue))
    {
        return;
    }
    m_geometryWidth = nextValue;
    emitInputChanged(&ContentsLineNumberRailMetrics::geometryWidthChanged);
}

qreal ContentsLineNumberRailMetrics::displayContentHeight() const noexcept
{
    return m_displayContentHeight;
}

void ContentsLineNumberRailMetrics::setDisplayContentHeight(const qreal value)
{
    const qreal nextValue = std::max<qreal>(0.0, std::isfinite(value) ? value : 0.0);
    if (qFuzzyCompare(m_displayContentHeight, nextValue))
    {
        return;
    }
    m_displayContentHeight = nextValue;
    emitInputChanged(&ContentsLineNumberRailMetrics::displayContentHeightChanged);
}

QVariantList ContentsLineNumberRailMetrics::logicalLineRanges() const
{
    QVariantList normalizedBlocks;
    QSet<QString> seenKeys;
    for (int index = 0; index < m_normalizedHtmlBlocks.size(); ++index)
    {
        const QVariantMap block = m_normalizedHtmlBlocks.at(index).toMap();
        if (block.isEmpty())
        {
            continue;
        }

        const int blockStart = htmlBlockSourceStart(block);
        const int blockEnd = htmlBlockSourceEnd(block);
        if (blockStart < 0 || blockEnd < blockStart)
        {
            continue;
        }

        const QString key = blockStableKey(block, index);
        if (seenKeys.contains(key))
        {
            continue;
        }
        seenKeys.insert(key);
        normalizedBlocks.push_back(block);
    }

    if (normalizedBlocks.isEmpty())
    {
        normalizedBlocks.push_back(QVariantMap{
            {QStringLiteral("logicalLineCountHint"),
             std::max(1, static_cast<int>(m_sourceText.count(QLatin1Char('\n'))) + 1)},
            {QStringLiteral("sourceEnd"), m_sourceText.size()},
            {QStringLiteral("sourceStart"), 0},
            {QStringLiteral("sourceText"), m_sourceText},
        });
    }

    std::sort(
        normalizedBlocks.begin(),
        normalizedBlocks.end(),
        [](const QVariant& left, const QVariant& right) {
            const QVariantMap leftBlock = left.toMap();
            const QVariantMap rightBlock = right.toMap();
            const int leftStart = htmlBlockSourceStart(leftBlock);
            const int rightStart = htmlBlockSourceStart(rightBlock);
            if (leftStart != rightStart)
            {
                return leftStart < rightStart;
            }
            return htmlBlockSourceEnd(leftBlock) < htmlBlockSourceEnd(rightBlock);
        });

    QVariantList ranges;
    int lineNumber = 1;
    for (const QVariant& blockValue : normalizedBlocks)
    {
        const QVariantMap block = blockValue.toMap();
        const int safeStart = boundedOffset(htmlBlockSourceStart(block), m_sourceText.size());
        const int safeEnd = boundedOffset(std::max(safeStart, htmlBlockSourceEnd(block)), m_sourceText.size());
        const QString blockSource = sourceTextForBlock(block, m_sourceText);
        const QStringList segments = blockSource.split(QLatin1Char('\n'));
        const int hint = floorNumberOrFallback(block.value(QStringLiteral("logicalLineCountHint")), 0);
        const int lineCount = std::max(1, std::max(hint, static_cast<int>(segments.size())));
        int cursor = safeStart;
        for (int index = 0; index < lineCount; ++index)
        {
            const QString segment = index < segments.size() ? segments.at(index) : QString();
            const int lineStart = std::min(cursor, safeEnd);
            const int lineEnd = std::min(lineStart + static_cast<int>(segment.size()), safeEnd);
            ranges.push_back(QVariantMap{
                {QStringLiteral("block"), block},
                {QStringLiteral("number"), lineNumber},
                {QStringLiteral("sourceEnd"), lineEnd},
                {QStringLiteral("sourceStart"), lineStart},
            });
            cursor = std::min(safeEnd, lineEnd + 1);
            ++lineNumber;
        }
    }
    return ranges;
}

QVariantList ContentsLineNumberRailMetrics::rows() const
{
    const QVariantList ranges = logicalLineRanges();
    QVariantList result;
    result.reserve(ranges.size());

    const auto sourceOffsetToLogicalOffset = [this](const int sourceOffset, const bool preferAfter) {
        if (m_logicalToSourceOffsets.isEmpty())
        {
            return boundedOffset(sourceOffset, m_sourceText.size());
        }

        const int boundedSourceOffset = boundedOffset(sourceOffset, m_sourceText.size());
        if (preferAfter)
        {
            for (int index = 0; index < m_logicalToSourceOffsets.size(); ++index)
            {
                const int candidate = floorNumberOrFallback(m_logicalToSourceOffsets.at(index), -1);
                if (candidate >= boundedSourceOffset)
                {
                    return index;
                }
            }
            return std::max(0, static_cast<int>(m_logicalToSourceOffsets.size()) - 1);
        }

        for (int index = m_logicalToSourceOffsets.size() - 1; index >= 0; --index)
        {
            const int candidate = floorNumberOrFallback(m_logicalToSourceOffsets.at(index), -1);
            if (candidate >= 0 && candidate <= boundedSourceOffset)
            {
                return index;
            }
        }
        return 0;
    };

    const auto displayRectangleForLogicalRange =
        [this](QObject* geometryObject, const int logicalStart, const int logicalEnd) {
            const int itemLength =
                std::max(0,
                         floorNumberOrFallback(
                             propertyValue(geometryObject, "length"),
                             m_logicalText.size()));
            const int safeStart = boundedOffset(logicalStart, itemLength);
            const int safeEnd = std::max(safeStart, boundedOffset(logicalEnd, itemLength));
            const MeasuredRectangle startMeasurement =
                positionRectangle(geometryObject, safeStart, m_textLineHeight);
            const MeasuredRectangle endMeasurement =
                positionRectangle(geometryObject, std::max(safeStart, std::min(std::max(0, safeEnd - 1), itemLength)), m_textLineHeight);
            const QRectF startRectangle = startMeasurement.rectangle;
            const QRectF endRectangle = endMeasurement.rectangle;
            const QPointF startPoint =
                mapPoint(geometryObject, m_targetItem.data(), startRectangle.topLeft());
            const QPointF endPoint =
                mapPoint(geometryObject, m_targetItem.data(), endRectangle.topLeft());
            const qreal height =
                std::max(m_textLineHeight,
                         endPoint.y() + std::max(m_textLineHeight, endRectangle.height()) - startPoint.y());
            return MeasuredRectangle{
                QRectF(startPoint.x(), startPoint.y(), std::max<qreal>(0.0, m_geometryWidth), height),
                startMeasurement.geometryAvailable || endMeasurement.geometryAvailable,
            };
        };

    qreal fallbackNextY = 0.0;
    for (const QVariant& rangeValue : ranges)
    {
        const QVariantMap range = rangeValue.toMap();
        const QVariantMap block = range.value(QStringLiteral("block")).toMap();
        MeasuredRectangle measurement;
        if (isIiHtmlResourceBlock(block) && m_resourceGeometryItem != nullptr)
        {
            const int blockStart = htmlBlockSourceStart(block);
            const int blockEnd = htmlBlockSourceEnd(block);
            const int logicalStart = sourceOffsetToLogicalOffset(blockStart, false);
            const int logicalEnd = std::max(
                logicalStart + 1,
                sourceOffsetToLogicalOffset(blockEnd, true));
            measurement = displayRectangleForLogicalRange(
                m_resourceGeometryItem.data(),
                logicalStart,
                logicalEnd);
            QRectF rectangle = measurement.rectangle;

            const qreal contentHeight =
                numericProperty(m_resourceGeometryItem.data(), "contentHeight", rectangle.bottom());
            if (rectangle.height() <= m_textLineHeight && contentHeight > rectangle.y())
            {
                rectangle.setHeight(std::max(m_textLineHeight, contentHeight - rectangle.y()));
            }
            measurement.rectangle = rectangle;
        }
        else
        {
            const int sourceStart = range.value(QStringLiteral("sourceStart")).toInt();
            const int sourceEnd = range.value(QStringLiteral("sourceEnd")).toInt();
            const int logicalStart = sourceOffsetToLogicalOffset(sourceStart, false);
            const int logicalEnd = std::max(
                logicalStart,
                sourceOffsetToLogicalOffset(std::max(sourceStart, sourceEnd), true));
            measurement = displayRectangleForLogicalRange(
                m_textGeometryItem.data(),
                logicalStart,
                logicalEnd);
        }

        QRectF rectangle = measurement.rectangle;
        if (!measurement.geometryAvailable
            || rectangleYCollapsed(rectangle, fallbackNextY, m_textLineHeight, !result.isEmpty()))
        {
            rectangle.moveTop(fallbackNextY);
        }

        const qreal resolvedHeight = std::max(m_textLineHeight, rectangle.height());
        result.push_back(QVariantMap{
            {QStringLiteral("height"), resolvedHeight},
            {QStringLiteral("number"), range.value(QStringLiteral("number")).toInt()},
            {QStringLiteral("sourceEnd"), range.value(QStringLiteral("sourceEnd")).toInt()},
            {QStringLiteral("sourceStart"), range.value(QStringLiteral("sourceStart")).toInt()},
            {QStringLiteral("y"), std::max<qreal>(0.0, rectangle.y())},
        });
        fallbackNextY = std::max(fallbackNextY, std::max<qreal>(0.0, rectangle.y()) + resolvedHeight);
    }

    if (result.isEmpty())
    {
        result.push_back(QVariantMap{
            {QStringLiteral("height"), m_textLineHeight},
            {QStringLiteral("number"), 1},
            {QStringLiteral("sourceEnd"), 0},
            {QStringLiteral("sourceStart"), 0},
            {QStringLiteral("y"), 0.0},
        });
    }
    return result;
}

void ContentsLineNumberRailMetrics::requestRowsRefresh()
{
    emit rowsChanged();
}

void ContentsLineNumberRailMetrics::emitInputChanged(void (ContentsLineNumberRailMetrics::*signal)())
{
    emit (this->*signal)();
    emit rowsChanged();
}
