#include "app/models/editor/parser/ContentsWsnBodyBlockParser.hpp"

#include "app/models/file/note/WhatSonNoteBodySemanticTagSupport.hpp"
#include "app/models/editor/tags/WhatSonStructuredTagLinter.hpp"

#include <QRegularExpression>
#include <QStringList>

#include <iiXml.h>

#include <algorithm>
#include <limits>
#include <string_view>
#include <vector>

namespace
{
    namespace SemanticTags = WhatSon::NoteBodySemanticTagSupport;

    struct DocumentBlockSpan final
    {
        int end = 0;
        QVariantMap payload;
        int start = 0;
    };

    int boundedQStringSize(const QString& text)
    {
        constexpr qsizetype maxIntSize = static_cast<qsizetype>(std::numeric_limits<int>::max());
        return static_cast<int>(std::min<qsizetype>(text.size(), maxIntSize));
    }

    int boundedQSizeToInt(const qsizetype value)
    {
        constexpr qsizetype maxIntSize = static_cast<qsizetype>(std::numeric_limits<int>::max());
        if (value < 0)
        {
            return -1;
        }
        if (value > maxIntSize)
        {
            return std::numeric_limits<int>::max();
        }
        return static_cast<int>(value);
    }

    int boundedTextIndex(const QString& text, const int index)
    {
        return std::clamp(index, 0, boundedQStringSize(text));
    }

    QString QStringFromUtf8View(std::string_view view)
    {
        return QString::fromUtf8(view.data(), static_cast<qsizetype>(view.size()));
    }

    int utf8ByteOffsetToQStringIndex(const QByteArray& bytes, const std::size_t offset)
    {
        const std::size_t boundedOffset = std::min<std::size_t>(offset, static_cast<std::size_t>(bytes.size()));
        return boundedQSizeToInt(QString::fromUtf8(bytes.constData(), static_cast<qsizetype>(boundedOffset)).size());
    }

    QString normalizePlainText(QString text)
    {
        text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        text.replace(QChar(0x2028), QLatin1Char('\n'));
        text.replace(QChar(0x2029), QLatin1Char('\n'));
        text.replace(QChar(0x00A0), QLatin1Char(' '));
        return text;
    }

    QString decodeSourceEntities(QString text)
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

