#include "app/models/editor/lineNumber/ContentsLineNumberRailMetrics.hpp"

#include "app/models/editor/text/ContentsLogicalTextBridge.hpp"

#include <QRectF>
#include <QSet>
#include <QVariantMap>

#include <algorithm>
#include <cmath>

namespace
{
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

    QRectF rectangleFromRowGeometry(const QVariantMap& rowGeometry, const qreal lineHeight, const qreal width) noexcept
    {
        const qreal x = rowGeometry.value(QStringLiteral("x")).toDouble();
        const qreal y = rowGeometry.value(QStringLiteral("y")).toDouble();
        const qreal resolvedWidth = rowGeometry.contains(QStringLiteral("width"))
            ? rowGeometry.value(QStringLiteral("width")).toDouble()
            : width;
        const qreal height = rowGeometry.value(QStringLiteral("height")).toDouble();
        return QRectF(
            std::isfinite(x) ? x : 0.0,
            std::isfinite(y) ? y : 0.0,
            std::max<qreal>(0.0, std::isfinite(resolvedWidth) ? resolvedWidth : width),
            std::max<qreal>(1.0, std::isfinite(height) ? height : lineHeight));
    }
} // namespace

ContentsLineNumberRailMetrics::ContentsLineNumberRailMetrics(QObject* parent)
    : QObject(parent)
    , m_logicalTextBridge(new ContentsLogicalTextBridge(this))
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
    if (m_logicalTextBridge != nullptr)
    {
        m_logicalTextBridge->setText(value);
    }
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

QVariantList ContentsLineNumberRailMetrics::geometryRows() const
{
    return m_geometryRows;
}

void ContentsLineNumberRailMetrics::setGeometryRows(const QVariantList& value)
{
    if (m_geometryRows == value)
    {
        return;
    }
    m_geometryRows = value;
    emitInputChanged(&ContentsLineNumberRailMetrics::geometryRowsChanged);
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
    const auto sourceOffsetToLogicalOffset = [this](const int sourceOffset, const bool preferAfter) {
        return m_logicalTextBridge != nullptr
                   ? m_logicalTextBridge->logicalOffsetForSourceOffsetWithAffinity(sourceOffset, preferAfter)
                   : boundedOffset(sourceOffset, m_sourceText.size());
    };

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

    const auto sourceOffsetForLogicalOffset = [this](const int logicalOffset) {
        return m_logicalTextBridge != nullptr
                   ? m_logicalTextBridge->sourceOffsetForLogicalOffset(logicalOffset)
                   : boundedOffset(logicalOffset, m_sourceText.size());
    };

    const auto resourceBlockForSourceInterval =
        [&normalizedBlocks, this](const int sourceStart, const int sourceCoverageEnd) {
            const int safeStart = boundedOffset(sourceStart, m_sourceText.size());
            const int safeEnd = boundedOffset(std::max(sourceCoverageEnd, sourceStart), m_sourceText.size());
            for (const QVariant& blockValue : normalizedBlocks)
            {
                const QVariantMap block = blockValue.toMap();
                if (!isIiHtmlResourceBlock(block))
                {
                    continue;
                }

                const int blockStart = boundedOffset(htmlBlockSourceStart(block), m_sourceText.size());
                const int blockEnd = boundedOffset(std::max(blockStart, htmlBlockSourceEnd(block)), m_sourceText.size());
                const bool intersectsLine = blockStart < safeEnd && blockEnd > safeStart;
                const bool startsAtCollapsedLine = safeStart == safeEnd && blockStart == safeStart;
                if (intersectsLine || startsAtCollapsedLine)
                {
                    return block;
                }
            }
            return QVariantMap();
        };

    const QString logicalText = !m_logicalText.isEmpty() ? m_logicalText : m_sourceText;
    QVariantList ranges;
    QSet<QString> emittedResourceKeys;
    int lineNumber = 1;
    int lineLogicalStart = 0;
    while (lineLogicalStart <= logicalText.size())
    {
        const int newlineIndex = logicalText.indexOf(QLatin1Char('\n'), lineLogicalStart);
        const bool hasNewline = newlineIndex >= 0;
        const int lineLogicalEnd = hasNewline ? newlineIndex : logicalText.size();
        const int nextLineLogicalStart = hasNewline ? newlineIndex + 1 : logicalText.size();
        const int lineSourceStart = sourceOffsetForLogicalOffset(lineLogicalStart);
        const int lineSourceEnd = sourceOffsetForLogicalOffset(lineLogicalEnd);
        const int lineSourceCoverageEnd =
            hasNewline ? sourceOffsetForLogicalOffset(nextLineLogicalStart) : m_sourceText.size();
        const bool resourceLineEligible =
            lineLogicalStart == lineLogicalEnd || m_logicalText.isEmpty();
        const QVariantMap resourceBlock = resourceLineEligible
            ? resourceBlockForSourceInterval(lineSourceStart, lineSourceCoverageEnd)
            : QVariantMap();
        const bool resourceRange = !resourceBlock.isEmpty();

        int rowSourceStart = lineSourceStart;
        int rowSourceEnd = lineSourceEnd;
        int rowLogicalStart = lineLogicalStart;
        int rowLogicalEnd = lineLogicalEnd;
        if (resourceRange)
        {
            rowSourceStart = htmlBlockSourceStart(resourceBlock);
            rowSourceEnd = htmlBlockSourceEnd(resourceBlock);
            rowLogicalStart = sourceOffsetToLogicalOffset(rowSourceStart, false);
            rowLogicalEnd = std::max(
                rowLogicalStart + 1,
                sourceOffsetToLogicalOffset(rowSourceEnd, true));
        }

        bool emitRange = true;
        if (resourceRange)
        {
            const QString resourceKey =
                QStringLiteral("resource:%1:%2").arg(rowSourceStart).arg(rowSourceEnd);
            emitRange = !emittedResourceKeys.contains(resourceKey);
            emittedResourceKeys.insert(resourceKey);
        }

        if (emitRange)
        {
            ranges.push_back(QVariantMap{
                {QStringLiteral("block"), resourceBlock},
                {QStringLiteral("logicalEnd"), rowLogicalEnd},
                {QStringLiteral("logicalStart"), rowLogicalStart},
                {QStringLiteral("number"), lineNumber},
                {QStringLiteral("resourceRange"), resourceRange},
                {QStringLiteral("sourceEnd"), rowSourceEnd},
                {QStringLiteral("sourceStart"), rowSourceStart},
            });
            ++lineNumber;
        }

        if (!hasNewline)
        {
            break;
        }
        lineLogicalStart = nextLineLogicalStart;
    }
    return ranges;
}

