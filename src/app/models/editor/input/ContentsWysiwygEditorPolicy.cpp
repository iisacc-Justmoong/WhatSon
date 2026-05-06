#include "app/models/editor/input/ContentsWysiwygEditorPolicy.hpp"

#include <QChar>
#include <QJSValue>
#include <QMetaObject>
#include <QVariantList>

#include <algorithm>
#include <cmath>

namespace
{
    QVariantMap mapFromVariant(const QVariant& value)
    {
        if (value.metaType().id() == QMetaType::QVariantMap)
        {
            return value.toMap();
        }
        if (value.canConvert<QJSValue>())
        {
            return value.value<QJSValue>().toVariant().toMap();
        }
        return value.toMap();
    }

    QVariantList listFromVariant(const QVariant& value)
    {
        if (value.metaType().id() == QMetaType::QVariantList)
        {
            return value.toList();
        }
        if (value.canConvert<QJSValue>())
        {
            return value.value<QJSValue>().toVariant().toList();
        }
        return value.toList();
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

    bool sourceNameCharacter(const QChar character) noexcept
    {
        return character.isLetterOrNumber()
            || character == QLatin1Char('_')
            || character == QLatin1Char('.')
            || character == QLatin1Char(':')
            || character == QLatin1Char('-');
    }

    QVariantMap sourceRangeMap(const int start, const int end)
    {
        return {
            {QStringLiteral("start"), start},
            {QStringLiteral("end"), end}
        };
    }

    QVariantMap emptyRangeMap()
    {
        return sourceRangeMap(0, 0);
    }
} // namespace

ContentsWysiwygEditorPolicy::ContentsWysiwygEditorPolicy(QObject* parent)
    : QObject(parent)
{
}

int ContentsWysiwygEditorPolicy::boundedOffset(const int position, const int length) const noexcept
{
    return std::max(0, std::min(position, std::max(0, length)));
}

QString ContentsWysiwygEditorPolicy::normalizedSourceTagName(const QString& tagToken) const
{
    if (tagToken.size() < 3 || tagToken.front() != QLatin1Char('<'))
    {
        return {};
    }

    int cursor = 1;
    while (cursor < tagToken.size() && tagToken.at(cursor).isSpace())
    {
        ++cursor;
    }
    if (cursor < tagToken.size() && tagToken.at(cursor) == QLatin1Char('/'))
    {
        ++cursor;
    }
    while (cursor < tagToken.size() && tagToken.at(cursor).isSpace())
    {
        ++cursor;
    }

    const int nameStart = cursor;
    while (cursor < tagToken.size() && sourceNameCharacter(tagToken.at(cursor)))
    {
        ++cursor;
    }
    if (cursor <= nameStart)
    {
        return {};
    }
    return tagToken.mid(nameStart, cursor - nameStart).toLower();
}

bool ContentsWysiwygEditorPolicy::sourceTagProducesVisibleSelection(
    const QString& tagName,
    const bool closingTag) const
{
    if (closingTag)
    {
        return false;
    }

    const QString normalizedTagName = tagName.toLower();
    return normalizedTagName == QStringLiteral("resource")
        || normalizedTagName == QStringLiteral("break")
        || normalizedTagName == QStringLiteral("hr")
        || normalizedTagName == QStringLiteral("tag");
}

bool ContentsWysiwygEditorPolicy::sourceOffsetIsInsideTagToken(
    const QString& sourceText,
    const int sourceOffset) const
{
    if (sourceText.isEmpty())
    {
        return false;
    }

    const int boundedSourceOffset = boundedOffset(sourceOffset, sourceText.size() - 1);
    const int previousOpen = sourceText.lastIndexOf(QLatin1Char('<'), boundedSourceOffset);
    if (previousOpen < 0)
    {
        return false;
    }
    const int previousClose = sourceText.lastIndexOf(QLatin1Char('>'), boundedSourceOffset - 1);
    return previousOpen > previousClose;
}

QVariantMap ContentsWysiwygEditorPolicy::sourceTagTokenBoundsForCursor(
    const QString& sourceText,
    const int sourceOffset) const
{
    const int sourceLength = sourceText.size();
    if (sourceLength <= 0)
    {
        return {
            {QStringLiteral("inside"), false},
            {QStringLiteral("start"), 0},
            {QStringLiteral("end"), 0}
        };
    }

    const int offset = boundedOffset(sourceOffset, sourceLength);
    const int probeOffset = boundedOffset(offset, sourceLength - 1);
    const int tagStart = sourceText.lastIndexOf(QLatin1Char('<'), probeOffset);
    if (tagStart < 0)
    {
        return {
            {QStringLiteral("inside"), false},
            {QStringLiteral("start"), 0},
            {QStringLiteral("end"), 0}
        };
    }

    const int previousClose = sourceText.lastIndexOf(QLatin1Char('>'), probeOffset - 1);
    const int tagEnd = sourceText.indexOf(QLatin1Char('>'), tagStart + 1);
    const bool inside = tagEnd > tagStart
        && tagStart > previousClose
        && offset > tagStart
        && offset < tagEnd + 1;

    return {
        {QStringLiteral("inside"), inside},
        {QStringLiteral("start"), tagStart},
        {QStringLiteral("end"), tagEnd}
    };
}

bool ContentsWysiwygEditorPolicy::sourceRangeContainsVisibleLogicalContent(
    const QString& sourceText,
    const int selectionStart,
    const int selectionEnd) const
{
    const int sourceLength = sourceText.size();
    const int start = boundedOffset(selectionStart, sourceLength);
    const int end = std::max(start, boundedOffset(selectionEnd, sourceLength));

    int cursor = start;
    while (cursor < end)
    {
        if (sourceOffsetIsInsideTagToken(sourceText, cursor))
        {
            const int tagStart = sourceText.lastIndexOf(QLatin1Char('<'), cursor);
            const int tagEnd = sourceText.indexOf(QLatin1Char('>'), cursor);
            if (tagStart >= 0 && tagEnd >= cursor)
            {
                const QString tagToken = sourceText.mid(tagStart, tagEnd - tagStart + 1);
                int slashProbe = 1;
                while (slashProbe < tagToken.size() && tagToken.at(slashProbe).isSpace())
                {
                    ++slashProbe;
                }
                if (sourceTagProducesVisibleSelection(
                        normalizedSourceTagName(tagToken),
                        slashProbe < tagToken.size() && tagToken.at(slashProbe) == QLatin1Char('/')))
                {
                    return true;
                }
                cursor = tagEnd + 1;
                continue;
            }
        }

        if (sourceText.at(cursor) == QLatin1Char('<'))
        {
            const int tagEnd = sourceText.indexOf(QLatin1Char('>'), cursor + 1);
            if (tagEnd > cursor)
            {
                const QString tagToken = sourceText.mid(cursor, tagEnd - cursor + 1);
                int slashProbe = 1;
                while (slashProbe < tagToken.size() && tagToken.at(slashProbe).isSpace())
                {
                    ++slashProbe;
                }
                if (sourceTagProducesVisibleSelection(
                        normalizedSourceTagName(tagToken),
                        slashProbe < tagToken.size() && tagToken.at(slashProbe) == QLatin1Char('/')))
                {
                    return true;
                }
                cursor = tagEnd + 1;
                continue;
            }
        }

        return true;
    }

    return false;
}

int ContentsWysiwygEditorPolicy::htmlBlockSourceStart(const QVariant& block) const noexcept
{
    const int sourceStart = floorNumberOrFallback(
        mapFromVariant(block).value(QStringLiteral("sourceStart")),
        -1);
    return sourceStart >= 0 ? sourceStart : -1;
}

int ContentsWysiwygEditorPolicy::htmlBlockSourceEnd(const QVariant& block) const noexcept
{
    const int sourceEnd = floorNumberOrFallback(
        mapFromVariant(block).value(QStringLiteral("sourceEnd")),
        -1);
    return sourceEnd >= 0 ? sourceEnd : -1;
}

bool ContentsWysiwygEditorPolicy::htmlBlockIntersectsSourceRange(
    const QVariant& block,
    const int selectionStart,
    const int selectionEnd) const
{
    const int blockStart = htmlBlockSourceStart(block);
    const int blockEnd = htmlBlockSourceEnd(block);
    return blockStart >= 0
        && blockEnd > blockStart
        && selectionStart < blockEnd
        && selectionEnd > blockStart;
}

bool ContentsWysiwygEditorPolicy::htmlBlockIsAtomicResourceBlock(const QVariant& block) const
{
    const QVariantMap blockMap = mapFromVariant(block);
    if (blockMap.isEmpty())
    {
        return false;
    }

    const QString blockKind =
        blockMap.value(
                QStringLiteral("renderDelegateType"),
                blockMap.value(
                    QStringLiteral("blockType"),
                    blockMap.value(QStringLiteral("type"))))
            .toString()
            .toLower();
    return blockKind == QStringLiteral("resource")
        && blockMap.value(QStringLiteral("htmlBlockObjectSource")).toString() == QStringLiteral("iiHtmlBlock")
        && blockMap.value(QStringLiteral("htmlBlockIsDisplayBlock"), true).toBool();
}

bool ContentsWysiwygEditorPolicy::sourceRangeIntersectsAtomicResourceBlock(
    const QVariant& normalizedBlocks,
    const int selectionStart,
    const int selectionEnd) const
{
    const QVariantList blocks = listFromVariant(normalizedBlocks);
    for (const QVariant& block : blocks)
    {
        if (htmlBlockIsAtomicResourceBlock(block)
            && htmlBlockIntersectsSourceRange(block, selectionStart, selectionEnd))
        {
            return true;
        }
    }
    return false;
}

bool ContentsWysiwygEditorPolicy::hasAtomicRenderedResourceBlocks(const QVariant& normalizedBlocks) const
{
    const QVariantList blocks = listFromVariant(normalizedBlocks);
    return std::any_of(blocks.cbegin(), blocks.cend(), [this](const QVariant& block) {
        return htmlBlockIsAtomicResourceBlock(block);
    });
}

QVariantMap ContentsWysiwygEditorPolicy::resourceLogicalRangeForBlock(
    const QVariant& block,
    QObject* coordinateMapper) const
{
    const int blockStart = htmlBlockSourceStart(block);
    const int blockEnd = htmlBlockSourceEnd(block);
    if (blockStart < 0 || blockEnd <= blockStart)
    {
        return emptyRangeMap();
    }

    const int logicalStart = std::max(
        0,
        logicalOffsetForSourceOffsetWithAffinity(coordinateMapper, blockStart, false));
    const int logicalEnd = std::max(
        logicalStart + 1,
        logicalOffsetForSourceOffsetWithAffinity(coordinateMapper, blockEnd, true));
    return sourceRangeMap(logicalStart, logicalEnd);
}

QVariantMap ContentsWysiwygEditorPolicy::renderedLogicalSelectionRange(
    const QString& sourceText,
    const QVariant& normalizedBlocks,
    QObject* coordinateMapper,
    const int selectionStart,
    const int selectionEnd,
    const int renderedLength) const
{
    const int sourceLength = sourceText.size();
    const int startSource = boundedOffset(selectionStart, sourceLength);
    const int endSource = std::max(startSource, boundedOffset(selectionEnd, sourceLength));
    const bool intersectsAtomicResource = sourceRangeIntersectsAtomicResourceBlock(
        normalizedBlocks,
        startSource,
        endSource);
    if (!sourceRangeContainsVisibleLogicalContent(sourceText, startSource, endSource)
        && !intersectsAtomicResource)
    {
        return emptyRangeMap();
    }

    int start = logicalOffsetForSourceOffsetWithAffinity(coordinateMapper, startSource, false);
    int end = logicalOffsetForSourceOffsetWithAffinity(coordinateMapper, endSource, true);

    const QVariantList blocks = listFromVariant(normalizedBlocks);
    for (const QVariant& block : blocks)
    {
        if (!htmlBlockIsAtomicResourceBlock(block)
            || !htmlBlockIntersectsSourceRange(block, startSource, endSource))
        {
            continue;
        }

        const QVariantMap resourceRange = resourceLogicalRangeForBlock(block, coordinateMapper);
        start = std::min(start, resourceRange.value(QStringLiteral("start")).toInt());
        end = std::max(end, resourceRange.value(QStringLiteral("end")).toInt());
    }

    const int maxLength = std::max(0, renderedLength);
    start = boundedOffset(start, maxLength);
    end = std::max(start, boundedOffset(end, maxLength));
    return sourceRangeMap(start, end);
}

QVariantMap ContentsWysiwygEditorPolicy::rawSelectionForVisibleSurfaceSelection(
    QObject* coordinateMapper,
    const int surfaceSelectionStart,
    const int surfaceSelectionEnd,
    const int surfaceCursor,
    const int renderedLength,
    const int sourceLength) const
{
    const int boundedRenderedLength = std::max(0, renderedLength);
    const int start = boundedOffset(
        std::min(surfaceSelectionStart, surfaceSelectionEnd),
        boundedRenderedLength);
    const int end = std::max(
        start,
        boundedOffset(std::max(surfaceSelectionStart, surfaceSelectionEnd), boundedRenderedLength));
    const int cursor = std::max(
        start,
        std::min(boundedOffset(surfaceCursor, boundedRenderedLength), end));

    const int anchorSourceOffset = boundedOffset(
        sourceOffsetForVisibleLogicalOffset(coordinateMapper, start, boundedRenderedLength),
        sourceLength);
    const int currentSourceOffset = boundedOffset(
        sourceOffsetForVisibleLogicalOffset(coordinateMapper, end, boundedRenderedLength),
        sourceLength);
    const int cursorSourceOffset = boundedOffset(
        sourceOffsetForVisibleLogicalOffset(coordinateMapper, cursor, boundedRenderedLength),
        sourceLength);
    const int selectionStart = std::min(anchorSourceOffset, currentSourceOffset);
    const int selectionEnd = std::max(anchorSourceOffset, currentSourceOffset);

    return {
        {QStringLiteral("valid"), true},
        {QStringLiteral("selectionStart"), selectionStart},
        {QStringLiteral("selectionEnd"), selectionEnd},
        {QStringLiteral("cursorSourceOffset"), cursorSourceOffset},
        {QStringLiteral("surfaceCursor"), cursor}
    };
}

QVariantMap ContentsWysiwygEditorPolicy::visibleLogicalLineRange(
    const QString& visibleText,
    const int logicalOffset) const
{
    const int offset = boundedOffset(logicalOffset, visibleText.size());
    const int previousBreak = visibleText.lastIndexOf(
        QLatin1Char('\n'),
        std::max(0, offset - 1));
    const int start = previousBreak < 0 ? 0 : previousBreak + 1;
    const int nextBreak = visibleText.indexOf(QLatin1Char('\n'), offset);
    const int end = nextBreak < 0 ? visibleText.size() : nextBreak;
    return sourceRangeMap(start, std::max(start, end));
}

QVariantMap ContentsWysiwygEditorPolicy::visibleLogicalParagraphRange(
    const QString& visibleText,
    const int logicalOffset) const
{
    const int offset = boundedOffset(logicalOffset, visibleText.size());
    const int previousParagraphBreak = visibleText.lastIndexOf(
        QStringLiteral("\n\n"),
        std::max(0, offset - 1));
    int start = previousParagraphBreak < 0 ? 0 : previousParagraphBreak + 2;
    while (start < visibleText.size() && visibleText.at(start) == QLatin1Char('\n'))
    {
        ++start;
    }

    const int nextParagraphBreak = visibleText.indexOf(QStringLiteral("\n\n"), offset);
    int end = nextParagraphBreak < 0 ? visibleText.size() : nextParagraphBreak;
    while (end > start && visibleText.at(end - 1) == QLatin1Char('\n'))
    {
        --end;
    }

    return sourceRangeMap(start, std::max(start, end));
}

QVariantMap ContentsWysiwygEditorPolicy::hiddenTagCursorNormalizationPlan(
    const QString& sourceText,
    const int currentCursorPosition,
    const int previousRawCursorPosition,
    const bool renderedOverlayVisible,
    const bool nativeCompositionActive,
    const bool nativeSelectionActive,
    const bool visiblePointerCursorUpdateActive) const
{
    const int current = boundedOffset(currentCursorPosition, sourceText.size());
    QVariantMap plan{
        {QStringLiteral("changed"), false},
        {QStringLiteral("clearVisiblePointerCursor"), !visiblePointerCursorUpdateActive},
        {QStringLiteral("previousRawCursorPosition"), current},
        {QStringLiteral("targetCursorPosition"), current}
    };

    if (!renderedOverlayVisible || nativeCompositionActive || nativeSelectionActive)
    {
        return plan;
    }

    const QVariantMap tokenBounds = sourceTagTokenBoundsForCursor(sourceText, current);
    if (!tokenBounds.value(QStringLiteral("inside")).toBool())
    {
        return plan;
    }

    const bool movingForward = current >= boundedOffset(previousRawCursorPosition, sourceText.size());
    const int targetCursorPosition = movingForward
        ? tokenBounds.value(QStringLiteral("end")).toInt() + 1
        : tokenBounds.value(QStringLiteral("start")).toInt();
    const int boundedTarget = boundedOffset(targetCursorPosition, sourceText.size());
    if (boundedTarget == current)
    {
        return plan;
    }

    plan.insert(QStringLiteral("changed"), true);
    plan.insert(QStringLiteral("previousRawCursorPosition"), boundedTarget);
    plan.insert(QStringLiteral("targetCursorPosition"), boundedTarget);
    return plan;
}

int ContentsWysiwygEditorPolicy::sourceOffsetForVisibleLogicalOffset(
    QObject* coordinateMapper,
    const int logicalOffset,
    const int visibleLength) const
{
    if (!coordinateMapper)
    {
        return boundedOffset(logicalOffset, visibleLength);
    }

    int mappedOffset = boundedOffset(logicalOffset, visibleLength);
    const bool invoked = QMetaObject::invokeMethod(
        coordinateMapper,
        "sourceOffsetForVisibleLogicalOffset",
        Q_RETURN_ARG(int, mappedOffset),
        Q_ARG(int, boundedOffset(logicalOffset, visibleLength)),
        Q_ARG(int, std::max(0, visibleLength)));
    return invoked ? mappedOffset : boundedOffset(logicalOffset, visibleLength);
}

int ContentsWysiwygEditorPolicy::logicalOffsetForSourceOffsetWithAffinity(
    QObject* coordinateMapper,
    const int sourceOffset,
    const bool preferAfter) const
{
    if (!coordinateMapper)
    {
        return std::max(0, sourceOffset);
    }

    int mappedOffset = std::max(0, sourceOffset);
    const bool invoked = QMetaObject::invokeMethod(
        coordinateMapper,
        "logicalOffsetForSourceOffsetWithAffinity",
        Q_RETURN_ARG(int, mappedOffset),
        Q_ARG(int, std::max(0, sourceOffset)),
        Q_ARG(bool, preferAfter));
    return invoked ? mappedOffset : std::max(0, sourceOffset);
}
