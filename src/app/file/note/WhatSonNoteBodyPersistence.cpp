#include "WhatSonNoteBodyPersistence.hpp"

#include "WhatSonNoteMarkdownStyleObject.hpp"
#include "WhatSonLocalNoteFileStore.hpp"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QVector>
#include <QXmlStreamReader>

namespace
{
    struct BodyDocumentTextFragments final
    {
        QString fallbackText;
        QString fallbackRichText;
        QStringList blockLines;
        QStringList blockRichLines;
    };

    QString escapeXmlAttributeValue(QString value)
    {
        value.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
        value.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
        value.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
        value.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
        value.replace(QStringLiteral("'"), QStringLiteral("&apos;"));
        return value;
    }

    QString decodeXmlEntities(QString text)
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

    QString escapeHtmlText(QString value)
    {
        value.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
        value.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
        value.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
        value.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
        value.replace(QStringLiteral("'"), QStringLiteral("&#39;"));
        return value;
    }

    struct InlineStyleHtmlTag final
    {
        QString openingTag;
        QString closingTag;

        bool isEmpty() const
        {
            return openingTag.isEmpty() || closingTag.isEmpty();
        }
    };

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
        if (normalizedName == QStringLiteral("highlight")
            || normalizedName == QStringLiteral("mark"))
        {
            return QStringLiteral("highlight");
        }
        return {};
    }

    InlineStyleHtmlTag inlineStyleHtmlTagForElement(const QString& elementName)
    {
        const QString normalizedName = canonicalInlineStyleTagName(elementName);
        if (normalizedName == QStringLiteral("bold"))
        {
            return {
                QStringLiteral("<strong style=\"font-weight:900;\">"),
                QStringLiteral("</strong>")
            };
        }
        if (normalizedName == QStringLiteral("italic"))
        {
            return {
                QStringLiteral("<span style=\"font-style:italic;\">"),
                QStringLiteral("</span>")
            };
        }
        if (normalizedName == QStringLiteral("underline"))
        {
            return {
                QStringLiteral("<span style=\"text-decoration: underline;\">"),
                QStringLiteral("</span>")
            };
        }
        if (normalizedName == QStringLiteral("strikethrough"))
        {
            return {
                QStringLiteral("<span style=\"text-decoration: line-through;\">"),
                QStringLiteral("</span>")
            };
        }
        if (normalizedName == QStringLiteral("highlight"))
        {
            return {
                QStringLiteral("<span style=\"background-color:#8A4B00;color:#D6AE58;font-weight:600;\">"),
                QStringLiteral("</span>")
            };
        }
        return {};
    }

    void appendInlineStyleTag(QString* target, const InlineStyleHtmlTag& styleTag, bool opening)
    {
        if (target == nullptr || styleTag.isEmpty())
        {
            return;
        }
        *target += opening ? styleTag.openingTag : styleTag.closingTag;
    }

    QString richTextFromRichLines(const QStringList& richLines)
    {
        if (richLines.isEmpty())
        {
            return {};
        }
        return richLines.join(QStringLiteral("<br/>"));
    }

    QString normalizeResourceStartTag(const QString& rawTagText)
    {
        static const QRegularExpression attributePattern(
            QStringLiteral(
                R"ATTR(\b([A-Za-z_][A-Za-z0-9_.:-]*)\s*=\s*(?:"([^"]*)"|'([^']*)'|([^\s>]+?)(?=\s|/?>)))ATTR"),
            QRegularExpression::CaseInsensitiveOption);

        QStringList attributes;
        QRegularExpressionMatchIterator iterator = attributePattern.globalMatch(rawTagText);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            const QString attributeName = match.captured(1).trimmed();
            if (attributeName.isEmpty())
            {
                continue;
            }

            QString attributeValue;
            if (match.capturedStart(2) >= 0)
            {
                attributeValue = match.captured(2);
            }
            else if (match.capturedStart(3) >= 0)
            {
                attributeValue = match.captured(3);
            }
            else if (match.capturedStart(4) >= 0)
            {
                attributeValue = match.captured(4);
            }
            attributeValue = decodeXmlEntities(attributeValue);
            attributes.push_back(
                QStringLiteral("%1=\"%2\"")
                    .arg(attributeName, escapeXmlAttributeValue(attributeValue)));
        }

        if (attributes.isEmpty())
        {
            return QStringLiteral("<resource />");
        }
        return QStringLiteral("<resource %1 />").arg(attributes.join(QLatin1Char(' ')));
    }

    QString normalizeResourceTagsForXmlParser(QString bodyDocumentText)
    {
        if (bodyDocumentText.trimmed().isEmpty())
        {
            return bodyDocumentText;
        }

        bodyDocumentText.remove(
            QRegularExpression(
                QStringLiteral(R"(</\s*resource\s*>)"),
                QRegularExpression::CaseInsensitiveOption));

        static const QRegularExpression resourceStartTagPattern(
            QStringLiteral(R"(<resource\b[^>]*>)"),
            QRegularExpression::CaseInsensitiveOption);

        int searchOffset = 0;
        while (searchOffset >= 0 && searchOffset < bodyDocumentText.size())
        {
            const QRegularExpressionMatch match = resourceStartTagPattern.match(bodyDocumentText, searchOffset);
            if (!match.hasMatch())
            {
                break;
            }

            const QString replacement = normalizeResourceStartTag(match.captured(0));
            bodyDocumentText.replace(match.capturedStart(0), match.capturedLength(0), replacement);
            searchOffset = match.capturedStart(0) + replacement.size();
        }

        return bodyDocumentText;
    }

    QString removeInterTagFormattingWhitespace(QString text)
    {
        text.replace(
            QRegularExpression(QStringLiteral(R"(>([ \t]*\n[ \t\n]*)<)")),
            QStringLiteral("><"));
        return text;
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

    void closeMatchingInlineStyleTag(
        QString* output,
        QStringList* openStyleTags,
        const QString& styleTag)
    {
        if (output == nullptr || openStyleTags == nullptr || styleTag.isEmpty())
        {
            return;
        }

        const int openIndex = openStyleTags->lastIndexOf(styleTag);
        if (openIndex < 0)
        {
            return;
        }

        for (int index = openStyleTags->size() - 1; index >= openIndex; --index)
        {
            *output += QStringLiteral("</%1>").arg(openStyleTags->at(index));
            openStyleTags->removeAt(index);
        }
    }

    QStringList spanInlineStyleTagsFromCssDeclaration(const QString& cssDeclaration)
    {
        const WhatSon::WhatSonNoteMarkdownStyleObject::PromotionMatch markdownPromotionMatch =
            WhatSon::WhatSonNoteMarkdownStyleObject::promotionMatchForCss(cssDeclaration);
        if (markdownPromotionMatch.matched)
        {
            return markdownPromotionMatch.promotedInlineTags;
        }

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
            const QString value = decodeXmlEntities(styleMatch.captured(captureIndex));
            if (!value.isEmpty())
            {
                return value;
            }
        }
        return {};
    }

    bool isTextBlockElement(const QString& elementName);

    QString normalizeEditorSourceToInlineTaggedText(const QString& sourceText)
    {
        QString normalizedSource = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(sourceText);
        if (normalizedSource.isEmpty())
        {
            return {};
        }

        normalizedSource.remove(QRegularExpression(QStringLiteral(R"(<\?xml[\s\S]*?\?>)")));
        normalizedSource.remove(
            QRegularExpression(
                QStringLiteral(R"(<!DOCTYPE[\s\S]*?>)"),
                QRegularExpression::CaseInsensitiveOption));
        normalizedSource = removeInterTagFormattingWhitespace(normalizedSource);
        const bool hasExplicitBodyTag = normalizedSource.contains(
            QRegularExpression(QStringLiteral(R"(<\s*body\b)"), QRegularExpression::CaseInsensitiveOption));

        static const QRegularExpression tagPattern(
            QStringLiteral(R"(<\s*/?\s*([A-Za-z_][A-Za-z0-9_.:-]*)\b[^>]*>)"));

        QString output;
        qsizetype cursor = 0;
        bool insideBody = !hasExplicitBodyTag;
        int textBlockDepth = 0;
        bool encounteredTopLevelTextBlock = false;
        QVector<QStringList> spanStyleStack;

        QRegularExpressionMatchIterator iterator = tagPattern.globalMatch(normalizedSource);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            const qsizetype tagStart = match.capturedStart(0);
            const qsizetype tagEnd = match.capturedEnd(0);
            if (tagStart < 0 || tagEnd <= tagStart)
            {
                continue;
            }

            if (insideBody && tagStart > cursor)
            {
                output += decodeXmlEntities(normalizedSource.mid(cursor, tagStart - cursor));
            }

            const QString fullTagToken = match.captured(0);
            const QString rawTagName = match.captured(1);
            const QString normalizedTagName = rawTagName.trimmed().toCaseFolded();
            const bool closingTag = isClosingTagToken(fullTagToken);

            if (normalizedTagName == QStringLiteral("body"))
            {
                if (!closingTag)
                {
                    insideBody = true;
                }
                else
                {
                    insideBody = false;
                }
                cursor = tagEnd;
                continue;
            }

            if (!insideBody)
            {
                cursor = tagEnd;
                continue;
            }

            if (isTextBlockElement(rawTagName))
            {
                if (!closingTag)
                {
                    if (textBlockDepth == 0)
                    {
                        if (encounteredTopLevelTextBlock)
                        {
                            output += QLatin1Char('\n');
                        }
                        encounteredTopLevelTextBlock = true;
                    }
                    ++textBlockDepth;
                }
                else if (textBlockDepth > 0)
                {
                    --textBlockDepth;
                }
                cursor = tagEnd;
                continue;
            }

            if (normalizedTagName == QStringLiteral("br"))
            {
                output += QLatin1Char('\n');
                cursor = tagEnd;
                continue;
            }

            if (normalizedTagName == QStringLiteral("resource"))
            {
                if (!closingTag)
                {
                    output += normalizeResourceStartTag(fullTagToken);
                }
                cursor = tagEnd;
                continue;
            }

            if (normalizedTagName == QStringLiteral("span"))
            {
                if (!closingTag)
                {
                    const QStringList spanTags = spanInlineStyleTagsFromCssDeclaration(
                        cssStyleAttributeFromTagToken(fullTagToken));
                    for (const QString& spanTag : spanTags)
                    {
                        output += QStringLiteral("<%1>").arg(spanTag);
                    }
                    spanStyleStack.push_back(spanTags);
                }
                else if (!spanStyleStack.isEmpty())
                {
                    const QStringList spanTags = spanStyleStack.takeLast();
                    for (int index = spanTags.size() - 1; index >= 0; --index)
                    {
                        output += QStringLiteral("</%1>").arg(spanTags.at(index));
                    }
                }
                cursor = tagEnd;
                continue;
            }

            const QString inlineStyleTag = canonicalInlineStyleTagName(rawTagName);
            if (!inlineStyleTag.isEmpty())
            {
                output += closingTag
                              ? QStringLiteral("</%1>").arg(inlineStyleTag)
                              : QStringLiteral("<%1>").arg(inlineStyleTag);
                cursor = tagEnd;
                continue;
            }

            output += fullTagToken;
            cursor = tagEnd;
        }

        if (insideBody && cursor < normalizedSource.size())
        {
            output += decodeXmlEntities(normalizedSource.mid(cursor));
        }

        while (!spanStyleStack.isEmpty())
        {
            const QStringList spanTags = spanStyleStack.takeLast();
            for (int index = spanTags.size() - 1; index >= 0; --index)
            {
                output += QStringLiteral("</%1>").arg(spanTags.at(index));
            }
        }

        output = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(output);
        return output;
    }

    QString serializeInlineTaggedLine(const QString& lineText)
    {
        if (lineText.isEmpty())
        {
            return {};
        }

        static const QRegularExpression tagPattern(
            QStringLiteral(R"(<\s*/?\s*([A-Za-z_][A-Za-z0-9_.:-]*)\b[^>]*>)"));

        QString output;
        QStringList openStyleTags;
        qsizetype cursor = 0;

        QRegularExpressionMatchIterator iterator = tagPattern.globalMatch(lineText);
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
                output += escapeXmlAttributeValue(lineText.mid(cursor, tagStart - cursor));
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
                    output += normalizeResourceStartTag(fullTagToken);
                }
                cursor = tagEnd;
                continue;
            }

            if (normalizedTagName == QStringLiteral("br"))
            {
                output += QStringLiteral("<br/>");
                cursor = tagEnd;
                continue;
            }

            const QString inlineStyleTag = canonicalInlineStyleTagName(rawTagName);
            if (!inlineStyleTag.isEmpty())
            {
                if (closingTag)
                {
                    closeMatchingInlineStyleTag(&output, &openStyleTags, inlineStyleTag);
                }
                else if (!selfClosingTag)
                {
                    output += QStringLiteral("<%1>").arg(inlineStyleTag);
                    openStyleTags.push_back(inlineStyleTag);
                }
                cursor = tagEnd;
                continue;
            }

            output += escapeXmlAttributeValue(fullTagToken);
            cursor = tagEnd;
        }

        if (cursor < lineText.size())
        {
            output += escapeXmlAttributeValue(lineText.mid(cursor));
        }

        for (int index = openStyleTags.size() - 1; index >= 0; --index)
        {
            output += QStringLiteral("</%1>").arg(openStyleTags.at(index));
        }

        return output;
    }

    QString normalizePath(QString path)
    {
        path = path.trimmed();
        if (path.isEmpty())
        {
            return {};
        }
        return QDir::cleanPath(path);
    }

    bool isTextBlockElement(const QString& elementName)
    {
        const QString normalizedName = elementName.trimmed().toCaseFolded();
        return normalizedName == QStringLiteral("p")
            || normalizedName == QStringLiteral("paragraph")
            || normalizedName == QStringLiteral("div")
            || normalizedName == QStringLiteral("li")
            || normalizedName == QStringLiteral("blockquote")
            || normalizedName == QStringLiteral("pre")
            || normalizedName == QStringLiteral("h1")
            || normalizedName == QStringLiteral("h2")
            || normalizedName == QStringLiteral("h3")
            || normalizedName == QStringLiteral("h4")
            || normalizedName == QStringLiteral("h5")
            || normalizedName == QStringLiteral("h6");
    }

    BodyDocumentTextFragments parseBodyDocumentTextFragments(const QString& bodyDocumentText)
    {
        BodyDocumentTextFragments fragments;
        if (bodyDocumentText.isEmpty())
        {
            return fragments;
        }

        QXmlStreamReader reader(normalizeResourceTagsForXmlParser(bodyDocumentText));
        bool insideBody = false;
        bool encounteredBlockElement = false;
        int blockDepth = 0;
        QString currentBlockText;
        QString currentBlockRichText;

        while (!reader.atEnd())
        {
            reader.readNext();
            if (reader.isStartElement())
            {
                const QString elementName = reader.name().toString();
                if (!insideBody)
                {
                    if (elementName.compare(QStringLiteral("body"), Qt::CaseInsensitive) == 0)
                    {
                        insideBody = true;
                    }
                    continue;
                }

                if (elementName.compare(QStringLiteral("resource"), Qt::CaseInsensitive) == 0)
                {
                    continue;
                }

                const InlineStyleHtmlTag inlineStyleTag = inlineStyleHtmlTagForElement(elementName);
                if (!inlineStyleTag.isEmpty())
                {
                    if (blockDepth > 0)
                    {
                        appendInlineStyleTag(&currentBlockRichText, inlineStyleTag, true);
                    }
                    else if (!encounteredBlockElement)
                    {
                        appendInlineStyleTag(&fragments.fallbackRichText, inlineStyleTag, true);
                    }
                    continue;
                }

                if (elementName.compare(QStringLiteral("br"), Qt::CaseInsensitive) == 0)
                {
                    if (blockDepth > 0)
                    {
                        currentBlockText += QLatin1Char('\n');
                        currentBlockRichText += QLatin1Char('\n');
                    }
                    else if (!encounteredBlockElement)
                    {
                        fragments.fallbackText += QLatin1Char('\n');
                        fragments.fallbackRichText += QLatin1Char('\n');
                    }
                    continue;
                }

                if (isTextBlockElement(elementName))
                {
                    encounteredBlockElement = true;
                    if (blockDepth == 0)
                    {
                        currentBlockText.clear();
                        currentBlockRichText.clear();
                    }
                    ++blockDepth;
                }
                continue;
            }

            if (!insideBody)
            {
                continue;
            }

            if (reader.isCharacters())
            {
                const QString text = reader.text().toString();
                const bool whitespaceOnlyText = text.trimmed().isEmpty();
                if (blockDepth > 0)
                {
                    currentBlockText += text;
                    currentBlockRichText += escapeHtmlText(text);
                }
                else if (!whitespaceOnlyText)
                {
                    fragments.fallbackText += text;
                    fragments.fallbackRichText += escapeHtmlText(text);
                }
                continue;
            }

            if (!reader.isEndElement())
            {
                continue;
            }

            const QString elementName = reader.name().toString();
            const InlineStyleHtmlTag inlineStyleTag = inlineStyleHtmlTagForElement(elementName);
            if (!inlineStyleTag.isEmpty())
            {
                if (blockDepth > 0)
                {
                    appendInlineStyleTag(&currentBlockRichText, inlineStyleTag, false);
                }
                else if (!encounteredBlockElement)
                {
                    appendInlineStyleTag(&fragments.fallbackRichText, inlineStyleTag, false);
                }
                continue;
            }

            if (elementName.compare(QStringLiteral("body"), Qt::CaseInsensitive) == 0)
            {
                break;
            }

            if (!isTextBlockElement(elementName) || blockDepth <= 0)
            {
                continue;
            }

            --blockDepth;
            if (blockDepth == 0)
            {
                fragments.blockLines.append(
                    WhatSon::NoteBodyPersistence::normalizeBodyPlainText(currentBlockText).split(
                        QLatin1Char('\n'),
                        Qt::KeepEmptyParts));
                fragments.blockRichLines.append(
                    WhatSon::NoteBodyPersistence::normalizeBodyPlainText(currentBlockRichText).split(
                        QLatin1Char('\n'),
                        Qt::KeepEmptyParts));
                currentBlockText.clear();
                currentBlockRichText.clear();
            }
        }

        fragments.fallbackText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(fragments.fallbackText);
        fragments.fallbackRichText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(fragments.fallbackRichText);
        return fragments;
    }
} // namespace