QVariantList ContentsLineNumberRailMetrics::rows() const
{
    const QVariantList ranges = logicalLineRanges();
    QVariantList result;
    result.reserve(ranges.size());

    for (int index = 0; index < ranges.size(); ++index)
    {
        const QVariantMap range = ranges.at(index).toMap();
        const QVariantMap rowGeometry =
            index < m_geometryRows.size() ? m_geometryRows.at(index).toMap() : QVariantMap();
        QRectF rectangle = rectangleFromRowGeometry(rowGeometry, m_textLineHeight, m_geometryWidth);
        const bool geometryAvailable = rowGeometry.value(QStringLiteral("geometryAvailable"), false).toBool();
        if (!geometryAvailable)
        {
            const int lineNumber = std::max(1, range.value(QStringLiteral("number"), index + 1).toInt());
            rectangle.moveTop(static_cast<qreal>(lineNumber - 1) * m_textLineHeight);
        }

        const bool resourceRange = range.value(QStringLiteral("resourceRange"), false).toBool();
        const qreal resolvedHeight = resourceRange
            ? m_textLineHeight
            : std::max(m_textLineHeight, rectangle.height());
        result.push_back(QVariantMap{
            {QStringLiteral("height"), resolvedHeight},
            {QStringLiteral("number"), range.value(QStringLiteral("number")).toInt()},
            {QStringLiteral("resourceRange"), resourceRange},
            {QStringLiteral("sourceEnd"), range.value(QStringLiteral("sourceEnd")).toInt()},
            {QStringLiteral("sourceStart"), range.value(QStringLiteral("sourceStart")).toInt()},
            {QStringLiteral("y"), std::max<qreal>(0.0, rectangle.y())},
        });
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
    emit logicalLineRangesChanged();
    emit rowsChanged();
}
