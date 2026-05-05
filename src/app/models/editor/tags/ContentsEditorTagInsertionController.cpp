#include "app/models/editor/tags/ContentsEditorTagInsertionController.hpp"

#include "app/models/file/note/WhatSonNoteBodySemanticTagSupport.hpp"

#include <algorithm>

#include <QDate>
#include <QRegularExpression>
#include <QStringView>
#include <Qt>
#include <QVector>

namespace
{
    namespace SemanticTags = WhatSon::NoteBodySemanticTagSupport;

    struct SourceTagToken final
    {
        int start = 0;
        int end = 0;
        QString inlineStyleTagName;
        bool closing = false;
        bool selfClosing = false;

        bool isInlineStyleTag() const
        {
            return !inlineStyleTagName.isEmpty();
        }
    };

    struct SourceRange final
    {
        int start = 0;
        int end = 0;
    };

    QString normalizedText(const QVariant& value)
    {
        QString text = value.isValid() && !value.isNull() ? value.toString() : QString{};
        text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        text.replace(QChar(0x2028), QLatin1Char('\n'));
        text.replace(QChar(0x2029), QLatin1Char('\n'));
        text.replace(QChar(0x00A0), QLatin1Char(' '));
        return text;
    }

    QString normalizeEditorTagName(const QVariant& rawTagName)
    {
        const QString tag = normalizedText(rawTagName).trimmed().toLower();
        QString inlineStyleTagName = SemanticTags::canonicalInlineStyleTagName(tag);
        if (!inlineStyleTagName.isEmpty())
            return inlineStyleTagName;
        if (tag == QLatin1String("plain") || tag == QLatin1String("clear") || tag == QLatin1String("none"))
            return QStringLiteral("plain");
        if (tag == QLatin1String("callout"))
            return QStringLiteral("callout");
        if (tag == QLatin1String("agenda"))
            return QStringLiteral("agenda");
        if (tag == QLatin1String("task"))
            return QStringLiteral("task");
        if (tag == QLatin1String("resource"))
            return QStringLiteral("resource");
        if (tag == QLatin1String("break") || tag == QLatin1String("hr"))
            return QStringLiteral("break");
        return {};
    }

    bool isGeneratedBodyInsertionTag(const QString& tagName)
    {
        return tagName == QLatin1String("agenda")
            || tagName == QLatin1String("callout")
            || tagName == QLatin1String("break");
    }

    bool isInlineStyleInsertionTag(const QString& tagName)
    {
        return tagName == QLatin1String("bold")
            || tagName == QLatin1String("italic")
            || tagName == QLatin1String("underline")
            || tagName == QLatin1String("strikethrough")
            || tagName == QLatin1String("highlight");
    }

    bool isPairedTagInsertionTag(const QString& tagName)
    {
        return isInlineStyleInsertionTag(tagName)
            || tagName == QLatin1String("callout")
            || tagName == QLatin1String("agenda")
            || tagName == QLatin1String("task");
    }

    int boundedOffset(const int offset, const int sourceLength)
    {
        return std::clamp(offset, 0, std::max(0, sourceLength));
    }

    QString todayIsoDate()
    {
        return QDate::currentDate().toString(Qt::ISODate);
    }

    QString sourceTagName(QStringView tagToken)
    {
        if (tagToken.size() < 3 || tagToken.front() != QLatin1Char('<'))
            return {};

        int cursor = 1;
        while (cursor < tagToken.size() && tagToken.at(cursor).isSpace())
            ++cursor;
        if (cursor < tagToken.size() && tagToken.at(cursor) == QLatin1Char('/'))
            ++cursor;
        while (cursor < tagToken.size() && tagToken.at(cursor).isSpace())
            ++cursor;

        const int nameStart = cursor;
        while (cursor < tagToken.size())
        {
            const QChar ch = tagToken.at(cursor);
            if (!(ch.isLetterOrNumber()
                  || ch == QLatin1Char('_')
                  || ch == QLatin1Char('.')
                  || ch == QLatin1Char(':')
                  || ch == QLatin1Char('-')))
            {
                break;
            }
            ++cursor;
        }

        if (cursor <= nameStart)
            return {};

        return tagToken.mid(nameStart, cursor - nameStart).toString().trimmed().toCaseFolded();
    }

