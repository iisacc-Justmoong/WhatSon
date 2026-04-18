#include "ContentsTextFormatRenderer.hpp"
#include "ContentsHtmlBlockRenderPipeline.hpp"
#include "ContentsTextHighlightRenderer.hpp"
#include "file/WhatSonDebugTrace.hpp"
#include "file/note/WhatSonNoteBodySemanticTagSupport.hpp"
#include "file/note/WhatSonNoteBodyPersistence.hpp"
#include "file/note/WhatSonNoteMarkdownStyleObject.hpp"

#include <array>
#include <algorithm>
#include <limits>
#include <utility>
#include <QRegularExpression>
#include <QStringList>
#include <QVector>

namespace
{
    namespace SemanticTags = WhatSon::NoteBodySemanticTagSupport;

    constexpr int kResourceEditorPlaceholderLineCount = 1;

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
    QString applyPaperPaletteToHtml(QString html);
    QVariantList applyPaperPaletteToHtmlField(const QVariantList& entries, const QString& fieldName);

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

    QString visibleWhitespaceHtml(const QString& text)
    {
        QString html;
        html.reserve(text.size() * 6);

        bool lineStart = true;
        qsizetype cursor = 0;
        while (cursor < text.size())
        {
            const QChar ch = text.at(cursor);
            if (ch == QLatin1Char('\n'))
            {
                html += QLatin1Char('\n');
                lineStart = true;
                ++cursor;
                continue;
            }

            if (ch == QLatin1Char('\t'))
            {
                html += QStringLiteral("&nbsp;&nbsp;&nbsp;&nbsp;");
                lineStart = false;
                ++cursor;
                continue;
            }

            if (ch != QLatin1Char(' '))
            {
                html += escapeHtmlText(QString(ch));
                lineStart = false;
                ++cursor;
                continue;
            }

            const qsizetype runStart = cursor;
            while (cursor < text.size() && text.at(cursor) == QLatin1Char(' '))
            {
                ++cursor;
            }

            const qsizetype runLength = cursor - runStart;
            for (qsizetype runIndex = 0; runIndex < runLength; ++runIndex)
            {
                const bool preserveAsNbsp = lineStart || ((runLength - runIndex) % 2 == 0);
                html += preserveAsNbsp ? QStringLiteral("&nbsp;") : QStringLiteral(" ");
                lineStart = false;
            }
        }

        return html;
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

    QString editorDocumentParagraphHtml(const QString& innerHtml)
    {
        const QString paragraphBody = innerHtml.isEmpty()
                                          ? QStringLiteral("<br/>")
                                          : innerHtml;
        return QStringLiteral("<p style=\"margin-top:0px;margin-bottom:0px;\">%1</p>")
            .arg(paragraphBody);
    }

    QString editorBlankParagraphHtml()
    {
        return editorDocumentParagraphHtml(QStringLiteral("&nbsp;"));
    }

    void trimOneLeadingStructuralLineBreak(QString* text)
    {
        if (text == nullptr
            || text->isEmpty()
            || !text->startsWith(QLatin1Char('\n')))
        {
            return;
        }

        text->remove(0, 1);
    }

    void trimOneTrailingStructuralLineBreak(QString* text)
    {
        if (text == nullptr
            || text->isEmpty()
            || !text->endsWith(QLatin1Char('\n')))
        {
            return;
        }

        text->chop(1);
    }

    bool htmlFragmentOwnsBlockFlow(const QString& htmlFragment)
    {
        const QString trimmedFragment = htmlFragment.trimmed();
        return trimmedFragment.startsWith(QStringLiteral("<!--whatson-resource-block:"))
            || trimmedFragment.startsWith(QStringLiteral("<div"))
            || trimmedFragment.startsWith(QStringLiteral("<hr"))
            || trimmedFragment.startsWith(QStringLiteral("<p"))
            || trimmedFragment.startsWith(QStringLiteral("<table"))
            || trimmedFragment.startsWith(QStringLiteral("<ul"))
            || trimmedFragment.startsWith(QStringLiteral("<ol"))
            || trimmedFragment.startsWith(QStringLiteral("<blockquote"))
            || trimmedFragment.startsWith(QStringLiteral("<pre"));
    }

    QString joinHtmlDocumentFragments(const QStringList& fragments)
    {
        if (fragments.isEmpty())
        {
            return {};
        }

        QString html;
        for (const QString& fragment : fragments)
        {
            if (htmlFragmentOwnsBlockFlow(fragment))
            {
                html += fragment;
                continue;
            }

            html += fragment.isEmpty()
                        ? editorBlankParagraphHtml()
                        : editorDocumentParagraphHtml(fragment);
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
        html.reserve(displayText.size() * 6);

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

            const int literalRunStart = cursor;
            while (cursor < displayText.size())
            {
                if (renderMode == LiteralRenderMode::MarkdownAware)
                {
                    if (inlineCodeTokenLengthAt(displayText, cursor) > 0
                        || markdownLinkTokenLengthAt(displayText, cursor) > 0)
                    {
                        break;
                    }
                }
                ++cursor;
            }
            html += visibleWhitespaceHtml(displayText.mid(literalRunStart, cursor - literalRunStart));
        }

        return html;
    }

    QString renderInlineStyleEditingSurfaceHtml(const QString& sourceText);

    QString canonicalInlineStyleTagName(const QString& elementName)
    {
        return SemanticTags::canonicalInlineStyleTagName(elementName);
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
        return SemanticTags::isRenderedLineBreakTagName(normalizedTagName)
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

    QString renderResourceEditorBlockToHtml(const int resourceIndex)
    {
        QStringList placeholderParagraphs;
        placeholderParagraphs.reserve(kResourceEditorPlaceholderLineCount);
        for (int index = 0; index < kResourceEditorPlaceholderLineCount; ++index)
        {
            placeholderParagraphs.push_back(editorBlankParagraphHtml());
        }
        const QString placeholderHtml = placeholderParagraphs.join(QString());
        return QStringLiteral("<!--whatson-resource-block:%1-->%2<!--/whatson-resource-block:%1-->")
            .arg(QString::number(std::max(0, resourceIndex)), placeholderHtml);
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
        int resourceIndex = 0;
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
                    html += renderResourceEditorBlockToHtml(resourceIndex);
                    ++resourceIndex;
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

            if (SemanticTags::isTransparentContainerTagName(rawTagName)
                || SemanticTags::isRenderedTextBlockElement(rawTagName))
            {
                if (closingTag)
                {
                    html += SemanticTags::semanticTextClosingHtml(rawTagName);
                }
                else if (!selfClosingTag)
                {
                    html += SemanticTags::semanticTextOpeningHtml(rawTagName);
                }
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

            if (SemanticTags::isRenderedLineBreakTagName(rawTagName))
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

    QString applyPaperPaletteToHtml(QString html)
    {
        if (html.isEmpty())
        {
            return html;
        }

        static const std::array<std::pair<QString, QString>, 10> replacements{{
            {
                QStringLiteral("background-color:#8A4B00;color:#D6AE58;font-weight:600;"),
                QStringLiteral("background-color:#F4D37A;color:#111111;font-weight:600;")
            },
            {
                QStringLiteral("background-color:#1A1D22;color:#D7DADF;"),
                QStringLiteral("background-color:#E7EAEE;color:#111111;")
            },
            {
                QStringLiteral("background-color:#1A1D22;color:#8F96A3;"),
                QStringLiteral("background-color:#E7EAEE;color:#4E5763;")
            },
            {QStringLiteral("color:#F3F5F8;"), QStringLiteral("color:#111111;")},
            {QStringLiteral("color:#C9CDD4;"), QStringLiteral("color:#2F343B;")},
            {QStringLiteral("color:#8F96A3;"), QStringLiteral("color:#4E5763;")},
            {QStringLiteral("color:#8CB4FF;"), QStringLiteral("color:#1F5FBF;")},
            {QStringLiteral("color:#66727D;"), QStringLiteral("color:#4E5763;")},
            {QStringLiteral("color:#D6AE58;"), QStringLiteral("color:#111111;")},
            {QStringLiteral("color:#D7DADF;"), QStringLiteral("color:#111111;")},
        }};

        for (const auto& [from, to] : replacements)
        {
            html.replace(from, to);
        }

        return html;
    }

    QVariantList applyPaperPaletteToHtmlField(const QVariantList& entries, const QString& fieldName)
    {
        QVariantList recoloredEntries;
        recoloredEntries.reserve(entries.size());

        for (const QVariant& entryValue : entries)
        {
            QVariantMap entry = entryValue.toMap();
            const QString currentHtml = entry.value(fieldName).toString();
            if (!currentHtml.isEmpty())
            {
                entry.insert(fieldName, applyPaperPaletteToHtml(currentHtml));
            }
            recoloredEntries.push_back(entry);
        }

        return recoloredEntries;
    }

    QString renderInlineStyleEditingSurfaceHtml(const QString& sourceText)
    {
        const QString normalizedText = normalizeLineEndings(sourceText);
        if (normalizedText.isEmpty())
        {
            return {};
        }

        static const QRegularExpression tagPattern(
            QStringLiteral(R"(<\s*/?\s*([A-Za-z_][A-Za-z0-9_.:-]*)\b[^>]*>)"));

        auto flushBufferedSourceText =
            [](QString* bufferedSourceText, QStringList* documentFragments) {
                if (bufferedSourceText == nullptr
                    || documentFragments == nullptr
                    || bufferedSourceText->isNull())
                {
                    return;
                }

                const QString bufferedBodyDocument =
                    WhatSon::NoteBodyPersistence::serializeBodyDocument(
                        QStringLiteral("note"),
                        *bufferedSourceText);
                const QString bufferedHtmlProjection =
                    WhatSon::NoteBodyPersistence::htmlProjectionFromBodyDocument(bufferedBodyDocument);
                const QStringList bufferedLines =
                    bufferedHtmlProjection.split(QStringLiteral("<br/>"), Qt::KeepEmptyParts);
                for (const QString& bufferedLine : bufferedLines)
                {
                    documentFragments->push_back(bufferedLine);
                }
                bufferedSourceText->clear();
            };

        QStringList documentFragments;
        QString bufferedSourceText;
        int resourceIndex = 0;
        bool previousFragmentOwnsBlockFlow = false;
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
                QString sourceSegment = normalizedText.mid(cursor, tagStart - cursor);
                if (previousFragmentOwnsBlockFlow)
                {
                    trimOneLeadingStructuralLineBreak(&sourceSegment);
                    previousFragmentOwnsBlockFlow = false;
                }
                bufferedSourceText += sourceSegment;
            }

            const QString fullTagToken = match.captured(0);
            const QString rawTagName = match.captured(1);
            const QString normalizedTagName = rawTagName.trimmed().toCaseFolded();
            const bool closingTag = isClosingTagToken(fullTagToken);

            if (normalizedTagName == QStringLiteral("resource") && !closingTag)
            {
                trimOneTrailingStructuralLineBreak(&bufferedSourceText);
                flushBufferedSourceText(&bufferedSourceText, &documentFragments);
                documentFragments.push_back(renderResourceEditorBlockToHtml(resourceIndex));
                ++resourceIndex;
                previousFragmentOwnsBlockFlow = true;
                cursor = tagEnd;
                continue;
            }

            if (!closingTag
                && normalizedTagName == QStringLiteral("agenda"))
            {
                trimOneTrailingStructuralLineBreak(&bufferedSourceText);
                flushBufferedSourceText(&bufferedSourceText, &documentFragments);
                documentFragments.push_back(
                    renderAgendaEditorBlockToHtml(
                        normalizedText,
                        tagEnd,
                        LiteralRenderMode::SourceEditing));
                previousFragmentOwnsBlockFlow = true;
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
                trimOneTrailingStructuralLineBreak(&bufferedSourceText);
                flushBufferedSourceText(&bufferedSourceText, &documentFragments);
                documentFragments.push_back(
                    renderCalloutEditorBlockToHtml(
                        normalizedText,
                        tagEnd,
                        LiteralRenderMode::SourceEditing));
                previousFragmentOwnsBlockFlow = true;
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

            if (normalizedTagName == QStringLiteral("break")
                || normalizedTagName == QStringLiteral("hr"))
            {
                trimOneTrailingStructuralLineBreak(&bufferedSourceText);
                flushBufferedSourceText(&bufferedSourceText, &documentFragments);
                documentFragments.push_back(breakDividerHtml());
                previousFragmentOwnsBlockFlow = true;
                cursor = tagEnd;
                continue;
            }

            if (normalizedTagName == QStringLiteral("br"))
            {
                bufferedSourceText += QLatin1Char('\n');
                previousFragmentOwnsBlockFlow = false;
                cursor = tagEnd;
                continue;
            }

            bufferedSourceText += fullTagToken;
            previousFragmentOwnsBlockFlow = false;
            cursor = tagEnd;
        }

        if (cursor < normalizedText.size())
        {
            QString trailingSourceSegment = normalizedText.mid(cursor);
            if (previousFragmentOwnsBlockFlow)
            {
                trimOneLeadingStructuralLineBreak(&trailingSourceSegment);
            }
            bufferedSourceText += trailingSourceSegment;
        }

        flushBufferedSourceText(&bufferedSourceText, &documentFragments);
        return joinHtmlDocumentFragments(documentFragments);
    }

} // namespace

ContentsTextFormatRenderer::ContentsTextFormatRenderer(QObject* parent)
    : QObject(parent)
{
    WhatSon::Debug::traceEditorSelf(this, QStringLiteral("textFormatRenderer"), QStringLiteral("ctor"));
}

ContentsTextFormatRenderer::~ContentsTextFormatRenderer()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("textFormatRenderer"),
        QStringLiteral("dtor"),
        QStringLiteral("preview=%1 sourceSummary=%2")
            .arg(m_previewEnabled)
            .arg(WhatSon::Debug::summarizeText(m_sourceText)));
}

QString ContentsTextFormatRenderer::sourceText() const
{
    return m_sourceText;
}

void ContentsTextFormatRenderer::setSourceText(const QString& sourceText)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("textFormatRenderer"),
        QStringLiteral("setSourceText"),
        QStringLiteral("changed=%1 %2").arg(m_sourceText != sourceText).arg(WhatSon::Debug::summarizeText(sourceText)));
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

