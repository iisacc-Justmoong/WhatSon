#include "app/models/editor/tags/WhatSonStructuredTagLinter.hpp"
#include "app/models/file/note/WhatSonNoteBodyWebLinkSupport.hpp"
#include "app/models/file/note/WhatSonNoteBodySemanticTagSupport.hpp"

#include <QRegularExpression>
#include <QStringList>
#include <QVariantList>
#include <QXmlStreamReader>

namespace
{
    namespace SemanticTags = WhatSon::NoteBodySemanticTagSupport;
    namespace WebLinks = WhatSon::NoteBodyWebLinkSupport;

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

    QVariantList mergedIssues(
        const QVariantList& first,
        const QVariantList& second)
    {
        QVariantList issues = first;
        for (const QVariant& issue : second)
        {
            issues.push_back(issue);
        }
        return issues;
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
                R"(^(?:</break>|<resource\b[^>]*?/?>|<event\b[^>]*>|</event>)$)"),
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

QVariantMap WhatSonStructuredTagLinter::buildStructuredVerification(const QString& sourceText) const
{
    const QVariantMap breakVerification = buildBreakVerification(sourceText);
    const QString normalizedSourceText = normalizeStructuredSourceText(sourceText);
    const QVariantMap xmlVerification = buildGenericXmlVerification(normalizedSourceText);
    const QVariantList issues = mergedIssues(
        breakVerification.value(QStringLiteral("issues")).toList(),
        xmlVerification.value(QStringLiteral("issues")).toList());
    return QVariantMap {
        {QStringLiteral("break"), breakVerification},
        {QStringLiteral("xml"), xmlVerification},
        {QStringLiteral("sourceLength"), sourceText.size()},
        {QStringLiteral("canonicalizationSuggested"), normalizedSourceText != normalizePlainText(sourceText)},
        {QStringLiteral("wellFormed"),
         breakVerification.value(QStringLiteral("wellFormed")).toBool()
             && xmlVerification.value(QStringLiteral("wellFormed")).toBool()},
        {QStringLiteral("issues"), issues}
    };
}
