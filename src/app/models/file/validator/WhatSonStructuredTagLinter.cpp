#include "WhatSonStructuredTagLinter.hpp"
#include "models/file/note/WhatSonNoteBodyWebLinkSupport.hpp"
#include "models/file/note/WhatSonNoteBodySemanticTagSupport.hpp"

#include <QDate>
#include <QRegularExpression>
#include <QStringList>
#include <QVariantList>
#include <QXmlStreamReader>

namespace
{
    namespace SemanticTags = WhatSon::NoteBodySemanticTagSupport;
    namespace WebLinks = WhatSon::NoteBodyWebLinkSupport;

    const QRegularExpression kAgendaOpenPattern(
        QStringLiteral(R"(<agenda\b[^>]*>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kAgendaClosePattern(
        QStringLiteral(R"(</agenda>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kAgendaSelfClosingPattern(
        QStringLiteral(R"(<agenda\b[^>]*/\s*>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kAgendaBlockPattern(
        QStringLiteral(R"(<agenda\b([^>]*)>([\s\S]*?)</agenda>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kTaskOpenPattern(
        QStringLiteral(R"(<task\b[^>]*>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kTaskClosePattern(
        QStringLiteral(R"(</task>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kTaskSelfClosingPattern(
        QStringLiteral(R"(<task\b[^>]*/\s*>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kTaskBlockPattern(
        QStringLiteral(R"(<task\b([^>]*)>([\s\S]*?)</task>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kCalloutOpenPattern(
        QStringLiteral(R"(<callout\b[^>]*>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kCalloutClosePattern(
        QStringLiteral(R"(</callout>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kCalloutSelfClosingPattern(
        QStringLiteral(R"(<callout\b[^>]*/\s*>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kCalloutBlockPattern(
        QStringLiteral(R"(<callout\b([^>]*)>([\s\S]*?)</callout>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kCanonicalBreakPattern(
        QStringLiteral(R"(</break>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kLegacyBreakPattern(
        QStringLiteral(R"((?:<\s*(?:break|hr)\b[^>]*?/?>|</\s*hr\s*>))"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kTagAttributePattern(
        QStringLiteral(R"ATTR(\b([A-Za-z_][A-Za-z0-9_.:-]*)\s*=\s*(?:"([^"]*)"|'([^']*)'|([^\s>]+)))ATTR"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kXmlLikeTagPattern(
        QStringLiteral(R"(<\s*/?\s*([A-Za-z_][A-Za-z0-9_.:-]*)\b[^>]*>)"));

    QString normalizePlainText(QString text)
    {
        text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        text.replace(QChar::LineSeparator, QLatin1Char('\n'));
        text.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
        text.replace(QChar::Nbsp, QLatin1Char(' '));
        return text;
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

    QString escapeXmlAttributeValue(QString value)
    {
        value.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
        value.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
        value.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
        value.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
        value.replace(QStringLiteral("'"), QStringLiteral("&apos;"));
        return value;
    }

    QString normalizedTagName(const QString& rawTagName)
    {
        return rawTagName.trimmed().toCaseFolded();
    }

    int regexMatchCount(const QString& text, const QRegularExpression& pattern)
    {
        int count = 0;
        QRegularExpressionMatchIterator iterator = pattern.globalMatch(text);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            if (match.hasMatch())
            {
                ++count;
            }
        }
        return count;
    }

    QVariantMap issueMap(
        const QString& code,
        const QString& message,
        const QVariantMap& context = {})
    {
        QVariantMap issue;
        issue.insert(QStringLiteral("code"), code);
        issue.insert(QStringLiteral("message"), message);
        if (!context.isEmpty())
        {
            issue.insert(QStringLiteral("context"), context);
        }
        return issue;
    }

    QString attributeValue(const QString& rawTagText, const QString& attributeName)
    {
        const QString normalizedAttributeName = attributeName.trimmed();
        if (normalizedAttributeName.isEmpty())
        {
            return {};
        }

        const QRegularExpression attributePattern(
            QStringLiteral(R"ATTR(\b%1\s*=\s*(?:"([^"]*)"|'([^']*)'|([^\s>]+)))ATTR")
                .arg(QRegularExpression::escape(normalizedAttributeName)),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch match = attributePattern.match(rawTagText);
        if (!match.hasMatch())
        {
            return {};
        }

        if (match.capturedStart(1) >= 0)
        {
            return decodeXmlEntities(match.captured(1));
        }
        if (match.capturedStart(2) >= 0)
        {
            return decodeXmlEntities(match.captured(2));
        }
        if (match.capturedStart(3) >= 0)
        {
            return decodeXmlEntities(match.captured(3));
        }
        return {};
    }

    int attributeCount(QString rawTagText)
    {
        int count = 0;
        rawTagText.remove(QLatin1Char('/'));
        QRegularExpressionMatchIterator iterator = kTagAttributePattern.globalMatch(rawTagText);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            if (match.hasMatch())
            {
                ++count;
            }
        }
        return count;
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

    bool hasCanonicalIsoDate(const QString& rawDate)
    {
        const QString decodedDate = decodeXmlEntities(rawDate).trimmed();
        const QDate parsedDate = QDate::fromString(decodedDate, QStringLiteral("yyyy-MM-dd"));
        return parsedDate.isValid() && parsedDate.toString(QStringLiteral("yyyy-MM-dd")) == decodedDate;
    }

    QString normalizedAgendaDate(const QString& rawDate)
    {
        const QString decodedDate = decodeXmlEntities(rawDate).trimmed();
        const QDate parsedDate = QDate::fromString(decodedDate, QStringLiteral("yyyy-MM-dd"));
        if (parsedDate.isValid())
        {
            return parsedDate.toString(QStringLiteral("yyyy-MM-dd"));
        }
        return QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));
    }

    bool isCanonicalDoneLiteral(const QString& rawDone)
    {
        const QString decoded = decodeXmlEntities(rawDone).trimmed();
        return decoded == QStringLiteral("true") || decoded == QStringLiteral("false");
    }

    bool normalizedTaskDoneValue(const QString& rawDone)
    {
        const QString lowered = decodeXmlEntities(rawDone).trimmed().toCaseFolded();
        return lowered == QStringLiteral("true")
            || lowered == QStringLiteral("1")
            || lowered == QStringLiteral("yes");
    }

    QString canonicalAgendaStartTag(const QString& rawTagText)
    {
        return QStringLiteral("<agenda date=\"%1\">")
            .arg(escapeXmlAttributeValue(normalizedAgendaDate(attributeValue(rawTagText, QStringLiteral("date")))));
    }

    QString canonicalTaskStartTag(const QString& rawTagText)
    {
        const bool done = normalizedTaskDoneValue(attributeValue(rawTagText, QStringLiteral("done")));
        return QStringLiteral("<task done=\"%1\">")
            .arg(done ? QStringLiteral("true") : QStringLiteral("false"));
    }

    QString canonicalCalloutStartTag()
    {
        return QStringLiteral("<callout>");
    }

    QString normalizedXmlStartTag(
        const QString& rawTagText,
        const QString& tagName,
        const bool selfClosing)
    {
        const QString normalizedName = normalizedTagName(tagName);
        if (normalizedName.isEmpty())
        {
            return {};
        }

        QStringList attributes;
        QRegularExpressionMatchIterator iterator = kTagAttributePattern.globalMatch(rawTagText);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            const QString attributeName = match.captured(1).trimmed();
            if (attributeName.isEmpty())
            {
                continue;
            }

            QString value;
            if (match.capturedStart(2) >= 0)
            {
                value = match.captured(2);
            }
            else if (match.capturedStart(3) >= 0)
            {
                value = match.captured(3);
            }
            else if (match.capturedStart(4) >= 0)
            {
                value = match.captured(4);
            }

            attributes.push_back(
                QStringLiteral("%1=\"%2\"")
                    .arg(attributeName, escapeXmlAttributeValue(decodeXmlEntities(value))));
        }

        if (attributes.isEmpty())
        {
            return selfClosing
                ? QStringLiteral("<%1/>").arg(normalizedName)
                : QStringLiteral("<%1>").arg(normalizedName);
        }

        return selfClosing
            ? QStringLiteral("<%1 %2/>").arg(normalizedName, attributes.join(QLatin1Char(' ')))
            : QStringLiteral("<%1 %2>").arg(normalizedName, attributes.join(QLatin1Char(' ')));
    }

    QString normalizeResourceStartTag(const QString& rawTagText)
    {
        return normalizedXmlStartTag(rawTagText, QStringLiteral("resource"), true);
    }

    QString canonicalEmptyTask()
    {
        return QStringLiteral("<task done=\"false\"> </task>");
    }

    QString normalizeLegacyBreakTags(QString text)
    {
        text.replace(
            QRegularExpression(
                QStringLiteral(R"(</\s*hr\s*>)"),
                QRegularExpression::CaseInsensitiveOption),
            QStringLiteral("</break>"));
        text.replace(
            QRegularExpression(
                QStringLiteral(R"(<\s*(?!/)(?:break|hr)\b[^>]*?/?>)"),
                QRegularExpression::CaseInsensitiveOption),
            QStringLiteral("</break>"));
        return text;
    }

    QString normalizeSelfClosingCallouts(QString text)
    {
        QString output;
        int cursor = 0;
        QRegularExpressionMatchIterator iterator = kCalloutSelfClosingPattern.globalMatch(text);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            if (!match.hasMatch())
            {
                continue;
            }

            output += text.mid(cursor, match.capturedStart(0) - cursor);
            output += QStringLiteral("<callout></callout>");
            cursor = match.capturedEnd(0);
        }
        output += text.mid(cursor);
        return output;
    }

    QString normalizeSelfClosingTasks(QString text)
    {
        QString output;
        int cursor = 0;
        QRegularExpressionMatchIterator iterator = kTaskSelfClosingPattern.globalMatch(text);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            if (!match.hasMatch())
            {
                continue;
            }

            output += text.mid(cursor, match.capturedStart(0) - cursor);
            output += canonicalTaskStartTag(match.captured(0));
            output += QLatin1Char(' ');
            output += QStringLiteral("</task>");
            cursor = match.capturedEnd(0);
        }
        output += text.mid(cursor);
        return output;
    }

    QString normalizeSelfClosingAgendas(QString text)
    {
        QString output;
        int cursor = 0;
        QRegularExpressionMatchIterator iterator = kAgendaSelfClosingPattern.globalMatch(text);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            if (!match.hasMatch())
            {
                continue;
            }

            output += text.mid(cursor, match.capturedStart(0) - cursor);
            output += canonicalAgendaStartTag(match.captured(0));
            output += canonicalEmptyTask();
            output += QStringLiteral("</agenda>");
            cursor = match.capturedEnd(0);
        }
        output += text.mid(cursor);
        return output;
    }

    QString normalizeCalloutBlocks(QString text)
    {
        QString output;
        int cursor = 0;
        QRegularExpressionMatchIterator iterator = kCalloutBlockPattern.globalMatch(text);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            if (!match.hasMatch())
            {
                continue;
            }

            output += text.mid(cursor, match.capturedStart(0) - cursor);
            output += canonicalCalloutStartTag();
            output += match.captured(2);
            output += QStringLiteral("</callout>");
            cursor = match.capturedEnd(0);
        }
        output += text.mid(cursor);
        return output;
    }

    QString normalizeAgendaInnerTasks(const QString& innerSourceText)
    {
        QString output;
        int cursor = 0;
        int taskCount = 0;
        QRegularExpressionMatchIterator iterator = kTaskBlockPattern.globalMatch(innerSourceText);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            if (!match.hasMatch())
            {
                continue;
            }

            output += innerSourceText.mid(cursor, match.capturedStart(0) - cursor);
            output += canonicalTaskStartTag(match.captured(0));
            output += match.captured(2);
            output += QStringLiteral("</task>");
            cursor = match.capturedEnd(0);
            ++taskCount;
        }
        output += innerSourceText.mid(cursor);

        if (taskCount > 0)
        {
            return output;
        }

        const QString trimmedInnerSourceText = innerSourceText.trimmed();
        if (trimmedInnerSourceText.isEmpty())
        {
            return canonicalEmptyTask();
        }

        if (trimmedInnerSourceText.contains(QLatin1Char('<'))
            || trimmedInnerSourceText.contains(QLatin1Char('>')))
        {
            return innerSourceText;
        }

        return canonicalTaskStartTag(QStringLiteral("<task>"))
            + innerSourceText
            + QStringLiteral("</task>");
    }

    QString normalizeAgendaBlocks(QString text)
    {
        QString output;
        int cursor = 0;
        QRegularExpressionMatchIterator iterator = kAgendaBlockPattern.globalMatch(text);
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            if (!match.hasMatch())
            {
                continue;
            }

            output += text.mid(cursor, match.capturedStart(0) - cursor);
            output += canonicalAgendaStartTag(match.captured(0));
            output += normalizeAgendaInnerTasks(match.captured(2));
            output += QStringLiteral("</agenda>");
            cursor = match.capturedEnd(0);
        }
        output += text.mid(cursor);
        return output;
    }

    QVariantList mergedIssues(
        const QVariantList& first,
        const QVariantList& second,
        const QVariantList& third = {})
    {
        QVariantList issues = first;
        for (const QVariant& issue : second)
        {
            issues.push_back(issue);
        }
        for (const QVariant& issue : third)
        {
            issues.push_back(issue);
        }
        return issues;
    }

    QVariantMap normalizedVerificationFallback(const QVariantMap& verification, const QString& tagName)
    {
        if (!verification.isEmpty())
        {
            return verification;
        }
        return QVariantMap {
            {QStringLiteral("tagName"), tagName},
            {QStringLiteral("wellFormed"), true},
            {QStringLiteral("sourceLength"), 0},
            {QStringLiteral("issues"), QVariantList()}
        };
    }

    bool isStandaloneStructuredSourceLine(const QString& lineText)
    {
        const QString trimmedLine = normalizePlainText(lineText).trimmed();
        if (trimmedLine.isEmpty())
        {
            return false;
        }

        static const QRegularExpression standaloneStructuredLinePattern(
            QStringLiteral(
                R"(^(?:</break>|<resource\b[^>]*?/?>|<callout\b[^>]*>[\s\S]*</callout>|<agenda\b[^>]*>[\s\S]*</agenda>|<event\b[^>]*>|</event>)$)"),
            QRegularExpression::CaseInsensitiveOption);
        return standaloneStructuredLinePattern.match(trimmedLine).hasMatch();
    }

    QString serializeXmlLintLine(
        const QString& lineText,
        const QStringList& carriedOpenStyleTags,
        QStringList* nextCarriedOpenStyleTags)
    {
        if (nextCarriedOpenStyleTags != nullptr)
        {
            nextCarriedOpenStyleTags->clear();
        }

        if (lineText.isEmpty() && carriedOpenStyleTags.isEmpty())
        {
            return {};
        }

        QString output;
        QStringList openStyleTags = carriedOpenStyleTags;
        for (const QString& openStyleTag : carriedOpenStyleTags)
        {
            output += QStringLiteral("<%1>").arg(openStyleTag);
        }

        qsizetype cursor = 0;
        QRegularExpressionMatchIterator iterator = kXmlLikeTagPattern.globalMatch(lineText);
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
            const QString normalizedName = normalizedTagName(rawTagName);
            const bool closingTag = isClosingTagToken(fullTagToken);
            const bool selfClosingTag = isSelfClosingTagToken(fullTagToken);

            if (SemanticTags::isBreakDividerTagName(rawTagName))
            {
                output += QStringLiteral("<break/>");
                cursor = tagEnd;
                continue;
            }

            if (SemanticTags::isResourceTagName(rawTagName))
            {
                if (!closingTag)
                {
                    output += normalizeResourceStartTag(fullTagToken);
                }
                cursor = tagEnd;
                continue;
            }

            if (SemanticTags::isAgendaTagName(rawTagName))
            {
                if (closingTag)
                {
                    output += QStringLiteral("</agenda>");
                }
                else
                {
                    output += canonicalAgendaStartTag(fullTagToken);
                    if (selfClosingTag)
                    {
                        output += QStringLiteral("</agenda>");
                    }
                }
                cursor = tagEnd;
                continue;
            }

            if (SemanticTags::isTaskTagName(rawTagName))
            {
                if (closingTag)
                {
                    output += QStringLiteral("</task>");
                }
                else
                {
                    output += canonicalTaskStartTag(fullTagToken);
                    if (selfClosingTag)
                    {
                        output += QStringLiteral("</task>");
                    }
                }
                cursor = tagEnd;
                continue;
            }

            if (SemanticTags::isCalloutTagName(rawTagName))
            {
                if (closingTag)
                {
                    output += QStringLiteral("</callout>");
                }
                else
                {
                    output += canonicalCalloutStartTag();
                    if (selfClosingTag)
                    {
                        output += QStringLiteral("</callout>");
                    }
                }
                cursor = tagEnd;
                continue;
            }

            if (normalizedName == QStringLiteral("br"))
            {
                output += QStringLiteral("<br/>");
                cursor = tagEnd;
                continue;
            }

            if (SemanticTags::isSourceSemanticPassThroughTagName(rawTagName)
                || SemanticTags::isSourceProjectionTextBlockElement(rawTagName)
                || SemanticTags::isHashtagTagName(rawTagName))
            {
                const QString canonicalName =
                    SemanticTags::isHashtagTagName(rawTagName)
                    ? QStringLiteral("tag")
                    : normalizedName;
                if (closingTag)
                {
                    output += QStringLiteral("</%1>").arg(canonicalName);
                }
                else if (normalizedName == QStringLiteral("next"))
                {
                    output += QStringLiteral("<next/>");
                }
                else
                {
                    output += normalizedXmlStartTag(fullTagToken, canonicalName, selfClosingTag);
                }
                cursor = tagEnd;
                continue;
            }

            if (SemanticTags::isWebLinkTagName(rawTagName))
            {
                if (closingTag)
                {
                    output += QStringLiteral("</weblink>");
                }
                else
                {
                    output += WebLinks::canonicalStartTagFromRawToken(fullTagToken);
                    if (selfClosingTag)
                    {
                        output += QStringLiteral("</weblink>");
                    }
                }
                cursor = tagEnd;
                continue;
            }

            const QString inlineStyleTag = SemanticTags::canonicalInlineStyleTagName(rawTagName);
            if (!inlineStyleTag.isEmpty())
            {
                if (closingTag)
                {
                    for (int index = openStyleTags.size() - 1; index >= 0; --index)
                    {
                        output += QStringLiteral("</%1>").arg(openStyleTags.at(index));
                        if (openStyleTags.at(index) == inlineStyleTag)
                        {
                            openStyleTags.removeAt(index);
                            break;
                        }
                        openStyleTags.removeAt(index);
                    }
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

        if (nextCarriedOpenStyleTags != nullptr)
        {
            *nextCarriedOpenStyleTags = openStyleTags;
        }

        for (int index = openStyleTags.size() - 1; index >= 0; --index)
        {
            output += QStringLiteral("</%1>").arg(openStyleTags.at(index));
        }

        return output;
    }

    QString xmlLintDocumentFromSourceText(const QString& sourceText)
    {
        const QString normalizedSourceText = normalizePlainText(sourceText);
        QString xmlDocument = QStringLiteral("<contents id=\"lint\">\n<body>\n");
        if (normalizedSourceText.isEmpty())
        {
            xmlDocument += QStringLiteral("</body>\n</contents>");
            return xmlDocument;
        }

        const QStringList lines = normalizedSourceText.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
        QStringList carriedOpenStyleTags;
        for (const QString& line : lines)
        {
            const QString trimmedLine = line.trimmed();
            if (isStandaloneStructuredSourceLine(trimmedLine))
            {
                carriedOpenStyleTags.clear();
                xmlDocument += serializeXmlLintLine(trimmedLine, {}, nullptr);
                xmlDocument += QLatin1Char('\n');
                continue;
            }

            xmlDocument += QStringLiteral("<paragraph>");
            QStringList nextCarriedOpenStyleTags;
            xmlDocument += serializeXmlLintLine(line, carriedOpenStyleTags, &nextCarriedOpenStyleTags);
            carriedOpenStyleTags = nextCarriedOpenStyleTags;
            xmlDocument += QStringLiteral("</paragraph>\n");
        }

        xmlDocument += QStringLiteral("</body>\n</contents>");
        return xmlDocument;
    }

    QVariantMap buildGenericXmlVerification(const QString& sourceText)
    {
        const QString xmlDocument = xmlLintDocumentFromSourceText(sourceText);
        QXmlStreamReader reader(xmlDocument);
        int startElementCount = 0;
        while (!reader.atEnd())
        {
            reader.readNext();
            if (reader.isStartElement())
            {
                ++startElementCount;
            }
        }

        QVariantList issues;
        if (reader.hasError())
        {
            const int xmlLineNumber = static_cast<int>(reader.lineNumber());
            const int xmlColumnNumber = static_cast<int>(reader.columnNumber());
            issues.push_back(issueMap(
                QStringLiteral("body_xml_not_well_formed"),
                reader.errorString().trimmed().isEmpty()
                    ? QStringLiteral("Body semantic/source tags do not form a well-formed XML projection.")
                    : QStringLiteral("Body semantic/source tags do not form a well-formed XML projection: %1")
                          .arg(reader.errorString().trimmed()),
                {
                    {QStringLiteral("columnNumber"), xmlColumnNumber},
                    {QStringLiteral("lineNumber"), xmlLineNumber},
                    {QStringLiteral("sourceLineNumber"), std::max(1, xmlLineNumber - 2)}
                }));
        }

        return QVariantMap {
            {QStringLiteral("tagName"), QStringLiteral("xml")},
            {QStringLiteral("wellFormed"), issues.isEmpty()},
            {QStringLiteral("sourceLength"), sourceText.size()},
            {QStringLiteral("syntheticXmlLength"), xmlDocument.size()},
            {QStringLiteral("startElementCount"), startElementCount},
            {QStringLiteral("issues"), issues}
        };
    }
}

WhatSonStructuredTagLinter::WhatSonStructuredTagLinter() = default;

WhatSonStructuredTagLinter::~WhatSonStructuredTagLinter() = default;

QString WhatSonStructuredTagLinter::normalizeStructuredSourceText(const QString& sourceText) const
{
    QString normalizedSourceText = normalizePlainText(sourceText);
    if (normalizedSourceText.isEmpty())
    {
        return normalizedSourceText;
    }

    normalizedSourceText = normalizeLegacyBreakTags(normalizedSourceText);
    normalizedSourceText = normalizeSelfClosingCallouts(normalizedSourceText);
    normalizedSourceText = normalizeSelfClosingTasks(normalizedSourceText);
    normalizedSourceText = normalizeSelfClosingAgendas(normalizedSourceText);
    normalizedSourceText = normalizeCalloutBlocks(normalizedSourceText);
    normalizedSourceText = normalizeAgendaBlocks(normalizedSourceText);
    return normalizedSourceText;
}

QVariantMap WhatSonStructuredTagLinter::buildBreakVerification(const QString& sourceText) const
{
    const int canonicalBreakCount = regexMatchCount(sourceText, kCanonicalBreakPattern);
    const int legacyBreakCount = regexMatchCount(sourceText, kLegacyBreakPattern);

    QVariantList issues;
    if (legacyBreakCount > 0)
    {
        issues.push_back(issueMap(
            QStringLiteral("break_legacy_variant"),
            QStringLiteral("Break tag uses a legacy or non-canonical variant."),
            {
                {QStringLiteral("legacyBreakCount"), legacyBreakCount}
            }));
    }

    return QVariantMap {
        {QStringLiteral("tagName"), QStringLiteral("break")},
        {QStringLiteral("wellFormed"), issues.isEmpty()},
        {QStringLiteral("sourceLength"), sourceText.size()},
        {QStringLiteral("canonicalBreakCount"), canonicalBreakCount},
        {QStringLiteral("legacyBreakCount"), legacyBreakCount},
        {QStringLiteral("issues"), issues}
    };
}

QVariantMap WhatSonStructuredTagLinter::buildAgendaVerification(
    const QString& sourceText,
    const int parsedAgendaCount,
    const int parsedTaskCount,
    const int invalidAgendaChildCount) const
{
    const int agendaOpenCount = regexMatchCount(sourceText, kAgendaOpenPattern);
    const int agendaCloseCount = regexMatchCount(sourceText, kAgendaClosePattern);
    const int agendaSelfClosingCount = regexMatchCount(sourceText, kAgendaSelfClosingPattern);
    const int taskOpenCount = regexMatchCount(sourceText, kTaskOpenPattern);
    const int taskCloseCount = regexMatchCount(sourceText, kTaskClosePattern);
    const int taskSelfClosingCount = regexMatchCount(sourceText, kTaskSelfClosingPattern);

    int invalidAgendaDateCount = 0;
    QRegularExpressionMatchIterator agendaIterator = kAgendaOpenPattern.globalMatch(sourceText);
    while (agendaIterator.hasNext())
    {
        const QRegularExpressionMatch match = agendaIterator.next();
        if (!match.hasMatch())
        {
            continue;
        }
        if (!hasCanonicalIsoDate(attributeValue(match.captured(0), QStringLiteral("date"))))
        {
            ++invalidAgendaDateCount;
        }
    }

    int missingTaskDoneCount = 0;
    int nonCanonicalTaskDoneCount = 0;
    QRegularExpressionMatchIterator taskIterator = kTaskOpenPattern.globalMatch(sourceText);
    while (taskIterator.hasNext())
    {
        const QRegularExpressionMatch match = taskIterator.next();
        if (!match.hasMatch())
        {
            continue;
        }

        const QString rawDone = attributeValue(match.captured(0), QStringLiteral("done"));
        if (rawDone.isEmpty())
        {
            ++missingTaskDoneCount;
            continue;
        }
        if (!isCanonicalDoneLiteral(rawDone))
        {
            ++nonCanonicalTaskDoneCount;
        }
    }

    QVariantList issues;
    if (agendaOpenCount != agendaCloseCount)
    {
        issues.push_back(issueMap(
            QStringLiteral("agenda_pair_mismatch"),
            QStringLiteral("Agenda open/close tag counts differ.")));
    }
    if (agendaOpenCount != parsedAgendaCount)
    {
        issues.push_back(issueMap(
            QStringLiteral("agenda_parse_mismatch"),
            QStringLiteral("Agenda parser did not confirm every agenda tag pair.")));
    }
    if (taskOpenCount != taskCloseCount)
    {
        issues.push_back(issueMap(
            QStringLiteral("task_pair_mismatch"),
            QStringLiteral("Task open/close tag counts differ.")));
    }
    if (taskOpenCount != parsedTaskCount)
    {
        issues.push_back(issueMap(
            QStringLiteral("task_parse_mismatch"),
            QStringLiteral("Agenda parser did not confirm every task tag pair.")));
    }
    if (invalidAgendaChildCount > 0)
    {
        issues.push_back(issueMap(
            QStringLiteral("agenda_invalid_children"),
            QStringLiteral("Agenda contains non-task child content."),
            {
                {QStringLiteral("invalidAgendaChildCount"), invalidAgendaChildCount}
            }));
    }
    if (invalidAgendaDateCount > 0)
    {
        issues.push_back(issueMap(
            QStringLiteral("agenda_invalid_date"),
            QStringLiteral("Agenda date is missing or not canonical YYYY-MM-DD."),
            {
                {QStringLiteral("invalidAgendaDateCount"), invalidAgendaDateCount}
            }));
    }
    if (agendaSelfClosingCount > 0)
    {
        issues.push_back(issueMap(
            QStringLiteral("agenda_self_closing"),
            QStringLiteral("Self-closing agenda tags must be expanded into explicit agenda/task blocks."),
            {
                {QStringLiteral("agendaSelfClosingCount"), agendaSelfClosingCount}
            }));
    }
    if (taskSelfClosingCount > 0)
    {
        issues.push_back(issueMap(
            QStringLiteral("task_self_closing"),
            QStringLiteral("Self-closing task tags must be expanded into explicit task blocks."),
            {
                {QStringLiteral("taskSelfClosingCount"), taskSelfClosingCount}
            }));
    }
    if (missingTaskDoneCount > 0)
    {
        issues.push_back(issueMap(
            QStringLiteral("task_missing_done"),
            QStringLiteral("Task tag is missing the mandatory done attribute."),
            {
                {QStringLiteral("missingTaskDoneCount"), missingTaskDoneCount}
            }));
    }
    if (nonCanonicalTaskDoneCount > 0)
    {
        issues.push_back(issueMap(
            QStringLiteral("task_noncanonical_done"),
            QStringLiteral("Task done attribute is not canonical true|false."),
            {
                {QStringLiteral("nonCanonicalTaskDoneCount"), nonCanonicalTaskDoneCount}
            }));
    }

    return QVariantMap {
        {QStringLiteral("tagName"), QStringLiteral("agenda")},
        {QStringLiteral("wellFormed"), issues.isEmpty()},
        {QStringLiteral("sourceLength"), sourceText.size()},
        {QStringLiteral("agendaOpenCount"), agendaOpenCount},
        {QStringLiteral("agendaCloseCount"), agendaCloseCount},
        {QStringLiteral("agendaParsedCount"), parsedAgendaCount},
        {QStringLiteral("agendaSelfClosingCount"), agendaSelfClosingCount},
        {QStringLiteral("agendaInvalidDateCount"), invalidAgendaDateCount},
        {QStringLiteral("taskOpenCount"), taskOpenCount},
        {QStringLiteral("taskCloseCount"), taskCloseCount},
        {QStringLiteral("taskParsedCount"), parsedTaskCount},
        {QStringLiteral("taskSelfClosingCount"), taskSelfClosingCount},
        {QStringLiteral("taskMissingDoneCount"), missingTaskDoneCount},
        {QStringLiteral("taskNonCanonicalDoneCount"), nonCanonicalTaskDoneCount},
        {QStringLiteral("invalidAgendaChildCount"), invalidAgendaChildCount},
        {QStringLiteral("issues"), issues}
    };
}

QVariantMap WhatSonStructuredTagLinter::buildCalloutVerification(
    const QString& sourceText,
    const int parsedCalloutCount) const
{
    const int calloutOpenCount = regexMatchCount(sourceText, kCalloutOpenPattern);
    const int calloutCloseCount = regexMatchCount(sourceText, kCalloutClosePattern);
    const int calloutSelfClosingCount = regexMatchCount(sourceText, kCalloutSelfClosingPattern);

    int calloutAttributeCount = 0;
    QRegularExpressionMatchIterator iterator = kCalloutOpenPattern.globalMatch(sourceText);
    while (iterator.hasNext())
    {
        const QRegularExpressionMatch match = iterator.next();
        if (!match.hasMatch())
        {
            continue;
        }
        if (attributeCount(match.captured(0)) > 0)
        {
            ++calloutAttributeCount;
        }
    }

    QVariantList issues;
    if (calloutOpenCount != calloutCloseCount)
    {
        issues.push_back(issueMap(
            QStringLiteral("callout_pair_mismatch"),
            QStringLiteral("Callout open/close tag counts differ.")));
    }
    if (calloutOpenCount != parsedCalloutCount)
    {
        issues.push_back(issueMap(
            QStringLiteral("callout_parse_mismatch"),
            QStringLiteral("Callout parser did not confirm every callout tag pair.")));
    }
    if (calloutSelfClosingCount > 0)
    {
        issues.push_back(issueMap(
            QStringLiteral("callout_self_closing"),
            QStringLiteral("Self-closing callout tags must be expanded into explicit callout blocks."),
            {
                {QStringLiteral("calloutSelfClosingCount"), calloutSelfClosingCount}
            }));
    }
    if (calloutAttributeCount > 0)
    {
        issues.push_back(issueMap(
            QStringLiteral("callout_noncanonical_attributes"),
            QStringLiteral("Callout start tags must not carry custom attributes."),
            {
                {QStringLiteral("calloutAttributeCount"), calloutAttributeCount}
            }));
    }

    return QVariantMap {
        {QStringLiteral("tagName"), QStringLiteral("callout")},
        {QStringLiteral("wellFormed"), issues.isEmpty()},
        {QStringLiteral("sourceLength"), sourceText.size()},
        {QStringLiteral("calloutOpenCount"), calloutOpenCount},
        {QStringLiteral("calloutCloseCount"), calloutCloseCount},
        {QStringLiteral("calloutParsedCount"), parsedCalloutCount},
        {QStringLiteral("calloutSelfClosingCount"), calloutSelfClosingCount},
        {QStringLiteral("calloutAttributeCount"), calloutAttributeCount},
        {QStringLiteral("issues"), issues}
    };
}

QVariantMap WhatSonStructuredTagLinter::buildStructuredVerification(
    const QVariantMap& agendaVerification,
    const QVariantMap& calloutVerification,
    const QString& sourceText) const
{
    const QVariantMap normalizedAgendaVerification =
        normalizedVerificationFallback(agendaVerification, QStringLiteral("agenda"));
    const QVariantMap normalizedCalloutVerification =
        normalizedVerificationFallback(calloutVerification, QStringLiteral("callout"));
    const QVariantMap breakVerification = buildBreakVerification(sourceText);
    const QString normalizedSourceText = normalizeStructuredSourceText(sourceText);
    const QVariantMap xmlVerification = buildGenericXmlVerification(normalizedSourceText);
    const QVariantList issues = mergedIssues(
        normalizedAgendaVerification.value(QStringLiteral("issues")).toList(),
        normalizedCalloutVerification.value(QStringLiteral("issues")).toList(),
        mergedIssues(
            breakVerification.value(QStringLiteral("issues")).toList(),
            xmlVerification.value(QStringLiteral("issues")).toList()));
    return QVariantMap {
        {QStringLiteral("agenda"), normalizedAgendaVerification},
        {QStringLiteral("callout"), normalizedCalloutVerification},
        {QStringLiteral("break"), breakVerification},
        {QStringLiteral("xml"), xmlVerification},
        {QStringLiteral("sourceLength"), sourceText.size()},
        {QStringLiteral("canonicalizationSuggested"), normalizedSourceText != normalizePlainText(sourceText)},
        {QStringLiteral("wellFormed"),
         normalizedAgendaVerification.value(QStringLiteral("wellFormed")).toBool()
             && normalizedCalloutVerification.value(QStringLiteral("wellFormed")).toBool()
             && breakVerification.value(QStringLiteral("wellFormed")).toBool()
             && xmlVerification.value(QStringLiteral("wellFormed")).toBool()},
        {QStringLiteral("issues"), issues}
    };
}