namespace WhatSon::NoteBodyPersistence
{
    QString normalizeBodyPlainText(QString text)
    {
        text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        text.replace(QChar::LineSeparator, QLatin1Char('\n'));
        text.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
        text.replace(QChar::Nbsp, QLatin1Char(' '));
        return text;
    }

    QString serializeBodyDocument(const QString& noteId, const QString& bodySourceText)
    {
        const QString normalizedId = noteId.trimmed().isEmpty()
                                         ? QStringLiteral("note")
                                         : escapeXmlAttributeValue(noteId.trimmed());
        const QString inlineTaggedText = normalizeEditorSourceToInlineTaggedText(bodySourceText);

        QString text;
        text += QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        text += QStringLiteral("<!DOCTYPE WHATSONNOTE>\n");
        text += QStringLiteral("<contents id=\"") + normalizedId + QStringLiteral("\">\n");
        text += QStringLiteral("  <body>\n");
        if (inlineTaggedText.isEmpty())
        {
            text += QStringLiteral("  </body>\n");
            text += QStringLiteral("</contents>\n");
            return text;
        }

        const QStringList lines = inlineTaggedText.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
        for (const QString& line : lines)
        {
            text += QStringLiteral("    <paragraph>");
            text += serializeInlineTaggedLine(line);
            text += QStringLiteral("</paragraph>\n");
        }
        text += QStringLiteral("  </body>\n");
        text += QStringLiteral("</contents>\n");
        return text;
    }