QVariantList ContentsTextFormatRenderer::htmlTokens() const
{
    return m_htmlTokens;
}

QVariantList ContentsTextFormatRenderer::normalizedHtmlBlocks() const
{
    return m_normalizedHtmlBlocks;
}

bool ContentsTextFormatRenderer::htmlOverlayVisible() const noexcept
{
    return m_htmlOverlayVisible;
}

bool ContentsTextFormatRenderer::previewEnabled() const noexcept
{
    return m_previewEnabled;
}

void ContentsTextFormatRenderer::setPreviewEnabled(const bool enabled)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("textFormatRenderer"),
        QStringLiteral("setPreviewEnabled"),
        QStringLiteral("previous=%1 next=%2").arg(m_previewEnabled).arg(enabled));
    if (m_previewEnabled == enabled)
    {
        return;
    }

    m_previewEnabled = enabled;
    emit previewEnabledChanged();
    refreshRenderedOutputs();
}

bool ContentsTextFormatRenderer::paperPaletteEnabled() const noexcept
{
    return m_paperPaletteEnabled;
}

void ContentsTextFormatRenderer::setPaperPaletteEnabled(const bool enabled)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("textFormatRenderer"),
        QStringLiteral("setPaperPaletteEnabled"),
        QStringLiteral("previous=%1 next=%2").arg(m_paperPaletteEnabled).arg(enabled));
    if (m_paperPaletteEnabled == enabled)
    {
        return;
    }

    m_paperPaletteEnabled = enabled;
    emit paperPaletteEnabledChanged();
    refreshRenderedOutputs();
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
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("textFormatRenderer"),
        QStringLiteral("requestRenderRefresh"),
        QStringLiteral("preview=%1").arg(m_previewEnabled));
    refreshRenderedOutputs();
}

