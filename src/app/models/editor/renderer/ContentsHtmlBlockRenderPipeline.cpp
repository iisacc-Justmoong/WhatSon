#include "app/models/editor/renderer/ContentsHtmlBlockRenderPipeline.hpp"

#include "app/models/editor/parser/ContentsWsnBodyBlockParser.hpp"
#include "app/models/file/note/WhatSonNoteBodyPersistence.hpp"
#include "app/models/file/note/WhatSonNoteBodySemanticTagSupport.hpp"

#include <QRegularExpression>
#include <QStringList>
#include <QVariantMap>

#include <iiHtmlBlock.h>
#include <iiXml.h>

#include <algorithm>
#include <string_view>
#include <vector>

namespace
{
    namespace SemanticTags = WhatSon::NoteBodySemanticTagSupport;

    constexpr int kResourceEditorPlaceholderLineCount = 1;

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

    QString QStringFromStdString(const std::string& value)
    {
        return QString::fromUtf8(value.data(), static_cast<qsizetype>(value.size()));
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

    QString normalizedBlockType(const QVariantMap& block)
    {
        return block.value(QStringLiteral("type")).toString().trimmed().toCaseFolded();
    }

    QString renderDelegateTypeForBlock(const QString& blockType)
    {
        if (blockType == QStringLiteral("agenda")
            || blockType == QStringLiteral("callout")
            || blockType == QStringLiteral("resource")
            || blockType == QStringLiteral("break"))
        {
            return blockType;
        }

        return QStringLiteral("text");
    }

    QString normalizeSourceText(const QString& sourceText)
    {
        return WhatSon::NoteBodyPersistence::normalizeBodyPlainText(sourceText);
    }

    QString textFragmentHtml(const QString& sourceText)
    {
        const QString normalizedText = normalizeSourceText(sourceText);
        if (normalizedText.isEmpty())
        {
            return QStringLiteral("&nbsp;");
        }

        const QString bodyDocument = WhatSon::NoteBodyPersistence::serializeBodyDocument(
            QStringLiteral("note"),
            normalizedText);
        const QString htmlProjection =
            WhatSon::NoteBodyPersistence::htmlProjectionFromBodyDocument(bodyDocument);
        return htmlProjection.isEmpty() ? QStringLiteral("&nbsp;") : htmlProjection;
    }

    QString wrapSemanticTextHtml(const QString& blockType, const QString& htmlFragment)
    {
        const QString openingHtml = SemanticTags::semanticTextOpeningHtml(blockType);
        const QString closingHtml = SemanticTags::semanticTextClosingHtml(blockType);
        if (openingHtml.isEmpty())
        {
            return htmlFragment;
        }

        return openingHtml + htmlFragment + closingHtml;
    }

    QString renderTextBlockHtml(const QVariantMap& block)
    {
        const QString blockType = normalizedBlockType(block);
        const QString sourceText = block.value(QStringLiteral("sourceText")).toString();
        return wrapSemanticTextHtml(blockType, textFragmentHtml(sourceText));
    }

    QString renderAgendaBlockHtml(const QVariantMap& block)
    {
        const QVariantList tasks = block.value(QStringLiteral("tasks")).toList();
        QStringList renderedTasks;
        renderedTasks.reserve(tasks.size());

        for (const QVariant& taskValue : tasks)
        {
            const QVariantMap task = taskValue.toMap();
            renderedTasks.push_back(textFragmentHtml(task.value(QStringLiteral("text")).toString()));
        }

        if (renderedTasks.isEmpty())
        {
            renderedTasks.push_back(textFragmentHtml(block.value(QStringLiteral("text")).toString()));
        }

        return QStringLiteral("<div style=\"margin:0;padding:27px 16px 8px 39px;\">%1</div>")
            .arg(renderedTasks.join(QStringLiteral("<br/>")));
    }

    QString renderCalloutBlockHtml(const QVariantMap& block)
    {
        return QStringLiteral("<div style=\"margin:0;padding:4px 4px 4px 17px;\">%1</div>")
            .arg(textFragmentHtml(block.value(QStringLiteral("text")).toString()));
    }

    QString renderResourceBlockHtml(const QVariantMap& block)
    {
        QStringList placeholderParagraphs;
        placeholderParagraphs.reserve(kResourceEditorPlaceholderLineCount);
        for (int index = 0; index < kResourceEditorPlaceholderLineCount; ++index)
        {
            placeholderParagraphs.push_back(editorBlankParagraphHtml());
        }

        const int resourceIndex = std::max(
            0,
            block.value(QStringLiteral("resourceIndex")).toInt());
        return QStringLiteral("<!--whatson-resource-block:%1-->%2<!--/whatson-resource-block:%1-->")
            .arg(QString::number(resourceIndex), placeholderParagraphs.join(QString()));
    }

    QString renderBreakBlockHtml()
    {
        return QStringLiteral("<hr/>");
    }

    bool hasTextHtmlOverlay(const QVariantMap& block)
    {
        const QString blockType = normalizedBlockType(block);
        const QString sourceText = block.value(QStringLiteral("sourceText")).toString();
        if (!SemanticTags::semanticTextOpeningHtml(blockType).isEmpty())
        {
            return true;
        }

        return sourceText.contains(QLatin1Char('<'))
            || sourceText.contains(QLatin1Char('&'))
            || sourceText.contains(QLatin1Char('\n'));
    }

    bool containsInlineStyleMarkup(const QString& sourceText)
    {
        static const QRegularExpression inlineStyleTagPattern(
            QStringLiteral(
                R"(<\s*/?\s*(?:bold|b|strong|italic|i|em|underline|u|strikethrough|strike|s|del|highlight|mark|span)\b)"),
            QRegularExpression::CaseInsensitiveOption);
        return inlineStyleTagPattern.match(sourceText).hasMatch();
    }

    QVariantMap buildHtmlToken(
        const QVariantMap& block,
        const int blockIndex,
        const int tokenIndex)
    {
        const QString blockType = normalizedBlockType(block);
        const QString renderDelegateType = renderDelegateTypeForBlock(blockType);

        QString htmlFragment;
        bool overlayVisible = false;
        if (renderDelegateType == QStringLiteral("agenda"))
        {
            htmlFragment = renderAgendaBlockHtml(block);
        }
        else if (renderDelegateType == QStringLiteral("callout"))
        {
            htmlFragment = renderCalloutBlockHtml(block);
        }
        else if (renderDelegateType == QStringLiteral("resource"))
        {
            htmlFragment = renderResourceBlockHtml(block);
        }
        else if (renderDelegateType == QStringLiteral("break"))
        {
            htmlFragment = renderBreakBlockHtml();
        }
        else
        {
            htmlFragment = renderTextBlockHtml(block);
            overlayVisible = hasTextHtmlOverlay(block);
        }

        QVariantMap token;
        token.insert(QStringLiteral("blockIndex"), blockIndex);
        token.insert(QStringLiteral("tokenIndex"), tokenIndex);
        token.insert(QStringLiteral("blockType"), blockType);
        token.insert(QStringLiteral("renderDelegateType"), renderDelegateType);
        token.insert(QStringLiteral("sourceStart"), block.value(QStringLiteral("sourceStart")).toInt());
        token.insert(QStringLiteral("sourceEnd"), block.value(QStringLiteral("sourceEnd")).toInt());
        token.insert(QStringLiteral("ownsBlockFlow"), htmlFragmentOwnsBlockFlow(htmlFragment));
        token.insert(QStringLiteral("overlayVisible"), overlayVisible);
        token.insert(QStringLiteral("html"), htmlFragment);
        token.insert(QStringLiteral("semanticTagName"), block.value(QStringLiteral("semanticTagName")));
        token.insert(QStringLiteral("plainText"), block.value(QStringLiteral("plainText")));
        return token;
    }

    QString htmlBlockProjectionForToken(const QVariantMap& token)
    {
        const QString renderDelegateType = token.value(QStringLiteral("renderDelegateType")).toString();
        if (renderDelegateType == QStringLiteral("resource"))
        {
            return editorBlankParagraphHtml();
        }
        if (renderDelegateType == QStringLiteral("break"))
        {
            return QStringLiteral("<hr/>");
        }

        const QString html = token.value(QStringLiteral("html")).toString();
        if (token.value(QStringLiteral("ownsBlockFlow")).toBool())
        {
            return html;
        }
        return html.isEmpty() ? editorBlankParagraphHtml() : editorDocumentParagraphHtml(html);
    }

    struct IiHtmlBlockProjection final
    {
        std::vector<iiHtmlBlock::DivideBlock::ElementInfo> blocks;
        QString error;
        QString html;
        bool parsed = false;
    };

    IiHtmlBlockProjection buildIiHtmlBlockProjection(const QVariantMap& token)
    {
        IiHtmlBlockProjection projection;
        const QString xmlProjection = QStringLiteral("<whatsonhtmlblocks>%1</whatsonhtmlblocks>")
            .arg(htmlBlockProjectionForToken(token));

        const QByteArray xmlBytes = xmlProjection.toUtf8();
        const iiXml::Parser::TagParser xmlParser;
        const iiXml::Parser::TagDocumentResult parsedXml =
            xmlParser.ParseAllDocumentResult(
                std::string_view(xmlBytes.constData(), static_cast<std::size_t>(xmlBytes.size())));
        if (parsedXml.Status != iiXml::Parser::TagTreeParseStatus::Parsed || !parsedXml.Document.has_value())
        {
            projection.error = QString::fromStdString(parsedXml.Diagnostic.Reason);
            return projection;
        }

        iiHtmlBlock::iiXmlToHTML xmlToHtml;
        if (!xmlToHtml.Convert(xmlProjection))
        {
            projection.error = QStringFromStdString(xmlToHtml.GetError());
            return projection;
        }

        projection.html = QStringFromStdString(xmlToHtml.GetHTMLText());
        iiHtmlBlock::DivideBlock divider;
        if (!divider.Parse(projection.html))
        {
            projection.error = QStringFromStdString(divider.GetError());
            return projection;
        }

        projection.blocks = divider.GetBlockElements();
        projection.parsed = true;
        return projection;
    }

    void applyIiHtmlBlockMetadata(
        QVariantMap* payload,
        const iiHtmlBlock::DivideBlock::ElementInfo& block)
    {
        if (payload == nullptr)
        {
            return;
        }

        payload->insert(QStringLiteral("htmlBlockObjectSource"), QStringLiteral("iiHtmlBlock"));
        payload->insert(QStringLiteral("htmlBlockTagName"), QStringFromStdString(block.tag_name));
        payload->insert(QStringLiteral("htmlBlockRaw"), QStringFromStdString(block.raw));
        payload->insert(QStringLiteral("htmlBlockValue"), QStringFromStdString(block.value));
        payload->insert(QStringLiteral("htmlBlockRawBegin"), static_cast<qlonglong>(block.raw_begin));
        payload->insert(QStringLiteral("htmlBlockRawEnd"), static_cast<qlonglong>(block.raw_end));
        payload->insert(QStringLiteral("htmlBlockValueBegin"), static_cast<qlonglong>(block.value_begin));
        payload->insert(QStringLiteral("htmlBlockValueEnd"), static_cast<qlonglong>(block.value_end));
        payload->insert(QStringLiteral("htmlBlockDisplayOverride"), block.has_display_override);
        payload->insert(QStringLiteral("htmlBlockDisplayValue"), QStringFromStdString(block.display_value));
        payload->insert(QStringLiteral("htmlBlockIsDisplayBlock"), block.is_block);
    }

    QVariantMap buildNormalizedHtmlBlock(
        const QVariantMap& token,
        const iiHtmlBlock::DivideBlock::ElementInfo& block,
        const int htmlBlockIndex)
    {
        QVariantMap normalizedBlock = token;
        applyIiHtmlBlockMetadata(&normalizedBlock, block);
        normalizedBlock.insert(QStringLiteral("htmlBlockIndex"), htmlBlockIndex);
        normalizedBlock.insert(
            QStringLiteral("htmlTokenStartIndex"),
            token.value(QStringLiteral("tokenIndex")).toInt());
        normalizedBlock.insert(QStringLiteral("htmlTokenCount"), 1);
        normalizedBlock.insert(
            QStringLiteral("htmlBlockHtml"),
            normalizedBlock.value(QStringLiteral("htmlBlockRaw")).toString());
        return normalizedBlock;
    }

    QVariantMap buildErroredNormalizedHtmlBlock(
        const QVariantMap& token,
        const QString& error,
        const int htmlBlockIndex)
    {
        QVariantMap normalizedBlock = token;
        normalizedBlock.insert(QStringLiteral("htmlBlockObjectSource"), QStringLiteral("iiHtmlBlock"));
        normalizedBlock.insert(QStringLiteral("htmlBlockParseError"), error);
        normalizedBlock.insert(QStringLiteral("htmlBlockIndex"), htmlBlockIndex);
        normalizedBlock.insert(
            QStringLiteral("htmlTokenStartIndex"),
            token.value(QStringLiteral("tokenIndex")).toInt());
        normalizedBlock.insert(QStringLiteral("htmlTokenCount"), 1);
        normalizedBlock.insert(
            QStringLiteral("htmlBlockHtml"),
            token.value(QStringLiteral("html")).toString());
        return normalizedBlock;
    }
} // namespace

ContentsHtmlBlockRenderPipeline::ContentsHtmlBlockRenderPipeline() = default;

ContentsHtmlBlockRenderPipeline::~ContentsHtmlBlockRenderPipeline() = default;

ContentsHtmlBlockRenderPipeline::RenderResult ContentsHtmlBlockRenderPipeline::renderEditorDocument(
    const QString& sourceText) const
{
    RenderResult result;
    const QString normalizedSourceText = normalizeSourceText(sourceText);
    if (normalizedSourceText.isEmpty())
    {
        return result;
    }

    const ContentsWsnBodyBlockParser parser;
    ContentsWsnBodyBlockParser::ParseResult parseResult = parser.parse(normalizedSourceText);
    result.correctedSourceText = normalizeSourceText(
        parseResult.correctedSourceText.isEmpty()
            ? normalizedSourceText
            : parseResult.correctedSourceText);
    if (result.correctedSourceText != normalizedSourceText)
    {
        parseResult = parser.parse(result.correctedSourceText);
    }
    result.requiresLegacyDocumentComposition =
        parseResult.renderedDocumentBlocks.size() > 1
        && containsInlineStyleMarkup(result.correctedSourceText);

    QStringList documentFragments;
    documentFragments.reserve(parseResult.renderedDocumentBlocks.size());
    QVariantList tokens;
    tokens.reserve(parseResult.renderedDocumentBlocks.size());

    int tokenIndex = 0;
    for (int blockIndex = 0; blockIndex < parseResult.renderedDocumentBlocks.size(); ++blockIndex)
    {
        const QVariantMap block = parseResult.renderedDocumentBlocks.at(blockIndex).toMap();
        if (block.isEmpty())
        {
            continue;
        }

        const QVariantMap token = buildHtmlToken(block, blockIndex, tokenIndex);
        tokens.push_back(token);
        documentFragments.push_back(token.value(QStringLiteral("html")).toString());
        result.htmlOverlayVisible =
            result.htmlOverlayVisible || token.value(QStringLiteral("overlayVisible")).toBool();
        ++tokenIndex;
    }

    int htmlBlockIndex = 0;
    for (int index = 0; index < tokens.size(); ++index)
    {
        QVariantMap token = tokens.at(index).toMap();
        const IiHtmlBlockProjection htmlBlockProjection = buildIiHtmlBlockProjection(token);
        if (htmlBlockProjection.parsed && !htmlBlockProjection.blocks.empty())
        {
            token.insert(
                QStringLiteral("htmlBlockCount"),
                static_cast<int>(htmlBlockProjection.blocks.size()));
            applyIiHtmlBlockMetadata(&token, htmlBlockProjection.blocks.front());
            for (const iiHtmlBlock::DivideBlock::ElementInfo& block : htmlBlockProjection.blocks)
            {
                result.normalizedHtmlBlocks.push_back(
                    buildNormalizedHtmlBlock(token, block, htmlBlockIndex));
                ++htmlBlockIndex;
            }
        }
        else
        {
            token.insert(QStringLiteral("htmlBlockObjectSource"), QStringLiteral("iiHtmlBlock"));
            token.insert(
                QStringLiteral("htmlBlockParseError"),
                htmlBlockProjection.error.isEmpty()
                    ? QStringLiteral("iiHtmlBlock produced no display blocks for token.")
                    : htmlBlockProjection.error);
            result.normalizedHtmlBlocks.push_back(
                buildErroredNormalizedHtmlBlock(
                    token,
                    token.value(QStringLiteral("htmlBlockParseError")).toString(),
                    htmlBlockIndex));
            ++htmlBlockIndex;
        }

        result.htmlTokens.push_back(token);
    }

    result.documentHtml = joinHtmlDocumentFragments(documentFragments);
    return result;
}
