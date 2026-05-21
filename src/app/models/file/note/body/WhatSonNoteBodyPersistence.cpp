#include "app/models/file/note/body/WhatSonNoteBodyPersistence.hpp"

#include "app/models/file/note/support/WhatSonIiXmlDocumentSupport.hpp"
#include "app/models/editor/component/Break.h"
#include "app/models/editor/component/Callout.h"
#include "app/models/file/note/body/WhatSonNoteBodySemanticTagSupport.hpp"
#include "app/models/file/note/body/WhatSonNoteBodyWebLinkSupport.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/models/file/note/local/WhatSonLocalNoteFileStore.hpp"

#include <QByteArray>
#include <QBrush>
#include <QColor>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QRegularExpression>
#include <QSet>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextFragment>
#include <QVector>

#include <algorithm>
#include <cmath>
#include <utility>

namespace
{
    namespace IiXml = WhatSon::IiXmlDocumentSupport;
    namespace SemanticTags = WhatSon::NoteBodySemanticTagSupport;

    QString renderInlineSourceToHtml(const QString& sourceFragment, int editorViewportWidth = 0);
    QString sourceTextFromRichEditorDocument(const QString& editorDocumentText);
    QString wrapSourceTextWithInlineTags(const QString& text, const QStringList& tags);

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

    QString escapeHtmlAttribute(QString value)
    {
        value.replace(QStringLiteral("&"), QStringLiteral("&amp;"));
        value.replace(QStringLiteral("<"), QStringLiteral("&lt;"));
        value.replace(QStringLiteral(">"), QStringLiteral("&gt;"));
        value.replace(QStringLiteral("\""), QStringLiteral("&quot;"));
        value.replace(QStringLiteral("'"), QStringLiteral("&#39;"));
        return value;
    }

    QString sanitizedCssValue(QString value)
    {
        value = value.trimmed();
        value.remove(QLatin1Char(';'));
        value.remove(QLatin1Char('<'));
        value.remove(QLatin1Char('>'));
        value.remove(QLatin1Char('{'));
        value.remove(QLatin1Char('}'));
        return value.simplified();
    }