    QString plainTextFromBodyDocument(const QString& bodyDocumentText)
    {
        const BodyDocumentTextFragments fragments = parseBodyDocumentTextFragments(bodyDocumentText);
        if (!fragments.blockLines.isEmpty())
        {
            return fragments.blockLines.join(QLatin1Char('\n'));
        }
        return fragments.fallbackText;
    }

    QString sourceTextFromBodyDocument(const QString& bodyDocumentText)
    {
        return normalizeEditorSourceToInlineTaggedText(bodyDocumentText);
    }

    QString richTextFromBodyDocument(const QString& bodyDocumentText)
    {
        const BodyDocumentTextFragments fragments = parseBodyDocumentTextFragments(bodyDocumentText);
        if (!fragments.blockRichLines.isEmpty())
        {
            return richTextFromRichLines(fragments.blockRichLines);
        }

        return richTextFromRichLines(
            fragments.fallbackRichText.split(QLatin1Char('\n'), Qt::KeepEmptyParts));
    }

    QString firstLineFromBodyDocument(const QString& bodyDocumentText)
    {
        const BodyDocumentTextFragments fragments = parseBodyDocumentTextFragments(bodyDocumentText);
        const QString fallbackFirstLine = firstLineFromBodyPlainText(fragments.fallbackText);
        if (!fallbackFirstLine.isEmpty())
        {
            return fallbackFirstLine;
        }
        if (!fragments.blockLines.isEmpty())
        {
            return firstLineFromBodyPlainText(fragments.blockLines.join(QLatin1Char('\n')));
        }
        return {};
    }