    bool sourceTagIsClosing(QStringView tagToken)
    {
        if (tagToken.size() < 3 || tagToken.front() != QLatin1Char('<'))
            return false;

        int cursor = 1;
        while (cursor < tagToken.size() && tagToken.at(cursor).isSpace())
            ++cursor;
        return cursor < tagToken.size() && tagToken.at(cursor) == QLatin1Char('/');
    }

    bool sourceTagIsSelfClosing(QStringView tagToken)
    {
        if (tagToken.size() < 3 || tagToken.back() != QLatin1Char('>'))
            return false;

        int cursor = tagToken.size() - 2;
        while (cursor >= 0 && tagToken.at(cursor).isSpace())
            --cursor;
        return cursor >= 0 && tagToken.at(cursor) == QLatin1Char('/');
    }

    QVector<SourceTagToken> collectSourceTagTokens(const QString& source)
    {
        QVector<SourceTagToken> tokens;
        int cursor = 0;
        while (cursor < source.size())
        {
            const int tokenStart = source.indexOf(QLatin1Char('<'), cursor);
            if (tokenStart < 0)
                break;

            const int tokenEnd = source.indexOf(QLatin1Char('>'), tokenStart + 1);
            if (tokenEnd <= tokenStart)
                break;

            const QStringView tagToken(source.constData() + tokenStart, tokenEnd - tokenStart + 1);
            const QString tagName = sourceTagName(tagToken);
            if (!tagName.isEmpty())
            {
                tokens.push_back(SourceTagToken{
                    tokenStart,
                    tokenEnd + 1,
                    SemanticTags::canonicalInlineStyleTagName(tagName),
                    sourceTagIsClosing(tagToken),
                    sourceTagIsSelfClosing(tagToken),
                });
            }

            cursor = tokenEnd + 1;
        }
        return tokens;
    }

    int sourceOffsetForCollapsedInlineInsertion(const QVector<SourceTagToken>& tokens, const int offset)
    {
        for (const SourceTagToken& token : tokens)
        {
            if (offset <= token.start || offset >= token.end)
                continue;
            if (token.closing)
                return token.start;
            return token.end;
        }
        return offset;
    }

    int sourceStartOutsideTagTokens(const QVector<SourceTagToken>& tokens, const int start)
    {
        for (const SourceTagToken& token : tokens)
        {
            if (start > token.start && start < token.end)
                return token.end;
        }
        return start;
    }

    int sourceEndOutsideTagTokens(const QVector<SourceTagToken>& tokens, const int end)
    {
        for (const SourceTagToken& token : tokens)
        {
            if (end > token.start && end < token.end)
                return token.start;
        }
        return end;
    }

    int matchingInlineClosingTokenIndex(const QVector<SourceTagToken>& tokens, const int openingTokenIndex)
    {
        const SourceTagToken& openingToken = tokens.at(openingTokenIndex);
        if (!openingToken.isInlineStyleTag() || openingToken.closing || openingToken.selfClosing)
            return -1;

        int nestedDepth = 0;
        for (int index = openingTokenIndex + 1; index < tokens.size(); ++index)
        {
            const SourceTagToken& token = tokens.at(index);
            if (token.inlineStyleTagName != openingToken.inlineStyleTagName || token.selfClosing)
                continue;
            if (!token.closing)
            {
                ++nestedDepth;
                continue;
            }
            if (nestedDepth == 0)
                return index;
            --nestedDepth;
        }
        return -1;
    }

    int matchingInlineOpeningTokenIndex(const QVector<SourceTagToken>& tokens, const int closingTokenIndex)
    {
        const SourceTagToken& closingToken = tokens.at(closingTokenIndex);
        if (!closingToken.isInlineStyleTag() || !closingToken.closing)
            return -1;

        int nestedDepth = 0;
        for (int index = closingTokenIndex - 1; index >= 0; --index)
        {
            const SourceTagToken& token = tokens.at(index);
            if (token.inlineStyleTagName != closingToken.inlineStyleTagName || token.selfClosing)
                continue;
            if (token.closing)
            {
                ++nestedDepth;
                continue;
            }
            if (nestedDepth == 0)
                return index;
            --nestedDepth;
        }
        return -1;
    }