    QString attributeValueFromRawToken(const QString& rawTagText, const QString& attributeName)
    {
        const QRegularExpression attributePattern(
            QStringLiteral("\\b%1\\s*=\\s*(?:\"([^\"]*)\"|'([^']*)'|([^\\s>/]+))")
                .arg(QRegularExpression::escape(attributeName)),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch match = attributePattern.match(rawTagText);
        if (!match.hasMatch())
        {
            return {};
        }

        for (int captureIndex = 1; captureIndex <= 3; ++captureIndex)
        {
            if (match.capturedStart(captureIndex) >= 0)
            {
                return IiXml::decodeXmlEntities(match.captured(captureIndex)).trimmed();
            }
        }
        return {};
    }

    QString integerCssValue(const QString& value)
    {
        bool ok = false;
        const int number = value.trimmed().toInt(&ok);
        return ok && number > 0 ? QString::number(number) : QString();
    }

    QString floatingCssValue(const QString& value)
    {
        bool ok = false;
        const double number = value.trimmed().toDouble(&ok);
        if (!ok || !std::isfinite(number) || number <= 0.0)
        {
            return {};
        }
        return QString::number(number, 'g', 16);
    }

    struct LvrsTextStyleToken final
    {
        bool valid = false;
        QString name;
        int pixelSize = 0;
        int weight = QFont::Normal;
        QString styleName;
        int lineHeight = 0;
        QString color;
    };

    LvrsTextStyleToken lvrsTextStyleTokenFromName(QString tokenName)
    {
        tokenName = tokenName.trimmed().toCaseFolded();
        tokenName.remove(QLatin1Char('-'));
        tokenName.remove(QLatin1Char('_'));
        tokenName.remove(QLatin1Char(' '));

        if (tokenName == QStringLiteral("title"))
        {
            return {true, QStringLiteral("title"), 26, QFont::Bold, QStringLiteral("Bold"), 26, QStringLiteral("#E5FFFFFF")};
        }
        if (tokenName == QStringLiteral("title2"))
        {
            return {true, QStringLiteral("title2"), 22, QFont::Bold, QStringLiteral("Bold"), 22, QStringLiteral("#E5FFFFFF")};
        }
        if (tokenName == QStringLiteral("header"))
        {
            return {true, QStringLiteral("header"), 17, QFont::DemiBold, QStringLiteral("SemiBold"), 17, QStringLiteral("#E5FFFFFF")};
        }
        if (tokenName == QStringLiteral("header2"))
        {
            return {true, QStringLiteral("header2"), 15, QFont::DemiBold, QStringLiteral("SemiBold"), 15, QStringLiteral("#E5FFFFFF")};
        }
        if (tokenName == QStringLiteral("body"))
        {
            return {true, QStringLiteral("body"), 12, QFont::Medium, QStringLiteral("Medium"), 12, QStringLiteral("#CCFFFFFF")};
        }
        if (tokenName == QStringLiteral("description"))
        {
            return {true, QStringLiteral("description"), 12, QFont::DemiBold, QStringLiteral("SemiBold"), 12, QStringLiteral("#99FFFFFF")};
        }
        if (tokenName == QStringLiteral("caption"))
        {
            return {true, QStringLiteral("caption"), 11, QFont::Normal, QStringLiteral("Regular"), 11, QStringLiteral("#80FFFFFF")};
        }
        if (tokenName == QStringLiteral("disabled"))
        {
            return {true, QStringLiteral("disabled"), 11, QFont::Normal, QStringLiteral("Regular"), 11, QStringLiteral("#4DFFFFFF")};
        }
        return {};
    }

    QString lvrsDefaultEditorFontFamily()
    {
        return QStringLiteral("Pretendard");
    }

    QString lvrsBodyEditorCssDeclaration()
    {
        const LvrsTextStyleToken bodyToken = lvrsTextStyleTokenFromName(QStringLiteral("body"));
        return QStringLiteral("font-family:%1;font-size:%2px;font-weight:%3;line-height:%4px;color:%5;")
            .arg(
                lvrsDefaultEditorFontFamily(),
                QString::number(bodyToken.pixelSize),
                QString::number(bodyToken.weight),
                QString::number(bodyToken.lineHeight),
                bodyToken.color);
    }

    QString editorHtmlDocumentFromProjection(const QString& bodyHtml)
    {
        return QStringLiteral(
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" "
            "\"http://www.w3.org/TR/REC-html40/strict.dtd\">"
            "<html><head><meta name=\"qrichtext\" content=\"1\" />"
            "<meta charset=\"utf-8\" /></head>"
            "<body style=\"%1\">%2</body></html>")
            .arg(lvrsBodyEditorCssDeclaration(), bodyHtml);
    }

    QString styleCssDeclarationFromRawToken(const QString& rawTagText)
    {
        QStringList declarations;
        const LvrsTextStyleToken styleToken =
            lvrsTextStyleTokenFromName(attributeValueFromRawToken(rawTagText, QStringLiteral("style")));
        if (styleToken.valid)
        {
            declarations.push_back(QStringLiteral("font-family:%1;").arg(lvrsDefaultEditorFontFamily()));
            declarations.push_back(QStringLiteral("font-size:%1px;").arg(styleToken.pixelSize));
            declarations.push_back(QStringLiteral("font-weight:%1;").arg(styleToken.weight));
            declarations.push_back(QStringLiteral("line-height:%1px;").arg(styleToken.lineHeight));
            declarations.push_back(QStringLiteral("color:%1;").arg(styleToken.color));
        }

        const auto appendStringDeclaration =
            [&](const QString& attributeName, const QString& cssPropertyName)
        {
            const QString value = sanitizedCssValue(attributeValueFromRawToken(rawTagText, attributeName));
            if (!value.isEmpty())
            {
                declarations.push_back(QStringLiteral("%1:%2;").arg(cssPropertyName, value));
            }
        };

        appendStringDeclaration(QStringLiteral("font"), QStringLiteral("font-family"));
        appendStringDeclaration(QStringLiteral("weight"), QStringLiteral("font-weight"));

        const QString size = integerCssValue(attributeValueFromRawToken(rawTagText, QStringLiteral("size")));
        if (!size.isEmpty())
        {
            declarations.push_back(QStringLiteral("font-size:%1px;").arg(size));
        }

        appendStringDeclaration(QStringLiteral("color"), QStringLiteral("color"));
        appendStringDeclaration(QStringLiteral("background"), QStringLiteral("background-color"));
        appendStringDeclaration(QStringLiteral("align"), QStringLiteral("text-align"));

        const QString height = floatingCssValue(attributeValueFromRawToken(rawTagText, QStringLiteral("height")));
        if (!height.isEmpty())
        {
            declarations.push_back(QStringLiteral("line-height:%1;").arg(height));
        }

        return declarations.join(QString());
    }

    QString styleOpeningHtmlFromRawToken(const QString& rawTagText)
    {
        const QString encodedSource = QString::fromLatin1(rawTagText.toUtf8().toHex());
        const QString marker = QStringLiteral("<!--whatson-style-source:%1-->"
                                              "<a name=\"whatson-style-source:%1\"></a>")
            .arg(encodedSource);
        const QString cssDeclaration = styleCssDeclarationFromRawToken(rawTagText);
        if (cssDeclaration.isEmpty())
        {
            return marker + QStringLiteral("<span>");
        }
        return marker
            + QStringLiteral("<span style=\"%1\">").arg(escapeHtmlAttribute(cssDeclaration));
    }

    QString styleClosingHtml()
    {
        return QStringLiteral("</span><a name=\"whatson-style-source-end\"></a><!--/whatson-style-source-->");
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
            || SemanticTags::isStyleTagName(tagName)
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

    bool isCalloutTagName(const QString& tagName)
    {
        return tagName.trimmed().toCaseFolded() == QStringLiteral("callout");
    }

    struct PairedCalloutRange final
    {
        int contentStart = -1;
        int contentEnd = -1;
        int closingEnd = -1;

        bool isValid() const noexcept
        {
            return contentStart >= 0 && contentEnd >= contentStart && closingEnd >= contentEnd;
        }
    };

    PairedCalloutRange pairedCalloutRangeFromOpening(
        const QString& sourceFragment,
        const int openingTagEnd)
    {
        int depth = 1;
        int cursor = openingTagEnd + 1;
        while (cursor < sourceFragment.size())
        {
            const int tagStart = sourceFragment.indexOf(QLatin1Char('<'), cursor);
            if (tagStart < 0)
            {
                break;
            }

            const int tagEnd = sourceFragment.indexOf(QLatin1Char('>'), tagStart + 1);
            if (tagEnd <= tagStart)
            {
                break;
            }

            const QStringView tagToken(sourceFragment.constData() + tagStart, tagEnd - tagStart + 1);
            const QString tagName = sourceTagName(tagToken);
            if (isCalloutTagName(tagName))
            {
                if (isClosingTagToken(tagToken))
                {
                    --depth;
                    if (depth == 0)
                    {
                        return {openingTagEnd + 1, tagStart, tagEnd + 1};
                    }
                }
                else if (!isSelfClosingTagToken(tagToken))
                {
                    ++depth;
                }
            }

            cursor = tagEnd + 1;
        }

        return {};
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
        if (SemanticTags::isStyleTagName(tagName))
        {
            return styleOpeningHtmlFromRawToken(tagToken.toString());
        }
        if (SemanticTags::isHashtagTagName(tagName))
        {
            return QStringLiteral("#");
        }
        if (SemanticTags::isTransparentContainerTagName(tagName))
        {
            return {};
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
        if (SemanticTags::isStyleTagName(tagName))
        {
            return styleClosingHtml();
        }
        if (SemanticTags::isHashtagTagName(tagName))
        {
            return {};
        }
        if (SemanticTags::isTransparentContainerTagName(tagName))
        {
            return {};
        }
        return SemanticTags::semanticTextClosingHtml(tagName);
    }

    QString renderInlineSourceToHtml(const QString& sourceFragment, const int editorViewportWidth)
    {
        if (WhatSon::EditorComponent::Break::isSourceLine(sourceFragment))
        {
            return WhatSon::EditorComponent::Break::renderHtml();
        }

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
                    if (isCalloutTagName(tagName)
                        && !isClosingTagToken(tagToken)
                        && !isSelfClosingTagToken(tagToken))
                    {
                        const PairedCalloutRange calloutRange =
                            pairedCalloutRangeFromOpening(sourceFragment, tagEnd);
                        if (calloutRange.isValid())
                        {
                            const QString calloutSource =
                                sourceFragment.mid(cursor, calloutRange.closingEnd - cursor);
                            const QString calloutContentSource = sourceFragment.mid(
                                calloutRange.contentStart,
                                calloutRange.contentEnd - calloutRange.contentStart);
                            rendered += WhatSon::EditorComponent::Callout::renderHtml({
                                calloutSource,
                                renderInlineSourceToHtml(calloutContentSource, editorViewportWidth),
                                editorViewportWidth
                            });
                            cursor = calloutRange.closingEnd;
                            continue;
                        }
                    }
                    const bool recognizedTag = !SemanticTags::canonicalInlineStyleTagName(tagName).isEmpty()
                        || SemanticTags::isWebLinkTagName(tagName)
                        || SemanticTags::isStyleTagName(tagName)
                        || SemanticTags::isHashtagTagName(tagName)
                        || SemanticTags::isTransparentContainerTagName(tagName)
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
        if (WhatSon::EditorComponent::Break::isSourceLine(sourceFragment))
        {
            return {};
        }

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
                        || SemanticTags::isStyleTagName(tagName)
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
            return WhatSon::EditorComponent::Break::sourceToken();
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

            if (WhatSon::EditorComponent::Break::isSourceLine(line))
            {
                rawLines.push_back(WhatSon::EditorComponent::Break::sourceToken());
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

    struct RenderedResourceSourceToken final
    {
        QString token;
        QString sourceTag;
    };

    struct RenderedCalloutSourceToken final
    {
        QString token;
        QString sourceText;
    };

    struct RenderedStyleSourceToken final
    {
        QString token;
        QString sourceText;
    };

    QString styleMarkerContentHtml(QString markerBody)
    {
        static const QRegularExpression startAnchorPattern(
            QStringLiteral(
                R"(<a\b(?=[^>]*\bname\s*=\s*["']whatson-style-source:[0-9a-fA-F]*["'])[^>]*>[^<]*</a>)"),
            QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression endAnchorPattern(
            QStringLiteral(
                R"(<a\b(?=[^>]*\bname\s*=\s*["']whatson-style-source-end["'])[^>]*>[^<]*</a>)"),
            QRegularExpression::CaseInsensitiveOption);
        markerBody.remove(startAnchorPattern);
        markerBody.remove(endAnchorPattern);
        markerBody.remove(QChar(0x200B));
        return markerBody;
    }

    struct StyleSourceBaseline final
    {
        bool italic = false;
        int weight = QFont::Normal;
        QString background;
    };

    int styleWeightValue(const QString& value)
    {
        bool ok = false;
        const int numericWeight = value.trimmed().toInt(&ok);
        if (ok)
        {
            return numericWeight;
        }

        const QString normalized = value.trimmed().toCaseFolded();
        if (normalized == QStringLiteral("bold"))
        {
            return QFont::Black;
        }
        if (normalized == QStringLiteral("normal") || normalized.isEmpty())
        {
            return QFont::Normal;
        }
        return QFont::Normal;
    }

    QColor colorFromCssValue(QString value)
    {
        value = value.trimmed();
        const QColor namedColor(value);
        if (namedColor.isValid())
        {
            return namedColor;
        }

        static const QRegularExpression rgbaPattern(
            QStringLiteral(
                R"(^rgba?\s*\(\s*(\d{1,3})\s*,\s*(\d{1,3})\s*,\s*(\d{1,3})(?:\s*,\s*([0-9]*\.?[0-9]+))?\s*\)$)"),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch match = rgbaPattern.match(value);
        if (!match.hasMatch())
        {
            return {};
        }

        const int red = std::clamp(match.captured(1).toInt(), 0, 255);
        const int green = std::clamp(match.captured(2).toInt(), 0, 255);
        const int blue = std::clamp(match.captured(3).toInt(), 0, 255);
        QColor color(red, green, blue);
        if (match.capturedStart(4) >= 0)
        {
            bool ok = false;
            const double alpha = match.captured(4).toDouble(&ok);
            if (ok)
            {
                color.setAlphaF(std::clamp(alpha, 0.0, 1.0));
            }
        }
        return color;
    }

    QString normalizedColorName(const QString& value)
    {
        const QColor color = colorFromCssValue(value);
        if (!color.isValid())
        {
            return value.trimmed().toCaseFolded();
        }
        return color.alpha() < 255
            ? color.name(QColor::HexArgb).toCaseFolded()
            : color.name(QColor::HexRgb).toCaseFolded();
    }

    QString normalizedCssIdentifier(QString value)
    {
        value = value.trimmed();
        if ((value.startsWith(QLatin1Char('\'')) && value.endsWith(QLatin1Char('\'')))
            || (value.startsWith(QLatin1Char('"')) && value.endsWith(QLatin1Char('"'))))
        {
            value = value.mid(1, value.size() - 2);
        }
        return value.simplified().toCaseFolded();
    }

    QString cssDeclarationValue(const QString& declaration, const QString& propertyName)
    {
        QString resolvedValue;
        const QStringList parts = declaration.split(QLatin1Char(';'), Qt::SkipEmptyParts);
        for (const QString& part : parts)
        {
            const int separator = part.indexOf(QLatin1Char(':'));
            if (separator < 0)
            {
                continue;
            }

            const QString name = part.left(separator).trimmed();
            if (name.compare(propertyName, Qt::CaseInsensitive) == 0)
            {
                resolvedValue = IiXml::decodeXmlEntities(part.mid(separator + 1).trimmed());
            }
        }
        return resolvedValue;
    }

    double cssNumericValue(QString value, bool* ok)
    {
        value = value.trimmed().toCaseFolded();
        static const QRegularExpression numericPattern(
            QStringLiteral(R"(^\s*([+-]?(?:\d+(?:\.\d*)?|\.\d+))\s*(?:px|pt|em|rem|%)?\s*$)"));
        const QRegularExpressionMatch match = numericPattern.match(value);
        if (!match.hasMatch())
        {
            if (ok != nullptr)
            {
                *ok = false;
            }
            return 0.0;
        }
        return match.captured(1).toDouble(ok);
    }

    bool cssNumericValuesMatch(const QString& left, const QString& right)
    {
        bool leftOk = false;
        bool rightOk = false;
        const double leftValue = cssNumericValue(left, &leftOk);
        const double rightValue = cssNumericValue(right, &rightOk);
        return leftOk && rightOk && std::abs(leftValue - rightValue) < 0.001;
    }

    bool styleSpanMatchesOpeningToken(const QString& spanOpeningTag, const QString& openingToken)
    {
        const QString cssDeclaration =
            attributeValueFromRawToken(spanOpeningTag, QStringLiteral("style"));
        if (cssDeclaration.isEmpty())
        {
            return false;
        }

        const QString expectedDeclaration = styleCssDeclarationFromRawToken(openingToken);
        if (expectedDeclaration.isEmpty())
        {
            return false;
        }

        int matchedProperties = 0;
        const auto matchIdentifier =
            [&](const QString& cssPropertyName, bool requiredWhenMissing) -> bool
        {
            const QString expected = cssDeclarationValue(expectedDeclaration, cssPropertyName);
            if (expected.isEmpty())
            {
                return true;
            }

            const QString actual = cssDeclarationValue(cssDeclaration, cssPropertyName);
            if (actual.isEmpty())
            {
                return !requiredWhenMissing;
            }

            ++matchedProperties;
            return normalizedCssIdentifier(actual) == normalizedCssIdentifier(expected);
        };
        const auto matchColor =
            [&](const QString& cssPropertyName, bool requiredWhenMissing) -> bool
        {
            const QString expected = cssDeclarationValue(expectedDeclaration, cssPropertyName);
            if (expected.isEmpty())
            {
                return true;
            }

            const QString actual = cssDeclarationValue(cssDeclaration, cssPropertyName);
            if (actual.isEmpty())
            {
                return !requiredWhenMissing;
            }

            ++matchedProperties;
            const QColor actualColor = colorFromCssValue(actual);
            const QColor expectedColor = colorFromCssValue(expected);
            if (actualColor.isValid() && expectedColor.isValid())
            {
                return actualColor.rgb() == expectedColor.rgb()
                    && (actualColor.alpha() == expectedColor.alpha()
                        || actualColor.alpha() == 255
                        || expectedColor.alpha() == 255);
            }
            return normalizedColorName(actual) == normalizedColorName(expected);
        };
        const auto matchNumber =
            [&](const QString& cssPropertyName, bool requiredWhenMissing) -> bool
        {
            const QString expected = cssDeclarationValue(expectedDeclaration, cssPropertyName);
            if (expected.isEmpty())
            {
                return true;
            }

            const QString actual = cssDeclarationValue(cssDeclaration, cssPropertyName);
            if (actual.isEmpty())
            {
                return !requiredWhenMissing;
            }

            ++matchedProperties;
            return cssNumericValuesMatch(actual, expected);
        };
        const auto matchWeight =
            [&](bool requiredWhenMissing) -> bool
        {
            const QString expected = cssDeclarationValue(expectedDeclaration, QStringLiteral("font-weight"));
            if (expected.isEmpty())
            {
                return true;
            }

            const QString actualWeight = cssDeclarationValue(cssDeclaration, QStringLiteral("font-weight"));
            if (actualWeight.isEmpty())
            {
                return !requiredWhenMissing || styleWeightValue(expected) == QFont::Normal;
            }

            ++matchedProperties;
            return styleWeightValue(actualWeight) >= styleWeightValue(expected);
        };

        return matchIdentifier(QStringLiteral("font-family"), true)
            && matchNumber(QStringLiteral("font-size"), true)
            && matchWeight(true)
            && matchColor(QStringLiteral("color"), true)
            && matchColor(QStringLiteral("background-color"), true)
            && matchIdentifier(QStringLiteral("text-align"), false)
            && matchNumber(QStringLiteral("line-height"), false)
            && matchedProperties > 0;
    }

    StyleSourceBaseline styleSourceBaselineFromOpeningToken(const QString& openingToken)
    {
        const QString styleValue = attributeValueFromRawToken(openingToken, QStringLiteral("style"));
        const LvrsTextStyleToken styleToken = lvrsTextStyleTokenFromName(styleValue);
        const QString explicitWeight = attributeValueFromRawToken(openingToken, QStringLiteral("weight"));
        return {
            false,
            explicitWeight.isEmpty()
                ? (styleToken.valid ? styleToken.weight : QFont::Normal)
                : styleWeightValue(explicitWeight),
            normalizedColorName(attributeValueFromRawToken(openingToken, QStringLiteral("background")))
        };
    }

    QStringList sourceInlineTagsForStyleMarkerFormat(
        const QTextCharFormat& format,
        const StyleSourceBaseline& baseline)
    {
        QStringList tags;
        if (format.fontWeight() >= QFont::Black && baseline.weight < QFont::Black)
        {
            tags.push_back(QStringLiteral("bold"));
        }
        if (format.fontItalic() && !baseline.italic)
        {
            tags.push_back(QStringLiteral("italic"));
        }
        if (format.fontUnderline())
        {
            tags.push_back(QStringLiteral("underline"));
        }
        if (format.fontStrikeOut())
        {
            tags.push_back(QStringLiteral("strikethrough"));
        }

        const QBrush background = format.background();
        if (background.style() != Qt::NoBrush)
        {
            const QString backgroundName = background.color().name(QColor::HexRgb).toCaseFolded();
            if (backgroundName == QStringLiteral("#8a4b00")
                && baseline.background != QStringLiteral("#8a4b00"))
            {
                tags.push_back(QStringLiteral("highlight"));
            }
        }
        return tags;
    }

    QString sourceTextFromStyleMarkerHtml(const QString& markerBody, const QString& openingToken)
    {
        const QString contentHtml = styleMarkerContentHtml(markerBody);
        if (contentHtml.trimmed().isEmpty())
        {
            return {};
        }

        QTextDocument document;
        document.setHtml(contentHtml);
        const StyleSourceBaseline baseline = styleSourceBaselineFromOpeningToken(openingToken);

        QStringList sourceLines;
        for (QTextBlock block = document.begin(); block.isValid(); block = block.next())
        {
            QString sourceLine;
            for (QTextBlock::iterator fragmentIt = block.begin(); !fragmentIt.atEnd(); ++fragmentIt)
            {
                const QTextFragment fragment = fragmentIt.fragment();
                if (!fragment.isValid())
                {
                    continue;
                }

                QString fragmentText =
                    WhatSon::NoteBodyPersistence::normalizeBodyPlainText(fragment.text());
                fragmentText.remove(QChar(0x200B));
                if (fragmentText.isEmpty())
                {
                    continue;
                }

                sourceLine += wrapSourceTextWithInlineTags(
                    fragmentText,
                    sourceInlineTagsForStyleMarkerFormat(fragment.charFormat(), baseline));
            }
            sourceLines.push_back(sourceLine);
        }

        if (sourceLines.isEmpty())
        {
            QString plainText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(document.toPlainText());
            plainText.remove(QChar(0x200B));
            return plainText;
        }
        return WhatSon::NoteBodyPersistence::normalizeBodyPlainText(sourceLines.join(QLatin1Char('\n')));
    }

    void replaceStyleMarkersWithSourceTokens(
        QString* editorHtml,
        const QRegularExpression& markerPattern,
        QVector<RenderedStyleSourceToken>* tokens)
    {
        if (editorHtml == nullptr || tokens == nullptr || editorHtml->isEmpty())
        {
            return;
        }

        QString rewrittenHtml;
        rewrittenHtml.reserve(editorHtml->size());

        int lastOffset = 0;
        QRegularExpressionMatchIterator matchIterator = markerPattern.globalMatch(*editorHtml);
        while (matchIterator.hasNext())
        {
            const QRegularExpressionMatch match = matchIterator.next();
            rewrittenHtml += editorHtml->mid(lastOffset, match.capturedStart(0) - lastOffset);

            const QString originalOpening =
                QString::fromUtf8(QByteArray::fromHex(match.captured(1).toLatin1())).trimmed();
            if (originalOpening.isEmpty())
            {
                rewrittenHtml += match.captured(0);
                lastOffset = match.capturedEnd(0);
                continue;
            }

            const QString contentSource =
                sourceTextFromStyleMarkerHtml(match.captured(2), originalOpening);
            const QString sourceText =
                originalOpening
                + contentSource
                + QStringLiteral("</style>");
            const QString token = QStringLiteral("__WHATSON_STYLE_SOURCE_TOKEN_%1__").arg(tokens->size());
            tokens->push_back({token, sourceText});
            rewrittenHtml += token;

            lastOffset = match.capturedEnd(0);
        }

        rewrittenHtml += editorHtml->mid(lastOffset);
        *editorHtml = rewrittenHtml;
    }

    int skipSourceWhitespace(const QString& text, int offset)
    {
        while (offset < text.size() && text.at(offset).isSpace())
        {
            ++offset;
        }
        return offset;
    }

    void replaceStartAnchoredStyleSpansWithSourceTokens(
        QString* editorHtml,
        QVector<RenderedStyleSourceToken>* tokens)
    {
        if (editorHtml == nullptr || tokens == nullptr || editorHtml->isEmpty())
        {
            return;
        }

        static const QRegularExpression startAnchorPattern(
            QStringLiteral(
                R"(<a\b(?=[^>]*\bname\s*=\s*["']whatson-style-source:([0-9a-fA-F]*)["'])[^>]*>[^<]*</a>)"),
            QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression endAnchorPattern(
            QStringLiteral(
                R"(<a\b(?=[^>]*\bname\s*=\s*["']whatson-style-source-end["'])[^>]*>[^<]*</a>)"),
            QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression spanPattern(
            QStringLiteral(R"((<span\b[^>]*>)([\s\S]*?)</span>)"),
            QRegularExpression::CaseInsensitiveOption);

        QString rewrittenHtml;
        rewrittenHtml.reserve(editorHtml->size());

        int lastOffset = 0;
        QRegularExpressionMatch match = startAnchorPattern.match(*editorHtml, lastOffset);
        while (match.hasMatch())
        {
            rewrittenHtml += editorHtml->mid(lastOffset, match.capturedStart(0) - lastOffset);

            const QString originalOpening =
                QString::fromUtf8(QByteArray::fromHex(match.captured(1).toLatin1())).trimmed();
            if (originalOpening.isEmpty())
            {
                rewrittenHtml += match.captured(0);
                lastOffset = match.capturedEnd(0);
                match = startAnchorPattern.match(*editorHtml, lastOffset);
                continue;
            }

            QString contentHtml;
            int cursor = match.capturedEnd(0);
            int consumedEnd = cursor;
            while (cursor < editorHtml->size())
            {
                const int nextTokenOffset = skipSourceWhitespace(*editorHtml, cursor);
                const QRegularExpressionMatch endAnchorMatch =
                    endAnchorPattern.match(*editorHtml, nextTokenOffset);
                if (endAnchorMatch.hasMatch()
                    && endAnchorMatch.capturedStart(0) == nextTokenOffset)
                {
                    consumedEnd = endAnchorMatch.capturedEnd(0);
                    cursor = consumedEnd;
                    break;
                }

                const QRegularExpressionMatch spanMatch =
                    spanPattern.match(*editorHtml, nextTokenOffset);
                if (!spanMatch.hasMatch()
                    || spanMatch.capturedStart(0) != nextTokenOffset
                    || !styleSpanMatchesOpeningToken(spanMatch.captured(1), originalOpening))
                {
                    break;
                }

                contentHtml += spanMatch.captured(0);
                consumedEnd = spanMatch.capturedEnd(0);
                cursor = consumedEnd;
            }

            if (contentHtml.isEmpty())
            {
                rewrittenHtml += match.captured(0);
                lastOffset = match.capturedEnd(0);
                match = startAnchorPattern.match(*editorHtml, lastOffset);
                continue;
            }

            const QString sourceText =
                originalOpening
                + sourceTextFromStyleMarkerHtml(contentHtml, originalOpening)
                + QStringLiteral("</style>");
            const QString token = QStringLiteral("__WHATSON_STYLE_SOURCE_TOKEN_%1__").arg(tokens->size());
            tokens->push_back({token, sourceText});
            rewrittenHtml += token;

            lastOffset = consumedEnd;
            match = startAnchorPattern.match(*editorHtml, lastOffset);
        }

        rewrittenHtml += editorHtml->mid(lastOffset);
        *editorHtml = rewrittenHtml;
    }

    QVector<RenderedStyleSourceToken> replaceRenderedStyleSpansWithSourceTokens(QString* editorHtml)
    {
        QVector<RenderedStyleSourceToken> tokens;
        if (editorHtml == nullptr || editorHtml->isEmpty())
        {
            return tokens;
        }

        static const QRegularExpression commentMarkerPattern(
            QStringLiteral(
                R"(<!--whatson-style-source:([0-9a-fA-F]*)-->([\s\S]*?)<!--\/whatson-style-source-->)"));
        replaceStyleMarkersWithSourceTokens(editorHtml, commentMarkerPattern, &tokens);

        static const QRegularExpression anchorMarkerPattern(
            QStringLiteral(
                R"(<a\b(?=[^>]*\bname\s*=\s*["']whatson-style-source:([0-9a-fA-F]*)["'])[^>]*>[^<]*</a>([\s\S]*?)<a\b(?=[^>]*\bname\s*=\s*["']whatson-style-source-end["'])[^>]*>[^<]*</a>\x{200B}?)"),
            QRegularExpression::CaseInsensitiveOption);
        replaceStyleMarkersWithSourceTokens(editorHtml, anchorMarkerPattern, &tokens);
        replaceStartAnchoredStyleSpansWithSourceTokens(editorHtml, &tokens);

        return tokens;
    }

    QString restoreRenderedStyleSourceTokens(QString text, const QVector<RenderedStyleSourceToken>& tokens)
    {
        for (const RenderedStyleSourceToken& token : tokens)
        {
            text.replace(token.token, token.sourceText);
        }
        return text;
    }

    bool markerBodyContainsLiveRenderedCallout(const QString& markerBody)
    {
        const QString foldedMarkerBody = markerBody.trimmed().toCaseFolded();
        return foldedMarkerBody.contains(QStringLiteral("whatson-callout"))
            && foldedMarkerBody.contains(QStringLiteral("data-callout-content"));
    }

    QString calloutOpeningTokenFromSource(const QString& sourceText)
    {
        static const QRegularExpression openingPattern(
            QStringLiteral(R"(^\s*(<\s*callout\b[^>]*>))"),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch match = openingPattern.match(sourceText);
        return match.hasMatch() ? match.captured(1) : QStringLiteral("<callout>");
    }

    QString renderedCalloutContentHtml(const QString& markerBody)
    {
        static const QRegularExpression contentMarkerPattern(
            QStringLiteral(R"(<!--whatson-callout-content-->([\s\S]*?)<!--\/whatson-callout-content-->)"));
        const QRegularExpressionMatch contentMarkerMatch = contentMarkerPattern.match(markerBody);
        if (contentMarkerMatch.hasMatch())
        {
            return contentMarkerMatch.captured(1);
        }

        static const QRegularExpression contentPattern(
            QStringLiteral(
                R"(<([a-zA-Z][\w:-]*)\b(?=[^>]*\bdata-callout-content\s*=\s*["']true["'])[^>]*>([\s\S]*?)</\1>)"),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch match = contentPattern.match(markerBody);
        return match.hasMatch() ? match.captured(2) : QString();
    }

    QVector<RenderedCalloutSourceToken> replaceRenderedCalloutBlocksWithSourceTokens(QString* editorHtml)
    {
        QVector<RenderedCalloutSourceToken> tokens;
        if (editorHtml == nullptr || editorHtml->isEmpty())
        {
            return tokens;
        }

        static const QRegularExpression markerPattern(
            QStringLiteral(
                R"(<!--whatson-callout-source:([0-9a-fA-F]*)-->([\s\S]*?)<!--\/whatson-callout-source-->)"));

        QString rewrittenHtml;
        rewrittenHtml.reserve(editorHtml->size());

        int lastOffset = 0;
        QRegularExpressionMatchIterator matchIterator = markerPattern.globalMatch(*editorHtml);
        while (matchIterator.hasNext())
        {
            const QRegularExpressionMatch match = matchIterator.next();
            rewrittenHtml += editorHtml->mid(lastOffset, match.capturedStart(0) - lastOffset);

            if (!markerBodyContainsLiveRenderedCallout(match.captured(2)))
            {
                lastOffset = match.capturedEnd(0);
                continue;
            }

            const QString originalSource =
                QString::fromUtf8(QByteArray::fromHex(match.captured(1).toLatin1())).trimmed();
            const QString contentHtml = renderedCalloutContentHtml(match.captured(2));
            const QString contentSource = sourceTextFromRichEditorDocument(contentHtml).trimmed();
            const QString sourceText =
                calloutOpeningTokenFromSource(originalSource)
                + contentSource
                + QStringLiteral("</callout>");
            const QString token = QStringLiteral("__WHATSON_CALLOUT_SOURCE_TOKEN_%1__").arg(tokens.size());
            tokens.push_back({token, sourceText});
            rewrittenHtml += QStringLiteral("<p>%1</p>").arg(token);

            lastOffset = match.capturedEnd(0);
        }

        rewrittenHtml += editorHtml->mid(lastOffset);
        *editorHtml = rewrittenHtml;
        return tokens;
    }

    void replaceQtSerializedCalloutTablesWithSourceTokens(
        QString* editorHtml,
        QVector<RenderedCalloutSourceToken>* tokens)
    {
        if (editorHtml == nullptr || tokens == nullptr || editorHtml->isEmpty())
        {
            return;
        }

        static const QRegularExpression calloutTablePattern(
            QStringLiteral(
                R"(<table\b(?=[^>]*\bwidth\s*=\s*["']100%["'])(?=[^>]*\bbgcolor\s*=\s*["']#262728["'])[^>]*>\s*<tr>\s*<td\b(?=[^>]*\bwidth\s*=\s*["']3["'])(?=[^>]*\bbgcolor\s*=\s*["']#d9d9d9["'])[^>]*>[\s\S]*?</td>\s*<td\b(?=[^>]*\bwidth\s*=\s*["']12["'])[^>]*>[\s\S]*?</td>\s*<td\b[^>]*>([\s\S]*?)</td>\s*</tr>\s*</table>)"),
            QRegularExpression::CaseInsensitiveOption);

        QString rewrittenHtml;
        rewrittenHtml.reserve(editorHtml->size());

        int lastOffset = 0;
        QRegularExpressionMatchIterator matchIterator = calloutTablePattern.globalMatch(*editorHtml);
        while (matchIterator.hasNext())
        {
            const QRegularExpressionMatch match = matchIterator.next();
            rewrittenHtml += editorHtml->mid(lastOffset, match.capturedStart(0) - lastOffset);

            const QString contentSource = sourceTextFromRichEditorDocument(match.captured(1)).trimmed();
            const QString sourceText =
                QStringLiteral("<callout>")
                + contentSource
                + QStringLiteral("</callout>");
            const QString token = QStringLiteral("__WHATSON_CALLOUT_SOURCE_TOKEN_%1__").arg(tokens->size());
            tokens->push_back({token, sourceText});
            rewrittenHtml += QStringLiteral("<p>%1</p>").arg(token);

            lastOffset = match.capturedEnd(0);
        }

        rewrittenHtml += editorHtml->mid(lastOffset);
        *editorHtml = rewrittenHtml;
    }

    QString restoreRenderedCalloutSourceTokens(QString text, const QVector<RenderedCalloutSourceToken>& tokens)
    {
        for (const RenderedCalloutSourceToken& token : tokens)
        {
            text.replace(token.token, token.sourceText);
        }
        return text;
    }

    QString compactRenderedCalloutSourceLine(QString line, const QVector<RenderedCalloutSourceToken>& tokens)
    {
        const QString trimmedLine = line.trimmed();
        for (const RenderedCalloutSourceToken& token : tokens)
        {
            if (trimmedLine == token.sourceText)
            {
                return token.sourceText;
            }
        }
        return line;
    }

    bool isCanonicalCalloutSourceLine(const QString& line)
    {
        static const QRegularExpression calloutLinePattern(
            QStringLiteral(R"(^\s*<\s*callout\b[^>]*>[\s\S]*</\s*callout\s*>\s*$)"),
            QRegularExpression::CaseInsensitiveOption);
        return calloutLinePattern.match(line).hasMatch();
    }

    bool isRenderedCalloutSourceLine(const QString& line, const QVector<RenderedCalloutSourceToken>& tokens)
    {
        const QString trimmedLine = line.trimmed();
        if (isCanonicalCalloutSourceLine(trimmedLine))
        {
            return true;
        }
        for (const RenderedCalloutSourceToken& token : tokens)
        {
            if (trimmedLine == token.sourceText)
            {
                return true;
            }
        }
        return false;
    }

    QChar explicitEmptySourceLinePlaceholder()
    {
        return QChar(0x200B);
    }

    QString explicitEmptySourceLineHtml()
    {
        return QStringLiteral("&#8203;");
    }

    bool isExplicitEmptySourceLinePlaceholder(const QString& line)
    {
        QString withoutPlaceholder = line;
        withoutPlaceholder.remove(explicitEmptySourceLinePlaceholder());
        return withoutPlaceholder.trimmed().isEmpty()
            && withoutPlaceholder.size() != line.size();
    }

    QString restoreExplicitEmptySourceLinePlaceholders(QString sourceText)
    {
        QStringList sourceLines = sourceText.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
        for (QString& sourceLine : sourceLines)
        {
            if (isExplicitEmptySourceLinePlaceholder(sourceLine))
            {
                sourceLine.clear();
            }
        }
        return sourceLines.join(QLatin1Char('\n'));
    }

    bool previousLineIsRenderedCallout(
        const QStringList& sourceLines,
        const int index,
        const QVector<RenderedCalloutSourceToken>& tokens)
    {
        return index > 0
            && isRenderedCalloutSourceLine(sourceLines.at(index - 1), tokens);
    }

    bool nextLineIsRenderedCallout(
        const QStringList& sourceLines,
        const int index,
        const QVector<RenderedCalloutSourceToken>& tokens)
    {
        return index + 1 < sourceLines.size()
            && isRenderedCalloutSourceLine(sourceLines.at(index + 1), tokens);
    }

    QStringList removeRenderedCalloutPaddingLines(
        const QStringList& sourceLines,
        const QVector<RenderedCalloutSourceToken>& tokens)
    {
        QStringList compacted;
        compacted.reserve(sourceLines.size());
        for (int index = 0; index < sourceLines.size(); ++index)
        {
            const QString& line = sourceLines.at(index);
            if (line.trimmed().isEmpty())
            {
                if (previousLineIsRenderedCallout(sourceLines, index, tokens)
                    || nextLineIsRenderedCallout(sourceLines, index, tokens))
                {
                    continue;
                }
            }
            compacted.push_back(line);
        }
        return compacted;
    }

    QString removeRenderedCalloutPaddingText(
        QString sourceText,
        const QVector<RenderedCalloutSourceToken>& tokens)
    {
        for (const RenderedCalloutSourceToken& token : tokens)
        {
            while (sourceText.contains(QStringLiteral("\n\n") + token.sourceText))
            {
                sourceText.replace(
                    QStringLiteral("\n\n") + token.sourceText,
                    QLatin1Char('\n') + token.sourceText);
            }
            while (sourceText.contains(token.sourceText + QStringLiteral("\n\n")))
            {
                sourceText.replace(
                    token.sourceText + QStringLiteral("\n\n"),
                    token.sourceText + QLatin1Char('\n'));
            }
        }
        return sourceText;
    }

    bool markerBodyContainsLiveRenderedResourceFrame(const QString& markerBody)
    {
        const QString foldedMarkerBody = markerBody.trimmed().toCaseFolded();
        if (foldedMarkerBody.isEmpty())
        {
            return false;
        }
        return foldedMarkerBody.contains(QStringLiteral("whatson-resource-frame"))
            || foldedMarkerBody.contains(QStringLiteral("data-resource-preview"))
            || foldedMarkerBody.contains(QStringLiteral("<img"));
    }

    QVector<RenderedResourceSourceToken> replaceRenderedResourceBlocksWithSourceTokens(QString* editorHtml)
    {
        QVector<RenderedResourceSourceToken> tokens;
        if (editorHtml == nullptr || editorHtml->isEmpty())
        {
            return tokens;
        }

        static const QRegularExpression markerPattern(
            QStringLiteral(
                R"(<!--whatson-resource-source:([0-9a-fA-F]+)-->([\s\S]*?)<!--\/whatson-resource-source-->)"));

        QString rewrittenHtml;
        rewrittenHtml.reserve(editorHtml->size());

        int lastOffset = 0;
        QRegularExpressionMatchIterator matchIterator = markerPattern.globalMatch(*editorHtml);
        while (matchIterator.hasNext())
        {
            const QRegularExpressionMatch match = matchIterator.next();
            rewrittenHtml += editorHtml->mid(lastOffset, match.capturedStart(0) - lastOffset);

            const QString sourceTag = canonicalResourceLine(
                QString::fromUtf8(QByteArray::fromHex(match.captured(1).toLatin1())).trimmed());
            if (sourceTag.isEmpty())
            {
                rewrittenHtml += match.captured(0);
            }
            else if (!markerBodyContainsLiveRenderedResourceFrame(match.captured(2)))
            {
                // The marker can survive after Qt/backspace removes the image object. In that
                // case the canonical resource source must be deleted with the object.
            }
            else
            {
                const QString token = QStringLiteral("__WHATSON_RESOURCE_SOURCE_TOKEN_%1__").arg(tokens.size());
                tokens.push_back({token, sourceTag});
                rewrittenHtml += QStringLiteral("<p>%1</p>").arg(token);
            }

            lastOffset = match.capturedEnd(0);
        }

        rewrittenHtml += editorHtml->mid(lastOffset);
        *editorHtml = rewrittenHtml;
        return tokens;
    }

    QString restoreRenderedResourceSourceTokens(QString text, const QVector<RenderedResourceSourceToken>& tokens)
    {
        for (const RenderedResourceSourceToken& token : tokens)
        {
            text.replace(token.token, token.sourceTag);
        }
        return text;
    }

    bool isRenderedResourceSourceLine(const QString& line, const QVector<RenderedResourceSourceToken>& tokens)
    {
        const QString trimmedLine = line.trimmed();
        for (const RenderedResourceSourceToken& token : tokens)
        {
            if (trimmedLine == token.sourceTag)
            {
                return true;
            }
        }
        return false;
    }

    QString compactRenderedResourceSourceLine(QString line, const QVector<RenderedResourceSourceToken>& tokens)
    {
        const QString trimmedLine = line.trimmed();
        for (const RenderedResourceSourceToken& token : tokens)
        {
            if (trimmedLine == token.sourceTag)
            {
                return token.sourceTag;
            }
        }
        return line;
    }

    bool isStandaloneBreakLine(const QString& trimmedLine)
    {
        return WhatSon::EditorComponent::Break::isSourceLine(trimmedLine);
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

    bool looksLikeEditorRichText(const QString& text)
    {
        const QString folded = text.left(4096).toCaseFolded();
        return folded.contains(QStringLiteral("<!doctype html"))
            || folded.contains(QStringLiteral("<html"))
            || folded.contains(QStringLiteral("<body"))
            || folded.contains(QStringLiteral("<meta name=\"qrichtext\""))
            || folded.contains(QStringLiteral("<p"))
            || folded.contains(QStringLiteral("</p>"))
            || folded.contains(QStringLiteral("<div"))
            || folded.contains(QStringLiteral("</div>"))
            || folded.contains(QStringLiteral("<br"))
            || folded.contains(QStringLiteral("<table"))
            || folded.contains(QStringLiteral("</table>"))
            || folded.contains(QStringLiteral("<span"))
            || folded.contains(QStringLiteral("</span>"))
            || folded.contains(QStringLiteral("<strong"))
            || folded.contains(QStringLiteral("</strong>"))
            || folded.contains(QStringLiteral("<em"))
            || folded.contains(QStringLiteral("</em>"))
            || folded.contains(QStringLiteral("<a "))
            || folded.contains(QStringLiteral("whatson-callout"))
            || folded.contains(QStringLiteral("<hr"));
    }

    QStringList sourceInlineTagsForEditorFormat(const QTextCharFormat& format)
    {
        QStringList tags;
        if (format.fontWeight() >= QFont::Black)
        {
            tags.push_back(QStringLiteral("bold"));
        }
        if (format.fontItalic())
        {
            tags.push_back(QStringLiteral("italic"));
        }
        if (format.fontUnderline())
        {
            tags.push_back(QStringLiteral("underline"));
        }
        if (format.fontStrikeOut())
        {
            tags.push_back(QStringLiteral("strikethrough"));
        }

        const QBrush background = format.background();
        if (background.style() != Qt::NoBrush
            && background.color().name(QColor::HexRgb).compare(QStringLiteral("#8a4b00"), Qt::CaseInsensitive) == 0)
        {
            tags.push_back(QStringLiteral("highlight"));
        }
        return tags;
    }

    bool brushLooksLikeSerializedCallout(const QBrush& background)
    {
        if (background.style() != Qt::NoBrush
            && background.color().name(QColor::HexRgb).compare(QStringLiteral("#262728"), Qt::CaseInsensitive) == 0)
        {
            return true;
        }

        return false;
    }

    bool formatLooksLikeSerializedCallout(const QTextCharFormat& format)
    {
        return brushLooksLikeSerializedCallout(format.background());
    }

    bool blockLooksLikeSerializedCallout(const QTextBlockFormat& format)
    {
        return brushLooksLikeSerializedCallout(format.background());
    }

    QString removeSerializedCalloutFrameChrome(QString text)
    {
        text.remove(QChar::ObjectReplacementCharacter);
        return text;
    }

    QString wrapSourceTextWithInlineTags(const QString& text, const QStringList& tags)
    {
        if (text.isEmpty() || tags.isEmpty())
        {
            return text;
        }

        QString wrapped;
        for (const QString& tag : tags)
        {
            wrapped += QLatin1Char('<') + tag + QLatin1Char('>');
        }
        wrapped += text;
        for (auto tagIt = tags.crbegin(); tagIt != tags.crend(); ++tagIt)
        {
            wrapped += QStringLiteral("</") + *tagIt + QLatin1Char('>');
        }
        return wrapped;
    }

    QString repairSerializedEmptyCalloutPrefix(QString sourceLine)
    {
        static const QRegularExpression emptyCalloutPrefixPattern(
            QStringLiteral(R"(^\s*(<\s*callout\b[^>]*>)\s*</\s*callout\s*>(.+)$)"),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch match = emptyCalloutPrefixPattern.match(sourceLine);
        if (!match.hasMatch())
        {
            return sourceLine;
        }

        return match.captured(1)
            + match.captured(2)
            + QStringLiteral("</callout>");
    }

    QString normalizedRichEditorBlockSourceLine(QString sourceLine)
    {
        QString withoutLineBreaks = sourceLine;
        withoutLineBreaks.remove(QLatin1Char('\n'));
        return withoutLineBreaks.isEmpty() ? QString() : sourceLine;
    }

    QString sourceTextFromRichEditorDocument(const QString& editorDocumentText)
    {
        QString normalizedEditorDocumentText = editorDocumentText;
        const QVector<RenderedStyleSourceToken> renderedStyleTokens =
            replaceRenderedStyleSpansWithSourceTokens(&normalizedEditorDocumentText);
        QVector<RenderedCalloutSourceToken> renderedCalloutTokens =
            replaceRenderedCalloutBlocksWithSourceTokens(&normalizedEditorDocumentText);
        replaceQtSerializedCalloutTablesWithSourceTokens(
            &normalizedEditorDocumentText,
            &renderedCalloutTokens);
        const QVector<RenderedResourceSourceToken> renderedResourceTokens =
            replaceRenderedResourceBlocksWithSourceTokens(&normalizedEditorDocumentText);

        QTextDocument document;
        document.setHtml(normalizedEditorDocumentText);

        QStringList sourceLines;
        for (QTextBlock block = document.begin(); block.isValid(); block = block.next())
        {
            QString sourceLine;
            const bool serializedCalloutBlock =
                blockLooksLikeSerializedCallout(block.blockFormat());
            bool serializedCalloutOpen = serializedCalloutBlock;
            if (serializedCalloutBlock)
            {
                sourceLine += QStringLiteral("<callout>");
            }

            for (QTextBlock::iterator fragmentIt = block.begin(); !fragmentIt.atEnd(); ++fragmentIt)
            {
                const QTextFragment fragment = fragmentIt.fragment();
                if (!fragment.isValid())
                {
                    continue;
                }

                const QString fragmentText = restoreRenderedResourceSourceTokens(
                    restoreRenderedCalloutSourceTokens(
                        restoreRenderedStyleSourceTokens(
                            WhatSon::NoteBodyPersistence::normalizeBodyPlainText(fragment.text()),
                            renderedStyleTokens),
                        renderedCalloutTokens),
                    renderedResourceTokens);
                const QString sourceFragmentText = serializedCalloutBlock
                    ? removeSerializedCalloutFrameChrome(fragmentText)
                    : fragmentText;
                if (sourceFragmentText.isEmpty())
                {
                    continue;
                }

                const bool serializedCalloutFragment =
                    !serializedCalloutBlock
                    && formatLooksLikeSerializedCallout(fragment.charFormat());
                if (serializedCalloutFragment && !serializedCalloutOpen)
                {
                    sourceLine += QStringLiteral("<callout>");
                    serializedCalloutOpen = true;
                }
                else if (!serializedCalloutFragment && serializedCalloutOpen)
                {
                    sourceLine += QStringLiteral("</callout>");
                    serializedCalloutOpen = false;
                }

                sourceLine += wrapSourceTextWithInlineTags(
                    sourceFragmentText,
                    sourceInlineTagsForEditorFormat(fragment.charFormat()));
            }
            if (serializedCalloutOpen)
            {
                sourceLine += QStringLiteral("</callout>");
            }

            sourceLine = normalizedRichEditorBlockSourceLine(
                repairSerializedEmptyCalloutPrefix(sourceLine));
            sourceLine = compactRenderedCalloutSourceLine(
                compactRenderedResourceSourceLine(sourceLine, renderedResourceTokens),
                renderedCalloutTokens);
            sourceLines.push_back(sourceLine);
        }

        if (sourceLines.isEmpty())
        {
            return WhatSon::NoteBodyPersistence::normalizeBodyPlainText(document.toPlainText());
        }
        const QStringList sourceLinesWithoutCalloutPadding =
            removeRenderedCalloutPaddingLines(sourceLines, renderedCalloutTokens);
        const QString normalizedSourceText =
            WhatSon::NoteBodyPersistence::normalizeBodyPlainText(
                sourceLinesWithoutCalloutPadding.join(QLatin1Char('\n')));
        return restoreExplicitEmptySourceLinePlaceholders(
            removeRenderedCalloutPaddingText(normalizedSourceText, renderedCalloutTokens));
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

    QString htmlProjectionFromBodyDocument(const QString& bodyDocumentText, const int editorViewportWidth)
    {
        const QStringList lines = bodySourceLinesFromDocument(bodyDocumentText);
        const bool containsStandaloneCalloutLine = std::any_of(
            lines.cbegin(),
            lines.cend(),
            [](const QString& line)
            {
                return isCanonicalCalloutSourceLine(line);
            });
        QStringList htmlLines;
        htmlLines.reserve(lines.size());
        for (const QString& line : lines)
        {
            const QString renderedLine = renderInlineSourceToHtml(line, editorViewportWidth);
            if (!containsStandaloneCalloutLine || isCanonicalCalloutSourceLine(line))
            {
                htmlLines.push_back(renderedLine);
                continue;
            }

            htmlLines.push_back(QStringLiteral(
                "<p style=\"margin-top:0px;margin-bottom:0px;margin-left:0px;margin-right:0px;"
                "-qt-block-indent:0;text-indent:0px;\">%1</p>")
                .arg(line.isEmpty() ? explicitEmptySourceLineHtml() : renderedLine));
        }
        return htmlLines.join(containsStandaloneCalloutLine ? QString() : QStringLiteral("<br/>"));
    }

    QString editorHtmlDocumentFromProjection(const QString& bodyHtml)
    {
        return ::editorHtmlDocumentFromProjection(bodyHtml);
    }

    QString editorHtmlFromBodySource(
        const QString& noteId,
        const QString& bodySourceText,
        const int editorViewportWidth)
    {
        return editorHtmlDocumentFromProjection(
            htmlProjectionFromBodyDocument(serializeBodyDocument(noteId, bodySourceText), editorViewportWidth));
    }

    QString sourceTextFromEditorDocument(const QString&, const QString& editorDocumentText)
    {
        const QString normalizedEditorText = normalizeBodyPlainText(editorDocumentText);
        if (normalizedEditorText.trimmed().isEmpty())
        {
            return {};
        }

        if (!looksLikeEditorRichText(normalizedEditorText))
        {
            return normalizedEditorText;
        }

        return sourceTextFromRichEditorDocument(normalizedEditorText);
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
        updateRequest.incrementModifiedCount = true;

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