    QString firstLineFromBodyPlainText(const QString& text)
    {
        const QString normalizedText = normalizeBodyPlainText(text);
        const QStringList lines = normalizedText.split(QLatin1Char('\n'));
        for (const QString& line : lines)
        {
            const QString trimmed = line.trimmed();
            if (!trimmed.isEmpty())
            {
                return trimmed;
            }
        }
        return {};
    }

    QString resolveBodyPath(const QString& noteDirectoryPath)
    {
        const QString normalizedNoteDirectoryPath = normalizePath(noteDirectoryPath);
        if (normalizedNoteDirectoryPath.isEmpty())
        {
            return {};
        }

        const QDir noteDir(normalizedNoteDirectoryPath);
        if (!noteDir.exists())
        {
            return {};
        }

        const QString noteStem = QFileInfo(normalizedNoteDirectoryPath).completeBaseName().trimmed();
        if (!noteStem.isEmpty())
        {
            const QString stemBodyPath = noteDir.filePath(noteStem + QStringLiteral(".wsnbody"));
            if (QFileInfo(stemBodyPath).isFile())
            {
                return QDir::cleanPath(stemBodyPath);
            }
        }

        const QString canonicalBodyPath = noteDir.filePath(QStringLiteral("note.wsnbody"));
        if (QFileInfo(canonicalBodyPath).isFile())
        {
            return QDir::cleanPath(canonicalBodyPath);
        }

        const QFileInfoList bodyCandidates = noteDir.entryInfoList(
            QStringList{QStringLiteral("*.wsnbody")},
            QDir::Files,
            QDir::Name);
        QString draftBodyPath;
        for (const QFileInfo& fileInfo : bodyCandidates)
        {
            const QString loweredName = fileInfo.fileName().toCaseFolded();
            if (loweredName.contains(QStringLiteral(".draft.")))
            {
                if (draftBodyPath.isEmpty())
                {
                    draftBodyPath = fileInfo.absoluteFilePath();
                }
                continue;
            }
            return QDir::cleanPath(fileInfo.absoluteFilePath());
        }

        if (!draftBodyPath.isEmpty())
        {
            return QDir::cleanPath(draftBodyPath);
        }

        if (!noteStem.isEmpty())
        {
            return QDir::cleanPath(noteDir.filePath(noteStem + QStringLiteral(".wsnbody")));
        }
        return QDir::cleanPath(noteDir.filePath(QStringLiteral("note.wsnbody")));
    }