    QString extractXmlAttributeValue(const QString& tagText, const QStringList& attributeNames)
    {
        for (const QString& attributeName : attributeNames)
        {
            const QRegularExpression attributePattern(
                QStringLiteral(R"ATTR(\b%1\s*=\s*(?:"([^"]*)"|'([^']*)'|([^\s>]+?)(?=\s|/?>)))ATTR")
                    .arg(QRegularExpression::escape(attributeName)),
                QRegularExpression::CaseInsensitiveOption);
            const QRegularExpressionMatch match = attributePattern.match(tagText);
            if (!match.hasMatch())
            {
                continue;
            }

            for (int captureIndex = 1; captureIndex <= 3; ++captureIndex)
            {
                if (match.capturedStart(captureIndex) < 0)
                {
                    continue;
                }
                return decodeSourceEntities(match.captured(captureIndex)).trimmed();
            }
        }
        return {};
    }

    QString extractIiXmlAttributeValue(
        const QByteArray& parseBytes,
        const std::vector<iiXml::Parser::TagField>& fields,
        const QStringList& attributeNames)
    {
        for (const QString& attributeName : attributeNames)
        {
            const QString normalizedAttributeName = attributeName.trimmed().toCaseFolded();
            if (normalizedAttributeName.isEmpty())
            {
                continue;
            }

            for (const iiXml::Parser::TagField& field : fields)
            {
                const QString fieldName = QString::fromStdString(field.Name).trimmed().toCaseFolded();
                if (fieldName != normalizedAttributeName || !field.HasValue)
                {
                    continue;
                }

                const qsizetype valueBegin = static_cast<qsizetype>(
                    std::min<std::size_t>(field.ValueBegin, static_cast<std::size_t>(parseBytes.size())));
                const qsizetype valueEnd = static_cast<qsizetype>(
                    std::min<std::size_t>(field.ValueEnd, static_cast<std::size_t>(parseBytes.size())));
                if (valueEnd < valueBegin)
                {
                    continue;
                }

                QString value = QString::fromUtf8(
                    parseBytes.constData() + valueBegin,
                    valueEnd - valueBegin).trimmed();
                if (value.size() >= 2
                    && ((value.startsWith(QLatin1Char('"')) && value.endsWith(QLatin1Char('"')))
                        || (value.startsWith(QLatin1Char('\'')) && value.endsWith(QLatin1Char('\'')))))
                {
                    value = value.mid(1, value.size() - 2).trimmed();
                }
                return decodeSourceEntities(value).trimmed();
            }
        }
        return {};
    }

    QString resolvedTagAttributeValue(
        const QString& tagText,
        const QByteArray& parseBytes,
        const std::vector<iiXml::Parser::TagField>& fields,
        const QStringList& attributeNames)
    {
        const QString iiXmlValue = extractIiXmlAttributeValue(parseBytes, fields, attributeNames);
        return iiXmlValue.isEmpty() ? extractXmlAttributeValue(tagText, attributeNames) : iiXmlValue;
    }

    QString resolvedDocumentBlockTypeName(const QString& normalizedTypeName)
    {
        return normalizedTypeName.isEmpty() ? QStringLiteral("text") : normalizedTypeName;
    }

    bool isAtomicDocumentBlockType(const QString& normalizedTypeName)
    {
        const QString resolvedTypeName = resolvedDocumentBlockTypeName(normalizedTypeName);
        return resolvedTypeName == QStringLiteral("resource")
            || resolvedTypeName == QStringLiteral("break");
    }

    QString minimapVisualKindForDocumentBlockType(const QString& normalizedTypeName)
    {
        const QString resolvedTypeName = resolvedDocumentBlockTypeName(normalizedTypeName);
        if (resolvedTypeName == QStringLiteral("resource"))
        {
            return QStringLiteral("block");
        }
        return QStringLiteral("text");
    }

    int minimapRepresentativeCharCountForDocumentBlockType(const QString& normalizedTypeName)
    {
        const QString resolvedTypeName = resolvedDocumentBlockTypeName(normalizedTypeName);
        if (resolvedTypeName == QStringLiteral("resource"))
        {
            return 160;
        }
        if (resolvedTypeName == QStringLiteral("break"))
        {
            return 8;
        }
        return 0;
    }

    int logicalLineCountHintForPlainText(const QString& plainText)
    {
        return std::max(1, static_cast<int>(plainText.count(QLatin1Char('\n'))) + 1);
    }

    void applyDocumentBlockTraits(
        QVariantMap* payload,
        const QString& normalizedTypeName,
        const QString& plainText,
        const int explicitLogicalLineCountHint = -1)
    {
        if (payload == nullptr)
        {
            return;
        }

        const bool atomicBlock = isAtomicDocumentBlockType(normalizedTypeName);
        payload->insert(QStringLiteral("plainText"), plainText);
        payload->insert(QStringLiteral("textEditable"), !atomicBlock);
        payload->insert(QStringLiteral("atomicBlock"), atomicBlock);
        payload->insert(
            QStringLiteral("minimapVisualKind"),
            minimapVisualKindForDocumentBlockType(normalizedTypeName));
        payload->insert(
            QStringLiteral("minimapRepresentativeCharCount"),
            minimapRepresentativeCharCountForDocumentBlockType(normalizedTypeName));
        payload->insert(
            QStringLiteral("logicalLineCountHint"),
            explicitLogicalLineCountHint > 0
                ? explicitLogicalLineCountHint
                : logicalLineCountHintForPlainText(plainText));
    }

    QVariantMap documentBlockPayload(
        const QString& sourceText,
        const int sourceStart,
        const int sourceEnd,
        const QString& typeName = QString(),
        const QString& tagName = QString())
    {
        const int boundedStart = boundedTextIndex(sourceText, sourceStart);
        const int boundedEnd = boundedTextIndex(sourceText, std::max(boundedStart, sourceEnd));
        const QString normalizedTypeName = typeName.trimmed().toCaseFolded();
        const QString normalizedTagName = tagName.trimmed().toCaseFolded();
        const QString resolvedTypeName = resolvedDocumentBlockTypeName(normalizedTypeName);
        const QString blockSourceText = sourceText.mid(boundedStart, boundedEnd - boundedStart);

        QVariantMap payload;
        payload.insert(
            QStringLiteral("type"),
            resolvedTypeName);
        payload.insert(QStringLiteral("sourceStart"), boundedStart);
        payload.insert(QStringLiteral("sourceEnd"), boundedEnd);
        payload.insert(QStringLiteral("sourceText"), blockSourceText);
        if (!normalizedTypeName.isEmpty())
        {
            payload.insert(QStringLiteral("explicitBlock"), true);
            payload.insert(
                QStringLiteral("tagName"),
                normalizedTagName.isEmpty() ? normalizedTypeName : normalizedTagName);
            if (SemanticTags::isRenderedTextBlockElement(normalizedTypeName))
            {
                payload.insert(QStringLiteral("semanticTagName"), normalizedTypeName);
            }
        }
        applyDocumentBlockTraits(
            &payload,
            resolvedTypeName,
            isAtomicDocumentBlockType(resolvedTypeName) ? QString() : blockSourceText);
        return payload;
    }

    QVariantMap buildTextualExplicitPayload(
        const QString& sourceText,
        const QString& canonicalTypeName,
        const QString& rawTagName,
        const int blockStart,
        const int openTagEnd,
        const int blockEnd,
        const int closeTagStart,
        const bool hasCloseTag)
    {
        const int boundedBlockStart = boundedTextIndex(sourceText, blockStart);
        const int boundedOpenTagEnd = boundedTextIndex(sourceText, std::max(boundedBlockStart, openTagEnd));
        const int boundedBlockEnd = boundedTextIndex(sourceText, std::max(boundedOpenTagEnd, blockEnd));
        const int boundedCloseTagStart = hasCloseTag
            ? boundedTextIndex(sourceText, std::max(boundedOpenTagEnd, closeTagStart))
            : -1;
        const int contentEnd = hasCloseTag ? boundedCloseTagStart : boundedBlockEnd;

        QVariantMap payload = documentBlockPayload(
            sourceText,
            boundedOpenTagEnd,
            contentEnd,
            canonicalTypeName,
            rawTagName);
        payload.insert(QStringLiteral("blockSourceStart"), boundedBlockStart);
        payload.insert(QStringLiteral("blockSourceEnd"), boundedBlockEnd);
        payload.insert(QStringLiteral("contentStart"), boundedOpenTagEnd);
        payload.insert(QStringLiteral("contentEnd"), contentEnd);
        payload.insert(QStringLiteral("openTagStart"), boundedBlockStart);
        payload.insert(QStringLiteral("openTagEnd"), boundedOpenTagEnd);
        payload.insert(QStringLiteral("closeTagStart"), boundedCloseTagStart);
        payload.insert(QStringLiteral("closeTagEnd"), hasCloseTag ? boundedBlockEnd : -1);
        payload.insert(QStringLiteral("hasCloseTag"), hasCloseTag);
        payload.insert(QStringLiteral("focusSourceOffset"), boundedOpenTagEnd);
        return payload;
    }

    QVariantMap buildResourcePayload(
        const QString& sourceText,
        const QString& resourceTagText,
        const QByteArray& parseBytes,
        const std::vector<iiXml::Parser::TagField>& fields,
        const int sourceStart,
        const int sourceEnd,
        const int resourceIndex)
    {
        QVariantMap payload = documentBlockPayload(
            sourceText,
            sourceStart,
            sourceEnd,
            QStringLiteral("resource"),
            QStringLiteral("resource"));
        payload.insert(QStringLiteral("resourceIndex"), resourceIndex);
        payload.insert(
            QStringLiteral("resourceType"),
            resolvedTagAttributeValue(
                resourceTagText,
                parseBytes,
                fields,
                {QStringLiteral("type"), QStringLiteral("kind"), QStringLiteral("mime")}).toCaseFolded());
        payload.insert(
            QStringLiteral("resourceFormat"),
            resolvedTagAttributeValue(
                resourceTagText,
                parseBytes,
                fields,
                {QStringLiteral("format"), QStringLiteral("ext"), QStringLiteral("extension")}).toCaseFolded());
        payload.insert(
            QStringLiteral("resourcePath"),
            resolvedTagAttributeValue(
                resourceTagText,
                parseBytes,
                fields,
                {
                    QStringLiteral("resourcePath"),
                    QStringLiteral("path"),
                    QStringLiteral("src"),
                    QStringLiteral("href"),
                    QStringLiteral("url")
                }));
        payload.insert(
            QStringLiteral("resourceId"),
            resolvedTagAttributeValue(
                resourceTagText,
                parseBytes,
                fields,
                {QStringLiteral("id"), QStringLiteral("resourceId")}));
        payload.insert(QStringLiteral("focusSourceOffset"), boundedTextIndex(sourceText, sourceStart));
        return payload;
    }

    bool isFormattingWhitespaceBetweenExplicitBlocks(
        const QString& sourceText,
        const int sourceStart,
        const int sourceEnd)
    {
        if (sourceEnd <= sourceStart)
        {
            return false;
        }

        const QString gapText = sourceText.mid(sourceStart, sourceEnd - sourceStart);
        if (gapText.contains(QLatin1Char('\n'))
            || gapText.contains(QLatin1Char('\r'))
            || gapText.contains(QChar(0x2028))
            || gapText.contains(QChar(0x2029)))
        {
            return false;
        }
        return gapText.trimmed().isEmpty();
    }

    QVariantMap buildImplicitParagraphPayload(
        const QString& sourceText,
        const int sourceStart,
        const int sourceEnd)
    {
        QVariantMap payload = documentBlockPayload(sourceText, sourceStart, sourceEnd);
        payload.insert(QStringLiteral("type"), QStringLiteral("paragraph"));
        payload.insert(QStringLiteral("focusSourceOffset"), boundedTextIndex(sourceText, sourceStart));
        payload.insert(QStringLiteral("implicitTextBlock"), true);
        payload.insert(QStringLiteral("semanticTagName"), QStringLiteral("paragraph"));
        applyDocumentBlockTraits(
            &payload,
            QStringLiteral("paragraph"),
            payload.value(QStringLiteral("sourceText")).toString());
        return payload;
    }

    void appendImplicitParagraphBlocks(
        QVariantList* renderedDocumentBlocks,
        const QString& sourceText,
        const int sourceStart,
        const int sourceEnd,
        const bool gapFollowsExplicitBlock,
        const bool gapPrecedesExplicitBlock)
    {
        if (renderedDocumentBlocks == nullptr)
        {
            return;
        }

        const int boundedStart = boundedTextIndex(sourceText, sourceStart);
        const int boundedEnd = boundedTextIndex(sourceText, std::max(sourceStart, sourceEnd));
        if (boundedEnd < boundedStart)
        {
            return;
        }

        const QString gapText = sourceText.mid(boundedStart, boundedEnd - boundedStart);
        const QStringList lineSegments = gapText.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
        if (lineSegments.isEmpty())
        {
            renderedDocumentBlocks->push_back(
                buildImplicitParagraphPayload(sourceText, boundedStart, boundedEnd));
            return;
        }

        int keepStartIndex = 0;
        int keepEndIndex = lineSegments.size();
        if (gapFollowsExplicitBlock
            && keepStartIndex < keepEndIndex
            && lineSegments.at(keepStartIndex).trimmed().isEmpty())
        {
            ++keepStartIndex;
        }
        if (gapPrecedesExplicitBlock
            && keepEndIndex > keepStartIndex
            && lineSegments.at(keepEndIndex - 1).trimmed().isEmpty())
        {
            --keepEndIndex;
        }

        int segmentSourceStart = boundedStart;
        for (int segmentIndex = 0; segmentIndex < lineSegments.size(); ++segmentIndex)
        {
            const QString& segmentText = lineSegments.at(segmentIndex);
            const int segmentSourceEnd = segmentSourceStart + boundedQSizeToInt(segmentText.size());
            if (segmentIndex >= keepStartIndex && segmentIndex < keepEndIndex)
            {
                renderedDocumentBlocks->push_back(
                    buildImplicitParagraphPayload(
                        sourceText,
                        segmentSourceStart,
                        segmentSourceEnd));
            }

            segmentSourceStart = segmentSourceEnd;
            if (segmentIndex + 1 < lineSegments.size())
            {
                ++segmentSourceStart;
            }
        }
    }

    void pushDocumentBlockSpan(std::vector<DocumentBlockSpan>* spans, const DocumentBlockSpan& span)
    {
        if (spans == nullptr || span.end <= span.start)
        {
            return;
        }
        spans->push_back(span);
    }

    DocumentBlockSpan buildExplicitSpan(
        const QString& sourceText,
        const QString& fullTagToken,
        const QString& rawTagName,
        const QString& canonicalTypeName,
        const QByteArray& parseBytes,
        const std::vector<iiXml::Parser::TagField>& fields,
        const int blockStart,
        const int openTagEnd,
        const int blockEnd,
        const int closeTagStart,
        const bool hasCloseTag,
        int* resourceIndex)
    {
        QVariantMap payload;
        if (canonicalTypeName == QStringLiteral("resource"))
        {
            const int nextResourceIndex = resourceIndex != nullptr ? std::max(0, *resourceIndex) : 0;
            payload = buildResourcePayload(
                sourceText,
                fullTagToken,
                parseBytes,
                fields,
                blockStart,
                blockEnd,
                nextResourceIndex);
            if (resourceIndex != nullptr)
            {
                *resourceIndex = nextResourceIndex + 1;
            }
        }
        else if (SemanticTags::isRenderedTextBlockElement(canonicalTypeName))
        {
            payload = buildTextualExplicitPayload(
                sourceText,
                canonicalTypeName,
                rawTagName,
                blockStart,
                openTagEnd,
                blockEnd,
                closeTagStart,
                hasCloseTag);
        }
        else
        {
            payload = documentBlockPayload(sourceText, blockStart, blockEnd, canonicalTypeName, rawTagName);
        }

        return DocumentBlockSpan {
            boundedTextIndex(sourceText, blockEnd),
            payload,
            boundedTextIndex(sourceText, blockStart)
        };
    }

    QString sourceTextForIiXmlBlockParse(QString sourceText)
    {
        sourceText.replace(
            QStringLiteral("</break>"),
            QStringLiteral("<break/>"));
        sourceText.replace(
            QStringLiteral("</hr>"),
            QStringLiteral("<hr/>"));
        return sourceText;
    }

    bool shouldSkipIiXmlDocumentNodeButVisitChildren(const QString& rawTagName)
    {
        const QString normalizedTagName = rawTagName.trimmed().toCaseFolded();
        return normalizedTagName == QStringLiteral("contents")
            || normalizedTagName == QStringLiteral("body")
            || SemanticTags::isTransparentContainerTagName(rawTagName);
    }

    bool shouldSkipIiXmlDocumentNode(const QString& rawTagName)
    {
        return !SemanticTags::canonicalInlineStyleTagName(rawTagName).isEmpty()
            || SemanticTags::isRenderedLineBreakTagName(rawTagName);
    }

    int openTagEndForNode(const QString& sourceText, const int blockStart, const int blockEnd)
    {
        const int tagEnd = sourceText.indexOf(QLatin1Char('>'), blockStart);
        if (tagEnd < 0 || tagEnd >= blockEnd)
        {
            return blockStart;
        }
        return tagEnd + 1;
    }

    bool nodeSourceLooksSelfClosing(const QString& sourceText, const int blockStart, const int openTagEnd)
    {
        if (openTagEnd <= blockStart)
        {
            return false;
        }

        const QString token = sourceText.mid(blockStart, openTagEnd - blockStart).trimmed();
        return token.endsWith(QStringLiteral("/>")) || token.startsWith(QStringLiteral("</"));
    }

    void collectExplicitSpansFromIiXmlNodes(
        const QString& sourceText,
        const QByteArray& parseBytes,
        const std::vector<iiXml::Parser::TagNode>& nodes,
        std::vector<DocumentBlockSpan>* explicitSpans,
        int* resourceIndex)
    {
        if (explicitSpans == nullptr)
        {
            return;
        }

        for (const iiXml::Parser::TagNode& node : nodes)
        {
            const QString rawTagName = QString::fromStdString(node.Range.TagName);
            if (shouldSkipIiXmlDocumentNodeButVisitChildren(rawTagName))
            {
                collectExplicitSpansFromIiXmlNodes(
                    sourceText,
                    parseBytes,
                    node.Children,
                    explicitSpans,
                    resourceIndex);
                continue;
            }
            if (shouldSkipIiXmlDocumentNode(rawTagName))
            {
                continue;
            }

            const QString canonicalTypeName = SemanticTags::canonicalDocumentBlockTypeName(rawTagName);
            if (canonicalTypeName.isEmpty())
            {
                continue;
            }

            const int blockStart = boundedTextIndex(
                sourceText,
                utf8ByteOffsetToQStringIndex(parseBytes, node.Range.RawBegin));
            const int blockEnd = boundedTextIndex(
                sourceText,
                utf8ByteOffsetToQStringIndex(parseBytes, node.Range.RawEnd));
            if (blockEnd <= blockStart)
            {
                continue;
            }

            const int openTagEnd = boundedTextIndex(
                sourceText,
                std::max(blockStart, openTagEndForNode(sourceText, blockStart, blockEnd)));
            const bool selfClosingTag = nodeSourceLooksSelfClosing(sourceText, blockStart, openTagEnd);
            const bool hasCloseTag = !selfClosingTag
                && node.Range.ValueEnd < node.Range.RawEnd
                && !SemanticTags::isBreakDividerTagName(rawTagName);
            const int closeTagStart = hasCloseTag
                ? boundedTextIndex(
                      sourceText,
                      utf8ByteOffsetToQStringIndex(parseBytes, node.Range.ValueEnd))
                : -1;

            pushDocumentBlockSpan(
                explicitSpans,
                buildExplicitSpan(
                    sourceText,
                    sourceText.mid(blockStart, std::max(0, openTagEnd - blockStart)),
                    rawTagName,
                    canonicalTypeName,
                    parseBytes,
                    node.Fields,
                    blockStart,
                    openTagEnd,
                    blockEnd,
                    closeTagStart,
                    hasCloseTag,
                    resourceIndex));
        }
    }

    bool collectExplicitSpansFromIiXmlDocument(
        const QString& sourceText,
        std::vector<DocumentBlockSpan>* explicitSpans,
        int* resourceIndex)
    {
        if (sourceText.trimmed().isEmpty())
        {
            return true;
        }

        const QString parseSourceText = sourceTextForIiXmlBlockParse(sourceText);
        const QByteArray parseBytes = parseSourceText.toUtf8();
        const iiXml::Parser::TagParser parser;
        const iiXml::Parser::TagDocumentResult parsedDocument =
            parser.ParseAllDocumentResult(
                std::string_view(parseBytes.constData(), static_cast<std::size_t>(parseBytes.size())));
        if (parsedDocument.Status != iiXml::Parser::TagTreeParseStatus::Parsed
            || !parsedDocument.Document.has_value())
        {
            return false;
        }

        collectExplicitSpansFromIiXmlNodes(
            sourceText,
            parseBytes,
            parsedDocument.Document.value().Nodes,
            explicitSpans,
            resourceIndex);
        return true;
    }

} // namespace

ContentsWsnBodyBlockParser::ContentsWsnBodyBlockParser() = default;

ContentsWsnBodyBlockParser::~ContentsWsnBodyBlockParser() = default;

ContentsWsnBodyBlockParser::ParseResult ContentsWsnBodyBlockParser::parse(const QString& sourceText) const
{
    const WhatSonStructuredTagLinter tagLinter;
    ParseResult result;
    int resourceIndex = 0;

    result.correctedSourceText = tagLinter.normalizeStructuredSourceText(sourceText);

    std::vector<DocumentBlockSpan> explicitSpans;
    explicitSpans.reserve(16U);
    collectExplicitSpansFromIiXmlDocument(
        sourceText,
        &explicitSpans,
        &resourceIndex);

    std::sort(
        explicitSpans.begin(),
        explicitSpans.end(),
        [](const DocumentBlockSpan& lhs, const DocumentBlockSpan& rhs) {
            if (lhs.start != rhs.start)
            {
                return lhs.start < rhs.start;
            }
            return lhs.end < rhs.end;
        });

    int cursor = 0;
    for (const DocumentBlockSpan& span : explicitSpans)
    {
        const int boundedStart = boundedTextIndex(sourceText, span.start);
        const int boundedEnd = boundedTextIndex(sourceText, std::max(span.start, span.end));
        if (boundedStart < cursor)
        {
            continue;
        }

        if (cursor < boundedStart
            && !isFormattingWhitespaceBetweenExplicitBlocks(sourceText, cursor, boundedStart))
        {
            appendImplicitParagraphBlocks(
                &result.renderedDocumentBlocks,
                sourceText,
                cursor,
                boundedStart,
                cursor > 0,
                true);
        }

        result.renderedDocumentBlocks.push_back(span.payload);
        cursor = boundedEnd;
    }

    if (cursor < sourceText.size())
    {
        appendImplicitParagraphBlocks(
            &result.renderedDocumentBlocks,
            sourceText,
            cursor,
            sourceText.size(),
            cursor > 0,
            false);
    }

    if (result.renderedDocumentBlocks.isEmpty())
    {
        appendImplicitParagraphBlocks(
            &result.renderedDocumentBlocks,
            sourceText,
            0,
            sourceText.size(),
            false,
            false);
    }

    result.structuredParseVerification = tagLinter.buildStructuredVerification(sourceText);
    return result;
}