    int trimmedLeadingInlineTagBoundary(
        const QVector<SourceTagToken>& tokens,
        const int rangeStart,
        const int rangeEnd)
    {
        for (int index = 0; index < tokens.size(); ++index)
        {
            const SourceTagToken& token = tokens.at(index);
            if (token.start != rangeStart)
                continue;
            if (!token.isInlineStyleTag() || token.closing || token.selfClosing)
                return rangeStart;

            const int closingIndex = matchingInlineClosingTokenIndex(tokens, index);
            if (closingIndex < 0 || tokens.at(closingIndex).end > rangeEnd)
                return token.end;
            return rangeStart;
        }
        return rangeStart;
    }

    int trimmedTrailingInlineTagBoundary(
        const QVector<SourceTagToken>& tokens,
        const int rangeStart,
        const int rangeEnd)
    {
        for (int index = tokens.size() - 1; index >= 0; --index)
        {
            const SourceTagToken& token = tokens.at(index);
            if (token.end != rangeEnd)
                continue;
            if (!token.isInlineStyleTag() || !token.closing)
                return rangeEnd;

            const int openingIndex = matchingInlineOpeningTokenIndex(tokens, index);
            if (openingIndex < 0 || tokens.at(openingIndex).start < rangeStart)
                return token.start;
            return rangeEnd;
        }
        return rangeEnd;
    }

    SourceRange normalizeInlineStyleWrapRange(
        const QString& source,
        const int selectionStart,
        const int selectionEnd)
    {
        const int sourceLength = source.length();
        const int rawStart = boundedOffset(std::min(selectionStart, selectionEnd), sourceLength);
        const int rawEnd = boundedOffset(std::max(selectionStart, selectionEnd), sourceLength);
        const QVector<SourceTagToken> tokens = collectSourceTagTokens(source);
        if (rawStart == rawEnd)
        {
            const int insertionOffset = sourceOffsetForCollapsedInlineInsertion(tokens, rawStart);
            return {insertionOffset, insertionOffset};
        }

        SourceRange range{
            sourceStartOutsideTagTokens(tokens, rawStart),
            sourceEndOutsideTagTokens(tokens, rawEnd),
        };
        if (range.end < range.start)
            range.end = range.start;

        bool changed = true;
        while (changed)
        {
            changed = false;

            const int nextStart = trimmedLeadingInlineTagBoundary(tokens, range.start, range.end);
            if (nextStart != range.start)
            {
                range.start = nextStart;
                if (range.end < range.start)
                    range.end = range.start;
                changed = true;
                continue;
            }

            const int nextEnd = trimmedTrailingInlineTagBoundary(tokens, range.start, range.end);
            if (nextEnd != range.end)
            {
                range.end = std::max(range.start, nextEnd);
                changed = true;
            }
        }

        return range;
    }

    QString openingTagForName(const QString& tagName)
    {
        if (tagName == QLatin1String("agenda"))
            return QStringLiteral("<agenda date=\"%1\"><task done=\"false\">").arg(todayIsoDate());
        return QStringLiteral("<%1>").arg(tagName);
    }

    QString closingTagForName(const QString& tagName)
    {
        if (tagName == QLatin1String("agenda"))
            return QStringLiteral("</task></agenda>");
        return QStringLiteral("</%1>").arg(tagName);
    }

    QString canonicalSourceForGeneratedBodyTag(const QString& tagName)
    {
        if (tagName == QLatin1String("agenda"))
            return openingTagForName(tagName) + QLatin1Char(' ') + closingTagForName(tagName);
        if (tagName == QLatin1String("callout"))
            return QStringLiteral("<callout> </callout>");
        if (tagName == QLatin1String("break"))
            return QStringLiteral("</break>");
        return {};
    }

