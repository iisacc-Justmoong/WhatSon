#include "ContentsWsnBodyBlockParser.hpp"

#include "file/note/WhatSonNoteBodySemanticTagSupport.hpp"
#include "file/validator/WhatSonStructuredTagLinter.hpp"

#include <QDate>
#include <QRegularExpression>
#include <QStringList>
#include <QVector>

#include <algorithm>
#include <limits>
#include <vector>

namespace
{
    namespace SemanticTags = WhatSon::NoteBodySemanticTagSupport;

    const QRegularExpression kAgendaOpenTagPattern(
        QStringLiteral(R"(<agenda\b([^>]*)>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kTaskOpenTagPattern(
        QStringLiteral(R"(<task\b([^>]*)>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kCalloutOpenTagPattern(
        QStringLiteral(R"(<callout\b([^>]*)>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kAnyTagPattern(
        QStringLiteral(R"(<\s*(/?)\s*([A-Za-z_][A-Za-z0-9_.:-]*)\b[^>]*>)"));

    struct DocumentBlockSpan final
    {
        int end = 0;
        QVariantMap payload;
        int start = 0;
    };

    struct OpenExplicitBlock final
    {
        QString normalizedTagName;
        int openTagEnd = 0;
        int start = 0;
        QString typeName;
    };

    struct AgendaParseStats final
    {
        int confirmedAgendaCount = 0;
        int confirmedCalloutCount = 0;
        int confirmedTaskCount = 0;
        int invalidAgendaChildCount = 0;
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

    QString defaultDatePlaceholder()
    {
        return QStringLiteral("yyyy-mm-dd");
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

    QString tagAttributeValue(const QString& rawAttributes, const QString& attributeName)
    {
        if (attributeName.trimmed().isEmpty())
        {
            return {};
        }

        const QRegularExpression attributePattern(
            QStringLiteral(R"ATTR(\b%1\s*=\s*(?:"([^"]*)"|'([^']*)'|([^\s>]+)))ATTR")
                .arg(QRegularExpression::escape(attributeName.trimmed())),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch match = attributePattern.match(rawAttributes);
        if (!match.hasMatch())
        {
            return {};
        }

        for (int captureIndex = 1; captureIndex <= 3; ++captureIndex)
        {
            if (match.capturedStart(captureIndex) < 0)
            {
                continue;
            }
            return decodeSourceEntities(match.captured(captureIndex));
        }
        return {};
    }

    bool parseBooleanAttributeValue(const QString& rawValue)
    {
        const QString normalizedValue = rawValue.trimmed().toCaseFolded();
        return normalizedValue == QStringLiteral("true")
            || normalizedValue == QStringLiteral("1")
            || normalizedValue == QStringLiteral("yes");
    }

    QString visibleAgendaTaskText(QString rawTaskInnerText)
    {
        rawTaskInnerText = normalizePlainText(rawTaskInnerText);
        rawTaskInnerText.replace(
            QRegularExpression(
                QStringLiteral(R"(<\s*br\s*/?\s*>)"),
                QRegularExpression::CaseInsensitiveOption),
            QStringLiteral("\n"));
        rawTaskInnerText.remove(QRegularExpression(QStringLiteral(R"(<[^>]*>)")));
        rawTaskInnerText = decodeSourceEntities(rawTaskInnerText);
        return normalizePlainText(rawTaskInnerText).trimmed();
    }

    QString visibleCalloutText(QString rawCalloutInnerText)
    {
        rawCalloutInnerText = normalizePlainText(rawCalloutInnerText);
        rawCalloutInnerText.replace(
            QRegularExpression(
                QStringLiteral(R"(<\s*br\s*/?\s*>)"),
                QRegularExpression::CaseInsensitiveOption),
            QStringLiteral("\n"));
        rawCalloutInnerText.remove(QRegularExpression(QStringLiteral(R"(<[^>]*>)")));
        rawCalloutInnerText = decodeSourceEntities(rawCalloutInnerText);
        return normalizePlainText(rawCalloutInnerText).trimmed();
    }

    bool agendaContainsOnlyTaskChildren(QString agendaInnerSourceText)
    {
        agendaInnerSourceText.remove(
            QRegularExpression(
                QStringLiteral(R"(<task\b[^>]*>[\s\S]*?</task>)"),
                QRegularExpression::CaseInsensitiveOption));
        agendaInnerSourceText.replace(QRegularExpression(QStringLiteral("\\s+")), QString());
        return agendaInnerSourceText.isEmpty();
    }

    QString normalizedAgendaDateForDisplay(const QString& rawDate)
    {
        const QString decodedDate = decodeSourceEntities(rawDate).trimmed();
        const QDate parsedDate = QDate::fromString(decodedDate, QStringLiteral("yyyy-MM-dd"));
        if (parsedDate.isValid())
        {
            return parsedDate.toString(QStringLiteral("yyyy-MM-dd"));
        }
        return defaultDatePlaceholder();
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
        payload->insert(QStringLiteral("gutterCollapsed"), atomicBlock);
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
            extractXmlAttributeValue(
                resourceTagText,
                {QStringLiteral("type"), QStringLiteral("kind"), QStringLiteral("mime")}).toCaseFolded());
        payload.insert(
            QStringLiteral("resourceFormat"),
            extractXmlAttributeValue(
                resourceTagText,
                {QStringLiteral("format"), QStringLiteral("ext"), QStringLiteral("extension")}).toCaseFolded());
        payload.insert(
            QStringLiteral("resourcePath"),
            extractXmlAttributeValue(
                resourceTagText,
                {
                    QStringLiteral("resourcePath"),
                    QStringLiteral("path"),
                    QStringLiteral("src"),
                    QStringLiteral("href"),
                    QStringLiteral("url")
                }));
        payload.insert(
            QStringLiteral("resourceId"),
            extractXmlAttributeValue(
                resourceTagText,
                {QStringLiteral("id"), QStringLiteral("resourceId")}));
        payload.insert(QStringLiteral("focusSourceOffset"), boundedTextIndex(sourceText, sourceStart));
        return payload;
    }

    QVariantMap buildCalloutPayload(
        const QString& sourceText,
        const int sourceStart,
        const int openTagEnd,
        const int sourceEnd,
        const bool hasCloseTag)
    {
        const int boundedStart = boundedTextIndex(sourceText, sourceStart);
        const int boundedOpenEnd = boundedTextIndex(sourceText, std::max(boundedStart, openTagEnd));
        const int boundedEnd = boundedTextIndex(sourceText, std::max(boundedOpenEnd, sourceEnd));
        const int closeTagSize = QStringLiteral("</callout>").size();
        const int calloutCloseStart = hasCloseTag ? std::max(boundedOpenEnd, boundedEnd - closeTagSize) : -1;
        const int calloutContentEnd = hasCloseTag ? calloutCloseStart : boundedEnd;

        QVariantMap payload = documentBlockPayload(
            sourceText,
            boundedStart,
            boundedEnd,
            QStringLiteral("callout"),
            QStringLiteral("callout"));
        payload.insert(QStringLiteral("contentStart"), boundedOpenEnd);
        payload.insert(QStringLiteral("contentEnd"), calloutContentEnd);
        payload.insert(QStringLiteral("openTagStart"), boundedStart);
        payload.insert(QStringLiteral("openTagEnd"), boundedOpenEnd);
        payload.insert(QStringLiteral("closeTagStart"), calloutCloseStart);
        payload.insert(
            QStringLiteral("closeTagEnd"),
            hasCloseTag ? calloutCloseStart + closeTagSize : -1);
        payload.insert(QStringLiteral("hasCloseTag"), hasCloseTag);
        payload.insert(QStringLiteral("focusSourceOffset"), boundedOpenEnd);
        payload.insert(QStringLiteral("tagVerified"), hasCloseTag);
        payload.insert(
            QStringLiteral("text"),
            visibleCalloutText(
                sourceText.mid(
                    boundedOpenEnd,
                    std::max(0, calloutContentEnd - boundedOpenEnd))));
        applyDocumentBlockTraits(
            &payload,
            QStringLiteral("callout"),
            payload.value(QStringLiteral("text")).toString());
        return payload;
    }

    QVariantMap buildAgendaPayload(
        const QString& sourceText,
        const QString& openTagText,
        const int sourceStart,
        const int openTagEnd,
        const int sourceEnd,
        const bool hasCloseTag,
        AgendaParseStats* stats)
    {
        const int boundedStart = boundedTextIndex(sourceText, sourceStart);
        const int boundedOpenEnd = boundedTextIndex(sourceText, std::max(boundedStart, openTagEnd));
        const int boundedEnd = boundedTextIndex(sourceText, std::max(boundedOpenEnd, sourceEnd));
        const int closeTagSize = QStringLiteral("</agenda>").size();
        const int agendaCloseStart = hasCloseTag ? std::max(boundedOpenEnd, boundedEnd - closeTagSize) : -1;
        const int agendaContentEnd = hasCloseTag ? agendaCloseStart : boundedEnd;
        const QString innerSource = sourceText.mid(
            boundedOpenEnd,
            std::max(0, agendaContentEnd - boundedOpenEnd));
        const bool containsOnlyTaskChildren = agendaContainsOnlyTaskChildren(innerSource);

        if (stats != nullptr)
        {
            if (hasCloseTag)
            {
                ++stats->confirmedAgendaCount;
            }
            if (!containsOnlyTaskChildren)
            {
                ++stats->invalidAgendaChildCount;
            }
        }

        QVariantMap payload = documentBlockPayload(
            sourceText,
            boundedStart,
            boundedEnd,
            QStringLiteral("agenda"),
            QStringLiteral("agenda"));
        payload.insert(QStringLiteral("contentStart"), boundedOpenEnd);
        payload.insert(QStringLiteral("contentEnd"), agendaContentEnd);
        payload.insert(QStringLiteral("sourceStart"), boundedStart);
        payload.insert(QStringLiteral("sourceEnd"), boundedEnd);
        payload.insert(QStringLiteral("openTagStart"), boundedStart);
        payload.insert(QStringLiteral("openTagEnd"), boundedOpenEnd);
        payload.insert(QStringLiteral("closeTagStart"), agendaCloseStart);
        payload.insert(
            QStringLiteral("closeTagEnd"),
            hasCloseTag ? agendaCloseStart + closeTagSize : -1);
        payload.insert(QStringLiteral("hasCloseTag"), hasCloseTag);
        payload.insert(
            QStringLiteral("date"),
            normalizedAgendaDateForDisplay(tagAttributeValue(openTagText, QStringLiteral("date"))));

        QVariantList tasks;
        QStringList visibleTaskLines;
        int focusSourceOffset = boundedOpenEnd;
        QRegularExpressionMatchIterator taskIterator = kTaskOpenTagPattern.globalMatch(innerSource);
        while (taskIterator.hasNext())
        {
            const QRegularExpressionMatch taskMatch = taskIterator.next();
            if (!taskMatch.hasMatch())
            {
                continue;
            }

            const int taskOpenTagStart = boundedOpenEnd + std::max(0, boundedQSizeToInt(taskMatch.capturedStart(0)));
            const int taskOpenTagEnd = boundedOpenEnd + std::max(0, boundedQSizeToInt(taskMatch.capturedEnd(0)));
            const QRegularExpressionMatch nextTaskMatch = kTaskOpenTagPattern.match(sourceText, taskOpenTagEnd);
            const int nextTaskOpenStart = nextTaskMatch.hasMatch()
                ? std::max(0, boundedQSizeToInt(nextTaskMatch.capturedStart(0)))
                : -1;
            const int taskCloseStart = sourceText.indexOf(
                QStringLiteral("</task>"),
                taskOpenTagEnd,
                Qt::CaseInsensitive);

            int taskContentEnd = agendaContentEnd;
            if (taskCloseStart >= 0 && taskCloseStart < agendaContentEnd)
            {
                taskContentEnd = std::min(taskContentEnd, taskCloseStart);
            }
            if (nextTaskOpenStart >= 0 && nextTaskOpenStart < agendaContentEnd)
            {
                taskContentEnd = std::min(taskContentEnd, nextTaskOpenStart);
            }

            const bool taskHasCloseTag = taskCloseStart >= 0 && taskCloseStart == taskContentEnd;
            if (taskHasCloseTag && stats != nullptr)
            {
                ++stats->confirmedTaskCount;
            }

            QVariantMap taskEntry;
            taskEntry.insert(
                QStringLiteral("done"),
                parseBooleanAttributeValue(tagAttributeValue(taskMatch.captured(1), QStringLiteral("done"))));
            taskEntry.insert(QStringLiteral("contentStart"), taskOpenTagEnd);
            taskEntry.insert(QStringLiteral("contentEnd"), taskContentEnd);
            taskEntry.insert(QStringLiteral("hasSourceTag"), true);
            taskEntry.insert(QStringLiteral("hasCloseTag"), taskHasCloseTag);
            taskEntry.insert(QStringLiteral("closeTagStart"), taskHasCloseTag ? taskCloseStart : -1);
            taskEntry.insert(
                QStringLiteral("closeTagEnd"),
                taskHasCloseTag ? taskCloseStart + QStringLiteral("</task>").size() : -1);
            taskEntry.insert(QStringLiteral("openTagStart"), taskOpenTagStart);
            taskEntry.insert(QStringLiteral("openTagEnd"), taskOpenTagEnd);
            taskEntry.insert(QStringLiteral("tagVerified"), taskHasCloseTag);
            taskEntry.insert(
                QStringLiteral("text"),
                visibleAgendaTaskText(
                    sourceText.mid(
                        taskOpenTagEnd,
                        std::max(0, taskContentEnd - taskOpenTagEnd))));
            visibleTaskLines.push_back(taskEntry.value(QStringLiteral("text")).toString());
            tasks.push_back(taskEntry);

            if (focusSourceOffset <= boundedOpenEnd)
            {
                focusSourceOffset = taskOpenTagEnd;
            }
        }

        if (tasks.isEmpty())
        {
            QVariantMap placeholderTaskEntry;
            placeholderTaskEntry.insert(QStringLiteral("done"), false);
            placeholderTaskEntry.insert(QStringLiteral("hasSourceTag"), false);
            placeholderTaskEntry.insert(QStringLiteral("hasCloseTag"), false);
            placeholderTaskEntry.insert(QStringLiteral("contentStart"), boundedOpenEnd);
            placeholderTaskEntry.insert(QStringLiteral("contentEnd"), agendaContentEnd);
            placeholderTaskEntry.insert(QStringLiteral("closeTagStart"), -1);
            placeholderTaskEntry.insert(QStringLiteral("closeTagEnd"), -1);
            placeholderTaskEntry.insert(QStringLiteral("openTagStart"), -1);
            placeholderTaskEntry.insert(QStringLiteral("openTagEnd"), -1);
            placeholderTaskEntry.insert(QStringLiteral("tagVerified"), false);
            placeholderTaskEntry.insert(QStringLiteral("text"), visibleAgendaTaskText(innerSource));
            visibleTaskLines.push_back(placeholderTaskEntry.value(QStringLiteral("text")).toString());
            tasks.push_back(placeholderTaskEntry);
        }

        payload.insert(QStringLiteral("focusSourceOffset"), std::max(0, focusSourceOffset));
        payload.insert(QStringLiteral("tagVerified"), hasCloseTag && containsOnlyTaskChildren);
        payload.insert(QStringLiteral("tasks"), tasks);
        applyDocumentBlockTraits(
            &payload,
            QStringLiteral("agenda"),
            visibleTaskLines.join(QLatin1Char('\n')),
            std::max(1, static_cast<int>(tasks.size())));
        return payload;
    }

    bool isClosingTagToken(const QString& token)
    {
        return token.contains(QStringLiteral("</"));
    }

    bool isSelfClosingTagToken(const QString& token)
    {
        return token.trimmed().endsWith(QStringLiteral("/>"));
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
            && gapText.startsWith(QLatin1Char('\n'))
            && keepStartIndex < keepEndIndex)
        {
            ++keepStartIndex;
        }
        if (gapPrecedesExplicitBlock
            && gapText.endsWith(QLatin1Char('\n'))
            && keepEndIndex > keepStartIndex)
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
        const int blockStart,
        const int openTagEnd,
        const int blockEnd,
        const int closeTagStart,
        const bool hasCloseTag,
        int* resourceIndex,
        AgendaParseStats* agendaStats,
        QVariantList* renderedAgendas,
        QVariantList* renderedCallouts)
    {
        QVariantMap payload;
        if (canonicalTypeName == QStringLiteral("resource"))
        {
            const int nextResourceIndex = resourceIndex != nullptr ? std::max(0, *resourceIndex) : 0;
            payload = buildResourcePayload(
                sourceText,
                fullTagToken,
                blockStart,
                blockEnd,
                nextResourceIndex);
            if (resourceIndex != nullptr)
            {
                *resourceIndex = nextResourceIndex + 1;
            }
        }
        else if (canonicalTypeName == QStringLiteral("agenda"))
        {
            payload = buildAgendaPayload(
                sourceText,
                fullTagToken,
                blockStart,
                openTagEnd,
                blockEnd,
                hasCloseTag,
                agendaStats);
            if (renderedAgendas != nullptr)
            {
                renderedAgendas->push_back(payload);
            }
        }
        else if (canonicalTypeName == QStringLiteral("callout"))
        {
            payload = buildCalloutPayload(
                sourceText,
                blockStart,
                openTagEnd,
                blockEnd,
                hasCloseTag);
            if (hasCloseTag && agendaStats != nullptr)
            {
                ++agendaStats->confirmedCalloutCount;
            }
            if (renderedCallouts != nullptr)
            {
                renderedCallouts->push_back(payload);
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

    void recoverTopLevelOpenBlockBeforeSibling(
        const QString& sourceText,
        const int siblingTagStart,
        QVector<OpenExplicitBlock>* openBlocks,
        std::vector<DocumentBlockSpan>* explicitSpans,
        int* resourceIndex,
        AgendaParseStats* agendaStats,
        QVariantList* renderedAgendas,
        QVariantList* renderedCallouts)
    {
        if (openBlocks == nullptr || explicitSpans == nullptr || openBlocks->size() != 1)
        {
            return;
        }

        const OpenExplicitBlock openBlock = openBlocks->first();
        if (openBlock.start >= siblingTagStart)
        {
            openBlocks->clear();
            return;
        }

        pushDocumentBlockSpan(
            explicitSpans,
            buildExplicitSpan(
                sourceText,
                sourceText.mid(openBlock.start, std::max(0, openBlock.openTagEnd - openBlock.start)),
                openBlock.normalizedTagName,
                openBlock.typeName,
                openBlock.start,
                openBlock.openTagEnd,
                siblingTagStart,
                -1,
                false,
                resourceIndex,
                agendaStats,
                renderedAgendas,
                renderedCallouts));
        openBlocks->clear();
    }
} // namespace

ContentsWsnBodyBlockParser::ContentsWsnBodyBlockParser() = default;

ContentsWsnBodyBlockParser::~ContentsWsnBodyBlockParser() = default;

ContentsWsnBodyBlockParser::ParseResult ContentsWsnBodyBlockParser::parse(const QString& sourceText) const
{
    const WhatSonStructuredTagLinter tagLinter;
    ParseResult result;
    AgendaParseStats agendaStats;
    int resourceIndex = 0;

    result.correctedSourceText = tagLinter.normalizeStructuredSourceText(sourceText);

    std::vector<DocumentBlockSpan> explicitSpans;
    explicitSpans.reserve(16U);
    QVector<OpenExplicitBlock> openBlocks;

    QRegularExpressionMatchIterator iterator = kAnyTagPattern.globalMatch(sourceText);
    while (iterator.hasNext())
    {
        const QRegularExpressionMatch match = iterator.next();
        if (!match.hasMatch())
        {
            continue;
        }

        const QString fullTagToken = match.captured(0);
        const QString rawTagName = match.captured(2);
        const QString normalizedTagName = rawTagName.trimmed().toCaseFolded();
        const bool closingTag = match.captured(1) == QStringLiteral("/");
        const bool selfClosingTag = isSelfClosingTagToken(fullTagToken);
        const int tagStart = std::max(0, boundedQSizeToInt(match.capturedStart(0)));
        const int tagEnd = std::max(tagStart, boundedQSizeToInt(match.capturedEnd(0)));

        if (normalizedTagName == QStringLiteral("contents")
            || normalizedTagName == QStringLiteral("body")
            || SemanticTags::isTransparentContainerTagName(rawTagName)
            || SemanticTags::isTaskTagName(rawTagName)
            || !SemanticTags::canonicalInlineStyleTagName(rawTagName).isEmpty()
            || SemanticTags::isRenderedLineBreakTagName(rawTagName))
        {
            continue;
        }

        const QString canonicalTypeName = SemanticTags::canonicalDocumentBlockTypeName(rawTagName);
        if (canonicalTypeName.isEmpty())
        {
            continue;
        }

        const bool startsSiblingBlock = !closingTag || SemanticTags::isBreakDividerTagName(rawTagName);
        if (startsSiblingBlock)
        {
            recoverTopLevelOpenBlockBeforeSibling(
                sourceText,
                tagStart,
                &openBlocks,
                &explicitSpans,
                &resourceIndex,
                &agendaStats,
                &result.renderedAgendas,
                &result.renderedCallouts);
        }

        if (canonicalTypeName == QStringLiteral("resource"))
        {
            if (!closingTag && openBlocks.isEmpty())
            {
                pushDocumentBlockSpan(
                    &explicitSpans,
                    buildExplicitSpan(
                        sourceText,
                        fullTagToken,
                        rawTagName,
                        canonicalTypeName,
                        tagStart,
                        tagEnd,
                        tagEnd,
                        -1,
                        false,
                        &resourceIndex,
                        &agendaStats,
                        &result.renderedAgendas,
                        &result.renderedCallouts));
            }
            continue;
        }

        if (selfClosingTag)
        {
            if (openBlocks.isEmpty())
            {
                pushDocumentBlockSpan(
                    &explicitSpans,
                    buildExplicitSpan(
                        sourceText,
                        fullTagToken,
                        rawTagName,
                        canonicalTypeName,
                        tagStart,
                        tagEnd,
                        tagEnd,
                        -1,
                        false,
                        &resourceIndex,
                        &agendaStats,
                        &result.renderedAgendas,
                        &result.renderedCallouts));
            }
            continue;
        }

        const bool standaloneDividerClosingTag =
            closingTag && SemanticTags::isBreakDividerTagName(rawTagName);
        if (standaloneDividerClosingTag && openBlocks.isEmpty())
        {
            pushDocumentBlockSpan(
                &explicitSpans,
                buildExplicitSpan(
                    sourceText,
                    fullTagToken,
                    rawTagName,
                    canonicalTypeName,
                    tagStart,
                    tagEnd,
                    tagEnd,
                    -1,
                    false,
                    &resourceIndex,
                    &agendaStats,
                    &result.renderedAgendas,
                    &result.renderedCallouts));
            continue;
        }

        if (!closingTag)
        {
            openBlocks.push_back(OpenExplicitBlock {
                normalizedTagName,
                tagEnd,
                tagStart,
                canonicalTypeName
            });
            continue;
        }

        int matchedOpenIndex = -1;
        for (int index = openBlocks.size() - 1; index >= 0; --index)
        {
            if (openBlocks.at(index).normalizedTagName == normalizedTagName)
            {
                matchedOpenIndex = index;
                break;
            }
        }
        if (matchedOpenIndex < 0)
        {
            continue;
        }

        const OpenExplicitBlock matchedBlock = openBlocks.at(matchedOpenIndex);
        while (openBlocks.size() > matchedOpenIndex)
        {
            openBlocks.removeLast();
        }

        if (!openBlocks.isEmpty())
        {
            continue;
        }

        pushDocumentBlockSpan(
            &explicitSpans,
            buildExplicitSpan(
                sourceText,
                sourceText.mid(
                    matchedBlock.start,
                    std::max(0, matchedBlock.openTagEnd - matchedBlock.start)),
                rawTagName,
                matchedBlock.typeName,
                matchedBlock.start,
                matchedBlock.openTagEnd,
                tagEnd,
                tagStart,
                true,
                &resourceIndex,
                &agendaStats,
                &result.renderedAgendas,
                &result.renderedCallouts));
    }

    if (!openBlocks.isEmpty())
    {
        const OpenExplicitBlock openBlock = openBlocks.first();
        if (openBlock.start < sourceText.size())
        {
            pushDocumentBlockSpan(
                &explicitSpans,
                buildExplicitSpan(
                    sourceText,
                    sourceText.mid(openBlock.start, std::max(0, openBlock.openTagEnd - openBlock.start)),
                    openBlock.normalizedTagName,
                    openBlock.typeName,
                    openBlock.start,
                    openBlock.openTagEnd,
                    sourceText.size(),
                    -1,
                    false,
                    &resourceIndex,
                    &agendaStats,
                    &result.renderedAgendas,
                    &result.renderedCallouts));
        }
    }

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

    result.agendaParseVerification = tagLinter.buildAgendaVerification(
        sourceText,
        agendaStats.confirmedAgendaCount,
        agendaStats.confirmedTaskCount,
        agendaStats.invalidAgendaChildCount);
    result.calloutParseVerification = tagLinter.buildCalloutVerification(
        sourceText,
        agendaStats.confirmedCalloutCount);
    result.structuredParseVerification = tagLinter.buildStructuredVerification(
        result.agendaParseVerification,
        result.calloutParseVerification,
        sourceText);
    return result;
}
