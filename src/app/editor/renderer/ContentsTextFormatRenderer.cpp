#include "ContentsTextFormatRenderer.hpp"
#include "ContentsTextHighlightRenderer.hpp"
#include "file/note/WhatSonNoteBodyPersistence.hpp"
#include "file/note/WhatSonNoteMarkdownStyleObject.hpp"

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
    enum class LiteralRenderMode
    {
        MarkdownAware,
        SourceEditing,
    };

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

    QString normalizeInlineStyleAliasesForEditorSurface(const QString& sourceText)
    {
        return renderMarkdownAwareTextToHtml(sourceText);
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
    refreshRenderedHtml();
}

QString ContentsTextFormatRenderer::renderedHtml() const
{
    return m_renderedHtml;
}

QString ContentsTextFormatRenderer::renderRichText(const QString& sourceText) const
{
    return renderMarkdownAwareTextToHtml(sourceText);
}

QString ContentsTextFormatRenderer::normalizeInlineStyleAliasesForEditor(const QString& sourceText) const
{
    return renderMarkdownAwareTextToHtml(sourceText);
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

    QTextDocument document;
    document.setHtml(renderInlineStyleEditingSurfaceHtml(normalizedSourceText));

    const int boundedStart = boundedDocumentSelectionPosition(document, std::min(selectionStart, selectionEnd));
    const int boundedEnd = boundedDocumentSelectionPosition(document, std::max(selectionStart, selectionEnd));
    if (boundedEnd <= boundedStart)
    {
        return normalizedSourceText;
    }

    QTextCursor cursor(&document);
    cursor.setPosition(boundedStart);
    cursor.setPosition(boundedEnd, QTextCursor::KeepAnchor);
    if (!cursor.hasSelection())
    {
        return normalizedSourceText;
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

void ContentsTextFormatRenderer::requestRenderRefresh()
{
    refreshRenderedHtml();
}

void ContentsTextFormatRenderer::refreshRenderedHtml()
{
    const QString nextRenderedHtml = renderMarkdownAwareTextToHtml(m_sourceText);
    if (m_renderedHtml == nextRenderedHtml)
    {
        return;
    }

    m_renderedHtml = nextRenderedHtml;
    emit renderedHtmlChanged();
}
