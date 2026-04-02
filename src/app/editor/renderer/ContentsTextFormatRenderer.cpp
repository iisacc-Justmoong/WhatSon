#include "ContentsTextFormatRenderer.hpp"
#include "ContentsTextHighlightRenderer.hpp"
#include "file/note/WhatSonNoteBodyPersistence.hpp"

#include <algorithm>
#include <QColor>
#include <QFont>
#include <QRegularExpression>
#include <QStringList>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QVector>

namespace
{
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
            format.setForeground(QColor(QStringLiteral("#FFD9A3")));
            format.setFontWeight(QFont::DemiBold);
        }
        return format;
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

    QString renderInlineTaggedTextToHtml(const QString& sourceText)
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
                html += escapeHtmlText(normalizedText.mid(cursor, tagStart - cursor));
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
            html += escapeHtmlText(normalizedText.mid(cursor));
        }

        for (int index = openStyleTags.size() - 1; index >= 0; --index)
        {
            html += closingEditorTagForStyle(openStyleTags.at(index));
        }

        html.replace(QLatin1Char('\n'), QStringLiteral("<br/>"));
        return html;
    }

    QString normalizeInlineStyleAliasesForEditorSurface(const QString& sourceText)
    {
        const QString normalizedText = normalizeLineEndings(sourceText);
        if (normalizedText.isEmpty())
        {
            return {};
        }

        static const QRegularExpression tagPattern(
            QStringLiteral(R"(<\s*/?\s*([A-Za-z_][A-Za-z0-9_.:-]*)\b[^>]*>)"));

        QString output;
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
                output += normalizedText.mid(cursor, tagStart - cursor);
            }

            const QString fullTagToken = match.captured(0);
            const QString rawTagName = match.captured(1);
            const QString normalizedTagName = rawTagName.trimmed().toCaseFolded();
            const bool closingTag = isClosingTagToken(fullTagToken);
            const bool selfClosingTag = isSelfClosingTagToken(fullTagToken);

            if (normalizedTagName == QStringLiteral("br"))
            {
                output += QStringLiteral("<br/>");
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
                            closeMatchingTag(&openStyleTags, &output, spanTags.at(index));
                        }
                    }
                    else
                    {
                        output += fullTagToken;
                    }
                }
                else if (selfClosingTag)
                {
                    output += fullTagToken;
                }
                else
                {
                    const QStringList spanTags = spanInlineStyleTagsFromCssDeclaration(
                        cssStyleAttributeFromTagToken(fullTagToken));
                    if (spanTags.isEmpty())
                    {
                        output += fullTagToken;
                    }
                    else
                    {
                        for (const QString& spanTag : spanTags)
                        {
                            output += openingEditorTagForStyle(spanTag);
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
                    closeMatchingTag(&openStyleTags, &output, styleTag);
                }
                else if (!selfClosingTag)
                {
                    output += openingEditorTagForStyle(styleTag);
                    openStyleTags.append(styleTag);
                }
                cursor = tagEnd;
                continue;
            }

            output += fullTagToken;
            cursor = tagEnd;
        }

        if (cursor < normalizedText.size())
        {
            output += normalizedText.mid(cursor);
        }

        for (int index = openStyleTags.size() - 1; index >= 0; --index)
        {
            output += closingEditorTagForStyle(openStyleTags.at(index));
        }

        return output;
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
    return renderInlineTaggedTextToHtml(sourceText);
}

QString ContentsTextFormatRenderer::normalizeInlineStyleAliasesForEditor(const QString& sourceText) const
{
    return normalizeInlineStyleAliasesForEditorSurface(sourceText);
}

QString ContentsTextFormatRenderer::normalizeEditorSurfaceTextToSource(const QString& surfaceText) const
{
    if (surfaceText.isEmpty())
    {
        return {};
    }

    const QString serializedBodyDocument =
        WhatSon::NoteBodyPersistence::serializeBodyDocument(QStringLiteral("note"), surfaceText);
    return WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(serializedBodyDocument);
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
    cursor.mergeCharFormat(textCharFormatForInlineStyle(normalizedStyleTag));
    cursor.endEditBlock();

    return normalizeEditorSurfaceTextToSource(document.toHtml());
}

void ContentsTextFormatRenderer::requestRenderRefresh()
{
    refreshRenderedHtml();
}

void ContentsTextFormatRenderer::refreshRenderedHtml()
{
    const QString nextRenderedHtml = renderInlineTaggedTextToHtml(m_sourceText);
    if (m_renderedHtml == nextRenderedHtml)
    {
        return;
    }

    m_renderedHtml = nextRenderedHtml;
    emit renderedHtmlChanged();
}