    int cursorOffsetForGeneratedBodyTag(const QString& tagName)
    {
        if (tagName == QLatin1String("agenda") || tagName == QLatin1String("callout"))
            return openingTagForName(tagName).length();
        if (tagName == QLatin1String("break"))
            return canonicalSourceForGeneratedBodyTag(tagName).length();
        return 0;
    }

    bool sourceRangeOverlapsStructuredBodyBlock(const QString& sourceText, const int selectionStart, const int selectionEnd)
    {
        static const QRegularExpression structuredBodyBlockPattern(
            QStringLiteral(R"(<(?:agenda|callout)\b[^>]*>[\s\S]*?</(?:agenda|callout)>)"),
            QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator iterator = structuredBodyBlockPattern.globalMatch(sourceText);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            if (!match.hasMatch())
                continue;
            const int blockStart = std::max(0, static_cast<int>(match.capturedStart(0)));
            const int blockEnd = std::max(blockStart, static_cast<int>(match.capturedEnd(0)));
            if (selectionStart < blockEnd && selectionEnd > blockStart)
                return true;
        }
        return false;
    }

    bool sourceTextContainsDocumentBlockTagToken(const QString& sourceText)
    {
        static const QRegularExpression documentBlockTagPattern(
            QStringLiteral(R"(<\s*/?\s*(?:agenda|task|callout|resource|break|hr)\b[^>]*>)"),
            QRegularExpression::CaseInsensitiveOption);
        return sourceText.contains(documentBlockTagPattern);
    }

    int resolveStructuredBodyTagInsertionOffset(const QString& sourceText, const int requestedInsertionOffset)
    {
        const int safeRequestedOffset = boundedOffset(requestedInsertionOffset, sourceText.length());
        static const QRegularExpression structuredBodyBlockPattern(
            QStringLiteral(R"(<(?:agenda|callout)\b[^>]*>[\s\S]*?</(?:agenda|callout)>)"),
            QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator iterator = structuredBodyBlockPattern.globalMatch(sourceText);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            if (!match.hasMatch())
                continue;
            const int blockStart = std::max(0, static_cast<int>(match.capturedStart(0)));
            const int blockEnd = std::max(blockStart, static_cast<int>(match.capturedEnd(0)));
            if (safeRequestedOffset > blockStart && safeRequestedOffset < blockEnd)
                return blockEnd;
        }
        return safeRequestedOffset;
    }

    QVariantMap notAppliedPayload(
        const QString& reason,
        const QString& sourceText,
        const int selectionStart,
        const int selectionEnd,
        const QString& tagName)
    {
        const int boundedStart = boundedOffset(std::min(selectionStart, selectionEnd), sourceText.length());
        const int boundedEnd = boundedOffset(std::max(selectionStart, selectionEnd), sourceText.length());
        return QVariantMap{
            {QStringLiteral("applied"), false},
            {QStringLiteral("cursorPosition"), boundedEnd},
            {QStringLiteral("nextSourceText"), sourceText},
            {QStringLiteral("reason"), reason},
            {QStringLiteral("selectionEnd"), boundedEnd},
            {QStringLiteral("selectionStart"), boundedStart},
            {QStringLiteral("tagName"), tagName},
        };
    }

