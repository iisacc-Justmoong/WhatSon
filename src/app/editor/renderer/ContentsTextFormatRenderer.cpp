#include "ContentsTextFormatRenderer.hpp"
#include "ContentsTextHighlightRenderer.hpp"
#include "file/note/WhatSonNoteBodyPersistence.hpp"
#include "file/note/WhatSonNoteMarkdownStyleObject.hpp"

#include <array>
#include <algorithm>
#include <limits>
#include <QColor>
#include <QFont>
#include <QRegularExpression>
#include <QStringList>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>
#include <QVector>

namespace
{
    constexpr int kResourceEditorPlaceholderLineCount = 6;

    constexpr int kSupportedInlineStyleCount = 5;

    using InlineStyleCoverageMap = std::array<QVector<bool>, kSupportedInlineStyleCount>;

    struct SourceInlineStyleState final
    {
        int logicalLength = 0;
        InlineStyleCoverageMap coverage;
    };

    bool isLogicalBreakTagName(const QString& normalizedTagName);
    QString breakDividerHtml();
    bool isClosingTagToken(const QString& token);
    bool isSelfClosingTagToken(QString token);

    enum class LiteralRenderMode
    {
        MarkdownAware,
        SourceEditing,
    };

    QString renderInlineTaggedTextFragmentToHtml(
        const QString& sourceText,
        LiteralRenderMode renderMode);

    int boundedQStringSize(const QString& text) noexcept
    {
        constexpr qsizetype maxIntSize = static_cast<qsizetype>(std::numeric_limits<int>::max());
        return static_cast<int>(std::min<qsizetype>(text.size(), maxIntSize));
    }

    QString escapeHtmlText(QString value)
    {
        value.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
        value.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
        value.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
        value.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
        value.replace(QStringLiteral("'"), QStringLiteral("&#39;"));
        return value;
    }

    QString normalizeLineEndings(QString text)
    {
        text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        text.replace(QChar::LineSeparator, QLatin1Char('\n'));
        text.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
        text.replace(QChar::Nbsp, QLatin1Char(' '));
        return text;
    }

    QString decodeHtmlEntities(QString text)
    {
        text.replace(QStringLiteral("&lt;"), QStringLiteral("<"));
        text.replace(QStringLiteral("&gt;"), QStringLiteral(">"));
        text.replace(QStringLiteral("&quot;"), QStringLiteral("\""));
        text.replace(QStringLiteral("&apos;"), QStringLiteral("'"));
        text.replace(QStringLiteral("&#39;"), QStringLiteral("'"));
        text.replace(QStringLiteral("&nbsp;"), QStringLiteral(" "));
        text.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
        return text;
    }

    int htmlEntityLengthAt(const QString& text, const int sourceOffset)
    {
        if (sourceOffset < 0
            || sourceOffset >= text.size()
            || text.at(sourceOffset) != QLatin1Char('&'))
        {
            return 0;
        }

        const int semicolonOffset = text.indexOf(QLatin1Char(';'), sourceOffset + 1);
        if (semicolonOffset <= sourceOffset)
        {
            return 0;
        }

        const QString entityToken = text.mid(sourceOffset, semicolonOffset - sourceOffset + 1).toCaseFolded();
        if (entityToken == QStringLiteral("&amp;")
            || entityToken == QStringLiteral("&lt;")
            || entityToken == QStringLiteral("&gt;")
            || entityToken == QStringLiteral("&quot;")
            || entityToken == QStringLiteral("&apos;")
            || entityToken == QStringLiteral("&#39;")
            || entityToken == QStringLiteral("&nbsp;"))
        {
            return entityToken.size();
        }

        if (entityToken.startsWith(QStringLiteral("&#x"))
            || entityToken.startsWith(QStringLiteral("&#")))
        {
            return entityToken.size();
        }

        return 0;
    }

    QString whitespaceToHtml(const QString& value)
    {
        const QString decodedValue = decodeHtmlEntities(value);
        QString html;
        html.reserve(decodedValue.size() * 6);
        for (const QChar ch : decodedValue)
        {
            if (ch == QLatin1Char(' '))
            {
                html += QStringLiteral("&nbsp;");
            }
            else if (ch == QLatin1Char('\t'))
            {
                html += QStringLiteral("&nbsp;&nbsp;&nbsp;&nbsp;");
            }
            else
            {
                html += escapeHtmlText(QString(ch));
            }
        }
        return html;
    }

    int markdownLinkTokenLengthAt(const QString& text, const int startOffset)
    {
        if (startOffset < 0
            || startOffset >= text.size()
            || text.at(startOffset) != QLatin1Char('['))
        {
            return 0;
        }

        const int labelEnd = text.indexOf(QLatin1Char(']'), startOffset + 1);
        if (labelEnd <= startOffset + 1
            || labelEnd + 1 >= text.size()
            || text.at(labelEnd + 1) != QLatin1Char('('))
        {
            return 0;
        }

        const int urlEnd = text.indexOf(QLatin1Char(')'), labelEnd + 2);
        if (urlEnd <= labelEnd + 2)
        {
            return 0;
        }

        const QString label = text.mid(startOffset + 1, labelEnd - startOffset - 1);
        const QString url = text.mid(labelEnd + 2, urlEnd - labelEnd - 2).trimmed();
        if (label.contains(QLatin1Char('\n'))
            || label.contains(QLatin1Char('\r'))
            || url.isEmpty()
            || url.contains(QLatin1Char('\n'))
            || url.contains(QLatin1Char('\r')))
        {
            return 0;
        }

        return urlEnd - startOffset + 1;
    }

    int inlineCodeTokenLengthAt(const QString& text, const int startOffset)
    {
        if (startOffset < 0
            || startOffset >= text.size()
            || text.at(startOffset) != QLatin1Char('`'))
        {
            return 0;
        }

        const int endOffset = text.indexOf(QLatin1Char('`'), startOffset + 1);
        if (endOffset <= startOffset + 1)
        {
            return 0;
        }

        return endOffset - startOffset + 1;
    }

    QString renderStyledLiteralTextToHtml(const QString& text, const LiteralRenderMode renderMode)
    {
        const QString displayText = decodeHtmlEntities(text);
        if (displayText.isEmpty())
        {
            return {};
        }

        QString html;
        html.reserve(displayText.size() * 2);

        int cursor = 0;
        while (cursor < displayText.size())
        {
            const int codeTokenLength = inlineCodeTokenLengthAt(displayText, cursor);
            if (renderMode == LiteralRenderMode::MarkdownAware && codeTokenLength > 0)
            {
                const QString token = displayText.mid(cursor, codeTokenLength);
                html += WhatSon::WhatSonNoteMarkdownStyleObject::wrapHtmlSpan(
                    WhatSon::WhatSonNoteMarkdownStyleObject::Role::InlineCode,
                    escapeHtmlText(token));
                cursor += codeTokenLength;
                continue;
            }

            const int linkTokenLength = markdownLinkTokenLengthAt(displayText, cursor);
            if (renderMode == LiteralRenderMode::MarkdownAware && linkTokenLength > 0)
            {
                const QString token = displayText.mid(cursor, linkTokenLength);
                html += WhatSon::WhatSonNoteMarkdownStyleObject::wrapHtmlSpan(
                    WhatSon::WhatSonNoteMarkdownStyleObject::Role::LinkLiteral,
                    escapeHtmlText(token));
                cursor += linkTokenLength;
                continue;
            }

            html += escapeHtmlText(QString(displayText.at(cursor)));
            ++cursor;
        }

        return html;
    }

    QString renderInlineStyleEditingSurfaceHtml(const QString& sourceText);

