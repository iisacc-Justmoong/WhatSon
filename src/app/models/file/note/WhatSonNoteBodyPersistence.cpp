#include "app/models/file/note/WhatSonNoteBodyPersistence.hpp"

#include "app/models/file/note/WhatSonIiXmlDocumentSupport.hpp"
#include "app/models/file/note/WhatSonNoteBodySemanticTagSupport.hpp"
#include "app/models/file/note/WhatSonNoteBodyWebLinkSupport.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/models/file/note/WhatSonLocalNoteFileStore.hpp"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSet>

#include <utility>

namespace
{
    namespace IiXml = WhatSon::IiXmlDocumentSupport;
    namespace SemanticTags = WhatSon::NoteBodySemanticTagSupport;

    QString normalizePath(const QString& path)
    {
        const QString trimmed = path.trimmed();
        if (trimmed.isEmpty())
        {
            return {};
        }
        return QDir::cleanPath(trimmed);
    }

    QString escapeXml(QString value)
    {
        value.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
        value.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
        value.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
        value.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
        value.replace(QStringLiteral("'"), QStringLiteral("&apos;"));
        return value;
    }

    QString escapeHtml(QString value)
    {
        value.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
        value.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
        value.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
        value.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
        value.replace(QStringLiteral("'"), QStringLiteral("&#39;"));
        return value;
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

    bool shouldPreserveInlineSourceTag(const QString& tagName)
    {
        return !SemanticTags::canonicalInlineStyleTagName(tagName).isEmpty()
            || SemanticTags::isWebLinkTagName(tagName)
            || SemanticTags::isHashtagTagName(tagName)
            || SemanticTags::isTransparentContainerTagName(tagName)
            || SemanticTags::isRenderedTextBlockElement(tagName);
    }

    bool isClosingTagToken(QStringView tagToken)
    {
        for (int cursor = 1; cursor < tagToken.size(); ++cursor)
        {
            const QChar ch = tagToken.at(cursor);
            if (!ch.isSpace())
            {
                return ch == QLatin1Char('/');
            }
        }
        return false;
    }

    bool isSelfClosingTagToken(QStringView tagToken)
    {
        QStringView trimmed = tagToken.trimmed();
        return trimmed.size() >= 2
            && trimmed.at(trimmed.size() - 2) == QLatin1Char('/')
            && trimmed.back() == QLatin1Char('>');
    }

    QString inlineStyleOpeningHtml(const QString& canonicalStyleTag)
    {
        if (canonicalStyleTag == QStringLiteral("bold"))
        {
            return QStringLiteral("<strong style=\"font-weight:900;\">");
        }
        if (canonicalStyleTag == QStringLiteral("italic"))
        {
            return QStringLiteral("<span style=\"font-style:italic;\">");
        }
        if (canonicalStyleTag == QStringLiteral("underline"))
        {
            return QStringLiteral("<span style=\"text-decoration: underline;\">");
        }
        if (canonicalStyleTag == QStringLiteral("strikethrough"))
        {
            return QStringLiteral("<span style=\"text-decoration: line-through;\">");
        }
        if (canonicalStyleTag == QStringLiteral("highlight"))
        {
            return QStringLiteral("<span style=\"background-color:#8A4B00;color:#D6AE58;font-weight:600;\">");
        }
        return {};
    }

    QString inlineStyleClosingHtml(const QString& canonicalStyleTag)
    {
        if (canonicalStyleTag == QStringLiteral("bold"))
        {
            return QStringLiteral("</strong>");
        }
        if (!canonicalStyleTag.isEmpty())
        {
            return QStringLiteral("</span>");
        }
        return {};
    }

    QString openingHtmlForSourceTag(QStringView tagToken, const QString& tagName)
    {
        const QString canonicalStyleTag = SemanticTags::canonicalInlineStyleTagName(tagName);
        if (!canonicalStyleTag.isEmpty())
        {
            return inlineStyleOpeningHtml(canonicalStyleTag);
        }
        if (SemanticTags::isWebLinkTagName(tagName))
        {
            return WhatSon::NoteBodyWebLinkSupport::openingHtmlFromRawToken(tagToken.toString());
        }
        if (SemanticTags::isHashtagTagName(tagName))
        {
            return QStringLiteral("#");
        }
        return SemanticTags::semanticTextOpeningHtml(tagName);
    }

    QString closingHtmlForSourceTag(const QString& tagName)
    {
        const QString canonicalStyleTag = SemanticTags::canonicalInlineStyleTagName(tagName);
        if (!canonicalStyleTag.isEmpty())
        {
            return inlineStyleClosingHtml(canonicalStyleTag);
        }
        if (SemanticTags::isWebLinkTagName(tagName))
        {
            return QStringLiteral("</a>");
        }
        if (SemanticTags::isHashtagTagName(tagName))
        {
            return {};
        }
        return SemanticTags::semanticTextClosingHtml(tagName);
    }

    QString renderInlineSourceToHtml(const QString& sourceFragment)
    {
        QString rendered;
        rendered.reserve(sourceFragment.size() + 32);

        int cursor = 0;
        while (cursor < sourceFragment.size())
        {
            if (sourceFragment.at(cursor) == QLatin1Char('<'))
            {
                const int tagEnd = sourceFragment.indexOf(QLatin1Char('>'), cursor + 1);
                if (tagEnd > cursor)
                {
                    const QStringView tagToken(sourceFragment.constData() + cursor, tagEnd - cursor + 1);
                    const QString tagName = sourceTagName(tagToken);
                    const bool recognizedTag = !SemanticTags::canonicalInlineStyleTagName(tagName).isEmpty()
                        || SemanticTags::isWebLinkTagName(tagName)
                        || SemanticTags::isHashtagTagName(tagName)
                        || !SemanticTags::semanticTextOpeningHtml(tagName).isEmpty();
                    if (SemanticTags::isRenderedLineBreakTagName(tagName))
                    {
                        rendered += QStringLiteral("<br/>");
                        cursor = tagEnd + 1;
                        continue;
                    }
                    if (recognizedTag)
                    {
                        if (isClosingTagToken(tagToken))
                        {
                            rendered += closingHtmlForSourceTag(tagName);
                        }
                        else
                        {
                            rendered += openingHtmlForSourceTag(tagToken, tagName);
                            if (isSelfClosingTagToken(tagToken))
                            {
                                rendered += closingHtmlForSourceTag(tagName);
                            }
                        }
                        cursor = tagEnd + 1;
                        continue;
                    }
                }
            }

            const int nextTag = sourceFragment.indexOf(QLatin1Char('<'), cursor + 1);
            const int textEnd = nextTag >= 0 ? nextTag : sourceFragment.size();
            rendered += escapeHtml(sourceFragment.mid(cursor, textEnd - cursor));
            cursor = textEnd;
        }

        return rendered;
    }

    QString renderInlineSourceToPlainText(const QString& sourceFragment)
    {
        QString rendered;
        rendered.reserve(sourceFragment.size());

        int cursor = 0;
        while (cursor < sourceFragment.size())
        {
            if (sourceFragment.at(cursor) == QLatin1Char('<'))
            {
                const int tagEnd = sourceFragment.indexOf(QLatin1Char('>'), cursor + 1);
                if (tagEnd > cursor)
                {
                    const QStringView tagToken(sourceFragment.constData() + cursor, tagEnd - cursor + 1);
                    const QString tagName = sourceTagName(tagToken);
                    const bool recognizedTag = !SemanticTags::canonicalInlineStyleTagName(tagName).isEmpty()
                        || SemanticTags::isWebLinkTagName(tagName)
                        || SemanticTags::isHashtagTagName(tagName)
                        || SemanticTags::isTransparentContainerTagName(tagName)
                        || SemanticTags::isRenderedTextBlockElement(tagName)
                        || !SemanticTags::semanticTextOpeningHtml(tagName).isEmpty();
                    if (SemanticTags::isRenderedLineBreakTagName(tagName))
                    {
                        rendered += QLatin1Char('\n');
                        cursor = tagEnd + 1;
                        continue;
                    }
                    if (recognizedTag)
                    {
                        if (!isClosingTagToken(tagToken) && SemanticTags::isHashtagTagName(tagName))
                        {
                            rendered += QLatin1Char('#');
                        }
                        cursor = tagEnd + 1;
                        continue;
                    }
                }
            }

            const int nextTag = sourceFragment.indexOf(QLatin1Char('<'), cursor + 1);
            const int textEnd = nextTag >= 0 ? nextTag : sourceFragment.size();
            rendered += IiXml::decodeXmlEntities(sourceFragment.mid(cursor, textEnd - cursor));
            cursor = textEnd;
        }

        return rendered;
    }

    QString encodeInlineSourceFragment(const QString& sourceFragment)
    {
        QString encoded;
        encoded.reserve(sourceFragment.size() + 16);

        int cursor = 0;
        while (cursor < sourceFragment.size())
        {
            if (sourceFragment.at(cursor) == QLatin1Char('<'))
            {
                const int tagEnd = sourceFragment.indexOf(QLatin1Char('>'), cursor + 1);
                if (tagEnd > cursor)
                {
                    const QStringView tagToken(sourceFragment.constData() + cursor, tagEnd - cursor + 1);
                    const QString tagName = sourceTagName(tagToken);
                    if (shouldPreserveInlineSourceTag(tagName))
                    {
                        encoded += sourceFragment.mid(cursor, tagEnd - cursor + 1);
                        cursor = tagEnd + 1;
                        continue;
                    }
                }
            }

            const int nextTag = sourceFragment.indexOf(QLatin1Char('<'), cursor + 1);
            const int textEnd = nextTag >= 0 ? nextTag : sourceFragment.size();
            encoded += escapeXml(sourceFragment.mid(cursor, textEnd - cursor));
            cursor = textEnd;
        }

        return encoded;
    }

    QString decodeInlineSourceFragment(const QString& rawFragment)
    {
        QString decoded;
        decoded.reserve(rawFragment.size());

        int cursor = 0;
        while (cursor < rawFragment.size())
        {
            if (rawFragment.at(cursor) == QLatin1Char('<'))
            {
                const int tagEnd = rawFragment.indexOf(QLatin1Char('>'), cursor + 1);
                if (tagEnd > cursor)
                {
                    decoded += rawFragment.mid(cursor, tagEnd - cursor + 1);
                    cursor = tagEnd + 1;
                    continue;
                }
            }

            const int nextTag = rawFragment.indexOf(QLatin1Char('<'), cursor + 1);
            const int textEnd = nextTag >= 0 ? nextTag : rawFragment.size();
            decoded += IiXml::decodeXmlEntities(rawFragment.mid(cursor, textEnd - cursor));
            cursor = textEnd;
        }

        return decoded;
    }

    QString prepareWsnXmlForIiXml(QString source)
    {
        source = IiXml::stripXmlPreamble(source);
        source.remove(QRegularExpression(QStringLiteral(R"(<!--[\s\S]*?-->)")));
        source.remove(
            QRegularExpression(
                QStringLiteral(R"(</\s*resource\s*>)"),
                QRegularExpression::CaseInsensitiveOption));

        static const QRegularExpression resourceStartTagPattern(
            QStringLiteral(R"(<resource\b[^>]*>)"),
            QRegularExpression::CaseInsensitiveOption);

        int searchOffset = 0;
        while (searchOffset >= 0 && searchOffset < source.size())
        {
            const QRegularExpressionMatch match = resourceStartTagPattern.match(source, searchOffset);
            if (!match.hasMatch())
            {
                break;
            }

            QString replacement = match.captured(0).trimmed();
            if (!replacement.endsWith(QStringLiteral("/>")))
            {
                replacement.chop(1);
                replacement = replacement.trimmed() + QStringLiteral(" />");
            }
            source.replace(match.capturedStart(0), match.capturedLength(0), replacement);
            searchOffset = match.capturedStart(0) + replacement.size();
        }

        return source;
    }

    iiXml::Parser::TagDocumentResult parseBodyDocument(const QString& bodyDocumentText)
    {
        return IiXml::parseDocument(prepareWsnXmlForIiXml(bodyDocumentText));
    }

    const iiXml::Parser::TagNode* bodyNodeFromDocument(const iiXml::Parser::TagDocument& document)
    {
        return IiXml::findFirstDescendant(document.Nodes, QStringLiteral("body"));
    }

    QString fieldName(const iiXml::Parser::TagDocument& document, const iiXml::Parser::TagField& field)
    {
        return IiXml::stringFromUtf8View(document.FieldNameView(field));
    }

    QString fieldValue(const iiXml::Parser::TagDocument& document, const iiXml::Parser::TagField& field)
    {
        return IiXml::decodeXmlEntities(IiXml::stringFromUtf8View(document.FieldValueView(field)));
    }

    QString nodeName(const iiXml::Parser::TagNode& node)
    {
        return QString::fromStdString(node.Range.TagName);
    }

    QString serializeAttributes(const iiXml::Parser::TagDocument& document, const iiXml::Parser::TagNode& node)
    {
        QStringList attributes;
        attributes.reserve(static_cast<qsizetype>(node.Fields.size()));
        for (const iiXml::Parser::TagField& field : node.Fields)
        {
            if (!field.HasValue)
            {
                continue;
            }

            const QString name = fieldName(document, field).trimmed();
            if (name.isEmpty())
            {
                continue;
            }

            attributes.push_back(
                QStringLiteral("%1=\"%2\"").arg(name, escapeXml(fieldValue(document, field))));
        }

        return attributes.join(QLatin1Char(' '));
    }

    QString serializeDirectBodyNodeToSource(const iiXml::Parser::TagDocument& document, const iiXml::Parser::TagNode& node)
    {
        const QString tagName = nodeName(node).trimmed();
        if (tagName.isEmpty())
        {
            return {};
        }

        if (QString::compare(tagName, QStringLiteral("paragraph"), Qt::CaseInsensitive) == 0)
        {
            return WhatSon::NoteBodyPersistence::normalizeBodyPlainText(
                decodeInlineSourceFragment(IiXml::stringFromUtf8View(document.ValueView(node))));
        }

        if (SemanticTags::isSourceProjectionTextBlockElement(tagName))
        {
            return WhatSon::NoteBodyPersistence::normalizeBodyPlainText(IiXml::nodeText(document, &node));
        }

        if (QString::compare(tagName, QStringLiteral("break"), Qt::CaseInsensitive) == 0
            || QString::compare(tagName, QStringLiteral("hr"), Qt::CaseInsensitive) == 0)
        {
            return QStringLiteral("</break>");
        }

        const QString attributes = serializeAttributes(document, node);
        if (QString::compare(tagName, QStringLiteral("resource"), Qt::CaseInsensitive) == 0)
        {
            return attributes.isEmpty()
                       ? QStringLiteral("<resource />")
                       : QStringLiteral("<resource %1 />").arg(attributes);
        }

        const QString text = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(IiXml::nodeText(document, &node));
        if (text.isEmpty())
        {
            return attributes.isEmpty()
                       ? QStringLiteral("<%1 />").arg(tagName)
                       : QStringLiteral("<%1 %2 />").arg(tagName, attributes);
        }

        const QString openTag = attributes.isEmpty()
                                    ? QStringLiteral("<%1>").arg(tagName)
                                    : QStringLiteral("<%1 %2>").arg(tagName, attributes);
        return openTag + escapeXml(text) + QStringLiteral("</%1>").arg(tagName);
    }

    QStringList bodySourceLinesFromDocument(const QString& bodyDocumentText)
    {
        static const QRegularExpression paragraphLinePattern(
            QStringLiteral(R"(^\s*<paragraph>([\s\S]*)</paragraph>\s*$)"),
            QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression resourceLinePattern(
            QStringLiteral(R"(^\s*(<resource\b[^>]*?/?>)\s*$)"),
            QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression breakLinePattern(
            QStringLiteral(R"(^\s*<(?:break|hr)\b[^>]*?/?>\s*$)"),
            QRegularExpression::CaseInsensitiveOption);

        const QString normalizedDocumentText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(bodyDocumentText);
        const QStringList documentLines = normalizedDocumentText.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
        bool insideBody = false;
        QStringList rawLines;
        for (const QString& line : documentLines)
        {
            const QString trimmedLine = line.trimmed();
            if (!insideBody)
            {
                if (QString::compare(trimmedLine, QStringLiteral("<body>"), Qt::CaseInsensitive) == 0)
                    insideBody = true;
                continue;
            }

            if (QString::compare(trimmedLine, QStringLiteral("</body>"), Qt::CaseInsensitive) == 0)
                break;

            QRegularExpressionMatch paragraphMatch = paragraphLinePattern.match(line);
            if (paragraphMatch.hasMatch())
            {
                rawLines.push_back(decodeInlineSourceFragment(paragraphMatch.captured(1)));
                continue;
            }

            QRegularExpressionMatch resourceMatch = resourceLinePattern.match(line);
            if (resourceMatch.hasMatch())
            {
                rawLines.push_back(resourceMatch.captured(1).trimmed());
                continue;
            }

            if (breakLinePattern.match(line).hasMatch())
            {
                rawLines.push_back(QStringLiteral("</break>"));
                continue;
            }
        }

        if (!rawLines.isEmpty())
            return rawLines;

        const iiXml::Parser::TagDocumentResult parsedDocument = parseBodyDocument(bodyDocumentText);
        if (parsedDocument.Status != iiXml::Parser::TagTreeParseStatus::Parsed
            || !parsedDocument.Document.has_value())
        {
            return WhatSon::NoteBodyPersistence::normalizeBodyPlainText(bodyDocumentText)
                .split(QLatin1Char('\n'), Qt::KeepEmptyParts);
        }

        const iiXml::Parser::TagDocument& document = parsedDocument.Document.value();
        const iiXml::Parser::TagNode* bodyNode = bodyNodeFromDocument(document);
        if (bodyNode == nullptr)
        {
            return {};
        }

        QStringList lines;
        lines.reserve(static_cast<qsizetype>(bodyNode->Children.size()));
        for (const iiXml::Parser::TagNode& childNode : bodyNode->Children)
        {
            lines.push_back(serializeDirectBodyNodeToSource(document, childNode));
        }
        return lines;
    }

    QStringList bodyPlainLinesFromDocument(const QString& bodyDocumentText)
    {
        const iiXml::Parser::TagDocumentResult parsedDocument = parseBodyDocument(bodyDocumentText);
        if (parsedDocument.Status != iiXml::Parser::TagTreeParseStatus::Parsed
            || !parsedDocument.Document.has_value())
        {
            return WhatSon::NoteBodyPersistence::normalizeBodyPlainText(bodyDocumentText)
                .split(QLatin1Char('\n'), Qt::KeepEmptyParts);
        }

        const iiXml::Parser::TagDocument& document = parsedDocument.Document.value();
        const iiXml::Parser::TagNode* bodyNode = bodyNodeFromDocument(document);
        if (bodyNode == nullptr)
        {
            return {};
        }

        QStringList lines;
        lines.reserve(static_cast<qsizetype>(bodyNode->Children.size()));
        for (const iiXml::Parser::TagNode& childNode : bodyNode->Children)
        {
            const QString tagName = nodeName(childNode).trimmed();
            if (QString::compare(tagName, QStringLiteral("resource"), Qt::CaseInsensitive) == 0
                || QString::compare(tagName, QStringLiteral("break"), Qt::CaseInsensitive) == 0
                || QString::compare(tagName, QStringLiteral("hr"), Qt::CaseInsensitive) == 0)
            {
                lines.push_back(QString());
                continue;
            }
            lines.push_back(WhatSon::NoteBodyPersistence::normalizeBodyPlainText(
                renderInlineSourceToPlainText(IiXml::nodeText(document, &childNode))));
        }
        return lines;
    }

    QString firstNonEmptyLine(const QStringList& lines)
    {
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

    bool isStandaloneResourceLine(const QString& trimmedLine)
    {
        static const QRegularExpression resourcePattern(
            QStringLiteral(R"(^<\s*resource\b[^>]*?/?>$)"),
            QRegularExpression::CaseInsensitiveOption);
        return resourcePattern.match(trimmedLine).hasMatch();
    }

    QString canonicalResourceLine(QString trimmedLine)
    {
        if (!isStandaloneResourceLine(trimmedLine))
        {
            return {};
        }

        if (!trimmedLine.endsWith(QStringLiteral("/>")))
        {
            trimmedLine.chop(1);
            trimmedLine = trimmedLine.trimmed() + QStringLiteral(" />");
        }
        return trimmedLine;
    }

    bool isStandaloneBreakLine(const QString& trimmedLine)
    {
        static const QRegularExpression breakPattern(
            QStringLiteral(R"(^(?:</\s*break\s*>|<\s*break\s*/\s*>|<\s*hr\b[^>]*?/\s*>)$)"),
            QRegularExpression::CaseInsensitiveOption);
        return breakPattern.match(trimmedLine).hasMatch();
    }

    QString serializeParagraphLine(const QString& line)
    {
        return QStringLiteral("    <paragraph>%1</paragraph>\n").arg(encodeInlineSourceFragment(line));
    }

    QString sourceTextFallback(const QString& bodyPlainText)
    {
        const QString normalized = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(bodyPlainText);
        return normalized;
    }
}

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
                                         : escapeXml(noteId.trimmed());
        const QString normalizedSourceText =
            WhatSon::NoteBodyWebLinkSupport::autoWrapDetectedWebLinks(normalizeBodyPlainText(bodySourceText));
        const QStringList lines = normalizedSourceText.split(QLatin1Char('\n'), Qt::KeepEmptyParts);

        QString text;
        text += QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        text += QStringLiteral("<!DOCTYPE WHATSONNOTE>\n");
        text += QStringLiteral("<contents id=\"") + normalizedId + QStringLiteral("\">\n");
        text += QStringLiteral("  <body>\n");
        for (const QString& line : lines)
        {
            const QString trimmedLine = line.trimmed();
            const QString resourceLine = canonicalResourceLine(trimmedLine);
            if (!resourceLine.isEmpty())
            {
                text += QStringLiteral("    ") + resourceLine + QLatin1Char('\n');
                continue;
            }
            if (isStandaloneBreakLine(trimmedLine))
            {
                text += QStringLiteral("    <break />\n");
                continue;
            }
            text += serializeParagraphLine(line);
        }
        text += QStringLiteral("  </body>\n");
        text += QStringLiteral("</contents>\n");
        return text;
    }

    QStringList extractedInlineTagValues(const QString& bodySourceText)
    {
        static const QRegularExpression hashtagPattern(
            QStringLiteral(R"((?:^|[\s\(\[\{])#([\p{L}\p{N}_\-/]+))"));

        QStringList extractedTags;
        QSet<QString> seenTags;
        QRegularExpressionMatchIterator iterator = hashtagPattern.globalMatch(normalizeBodyPlainText(bodySourceText));
        while (iterator.hasNext())
        {
            const QString tag = iterator.next().captured(1).trimmed();
            if (tag.isEmpty())
            {
                continue;
            }

            const QString key = tag.toCaseFolded();
            if (seenTags.contains(key))
            {
                continue;
            }
            seenTags.insert(key);
            extractedTags.push_back(tag);
        }
        return extractedTags;
    }

    QString plainTextFromBodyDocument(const QString& bodyDocumentText)
    {
        return bodyPlainLinesFromDocument(bodyDocumentText).join(QLatin1Char('\n'));
    }

    QString sourceTextFromBodyDocument(const QString& bodyDocumentText)
    {
        const QString sourceText = bodySourceLinesFromDocument(bodyDocumentText).join(QLatin1Char('\n'));
        if (!sourceText.isEmpty())
        {
            return sourceText;
        }
        return sourceTextFallback(plainTextFromBodyDocument(bodyDocumentText));
    }

    QString htmlProjectionFromBodyDocument(const QString& bodyDocumentText)
    {
        const QStringList lines = bodySourceLinesFromDocument(bodyDocumentText);
        QStringList htmlLines;
        htmlLines.reserve(lines.size());
        for (const QString& line : lines)
        {
            htmlLines.push_back(renderInlineSourceToHtml(line));
        }
        return htmlLines.join(QStringLiteral("<br/>"));
    }

    QString firstLineFromBodyDocument(const QString& bodyDocumentText)
    {
        const QString firstLine = firstNonEmptyLine(bodyPlainLinesFromDocument(bodyDocumentText));
        if (!firstLine.isEmpty())
        {
            return firstLine;
        }
        return firstNonEmptyLine(bodySourceLinesFromDocument(bodyDocumentText));
    }

    QString firstLineFromBodyPlainText(const QString& text)
    {
        return firstNonEmptyLine(normalizeBodyPlainText(text).split(QLatin1Char('\n'), Qt::KeepEmptyParts));
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
        QString normalizedBodySourceText = normalizeBodyPlainText(bodyPlainText);
        if (normalizedBodySourceText.isEmpty())
        {
            normalizedBodySourceText = normalizedBodyText;
        }
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