    QVariantMap buildGeneratedBodyTagInsertionPayload(
        const QString& source,
        const int requestedInsertionOffset,
        const QString& tagName)
    {
        const QString rawSourceText = canonicalSourceForGeneratedBodyTag(tagName);
        if (rawSourceText.isEmpty())
            return notAppliedPayload(QStringLiteral("unsupported-generated-tag"), source, requestedInsertionOffset, requestedInsertionOffset, tagName);

        const int requestedOffset = boundedOffset(requestedInsertionOffset, source.length());
        const int insertionOffset = resolveStructuredBodyTagInsertionOffset(source, requestedOffset);
        const QString prefixNewline =
            insertionOffset > 0 && source.at(insertionOffset - 1) != QLatin1Char('\n')
            ? QStringLiteral("\n")
            : QString{};
        const QString suffixNewline =
            insertionOffset < source.length() && source.at(insertionOffset) != QLatin1Char('\n')
            ? QStringLiteral("\n")
            : QString{};
        const QString insertedSourceText = prefixNewline + rawSourceText + suffixNewline;
        const QString nextSourceText = source.left(insertionOffset) + insertedSourceText + source.mid(insertionOffset);
        const int cursorPosition = insertionOffset
            + prefixNewline.length()
            + boundedOffset(cursorOffsetForGeneratedBodyTag(tagName), rawSourceText.length());

        return QVariantMap{
            {QStringLiteral("applied"), nextSourceText != source},
            {QStringLiteral("cursorPosition"), cursorPosition},
            {QStringLiteral("insertedSourceText"), insertedSourceText},
            {QStringLiteral("nextSourceText"), nextSourceText},
            {QStringLiteral("rawSourceText"), rawSourceText},
            {QStringLiteral("reason"), nextSourceText != source ? QString{} : QStringLiteral("no-op")},
            {QStringLiteral("requestedInsertionOffset"), requestedOffset},
            {QStringLiteral("resolvedInsertionOffset"), insertionOffset},
            {QStringLiteral("selectionEnd"), cursorPosition},
            {QStringLiteral("selectionStart"), cursorPosition},
            {QStringLiteral("sourceOffset"), cursorPosition},
            {QStringLiteral("tagKind"), tagName},
            {QStringLiteral("tagName"), tagName},
        };
    }
} // namespace

ContentsEditorTagInsertionController::ContentsEditorTagInsertionController(QObject* parent)
    : QObject(parent)
{
}

QString ContentsEditorTagInsertionController::normalizedTagName(const QVariant& tagName) const
{
    return normalizeEditorTagName(tagName);
}

QString ContentsEditorTagInsertionController::tagNameForBodyShortcutKey(const int key) const
{
    switch (key)
    {
    case Qt::Key_A:
        return QStringLiteral("agenda");
    case Qt::Key_C:
        return QStringLiteral("callout");
    case Qt::Key_B:
    case Qt::Key_Enter:
    case Qt::Key_Return:
        return QStringLiteral("break");
    default:
        return {};
    }
}

QString ContentsEditorTagInsertionController::tagNameForShortcutKey(const int key) const
{
    switch (key)
    {
    case Qt::Key_B:
        return QStringLiteral("bold");
    case Qt::Key_I:
        return QStringLiteral("italic");
    case Qt::Key_U:
        return QStringLiteral("underline");
    case Qt::Key_E:
        return QStringLiteral("highlight");
    default:
        return {};
    }
}

QVariantMap ContentsEditorTagInsertionController::buildTagInsertionPayload(
    const QVariant& sourceText,
    const int selectionStart,
    const int selectionEnd,
    const QVariant& tagName)
{
    const QString source = normalizedText(sourceText);
    const QString normalizedTag = normalizeEditorTagName(tagName);
    if (normalizedTag.isEmpty())
        return notAppliedPayload(QStringLiteral("unsupported-tag-kind"), source, selectionStart, selectionEnd, normalizedTag);
    if (selectionStart == selectionEnd && isGeneratedBodyInsertionTag(normalizedTag))
    {
        QVariantMap payload = buildGeneratedBodyTagInsertionPayload(source, selectionStart, normalizedTag);
        emit tagInsertionPayloadBuilt(payload);
        return payload;
    }
    return buildWrappedTagInsertionPayload(source, selectionStart, selectionEnd, normalizedTag);
}