    QString normalizeRenderedMarkdownGlyphsToSource(QString text)
    {
        text = normalizeLineEndings(text);
        if (text.isEmpty())
        {
            return {};
        }

        QStringList lines = text.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
        bool insideCodeFence = false;
        for (QString& line : lines)
        {
            int cursor = 0;
            while (cursor < line.size()
                   && (line.at(cursor) == QLatin1Char(' ')
                       || line.at(cursor) == QLatin1Char('\t')))
            {
                ++cursor;
            }

            if (line.mid(cursor).startsWith(QStringLiteral("```")))
            {
                insideCodeFence = !insideCodeFence;
                continue;
            }

            if (insideCodeFence)
            {
                continue;
            }

            if (cursor + 1 < line.size()
                && line.at(cursor) == QChar(0x2022)
                && line.at(cursor + 1).isSpace())
            {
                const QString canonicalMarker =
                    WhatSon::WhatSonNoteMarkdownStyleObject::canonicalUnorderedListSourceMarker();
                if (!canonicalMarker.isEmpty())
                {
                    line.replace(cursor, 1, canonicalMarker);
                }
            }
        }

        return lines.join(QLatin1Char('\n'));
    }

    QString canonicalInlineStyleTagName(const QString& elementName)
    {
        const QString normalizedName = elementName.trimmed().toCaseFolded();
        if (normalizedName == QStringLiteral("bold")
            || normalizedName == QStringLiteral("b")
            || normalizedName == QStringLiteral("strong"))
        {
            return QStringLiteral("bold");
        }
        if (normalizedName == QStringLiteral("italic")
            || normalizedName == QStringLiteral("i")
            || normalizedName == QStringLiteral("em"))
        {
            return QStringLiteral("italic");
        }
        if (normalizedName == QStringLiteral("underline")
            || normalizedName == QStringLiteral("u"))
        {
            return QStringLiteral("underline");
        }
        if (normalizedName == QStringLiteral("strikethrough")
            || normalizedName == QStringLiteral("strike")
            || normalizedName == QStringLiteral("s")
            || normalizedName == QStringLiteral("del"))
        {
            return QStringLiteral("strikethrough");
        }
        return {};
    }

    QString normalizedTagElementName(const QString& tagToken)
    {
        if (tagToken.size() < 3 || !tagToken.startsWith(QLatin1Char('<')))
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
        {
            return {};
        }

        return tagToken.mid(nameStart, cursor - nameStart).trimmed().toCaseFolded();
    }

    bool isClosingTagToken(const QString& token);
    bool isSelfClosingTagToken(QString token);