    QString resolveHeaderPath(const QString& noteHeaderPath, const QString& noteDirectoryPath)
    {
        const QString directPath = normalizePath(noteHeaderPath);
        if (!directPath.isEmpty() && QFileInfo(directPath).isFile())
        {
            return directPath;
        }

        const QString normalizedNoteDirectoryPath = normalizePath(noteDirectoryPath);
        if (normalizedNoteDirectoryPath.isEmpty())
        {
            return {};
        }

        const QDir noteDir(normalizedNoteDirectoryPath);
        if (!noteDir.exists())
        {
            return {};
        }

        const QString noteStem = QFileInfo(normalizedNoteDirectoryPath).completeBaseName().trimmed();
        if (!noteStem.isEmpty())
        {
            const QString stemHeaderPath = noteDir.filePath(noteStem + QStringLiteral(".wsnhead"));
            if (QFileInfo(stemHeaderPath).isFile())
            {
                return QDir::cleanPath(stemHeaderPath);
            }
        }

        const QString canonicalHeaderPath = noteDir.filePath(QStringLiteral("note.wsnhead"));
        if (QFileInfo(canonicalHeaderPath).isFile())
        {
            return QDir::cleanPath(canonicalHeaderPath);
        }

        const QFileInfoList headerCandidates = noteDir.entryInfoList(
            QStringList{QStringLiteral("*.wsnhead")},
            QDir::Files,
            QDir::Name);
        QString draftHeaderPath;
        for (const QFileInfo& fileInfo : headerCandidates)
        {
            const QString loweredName = fileInfo.fileName().toCaseFolded();
            if (loweredName.contains(QStringLiteral(".draft.")))
            {
                if (draftHeaderPath.isEmpty())
                {
                    draftHeaderPath = fileInfo.absoluteFilePath();
                }
                continue;
            }
            return QDir::cleanPath(fileInfo.absoluteFilePath());
        }

        if (!draftHeaderPath.isEmpty())
        {
            return QDir::cleanPath(draftHeaderPath);
        }

        if (!noteStem.isEmpty())
        {
            return QDir::cleanPath(noteDir.filePath(noteStem + QStringLiteral(".wsnhead")));
        }
        return QDir::cleanPath(noteDir.filePath(QStringLiteral("note.wsnhead")));
    }