void ContentsTextFormatRenderer::refreshRenderedOutputs()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("textFormatRenderer"),
        QStringLiteral("refreshRenderedOutputs"),
        QStringLiteral("preview=%1 %2").arg(m_previewEnabled).arg(WhatSon::Debug::summarizeText(m_sourceText)));
    const ContentsHtmlBlockRenderPipeline renderPipeline;
    const ContentsHtmlBlockRenderPipeline::RenderResult editorRenderResult =
        renderPipeline.renderEditorDocument(m_sourceText);

    const QVariantList nextHtmlTokens = m_paperPaletteEnabled
        ? applyPaperPaletteToHtmlField(editorRenderResult.htmlTokens, QStringLiteral("html"))
        : editorRenderResult.htmlTokens;
    if (m_htmlTokens != nextHtmlTokens)
    {
        m_htmlTokens = nextHtmlTokens;
        emit htmlTokensChanged();
    }

    const QVariantList nextNormalizedHtmlBlocks = m_paperPaletteEnabled
        ? applyPaperPaletteToHtmlField(editorRenderResult.normalizedHtmlBlocks, QStringLiteral("htmlBlockHtml"))
        : editorRenderResult.normalizedHtmlBlocks;
    if (m_normalizedHtmlBlocks != nextNormalizedHtmlBlocks)
    {
        m_normalizedHtmlBlocks = nextNormalizedHtmlBlocks;
        emit normalizedHtmlBlocksChanged();
    }

    if (m_htmlOverlayVisible != editorRenderResult.htmlOverlayVisible)
    {
        m_htmlOverlayVisible = editorRenderResult.htmlOverlayVisible;
        emit htmlOverlayVisibleChanged();
    }

    const QString baseEditorSurfaceHtml = editorRenderResult.requiresLegacyDocumentComposition
        ? renderInlineStyleEditingSurfaceHtml(m_sourceText)
        : editorRenderResult.documentHtml;
    const QString nextEditorSurfaceHtml = m_paperPaletteEnabled
        ? applyPaperPaletteToHtml(baseEditorSurfaceHtml)
        : baseEditorSurfaceHtml;
    if (m_editorSurfaceHtml != nextEditorSurfaceHtml)
    {
        m_editorSurfaceHtml = nextEditorSurfaceHtml;
        emit editorSurfaceHtmlChanged();
    }

    const QString baseRenderedHtml = m_previewEnabled ? renderMarkdownAwareTextToHtml(m_sourceText) : QString();
    const QString nextRenderedHtml = m_paperPaletteEnabled
        ? applyPaperPaletteToHtml(baseRenderedHtml)
        : baseRenderedHtml;
    if (m_renderedHtml == nextRenderedHtml)
    {
        return;
    }

    m_renderedHtml = nextRenderedHtml;
    emit renderedHtmlChanged();
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("textFormatRenderer"),
        QStringLiteral("renderedHtmlChanged"),
        QStringLiteral("editorSurfaceLen=%1 renderedLen=%2 htmlTokens=%3 htmlBlocks=%4")
            .arg(m_editorSurfaceHtml.size())
            .arg(m_renderedHtml.size())
            .arg(m_htmlTokens.size())
            .arg(m_normalizedHtmlBlocks.size()));
}