    QStringList spanInlineStyleTagsFromCssDeclaration(const QString& cssDeclaration)
    {
        const QString normalizedCss = cssDeclaration.toCaseFolded();
        if (normalizedCss.isEmpty())
        {
            return {};
        }

        QStringList tags;

        bool hasHighlight = false;
        const QRegularExpression backgroundColorPattern(
            QStringLiteral(R"(background-color\s*:\s*([^;]+))"),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch backgroundColorMatch = backgroundColorPattern.match(normalizedCss);
        if (backgroundColorMatch.hasMatch())
        {
            const QString backgroundValue = backgroundColorMatch.captured(1).trimmed().toCaseFolded();
            if (!backgroundValue.isEmpty()
                && backgroundValue != QStringLiteral("transparent")
                && backgroundValue != QStringLiteral("none")
                && backgroundValue != QStringLiteral("initial")
                && backgroundValue != QStringLiteral("inherit"))
            {
                tags.push_back(QStringLiteral("highlight"));
                hasHighlight = true;
            }
        }

        const QRegularExpression fontWeightPattern(
            QStringLiteral(R"(font-weight\s*:\s*([^;]+))"),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch fontWeightMatch = fontWeightPattern.match(normalizedCss);
        if (!hasHighlight && fontWeightMatch.hasMatch())
        {
            const QString weightValue = fontWeightMatch.captured(1).trimmed();
            bool numericWeightParsed = false;
            const int numericWeight = weightValue.toInt(&numericWeightParsed);
            if (weightValue.contains(QStringLiteral("bold"))
                || (numericWeightParsed && numericWeight >= 600))
            {
                tags.push_back(QStringLiteral("bold"));
            }
        }

        const QRegularExpression fontStylePattern(
            QStringLiteral(R"(font-style\s*:\s*([^;]+))"),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch fontStyleMatch = fontStylePattern.match(normalizedCss);
        if (fontStyleMatch.hasMatch()
            && fontStyleMatch.captured(1).contains(QStringLiteral("italic"), Qt::CaseInsensitive))
        {
            tags.push_back(QStringLiteral("italic"));
        }

        const QRegularExpression textDecorationPattern(
            QStringLiteral(R"(text-decoration(?:-line)?\s*:\s*([^;]+))"),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch textDecorationMatch = textDecorationPattern.match(normalizedCss);
        if (textDecorationMatch.hasMatch())
        {
            const QString decorationValue = textDecorationMatch.captured(1).toCaseFolded();
            if (decorationValue.contains(QStringLiteral("underline")))
            {
                tags.push_back(QStringLiteral("underline"));
            }
            if (decorationValue.contains(QStringLiteral("line-through")))
            {
                tags.push_back(QStringLiteral("strikethrough"));
            }
        }

        return tags;
    }

    QString cssStyleAttributeFromTagToken(const QString& tagToken)
    {
        static const QRegularExpression stylePattern(
            QStringLiteral(R"ATTR(\bstyle\s*=\s*(?:"([^"]*)"|'([^']*)'|([^\s>]+)))ATTR"),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch styleMatch = stylePattern.match(tagToken);
        if (!styleMatch.hasMatch())
        {
            return {};
        }
        for (int captureIndex = 1; captureIndex <= 3; ++captureIndex)
        {
            const QString value = decodeHtmlEntities(styleMatch.captured(captureIndex));
            if (!value.isEmpty())
            {
                return value;
            }
        }
        return {};
    }

    QString openingEditorTagForStyle(const QString& normalizedStyleTag)
    {
        if (normalizedStyleTag == QStringLiteral("bold"))
        {
            return QStringLiteral("<strong style=\"font-weight:900;\">");
        }
        if (normalizedStyleTag == QStringLiteral("italic"))
        {
            return QStringLiteral("<span style=\"font-style:italic;\">");
        }
        if (normalizedStyleTag == QStringLiteral("underline"))
        {
            return QStringLiteral("<span style=\"text-decoration: underline;\">");
        }
        if (normalizedStyleTag == QStringLiteral("strikethrough"))
        {
            return QStringLiteral("<span style=\"text-decoration: line-through;\">");
        }
        if (normalizedStyleTag == QStringLiteral("highlight"))
        {
            return ContentsTextHighlightRenderer::highlightOpenHtmlTag();
        }
        return {};
    }

    QString closingEditorTagForStyle(const QString& normalizedStyleTag)
    {
        if (normalizedStyleTag == QStringLiteral("highlight"))
        {
            return ContentsTextHighlightRenderer::highlightCloseHtmlTag();
        }
        if (normalizedStyleTag == QStringLiteral("bold"))
        {
            return QStringLiteral("</strong>");
        }
        if (normalizedStyleTag == QStringLiteral("italic")
            || normalizedStyleTag == QStringLiteral("underline")
            || normalizedStyleTag == QStringLiteral("strikethrough"))
        {
            return QStringLiteral("</span>");
        }
        return {};
    }

    QString normalizeSupportedInlineStyleTag(const QString& rawStyleTag)
    {
        const QString normalizedRawStyleTag = rawStyleTag.trimmed().toCaseFolded();
        if (normalizedRawStyleTag == QStringLiteral("plain")
            || normalizedRawStyleTag == QStringLiteral("clear")
            || normalizedRawStyleTag == QStringLiteral("none"))
        {
            return QStringLiteral("plain");
        }

        QString normalizedStyleTag = canonicalInlineStyleTagName(rawStyleTag);
        if (normalizedStyleTag.isEmpty() && ContentsTextHighlightRenderer::isHighlightTagAlias(rawStyleTag))
        {
            normalizedStyleTag = QStringLiteral("highlight");
        }
        return normalizedStyleTag;
    }

    int supportedInlineStyleIndexForTag(const QString& rawStyleTag)
    {
        const QString normalizedStyleTag = normalizeSupportedInlineStyleTag(rawStyleTag);
        if (normalizedStyleTag == QStringLiteral("bold"))
        {
            return 0;
        }
        if (normalizedStyleTag == QStringLiteral("italic"))
        {
            return 1;
        }
        if (normalizedStyleTag == QStringLiteral("underline"))
        {
            return 2;
        }
        if (normalizedStyleTag == QStringLiteral("strikethrough"))
        {
            return 3;
        }
        if (normalizedStyleTag == QStringLiteral("highlight"))
        {
            return 4;
        }
        return -1;
    }

    QString sourceInlineStyleOpenTagForIndex(const int index)
    {
        switch (index)
        {
        case 0:
            return QStringLiteral("<bold>");
        case 1:
            return QStringLiteral("<italic>");
        case 2:
            return QStringLiteral("<underline>");
        case 3:
            return QStringLiteral("<strikethrough>");
        case 4:
            return QStringLiteral("<highlight>");
        default:
            return {};
        }
    }

    QString sourceInlineStyleCloseTagForIndex(const int index)
    {
        switch (index)
        {
        case 0:
            return QStringLiteral("</bold>");
        case 1:
            return QStringLiteral("</italic>");
        case 2:
            return QStringLiteral("</underline>");
        case 3:
            return QStringLiteral("</strikethrough>");
        case 4:
            return QStringLiteral("</highlight>");
        default:
            return {};
        }
    }

    SourceInlineStyleState buildSourceInlineStyleState(const QString& sourceText)
    {
        SourceInlineStyleState state;
        const QString normalizedSourceText = normalizeLineEndings(sourceText);
        std::array<int, kSupportedInlineStyleCount> styleDepths{};

        auto appendCoverageEntry = [&state, &styleDepths]() {
            for (int index = 0; index < kSupportedInlineStyleCount; ++index)
            {
                state.coverage[static_cast<std::size_t>(index)].push_back(styleDepths[static_cast<std::size_t>(index)] > 0);
            }
            state.logicalLength += 1;
        };

        int sourceOffset = 0;
        while (sourceOffset < normalizedSourceText.size())
        {
            if (normalizedSourceText.at(sourceOffset) == QLatin1Char('<'))
            {
                const int tagEnd = normalizedSourceText.indexOf(QLatin1Char('>'), sourceOffset + 1);
                if (tagEnd > sourceOffset)
                {
                    const QString fullTagToken =
                        normalizedSourceText.mid(sourceOffset, tagEnd - sourceOffset + 1);
                    const QString normalizedTagName = normalizedTagElementName(fullTagToken);
                    const bool closingTag = isClosingTagToken(fullTagToken);
                    const bool selfClosingTag = isSelfClosingTagToken(fullTagToken);
                    const int styleIndex = supportedInlineStyleIndexForTag(normalizedTagName);
                    if (styleIndex >= 0 && !selfClosingTag)
                    {
                        int& styleDepth = styleDepths[static_cast<std::size_t>(styleIndex)];
                        if (closingTag)
                        {
                            styleDepth = std::max(0, styleDepth - 1);
                        }
                        else
                        {
                            styleDepth += 1;
                        }
                        sourceOffset = tagEnd + 1;
                        continue;
                    }
                    if (isLogicalBreakTagName(normalizedTagName))
                    {
                        appendCoverageEntry();
                        sourceOffset = tagEnd + 1;
                        continue;
                    }
                    sourceOffset = tagEnd + 1;
                    continue;
                }
            }

            const int entityLength = htmlEntityLengthAt(normalizedSourceText, sourceOffset);
            if (entityLength > 0)
            {
                appendCoverageEntry();
                sourceOffset += entityLength;
                continue;
            }

            appendCoverageEntry();
            sourceOffset += 1;
        }

        return state;
    }

    bool selectionFullyHasSourceInlineStyle(
        const QVector<bool>& styleCoverage,
        const int selectionStart,
        const int selectionEnd)
    {
        if (selectionEnd <= selectionStart)
        {
            return false;
        }
        for (int logicalOffset = selectionStart; logicalOffset < selectionEnd; ++logicalOffset)
        {
            if (logicalOffset < 0
                || logicalOffset >= styleCoverage.size()
                || !styleCoverage.at(logicalOffset))
            {
                return false;
            }
        }
        return true;
    }

    void applySourceInlineStyleCoverageRange(
        QVector<bool>* styleCoverage,
        const int selectionStart,
        const int selectionEnd,
        const bool nextActive)
    {
        if (styleCoverage == nullptr || selectionEnd <= selectionStart)
        {
            return;
        }
        const int boundedStart = std::max(0, selectionStart);
        const int boundedEnd = std::min(selectionEnd, static_cast<int>(styleCoverage->size()));
        for (int logicalOffset = boundedStart; logicalOffset < boundedEnd; ++logicalOffset)
        {
            (*styleCoverage)[logicalOffset] = nextActive;
        }
    }

    int markdownLinePrefixLength(const QString& lineText)
    {
        if (lineText.isEmpty())
        {
            return 0;
        }

        int cursor = 0;
        while (cursor < lineText.size()
               && (lineText.at(cursor) == QLatin1Char(' ')
                   || lineText.at(cursor) == QLatin1Char('\t')))
        {
            ++cursor;
        }

        const int leadingIndentLength = cursor;
        if (cursor >= lineText.size())
        {
            return leadingIndentLength;
        }

        if (lineText.mid(cursor).startsWith(QStringLiteral("```")))
        {
            return cursor + 3;
        }

        if (lineText.at(cursor) == QLatin1Char('>'))
        {
            int prefixEnd = cursor + 1;
            while (prefixEnd < lineText.size()
                   && (lineText.at(prefixEnd) == QLatin1Char(' ')
                       || lineText.at(prefixEnd) == QLatin1Char('\t')))
            {
                ++prefixEnd;
            }
            return prefixEnd;
        }

        int headingCursor = cursor;
        while (headingCursor < lineText.size()
               && lineText.at(headingCursor) == QLatin1Char('#')
               && headingCursor - cursor < 6)
        {
            ++headingCursor;
        }
        if (headingCursor > cursor
            && headingCursor < lineText.size()
            && (lineText.at(headingCursor) == QLatin1Char(' ')
                || lineText.at(headingCursor) == QLatin1Char('\t')))
        {
            int prefixEnd = headingCursor;
            while (prefixEnd < lineText.size()
                   && (lineText.at(prefixEnd) == QLatin1Char(' ')
                       || lineText.at(prefixEnd) == QLatin1Char('\t')))
            {
                ++prefixEnd;
            }
            return prefixEnd;
        }

        const QChar marker = lineText.at(cursor);
        if (marker == QLatin1Char('-')
            || marker == QLatin1Char('+')
            || marker == QLatin1Char('*')
            || marker == QChar(0x2022))
        {
            int prefixEnd = cursor + 1;
            while (prefixEnd < lineText.size()
                   && (lineText.at(prefixEnd) == QLatin1Char(' ')
                       || lineText.at(prefixEnd) == QLatin1Char('\t')))
            {
                ++prefixEnd;
            }
            if (prefixEnd > cursor + 1)
            {
                return prefixEnd;
            }
        }

        int numberCursor = cursor;
        while (numberCursor < lineText.size()
               && lineText.at(numberCursor).isDigit())
        {
            ++numberCursor;
        }
        if (numberCursor > cursor
            && numberCursor < lineText.size()
            && (lineText.at(numberCursor) == QLatin1Char('.')
                || lineText.at(numberCursor) == QLatin1Char(')')))
        {
            int prefixEnd = numberCursor + 1;
            while (prefixEnd < lineText.size()
                   && (lineText.at(prefixEnd) == QLatin1Char(' ')
                       || lineText.at(prefixEnd) == QLatin1Char('\t')))
            {
                ++prefixEnd;
            }
            if (prefixEnd > numberCursor + 1)
            {
                return prefixEnd;
            }
        }

        return leadingIndentLength;
    }

    QChar logicalCharForHtmlEntity(const QString& entityToken)
    {
        const QString lowered = entityToken.toCaseFolded();
        if (lowered == QStringLiteral("&nbsp;"))
        {
            return QLatin1Char(' ');
        }
        if (lowered == QStringLiteral("&lt;"))
        {
            return QLatin1Char('<');
        }
        if (lowered == QStringLiteral("&gt;"))
        {
            return QLatin1Char('>');
        }
        if (lowered == QStringLiteral("&amp;"))
        {
            return QLatin1Char('&');
        }
        if (lowered == QStringLiteral("&quot;"))
        {
            return QLatin1Char('"');
        }
        if (lowered == QStringLiteral("&apos;")
            || lowered == QStringLiteral("&#39;"))
        {
            return QLatin1Char('\'');
        }
        if (lowered.startsWith(QStringLiteral("&#x"))
            || lowered.startsWith(QStringLiteral("&#")))
        {
            return QChar(0xFFFD);
        }
        return QChar(0xFFFD);
    }

    QString logicalTextForInlineStyleCoverage(const QString& sourceText)
    {
        const QString normalizedSourceText = normalizeLineEndings(sourceText);
        if (normalizedSourceText.isEmpty())
        {
            return {};
        }

        QString logicalText;
        logicalText.reserve(normalizedSourceText.size());

        int sourceOffset = 0;
        while (sourceOffset < normalizedSourceText.size())
        {
            if (normalizedSourceText.at(sourceOffset) == QLatin1Char('<'))
            {
                const int tagEnd = normalizedSourceText.indexOf(QLatin1Char('>'), sourceOffset + 1);
                if (tagEnd > sourceOffset)
                {
                    const QString fullTagToken =
                        normalizedSourceText.mid(sourceOffset, tagEnd - sourceOffset + 1);
                    const QString normalizedTagName = normalizedTagElementName(fullTagToken);
                    if (isLogicalBreakTagName(normalizedTagName))
                    {
                        logicalText += QLatin1Char('\n');
                    }
                    sourceOffset = tagEnd + 1;
                    continue;
                }
            }

            const int entityLength = htmlEntityLengthAt(normalizedSourceText, sourceOffset);
            if (entityLength > 0)
            {
                logicalText += logicalCharForHtmlEntity(
                    normalizedSourceText.mid(sourceOffset, entityLength));
                sourceOffset += entityLength;
                continue;
            }

            logicalText += normalizedSourceText.at(sourceOffset);
            sourceOffset += 1;
        }

        return logicalText;
    }

    void clearInlineStyleCoverageOnLinePrefixes(
        InlineStyleCoverageMap* coverage,
        const QString& sourceText,
        const int logicalLength)
    {
        if (coverage == nullptr || logicalLength <= 0)
        {
            return;
        }

        const QString logicalText = logicalTextForInlineStyleCoverage(sourceText);
        if (logicalText.isEmpty())
        {
            return;
        }

        int lineStartOffset = 0;
        while (lineStartOffset <= logicalText.size())
        {
            const int lineEndOffset = logicalText.indexOf(QLatin1Char('\n'), lineStartOffset);
            const int boundedLineEndOffset = lineEndOffset >= 0 ? lineEndOffset : logicalText.size();
            const QString lineText = logicalText.mid(
                lineStartOffset,
                std::max(0, boundedLineEndOffset - lineStartOffset));
            const int protectedPrefixLength = markdownLinePrefixLength(lineText);
            if (protectedPrefixLength > 0)
            {
                const int protectedPrefixEndOffset = std::min(
                    lineStartOffset + protectedPrefixLength,
                    logicalLength);
                for (int styleIndex = 0; styleIndex < kSupportedInlineStyleCount; ++styleIndex)
                {
                    applySourceInlineStyleCoverageRange(
                        &(*coverage)[static_cast<std::size_t>(styleIndex)],
                        lineStartOffset,
                        protectedPrefixEndOffset,
                        false);
                }
            }

            if (lineEndOffset < 0)
            {
                break;
            }
            lineStartOffset = lineEndOffset + 1;
        }
    }

    void synchronizeOutputInlineStyleState(
        QString* output,
        QVector<int>* emittedStyleOrder,
        const InlineStyleCoverageMap& desiredCoverage,
        const int logicalOffset)
    {
        if (output == nullptr || emittedStyleOrder == nullptr)
        {
            return;
        }

        QVector<int> targetStyleOrder;
        targetStyleOrder.reserve(kSupportedInlineStyleCount);
        for (int index = 0; index < kSupportedInlineStyleCount; ++index)
        {
            const QVector<bool>& coverage = desiredCoverage[static_cast<std::size_t>(index)];
            const bool shouldBeActive = logicalOffset >= 0
                && logicalOffset < coverage.size()
                && coverage.at(logicalOffset);
            if (shouldBeActive)
            {
                targetStyleOrder.push_back(index);
            }
        }

        int commonPrefixLength = 0;
        while (commonPrefixLength < emittedStyleOrder->size()
               && commonPrefixLength < targetStyleOrder.size()
               && emittedStyleOrder->at(commonPrefixLength) == targetStyleOrder.at(commonPrefixLength))
        {
            ++commonPrefixLength;
        }

        for (int index = emittedStyleOrder->size() - 1; index >= commonPrefixLength; --index)
        {
            *output += sourceInlineStyleCloseTagForIndex(emittedStyleOrder->at(index));
            emittedStyleOrder->removeAt(index);
        }

        for (int index = commonPrefixLength; index < targetStyleOrder.size(); ++index)
        {
            const int styleIndex = targetStyleOrder.at(index);
            *output += sourceInlineStyleOpenTagForIndex(styleIndex);
            emittedStyleOrder->push_back(styleIndex);
        }
    }

    QString rebuildSourceTextWithInlineStyleCoverage(
        const QString& sourceText,
        const InlineStyleCoverageMap& desiredCoverage,
        const int logicalLength)
    {
        const QString normalizedSourceText = normalizeLineEndings(sourceText);
        QString output;
        output.reserve(normalizedSourceText.size() + 32);

        QVector<int> emittedStyleOrder;
        emittedStyleOrder.reserve(kSupportedInlineStyleCount);

        int logicalOffset = 0;
        int sourceOffset = 0;
        while (sourceOffset < normalizedSourceText.size())
        {
            if (normalizedSourceText.at(sourceOffset) == QLatin1Char('<'))
            {
                const int tagEnd = normalizedSourceText.indexOf(QLatin1Char('>'), sourceOffset + 1);
                if (tagEnd > sourceOffset)
                {
                    const QString fullTagToken =
                        normalizedSourceText.mid(sourceOffset, tagEnd - sourceOffset + 1);
                    const QString normalizedTagName = normalizedTagElementName(fullTagToken);
                    const bool selfClosingTag = isSelfClosingTagToken(fullTagToken);
                    const int styleIndex = supportedInlineStyleIndexForTag(normalizedTagName);
                    if (styleIndex >= 0 && !selfClosingTag)
                    {
                        sourceOffset = tagEnd + 1;
                        continue;
                    }

                    synchronizeOutputInlineStyleState(
                        &output,
                        &emittedStyleOrder,
                        desiredCoverage,
                        logicalOffset);
                    output += fullTagToken;
                    sourceOffset = tagEnd + 1;
                    if (isLogicalBreakTagName(normalizedTagName))
                    {
                        logicalOffset += 1;
                    }
                    continue;
                }
            }

            synchronizeOutputInlineStyleState(
                &output,
                &emittedStyleOrder,
                desiredCoverage,
                logicalOffset);

            const int entityLength = htmlEntityLengthAt(normalizedSourceText, sourceOffset);
            if (entityLength > 0)
            {
                output += normalizedSourceText.mid(sourceOffset, entityLength);
                sourceOffset += entityLength;
                logicalOffset += 1;
                continue;
            }

            output += normalizedSourceText.at(sourceOffset);
            sourceOffset += 1;
            logicalOffset += 1;
        }

        synchronizeOutputInlineStyleState(
            &output,
            &emittedStyleOrder,
            desiredCoverage,
            logicalLength);
        return output;
    }

    QTextCharFormat textCharFormatForInlineStyle(const QString& normalizedStyleTag)
    {
        QTextCharFormat format;
        if (normalizedStyleTag == QStringLiteral("bold"))
        {
            format.setFontWeight(QFont::Black);
        }
        else if (normalizedStyleTag == QStringLiteral("italic"))
        {
            format.setFontItalic(true);
        }
        else if (normalizedStyleTag == QStringLiteral("underline"))
        {
            format.setFontUnderline(true);
        }
        else if (normalizedStyleTag == QStringLiteral("strikethrough"))
        {
            format.setFontStrikeOut(true);
        }
        else if (normalizedStyleTag == QStringLiteral("highlight"))
        {
            format.setBackground(QColor(QStringLiteral("#8A4B00")));
            format.setForeground(QColor(QStringLiteral("#D6AE58")));
            format.setFontWeight(QFont::DemiBold);
        }
        return format;
    }

    QTextCharFormat plainTextCharFormat()
    {
        QTextCharFormat format;
        format.setFontWeight(QFont::Normal);
        format.setFontItalic(false);
        format.setFontUnderline(false);
        format.setFontStrikeOut(false);
        format.clearProperty(QTextFormat::ForegroundBrush);
        format.clearProperty(QTextFormat::BackgroundBrush);
        return format;
    }

    bool textCharFormatHasInlineStyle(const QTextCharFormat& format, const QString& normalizedStyleTag)
    {
        if (normalizedStyleTag == QStringLiteral("bold"))
        {
            return format.fontWeight() >= QFont::DemiBold;
        }
        if (normalizedStyleTag == QStringLiteral("italic"))
        {
            return format.fontItalic();
        }
        if (normalizedStyleTag == QStringLiteral("underline"))
        {
            return format.fontUnderline();
        }
        if (normalizedStyleTag == QStringLiteral("strikethrough"))
        {
            return format.fontStrikeOut();
        }
        if (normalizedStyleTag == QStringLiteral("highlight"))
        {
            const QColor backgroundColor = format.background().color();
            return format.background().style() != Qt::NoBrush
                && backgroundColor.isValid()
                && backgroundColor.alpha() > 0
                && backgroundColor != Qt::transparent;
        }
        return false;
    }

    bool selectionFullyHasInlineStyle(const QTextCursor& selectionCursor, const QString& normalizedStyleTag)
    {
        if (!selectionCursor.hasSelection() || normalizedStyleTag.isEmpty())
        {
            return false;
        }

        const int selectionStart = selectionCursor.selectionStart();
        const int selectionEnd = selectionCursor.selectionEnd();
        if (selectionEnd <= selectionStart)
        {
            return false;
        }

        bool sawOverlappingFragment = false;
        for (QTextBlock block = selectionCursor.document()->findBlock(selectionStart);
             block.isValid() && block.position() < selectionEnd;
             block = block.next())
        {
            for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it)
            {
                const QTextFragment fragment = it.fragment();
                if (!fragment.isValid())
                {
                    continue;
                }

                const int fragmentStart = fragment.position();
                const int fragmentEnd = fragmentStart + fragment.length();
                if (fragmentEnd <= selectionStart || fragmentStart >= selectionEnd)
                {
                    continue;
                }

                sawOverlappingFragment = true;
                if (!textCharFormatHasInlineStyle(fragment.charFormat(), normalizedStyleTag))
                {
                    return false;
                }
            }
        }

        return sawOverlappingFragment;
    }

    QString plainSelectedText(const QTextCursor& selectionCursor)
    {
        QString selectedText = selectionCursor.selectedText();
        selectedText.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
        selectedText.replace(QChar::LineSeparator, QLatin1Char('\n'));
        return selectedText;
    }

    int boundedDocumentSelectionPosition(const QTextDocument& document, int position) noexcept
    {
        const int maxPosition = std::max(0, document.characterCount() - 1);
        return std::clamp(position, 0, maxPosition);
    }

    bool isClosingTagToken(const QString& token)
    {
        for (int index = 1; index < token.size(); ++index)
        {
            const QChar ch = token.at(index);
            if (!ch.isSpace())
            {
                return ch == QLatin1Char('/');
            }
        }
        return false;
    }

    bool isSelfClosingTagToken(QString token)
    {
        token = token.trimmed();
        return token.endsWith(QStringLiteral("/>"));
    }

    bool isLogicalBreakTagName(const QString& normalizedTagName)
    {
        return normalizedTagName == QStringLiteral("br")
            || normalizedTagName == QStringLiteral("break")
            || normalizedTagName == QStringLiteral("hr");
    }

    QString breakDividerHtml()
    {
        return QStringLiteral("<hr/>");
    }

    void closeMatchingTag(
        QStringList* openStyleTags,
        QString* htmlOutput,
        const QString& normalizedStyleTag)
    {
        if (openStyleTags == nullptr || htmlOutput == nullptr || normalizedStyleTag.isEmpty())
        {
            return;
        }

        const int openTagIndex = openStyleTags->lastIndexOf(normalizedStyleTag);
        if (openTagIndex < 0)
        {
            return;
        }

        for (int index = openStyleTags->size() - 1; index >= openTagIndex; --index)
        {
            const QString openTag = openStyleTags->at(index);
            *htmlOutput += closingEditorTagForStyle(openTag);
            openStyleTags->removeAt(index);
        }
    }

    qsizetype boundedTextPosition(const QString& text, const qsizetype position) noexcept
    {
        return std::clamp(position, qsizetype{0}, text.size());
    }

    qsizetype nextTagTokenStart(const QString& text, const QString& token, const qsizetype from)
    {
        const qsizetype safeFrom = boundedTextPosition(text, from);
        const qsizetype resolvedIndex = text.indexOf(token, safeFrom, Qt::CaseInsensitive);
        return resolvedIndex >= 0 ? resolvedIndex : text.size();
    }

    struct StructuredBlockRange final
    {
        qsizetype blockEnd = 0;
        qsizetype contentEnd = 0;
        qsizetype contentStart = 0;
        bool hasCloseTag = false;
    };

    StructuredBlockRange structuredBlockRange(
        const QString& sourceText,
        const qsizetype contentStart,
        const QString& closeTag,
        const QStringList& boundaryOpenTags)
    {
        StructuredBlockRange range;
        range.contentStart = boundedTextPosition(sourceText, contentStart);
        range.contentEnd = sourceText.size();
        range.blockEnd = sourceText.size();

        const qsizetype closeTagStart = nextTagTokenStart(sourceText, closeTag, range.contentStart);
        if (closeTagStart < range.contentEnd)
        {
            range.contentEnd = closeTagStart;
        }

        for (const QString& boundaryOpenTag : boundaryOpenTags)
        {
            const qsizetype boundaryStart = nextTagTokenStart(sourceText, boundaryOpenTag, range.contentStart);
            if (boundaryStart < range.contentEnd)
            {
                range.contentEnd = boundaryStart;
            }
        }

        range.hasCloseTag = closeTagStart < sourceText.size() && closeTagStart == range.contentEnd;
        range.blockEnd = range.hasCloseTag
            ? boundedTextPosition(sourceText, closeTagStart + closeTag.size())
            : range.contentEnd;
        return range;
    }

    QString renderStructuredLiteralLineToHtml(
        const QString& lineText,
        const LiteralRenderMode renderMode)
    {
        const QString normalizedLine = normalizeLineEndings(lineText);
        if (normalizedLine.isEmpty())
        {
            return QStringLiteral("&nbsp;");
        }

        if (normalizedLine.trimmed().isEmpty())
        {
            return whitespaceToHtml(normalizedLine);
        }

        const QString lineHtml = renderInlineTaggedTextFragmentToHtml(normalizedLine, renderMode);
        if (lineHtml.trimmed().isEmpty())
        {
            return QStringLiteral("&nbsp;");
        }

        return lineHtml;
    }

    QString renderStructuredLiteralTextToHtml(
        const QString& sourceText,
        const LiteralRenderMode renderMode)
    {
        const QString normalizedText = normalizeLineEndings(sourceText);
        const QStringList lines = normalizedText.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
        QStringList htmlLines;
        htmlLines.reserve(lines.size());
        for (const QString& line : lines)
        {
            htmlLines.push_back(renderStructuredLiteralLineToHtml(line, renderMode));
        }
        if (htmlLines.isEmpty())
        {
            htmlLines.push_back(QStringLiteral("&nbsp;"));
        }
        return htmlLines.join(QStringLiteral("<br/>"));
    }

    QString renderAgendaEditorBlockToHtml(
        const QString& sourceText,
        const qsizetype agendaContentStart,
        const LiteralRenderMode renderMode)
    {
        static const QRegularExpression taskOpenTagPattern(
            QStringLiteral(R"(<task\b([^>]*)>)"),
            QRegularExpression::CaseInsensitiveOption);

        const StructuredBlockRange agendaRange = structuredBlockRange(
            sourceText,
            agendaContentStart,
            QStringLiteral("</agenda>"),
            {
                QStringLiteral("<agenda"),
                QStringLiteral("<callout")
            });
        const QString innerSource = sourceText.mid(
            agendaRange.contentStart,
            agendaRange.contentEnd - agendaRange.contentStart);

        QStringList renderedTasks;
        QRegularExpressionMatchIterator taskIterator = taskOpenTagPattern.globalMatch(innerSource);
        while (taskIterator.hasNext())
        {
            const QRegularExpressionMatch taskMatch = taskIterator.next();
            if (!taskMatch.hasMatch())
            {
                continue;
            }

            const qsizetype taskOpenEnd = taskMatch.capturedEnd(0);
            if (taskOpenEnd <= 0)
            {
                continue;
            }

            const qsizetype taskCloseStart = nextTagTokenStart(
                innerSource,
                QStringLiteral("</task>"),
                taskOpenEnd);
            const qsizetype nextTaskOpenStart = nextTagTokenStart(
                innerSource,
                QStringLiteral("<task"),
                taskOpenEnd);

            qsizetype taskContentEnd = innerSource.size();
            if (taskCloseStart < taskContentEnd)
            {
                taskContentEnd = taskCloseStart;
            }
            if (nextTaskOpenStart < taskContentEnd)
            {
                taskContentEnd = nextTaskOpenStart;
            }

            renderedTasks.push_back(renderStructuredLiteralTextToHtml(
                innerSource.mid(taskOpenEnd, taskContentEnd - taskOpenEnd),
                renderMode));
        }

        if (renderedTasks.isEmpty())
        {
            renderedTasks.push_back(renderStructuredLiteralTextToHtml(innerSource, renderMode));
        }

        return QStringLiteral(
                   "<div style=\"margin:0;padding:27px 16px 8px 39px;\">%1</div>")
            .arg(renderedTasks.join(QStringLiteral("<br/>")));
    }

    QString renderCalloutEditorBlockToHtml(
        const QString& sourceText,
        const qsizetype calloutContentStart,
        const LiteralRenderMode renderMode)
    {
        const StructuredBlockRange calloutRange = structuredBlockRange(
            sourceText,
            calloutContentStart,
            QStringLiteral("</callout>"),
            {
                QStringLiteral("<callout"),
                QStringLiteral("<agenda")
            });
        return QStringLiteral(
                   "<div style=\"margin:0;padding:4px 4px 4px 17px;\">%1</div>")
            .arg(renderStructuredLiteralTextToHtml(
                sourceText.mid(calloutRange.contentStart, calloutRange.contentEnd - calloutRange.contentStart),
                renderMode));
    }

    QString renderResourceEditorBlockToHtml()
    {
        QStringList placeholderLines;
        placeholderLines.reserve(kResourceEditorPlaceholderLineCount);
        for (int index = 0; index < kResourceEditorPlaceholderLineCount; ++index)
        {
            placeholderLines.push_back(QStringLiteral("&nbsp;"));
        }
        return QStringLiteral("<div style=\"margin:0;padding:0;\">%1</div>")
            .arg(placeholderLines.join(QStringLiteral("<br/>")));
    }

    QString renderInlineTaggedTextFragmentToHtml(
        const QString& sourceText,
        const LiteralRenderMode renderMode = LiteralRenderMode::MarkdownAware)
    {
        const QString normalizedText = normalizeLineEndings(sourceText);
        if (normalizedText.isEmpty())
        {
            return {};
        }

        static const QRegularExpression tagPattern(
            QStringLiteral(R"(<\s*/?\s*([A-Za-z_][A-Za-z0-9_.:-]*)\b[^>]*>)"));

        QString html;
        QStringList openStyleTags;
        QVector<QStringList> spanStyleStack;
        qsizetype cursor = 0;

        QRegularExpressionMatchIterator iterator = tagPattern.globalMatch(normalizedText);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            const qsizetype tagStart = match.capturedStart(0);
            const qsizetype tagEnd = match.capturedEnd(0);
            if (tagStart < cursor)
            {
                continue;
            }
            if (tagStart < 0 || tagEnd <= tagStart)
            {
                continue;
            }

            if (tagStart > cursor)
            {
                html += renderStyledLiteralTextToHtml(normalizedText.mid(cursor, tagStart - cursor), renderMode);
            }

            const QString fullTagToken = match.captured(0);
            const QString rawTagName = match.captured(1);
            const QString normalizedTagName = rawTagName.trimmed().toCaseFolded();
            const bool closingTag = isClosingTagToken(fullTagToken);
            const bool selfClosingTag = isSelfClosingTagToken(fullTagToken);

            if (normalizedTagName == QStringLiteral("resource"))
            {
                if (!closingTag)
                {
                    html += renderResourceEditorBlockToHtml();
                }
                cursor = tagEnd;
                continue;
            }

            if (!closingTag
                && normalizedTagName == QStringLiteral("agenda"))
            {
                html += renderAgendaEditorBlockToHtml(normalizedText, tagEnd, renderMode);
                cursor = structuredBlockRange(
                    normalizedText,
                    tagEnd,
                    QStringLiteral("</agenda>"),
                    {
                        QStringLiteral("<agenda"),
                        QStringLiteral("<callout")
                    }).blockEnd;
                continue;
            }

            if (!closingTag
                && normalizedTagName == QStringLiteral("callout"))
            {
                html += renderCalloutEditorBlockToHtml(normalizedText, tagEnd, renderMode);
                cursor = structuredBlockRange(
                    normalizedText,
                    tagEnd,
                    QStringLiteral("</callout>"),
                    {
                        QStringLiteral("<callout"),
                        QStringLiteral("<agenda")
                    }).blockEnd;
                continue;
            }

            if (normalizedTagName == QStringLiteral("task"))
            {
                cursor = tagEnd;
                continue;
            }

            if (normalizedTagName == QStringLiteral("break")
                || normalizedTagName == QStringLiteral("hr"))
            {
                html += breakDividerHtml();
                cursor = tagEnd;
                continue;
            }

            if (normalizedTagName == QStringLiteral("br"))
            {
                html += QStringLiteral("<br/>");
                cursor = tagEnd;
                continue;
            }

            if (normalizedTagName == QStringLiteral("span"))
            {
                if (closingTag)
                {
                    if (!spanStyleStack.isEmpty())
                    {
                        const QStringList spanTags = spanStyleStack.takeLast();
                        for (int index = spanTags.size() - 1; index >= 0; --index)
                        {
                            closeMatchingTag(&openStyleTags, &html, spanTags.at(index));
                        }
                    }
                    else
                    {
                        html += escapeHtmlText(fullTagToken);
                    }
                }
                else if (selfClosingTag)
                {
                    html += escapeHtmlText(fullTagToken);
                }
                else
                {
                    const QStringList spanTags = spanInlineStyleTagsFromCssDeclaration(
                        cssStyleAttributeFromTagToken(fullTagToken));
                    if (spanTags.isEmpty())
                    {
                        html += escapeHtmlText(fullTagToken);
                    }
                    else
                    {
                        for (const QString& spanTag : spanTags)
                        {
                            html += openingEditorTagForStyle(spanTag);
                            openStyleTags.append(spanTag);
                        }
                        spanStyleStack.push_back(spanTags);
                    }
                }
                cursor = tagEnd;
                continue;
            }

            QString styleTag = canonicalInlineStyleTagName(rawTagName);
            if (styleTag.isEmpty() && ContentsTextHighlightRenderer::isHighlightTagAlias(rawTagName))
            {
                styleTag = QStringLiteral("highlight");
            }
            if (!styleTag.isEmpty())
            {
                if (closingTag)
                {
                    closeMatchingTag(&openStyleTags, &html, styleTag);
                }
                else if (!selfClosingTag)
                {
                    html += openingEditorTagForStyle(styleTag);
                    openStyleTags.append(styleTag);
                }
                cursor = tagEnd;
                continue;
            }

            html += escapeHtmlText(fullTagToken);
            cursor = tagEnd;
        }

        if (cursor < normalizedText.size())
        {
            html += renderStyledLiteralTextToHtml(normalizedText.mid(cursor), renderMode);
        }

        for (int index = openStyleTags.size() - 1; index >= 0; --index)
        {
            html += closingEditorTagForStyle(openStyleTags.at(index));
        }

        return html;
    }

    QString renderMarkdownAwareTextToHtml(const QString& sourceText)
    {
        const QString normalizedText = normalizeLineEndings(sourceText);
        if (normalizedText.isEmpty())
        {
            return {};
        }

        static const QRegularExpression unorderedListPattern(
            QStringLiteral(R"(^([ \t]*)([-+*\u2022])(\s+)(.*)$)"));
        static const QRegularExpression orderedListPattern(
            QStringLiteral(R"(^([ \t]*)(\d+)([.)])(\s+)(.*)$)"));
        static const QRegularExpression headingPattern(
            QStringLiteral(R"(^([ \t]*)(#{1,6})(\s+)(.*)$)"));
        static const QRegularExpression blockquotePattern(
            QStringLiteral(R"(^([ \t]*)(>)(\s?)(.*)$)"));
        static const QRegularExpression codeFencePattern(
            QStringLiteral(R"(^([ \t]*)```(.*)$)"));
        static const QRegularExpression horizontalRulePattern(
            QStringLiteral(R"(^[ \t]{0,3}(([-*_])(?:[ \t]*\2){2,})[ \t]*$)"));

        const QStringList lines = normalizedText.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
        QStringList htmlLines;
        htmlLines.reserve(lines.size());

        bool insideCodeFence = false;
        for (const QString& line : lines)
        {
            const QRegularExpressionMatch codeFenceMatch = codeFencePattern.match(line);
            if (codeFenceMatch.hasMatch())
            {
                htmlLines.push_back(
                    whitespaceToHtml(codeFenceMatch.captured(1))
                    + WhatSon::WhatSonNoteMarkdownStyleObject::wrapHtmlSpan(
                        WhatSon::WhatSonNoteMarkdownStyleObject::Role::CodeFence,
                        escapeHtmlText(decodeHtmlEntities(line.mid(codeFenceMatch.capturedLength(1))))));
                insideCodeFence = !insideCodeFence;
                continue;
            }

            if (insideCodeFence)
            {
                htmlLines.push_back(
                    WhatSon::WhatSonNoteMarkdownStyleObject::wrapHtmlSpan(
                        WhatSon::WhatSonNoteMarkdownStyleObject::Role::CodeBody,
                        whitespaceToHtml(line)));
                continue;
            }

            const QRegularExpressionMatch unorderedListMatch = unorderedListPattern.match(line);
            if (unorderedListMatch.hasMatch())
            {
                const QString trailingSpacing =
                    whitespaceToHtml(unorderedListMatch.captured(3).mid(1));
                htmlLines.push_back(
                    whitespaceToHtml(unorderedListMatch.captured(1))
                    + WhatSon::WhatSonNoteMarkdownStyleObject::wrapHtmlSpan(
                        WhatSon::WhatSonNoteMarkdownStyleObject::Role::UnorderedListMarker,
                        QString(QChar(0x2022)))
                    + QStringLiteral("&nbsp;")
                    + trailingSpacing
                    + renderInlineTaggedTextFragmentToHtml(unorderedListMatch.captured(4)));
                continue;
            }

            const QRegularExpressionMatch orderedListMatch = orderedListPattern.match(line);
            if (orderedListMatch.hasMatch())
            {
                const QString markerToken =
                    orderedListMatch.captured(2) + orderedListMatch.captured(3);
                htmlLines.push_back(
                    whitespaceToHtml(orderedListMatch.captured(1))
                    + WhatSon::WhatSonNoteMarkdownStyleObject::wrapHtmlSpan(
                        WhatSon::WhatSonNoteMarkdownStyleObject::Role::OrderedListMarker,
                        escapeHtmlText(markerToken))
                    + whitespaceToHtml(orderedListMatch.captured(4))
                    + renderInlineTaggedTextFragmentToHtml(orderedListMatch.captured(5)));
                continue;
            }

            const QRegularExpressionMatch headingMatch = headingPattern.match(line);
            if (headingMatch.hasMatch())
            {
                const int headingLevel = headingMatch.captured(2).size();
                htmlLines.push_back(
                    whitespaceToHtml(headingMatch.captured(1))
                    + WhatSon::WhatSonNoteMarkdownStyleObject::wrapHtmlSpan(
                        WhatSon::WhatSonNoteMarkdownStyleObject::Role::HeadingMarker,
                        escapeHtmlText(headingMatch.captured(2) + headingMatch.captured(3)))
                    + WhatSon::WhatSonNoteMarkdownStyleObject::wrapHtmlSpan(
                        WhatSon::WhatSonNoteMarkdownStyleObject::Role::HeadingBody,
                        renderInlineTaggedTextFragmentToHtml(headingMatch.captured(4)),
                        headingLevel));
                continue;
            }

            const QRegularExpressionMatch blockquoteMatch = blockquotePattern.match(line);
            if (blockquoteMatch.hasMatch())
            {
                htmlLines.push_back(
                    whitespaceToHtml(blockquoteMatch.captured(1))
                    + WhatSon::WhatSonNoteMarkdownStyleObject::wrapHtmlSpan(
                        WhatSon::WhatSonNoteMarkdownStyleObject::Role::BlockquoteMarker,
                        escapeHtmlText(blockquoteMatch.captured(2)))
                    + whitespaceToHtml(blockquoteMatch.captured(3))
                    + WhatSon::WhatSonNoteMarkdownStyleObject::wrapHtmlSpan(
                        WhatSon::WhatSonNoteMarkdownStyleObject::Role::BlockquoteBody,
                        renderInlineTaggedTextFragmentToHtml(blockquoteMatch.captured(4))));
                continue;
            }

            if (horizontalRulePattern.match(line).hasMatch())
            {
                htmlLines.push_back(
                    WhatSon::WhatSonNoteMarkdownStyleObject::wrapHtmlSpan(
                        WhatSon::WhatSonNoteMarkdownStyleObject::Role::HorizontalRule,
                        escapeHtmlText(line)));
                continue;
            }

            htmlLines.push_back(renderInlineTaggedTextFragmentToHtml(line));
        }

        return htmlLines.join(QStringLiteral("<br/>"));
    }

    QString renderInlineStyleEditingSurfaceHtml(const QString& sourceText)
    {
        QString html = renderInlineTaggedTextFragmentToHtml(sourceText, LiteralRenderMode::SourceEditing);
        html.replace(QLatin1Char('\n'), QStringLiteral("<br/>"));
        return html;
    }

} // namespace

ContentsTextFormatRenderer::ContentsTextFormatRenderer(QObject* parent)
    : QObject(parent)
{
}

ContentsTextFormatRenderer::~ContentsTextFormatRenderer() = default;

QString ContentsTextFormatRenderer::sourceText() const
{
    return m_sourceText;
}

void ContentsTextFormatRenderer::setSourceText(const QString& sourceText)
{
    if (m_sourceText == sourceText)
    {
        return;
    }

    m_sourceText = sourceText;
    emit sourceTextChanged();
    refreshRenderedOutputs();
}

QString ContentsTextFormatRenderer::editorSurfaceHtml() const
{
    return m_editorSurfaceHtml;
}

QString ContentsTextFormatRenderer::renderedHtml() const
{
    return m_renderedHtml;
}

bool ContentsTextFormatRenderer::previewEnabled() const noexcept
{
    return m_previewEnabled;
}

void ContentsTextFormatRenderer::setPreviewEnabled(const bool enabled)
{
    if (m_previewEnabled == enabled)
    {
        return;
    }

    m_previewEnabled = enabled;
    emit previewEnabledChanged();
    refreshRenderedOutputs();
}

QString ContentsTextFormatRenderer::renderRichText(const QString& sourceText) const
{
    return renderMarkdownAwareTextToHtml(sourceText);
}

QString ContentsTextFormatRenderer::normalizeInlineStyleAliasesForEditor(const QString& sourceText) const
{
    return renderInlineStyleEditingSurfaceHtml(sourceText);
}

QString ContentsTextFormatRenderer::normalizeEditorSurfaceTextToSource(const QString& surfaceText) const
{
    if (surfaceText.isEmpty())
    {
        return {};
    }

    const QString serializedBodyDocument =
        WhatSon::NoteBodyPersistence::serializeBodyDocument(QStringLiteral("note"), surfaceText);
    return normalizeRenderedMarkdownGlyphsToSource(
        WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(serializedBodyDocument));
}

QString ContentsTextFormatRenderer::applyPlainTextReplacementToSource(
    const QString& sourceText,
    int sourceStart,
    int sourceEnd,
    const QString& replacementText) const
{
    const QString normalizedSourceText = normalizeLineEndings(sourceText);
    const QString normalizedReplacementText = normalizeLineEndings(replacementText);
    const int maximumSourceLength = boundedQStringSize(normalizedSourceText);
    const int boundedStart = std::clamp(std::min(sourceStart, sourceEnd), 0, maximumSourceLength);
    const int boundedEnd = std::clamp(std::max(sourceStart, sourceEnd), 0, maximumSourceLength);
    return normalizedSourceText.left(boundedStart)
        + escapeHtmlText(normalizedReplacementText)
        + normalizedSourceText.mid(boundedEnd);
}

QString ContentsTextFormatRenderer::applyInlineStyleToSelectionSource(
    const QString& surfaceText,
    int selectionStart,
    int selectionEnd,
    const QString& styleTag) const
{
    if (surfaceText.isEmpty())
    {
        return {};
    }

    const QString normalizedStyleTag = normalizeSupportedInlineStyleTag(styleTag);
    if (normalizedStyleTag.isEmpty())
    {
        return normalizeEditorSurfaceTextToSource(surfaceText);
    }

    QTextDocument document;
    document.setHtml(surfaceText);

    const int boundedStart = boundedDocumentSelectionPosition(document, std::min(selectionStart, selectionEnd));
    const int boundedEnd = boundedDocumentSelectionPosition(document, std::max(selectionStart, selectionEnd));
    if (boundedEnd <= boundedStart)
    {
        return normalizeEditorSurfaceTextToSource(surfaceText);
    }

    QTextCursor cursor(&document);
    cursor.setPosition(boundedStart);
    cursor.setPosition(boundedEnd, QTextCursor::KeepAnchor);
    if (!cursor.hasSelection())
    {
        return normalizeEditorSurfaceTextToSource(surfaceText);
    }

    cursor.beginEditBlock();
    if (normalizedStyleTag == QStringLiteral("plain"))
    {
        const QString replacementText = plainSelectedText(cursor);
        cursor.removeSelectedText();
        cursor.insertText(replacementText, plainTextCharFormat());
    }
    else if (selectionFullyHasInlineStyle(cursor, normalizedStyleTag))
    {
        const QString replacementText = plainSelectedText(cursor);
        cursor.removeSelectedText();
        cursor.insertText(replacementText, plainTextCharFormat());
    }
    else
    {
        cursor.mergeCharFormat(textCharFormatForInlineStyle(normalizedStyleTag));
    }
    cursor.endEditBlock();

    return normalizeEditorSurfaceTextToSource(document.toHtml());
}

QString ContentsTextFormatRenderer::applyInlineStyleToLogicalSelectionSource(
    const QString& sourceText,
    int selectionStart,
    int selectionEnd,
    const QString& styleTag) const
{
    const QString normalizedSourceText = normalizeLineEndings(sourceText);
    if (normalizedSourceText.isEmpty())
    {
        return {};
    }

    const QString normalizedStyleTag = normalizeSupportedInlineStyleTag(styleTag);
    if (normalizedStyleTag.isEmpty())
    {
        return normalizedSourceText;
    }

    const SourceInlineStyleState styleState = buildSourceInlineStyleState(normalizedSourceText);
    const int boundedStart = std::clamp(
        std::min(selectionStart, selectionEnd),
        0,
        styleState.logicalLength);
    const int boundedEnd = std::clamp(
        std::max(selectionStart, selectionEnd),
        0,
        styleState.logicalLength);
    if (boundedEnd <= boundedStart)
    {
        return normalizedSourceText;
    }

    InlineStyleCoverageMap desiredCoverage = styleState.coverage;
    if (normalizedStyleTag == QStringLiteral("plain"))
    {
        for (int index = 0; index < kSupportedInlineStyleCount; ++index)
        {
            applySourceInlineStyleCoverageRange(
                &desiredCoverage[static_cast<std::size_t>(index)],
                boundedStart,
                boundedEnd,
                false);
        }
    }
    else
    {
        const int targetStyleIndex = supportedInlineStyleIndexForTag(normalizedStyleTag);
        if (targetStyleIndex < 0)
        {
            return normalizedSourceText;
        }
        const bool fullyStyled = selectionFullyHasSourceInlineStyle(
            styleState.coverage[static_cast<std::size_t>(targetStyleIndex)],
            boundedStart,
            boundedEnd);
        applySourceInlineStyleCoverageRange(
            &desiredCoverage[static_cast<std::size_t>(targetStyleIndex)],
            boundedStart,
            boundedEnd,
            !fullyStyled);
    }

    clearInlineStyleCoverageOnLinePrefixes(
        &desiredCoverage,
        normalizedSourceText,
        styleState.logicalLength);

    return rebuildSourceTextWithInlineStyleCoverage(
        normalizedSourceText,
        desiredCoverage,
        styleState.logicalLength);
}

void ContentsTextFormatRenderer::requestRenderRefresh()
{
    refreshRenderedOutputs();
}

void ContentsTextFormatRenderer::refreshRenderedOutputs()
{
    const QString nextEditorSurfaceHtml = renderInlineStyleEditingSurfaceHtml(m_sourceText);
    if (m_editorSurfaceHtml != nextEditorSurfaceHtml)
    {
        m_editorSurfaceHtml = nextEditorSurfaceHtml;
        emit editorSurfaceHtmlChanged();
    }

    const QString nextRenderedHtml = m_previewEnabled ? renderMarkdownAwareTextToHtml(m_sourceText) : QString();
    if (m_renderedHtml == nextRenderedHtml)
    {
        return;
    }

    m_renderedHtml = nextRenderedHtml;
    emit renderedHtmlChanged();
}