QVariantMap ContentsEditorTagInsertionController::buildWrappedTagInsertionPayload(
    const QVariant& sourceText,
    const int selectionStart,
    const int selectionEnd,
    const QVariant& tagName)
{
    const QString source = normalizedText(sourceText);
    const QString normalizedTag = normalizeEditorTagName(tagName);
    if (normalizedTag.isEmpty())
        return notAppliedPayload(QStringLiteral("unsupported-tag-kind"), source, selectionStart, selectionEnd, normalizedTag);
    if (normalizedTag == QLatin1String("plain"))
        return notAppliedPayload(QStringLiteral("plain-tag-not-wrappable"), source, selectionStart, selectionEnd, normalizedTag);
    if (!isPairedTagInsertionTag(normalizedTag))
        return notAppliedPayload(QStringLiteral("tag-not-paired-wrappable"), source, selectionStart, selectionEnd, normalizedTag);

    const int sourceLength = source.length();
    int start = boundedOffset(std::min(selectionStart, selectionEnd), sourceLength);
    int end = boundedOffset(std::max(selectionStart, selectionEnd), sourceLength);
    if (isInlineStyleInsertionTag(normalizedTag))
    {
        const SourceRange normalizedRange = normalizeInlineStyleWrapRange(source, start, end);
        start = normalizedRange.start;
        end = normalizedRange.end;
    }
    if (end <= start && isGeneratedBodyInsertionTag(normalizedTag))
    {
        QVariantMap payload = buildGeneratedBodyTagInsertionPayload(source, start, normalizedTag);
        emit tagInsertionPayloadBuilt(payload);
        return payload;
    }

    const bool bodyBlockWrap = normalizedTag == QLatin1String("agenda")
        || normalizedTag == QLatin1String("callout");
    if (bodyBlockWrap)
    {
        if (sourceRangeOverlapsStructuredBodyBlock(source, start, end))
            return notAppliedPayload(QStringLiteral("body-tag-range-overlaps-structured-block"), source, selectionStart, selectionEnd, normalizedTag);
        if (sourceTextContainsDocumentBlockTagToken(source.mid(start, end - start)))
            return notAppliedPayload(QStringLiteral("body-tag-range-contains-document-block-tag"), source, selectionStart, selectionEnd, normalizedTag);
    }

    const QString openingTag = openingTagForName(normalizedTag);
    const QString closingTag = closingTagForName(normalizedTag);

    QVariantMap payload;
    if (end <= start)
    {
        const QString nextSource = source.left(start) + openingTag + closingTag + source.mid(start);
        const int cursorPosition = start + openingTag.length();
        payload = {
            {QStringLiteral("applied"), nextSource != source},
            {QStringLiteral("cursorPosition"), cursorPosition},
            {QStringLiteral("insertedSourceText"), openingTag + closingTag},
            {QStringLiteral("nextSourceText"), nextSource},
            {QStringLiteral("reason"), nextSource != source ? QString{} : QStringLiteral("no-op")},
            {QStringLiteral("selectionEnd"), cursorPosition},
            {QStringLiteral("selectionStart"), cursorPosition},
            {QStringLiteral("tagName"), normalizedTag},
        };
    }
    else
    {
        const QString selectedSource = source.mid(start, end - start);
        const QString prefixNewline =
            bodyBlockWrap && start > 0 && source.at(start - 1) != QLatin1Char('\n')
            ? QStringLiteral("\n")
            : QString{};
        const QString suffixNewline =
            bodyBlockWrap && end < sourceLength && source.at(end) != QLatin1Char('\n')
            ? QStringLiteral("\n")
            : QString{};
        const QString replacementSourceText = openingTag + selectedSource + closingTag;
        const QString fullReplacementSourceText = prefixNewline + replacementSourceText + suffixNewline;
        const QString nextSource = source.left(start) + fullReplacementSourceText + source.mid(end);
        const int wrappedSelectionStart = start + prefixNewline.length() + openingTag.length();
        const int wrappedSelectionEnd = wrappedSelectionStart + selectedSource.length();
        payload = {
            {QStringLiteral("applied"), nextSource != source},
            {QStringLiteral("cursorPosition"), wrappedSelectionEnd},
            {QStringLiteral("insertedSourceText"), openingTag + closingTag},
            {QStringLiteral("nextSourceText"), nextSource},
            {QStringLiteral("reason"), nextSource != source ? QString{} : QStringLiteral("no-op")},
            {QStringLiteral("replacementSourceText"), fullReplacementSourceText},
            {QStringLiteral("selectionEnd"), wrappedSelectionEnd},
            {QStringLiteral("selectionStart"), wrappedSelectionStart},
            {QStringLiteral("tagKind"), normalizedTag},
            {QStringLiteral("tagName"), normalizedTag},
            {QStringLiteral("wrappedSourceText"), selectedSource},
        };
    }

    emit tagInsertionPayloadBuilt(payload);
    return payload;
}