    bool persistBodyPlainText(
        const QString& noteId,
        const QString& noteDirectoryPath,
        const QString& noteHeaderPath,
        const QString& bodyPlainText,
        QString* outNormalizedBodyText,
        QString* outNormalizedBodySourceText,
        QString* outLastModifiedAt,
        QString* errorMessage)
    {
        WhatSonLocalNoteFileStore localNoteFileStore;
        WhatSonLocalNoteFileStore::ReadRequest readRequest;
        readRequest.noteId = noteId;
        readRequest.noteDirectoryPath = noteDirectoryPath;
        readRequest.noteHeaderPath = noteHeaderPath;

        WhatSonLocalNoteDocument document;
        QString readError;
        if (!localNoteFileStore.readNote(std::move(readRequest), &document, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            return false;
        }

        const QString serializedBodyDocument = serializeBodyDocument(noteId, bodyPlainText);
        const QString normalizedBodyText = plainTextFromBodyDocument(serializedBodyDocument);
        const QString normalizedBodySourceText = sourceTextFromBodyDocument(serializedBodyDocument);
        if (normalizedBodyText == document.bodyPlainText
            && normalizedBodySourceText == document.bodySourceText)
        {
            if (outNormalizedBodyText != nullptr)
            {
                *outNormalizedBodyText = normalizedBodyText;
            }
            if (outNormalizedBodySourceText != nullptr)
            {
                *outNormalizedBodySourceText = normalizedBodySourceText;
            }
            if (outLastModifiedAt != nullptr)
            {
                *outLastModifiedAt = document.headerStore.lastModifiedAt();
            }
            return true;
        }

        document.bodyPlainText = normalizedBodyText;
        document.bodySourceText = normalizedBodySourceText;

        WhatSonLocalNoteFileStore::UpdateRequest updateRequest;
        updateRequest.document = document;
        updateRequest.persistHeader = true;
        updateRequest.persistBody = true;
        updateRequest.touchLastModified = true;
        updateRequest.incrementModifiedCount = false;

        QString updateError;
        if (!localNoteFileStore.updateNote(std::move(updateRequest), &document, &updateError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = updateError;
            }
            return false;
        }

        if (outNormalizedBodyText != nullptr)
        {
            *outNormalizedBodyText = document.bodyPlainText;
        }
        if (outNormalizedBodySourceText != nullptr)
        {
            *outNormalizedBodySourceText = document.bodySourceText;
        }
        if (outLastModifiedAt != nullptr)
        {
            *outLastModifiedAt = document.headerStore.lastModifiedAt();
        }
        return true;
    }
} // namespace WhatSon::NoteBodyPersistence
