#include "app/models/editor/NoteEditorDocumentSession.hpp"

#include "app/models/editor/component/Break.h"
#include "app/models/editor/component/Callout.h"
#include "app/models/editor/component/ResourceImageFrame.h"
#include "app/models/editor/SetTag.h"
#include "app/models/file/conflict/WhatSonTimestampConflictResolver.hpp"
#include "app/models/file/note/body/WhatSonNoteBodyPersistence.hpp"
#include "app/models/file/note/body/WhatSonNoteBodyResourceTagGenerator.hpp"
#include "app/models/file/note/body/WhatSonNoteBodySemanticTagSupport.hpp"
#include "app/models/file/note/local/WhatSonLocalNoteFileStore.hpp"
#include "app/models/file/note/support/WhatSonIiXmlDocumentSupport.hpp"
#include "app/models/hierarchy/resources/WhatSonResourcePackageSupport.hpp"
#include "app/models/panel/NoteActiveStateTracker.hpp"

#include <algorithm>
#include <limits>
#include <utility>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextDocument>
#include <QVector>

namespace
{
    namespace IiXml = WhatSon::IiXmlDocumentSupport;

    QString normalizePath(const QString& path)
    {
        const QString trimmed = path.trimmed();
        return trimmed.isEmpty() ? QString() : QDir::cleanPath(trimmed);
    }

    QString sanitizeFileStem(const QString& value)
    {
        QString sanitized;
        const QString trimmed = value.trimmed();
        sanitized.reserve(trimmed.size());
        for (const QChar ch : trimmed)
        {
            if (ch.isLetterOrNumber() || ch == QLatin1Char('_') || ch == QLatin1Char('-'))
            {
                sanitized.push_back(ch);
            }
            else
            {
                sanitized.push_back(QLatin1Char('_'));
            }
        }
        return sanitized.trimmed().isEmpty() ? QStringLiteral("note") : sanitized;
    }

    QString shortHash(const QString& value)
    {
        return QString::fromLatin1(
            QCryptographicHash::hash(value.toUtf8(), QCryptographicHash::Sha256).toHex().left(16));
    }

    int lineCountForEditorSource(const QString& text)
    {
        QString normalized = text;
        normalized.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        normalized.replace(QChar('\r'), QChar('\n'));
        return normalized.split(QLatin1Char('\n'), Qt::KeepEmptyParts).size();
    }

    int clampedPosition(const int position, const int textSize)
    {
        return std::clamp(position, 0, textSize);
    }

    struct SourceVisibleCharacter final
    {
        int sourceStart = 0;
        int sourceEnd = 0;
        QString logicalText;
    };

    struct EditorSelectionRange final
    {
        int editorStart = 0;
        int editorEnd = 0;
    };

    struct SourceInsertionRange final
    {
        int sourceStart = 0;
        int sourceEnd = 0;
    };

    QString sourceTagName(QStringView tagToken)
    {
        if (tagToken.size() < 3 || tagToken.front() != QLatin1Char('<'))
        {
            return {};
        }

        int cursor = 1;
        while (cursor < tagToken.size() && tagToken.at(cursor).isSpace())
        {
            ++cursor;
        }
        if (cursor < tagToken.size() && tagToken.at(cursor) == QLatin1Char('/'))
        {
            ++cursor;
        }
        while (cursor < tagToken.size() && tagToken.at(cursor).isSpace())
        {
            ++cursor;
        }

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
        {
            return {};
        }

        return tagToken.mid(nameStart, cursor - nameStart).toString().trimmed().toCaseFolded();
    }

    bool isInvisibleEditorSourceTag(QStringView tagToken)
    {
        namespace SemanticTags = WhatSon::NoteBodySemanticTagSupport;

        const QString tagName = sourceTagName(tagToken);
        if (tagName.isEmpty())
        {
            return false;
        }

        return !SemanticTags::canonicalInlineStyleTagName(tagName).isEmpty()
            || SemanticTags::isWebLinkTagName(tagName)
            || SemanticTags::isStyleTagName(tagName)
            || SemanticTags::isHashtagTagName(tagName)
            || SemanticTags::isTransparentContainerTagName(tagName)
            || !SemanticTags::semanticTextOpeningHtml(tagName).isEmpty();
    }

    QVector<SourceVisibleCharacter> visibleCharactersForSourceText(const QString& bodySourceText)
    {
        QVector<SourceVisibleCharacter> visibleCharacters;
        visibleCharacters.reserve(bodySourceText.size());

        int cursor = 0;
        while (cursor < bodySourceText.size())
        {
            if (bodySourceText.at(cursor) == QLatin1Char('<'))
            {
                const int tagEnd = bodySourceText.indexOf(QLatin1Char('>'), cursor + 1);
                if (tagEnd > cursor)
                {
                    const QStringView tagToken(bodySourceText.constData() + cursor, tagEnd - cursor + 1);
                    const QString tagName = sourceTagName(tagToken);
                    if (WhatSon::NoteBodySemanticTagSupport::isRenderedLineBreakTagName(tagName))
                    {
                        visibleCharacters.push_back({cursor, tagEnd + 1, QStringLiteral("\n")});
                        cursor = tagEnd + 1;
                        continue;
                    }
                    if (WhatSon::EditorComponent::Break::isSourceLine(tagToken.toString()))
                    {
                        cursor = tagEnd + 1;
                        continue;
                    }
                    if (QString::compare(tagName, QStringLiteral("resource"), Qt::CaseInsensitive) == 0)
                    {
                        visibleCharacters.push_back({cursor, tagEnd + 1, QString(QChar::ObjectReplacementCharacter)});
                        cursor = tagEnd + 1;
                        continue;
                    }
                    if (isInvisibleEditorSourceTag(tagToken))
                    {
                        cursor = tagEnd + 1;
                        continue;
                    }
                }
            }

            const int sourceStart = cursor;
            if (bodySourceText.at(cursor) == QLatin1Char('&'))
            {
                const int entityEnd = bodySourceText.indexOf(QLatin1Char(';'), cursor + 1);
                if (entityEnd > cursor)
                {
                    cursor = entityEnd + 1;
                    visibleCharacters.push_back({
                        sourceStart,
                        cursor,
                        bodySourceText.mid(sourceStart, cursor - sourceStart)
                    });
                    continue;
                }
            }

            ++cursor;
            visibleCharacters.push_back({
                sourceStart,
                cursor,
                bodySourceText.mid(sourceStart, cursor - sourceStart)
            });
        }

        return visibleCharacters;
    }

    QString visibleTextForSourceText(const QString& bodySourceText)
    {
        const QVector<SourceVisibleCharacter> visibleCharacters = visibleCharactersForSourceText(bodySourceText);
        QString visibleText;
        visibleText.reserve(visibleCharacters.size());
        for (const SourceVisibleCharacter& visibleCharacter : visibleCharacters)
        {
            visibleText += visibleCharacter.logicalText;
        }
        return WhatSon::NoteBodyPersistence::normalizeBodyPlainText(visibleText);
    }

    QString visibleCursorMappingTextForSourceText(const QString& bodySourceText)
    {
        const QVector<SourceVisibleCharacter> visibleCharacters = visibleCharactersForSourceText(bodySourceText);
        QString visibleText;
        visibleText.reserve(visibleCharacters.size());
        for (const SourceVisibleCharacter& visibleCharacter : visibleCharacters)
        {
            visibleText += visibleCharacter.logicalText.isEmpty()
                ? QChar()
                : visibleCharacter.logicalText.front();
        }
        return visibleText;
    }

    int closestVisibleTextMatchStart(
        const QString& visibleText,
        const QString& selectedText,
        const int preferredStart)
    {
        if (visibleText.isEmpty() || selectedText.isEmpty())
        {
            return -1;
        }

        int bestStart = -1;
        int bestDistance = std::numeric_limits<int>::max();
        int searchFrom = 0;
        while (searchFrom <= visibleText.size())
        {
            const int candidateStart = visibleText.indexOf(selectedText, searchFrom);
            if (candidateStart < 0)
            {
                break;
            }

            const int candidateDistance = candidateStart > preferredStart
                ? candidateStart - preferredStart
                : preferredStart - candidateStart;
            if (candidateDistance < bestDistance)
            {
                bestStart = candidateStart;
                bestDistance = candidateDistance;
            }

            searchFrom = candidateStart + 1;
        }
        return bestStart;
    }

    EditorSelectionRange resolvedEditorSelectionRange(
        const QString& bodySourceText,
        const int cursorPosition,
        const int selectionLength,
        const QString& selectedText)
    {
        const QVector<SourceVisibleCharacter> visibleCharacters = visibleCharactersForSourceText(bodySourceText);
        const int editorTextSize = visibleCharacters.size();
        const int editorAnchor = clampedPosition(cursorPosition, editorTextSize);
        const int editorActive = clampedPosition(cursorPosition + selectionLength, editorTextSize);
        EditorSelectionRange range{
            std::min(editorAnchor, editorActive),
            std::max(editorAnchor, editorActive)
        };

        const QString normalizedSelectedText =
            WhatSon::NoteBodyPersistence::normalizeBodyPlainText(selectedText);
        if (normalizedSelectedText.isEmpty())
        {
            return range;
        }

        const QString visibleText = visibleTextForSourceText(bodySourceText);
        const QString currentSelectionText = visibleText.mid(
            range.editorStart,
            range.editorEnd - range.editorStart);
        if (currentSelectionText == normalizedSelectedText)
        {
            return range;
        }

        const int repairedStart = closestVisibleTextMatchStart(
            visibleText,
            normalizedSelectedText,
            range.editorStart);
        if (repairedStart < 0)
        {
            return range;
        }

        range.editorStart = repairedStart;
        range.editorEnd = repairedStart + normalizedSelectedText.size();
        return range;
    }

    int editorCursorPositionForSourcePosition(
        const QString& bodySourceText,
        const int sourceCursorPosition)
    {
        const QString normalizedSourceText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(bodySourceText);
        const int boundedSourcePosition = clampedPosition(sourceCursorPosition, normalizedSourceText.size());
        const QVector<SourceVisibleCharacter> visibleCharacters = visibleCharactersForSourceText(normalizedSourceText);

        int editorPosition = 0;
        for (const SourceVisibleCharacter& visibleCharacter : visibleCharacters)
        {
            if (visibleCharacter.sourceEnd <= boundedSourcePosition)
            {
                ++editorPosition;
                continue;
            }
            if (visibleCharacter.sourceStart < boundedSourcePosition)
            {
                ++editorPosition;
            }
            break;
        }
        return editorPosition;
    }

    QString plainTextForEditorDocumentText(const QString& editorDocumentText)
    {
        QTextDocument document;
        document.setHtml(editorDocumentText);
        return document.toPlainText();
    }

    int decoratedEditorCursorPositionForVisibleCursor(
        const QString& editorDocumentText,
        const int visibleCursorPosition)
    {
        const QString plainText = plainTextForEditorDocumentText(editorDocumentText);
        const int boundedVisibleCursorPosition = qMax(0, visibleCursorPosition);

        int visibleIndex = 0;
        for (int index = 0; index < plainText.size(); ++index)
        {
            if (visibleIndex >= boundedVisibleCursorPosition)
            {
                return index;
            }
            if (plainText.at(index) != QChar::ObjectReplacementCharacter)
            {
                ++visibleIndex;
            }
        }
        return plainText.size();
    }

    int sourcePositionForEditorSelectionStart(
        const QString& bodySourceText,
        const int editorPosition)
    {
        const QVector<SourceVisibleCharacter> visibleCharacters =
            visibleCharactersForSourceText(bodySourceText);
        if (visibleCharacters.isEmpty())
        {
            return 0;
        }

        const int boundedEditorPosition = clampedPosition(editorPosition, visibleCharacters.size());
        if (boundedEditorPosition >= visibleCharacters.size())
        {
            return bodySourceText.size();
        }

        return visibleCharacters.at(boundedEditorPosition).sourceStart;
    }

    int sourcePositionForEditorSelectionEnd(
        const QString& bodySourceText,
        const int editorPosition)
    {
        const QVector<SourceVisibleCharacter> visibleCharacters =
            visibleCharactersForSourceText(bodySourceText);
        if (visibleCharacters.isEmpty())
        {
            return 0;
        }

        const int boundedEditorPosition = clampedPosition(editorPosition, visibleCharacters.size());
        if (boundedEditorPosition <= 0)
        {
            return sourcePositionForEditorSelectionStart(bodySourceText, 0);
        }
        if (boundedEditorPosition >= visibleCharacters.size())
        {
            return visibleCharacters.constLast().sourceEnd;
        }

        return visibleCharacters.at(boundedEditorPosition - 1).sourceEnd;
    }

    int sourceLineStartForPosition(const QString& bodySourceText, const int sourcePosition)
    {
        if (bodySourceText.isEmpty())
        {
            return 0;
        }

        const int boundedPosition = clampedPosition(sourcePosition, bodySourceText.size());
        const int searchFrom = qMax(0, boundedPosition - 1);
        const int newlineIndex = bodySourceText.lastIndexOf(QLatin1Char('\n'), searchFrom);
        return newlineIndex < 0 ? 0 : newlineIndex + 1;
    }

    int sourceLineEndForStart(const QString& bodySourceText, const int lineStart)
    {
        const int boundedLineStart = clampedPosition(lineStart, bodySourceText.size());
        const int newlineIndex = bodySourceText.indexOf(QLatin1Char('\n'), boundedLineStart);
        return newlineIndex < 0 ? bodySourceText.size() : newlineIndex;
    }

    bool sourceLineIsBlank(const QString& bodySourceText, const int lineStart, const int lineEnd)
    {
        const int boundedStart = clampedPosition(lineStart, bodySourceText.size());
        const int boundedEnd = clampedPosition(lineEnd, bodySourceText.size());
        if (boundedEnd < boundedStart)
        {
            return false;
        }
        return bodySourceText.mid(boundedStart, boundedEnd - boundedStart).trimmed().isEmpty();
    }

    SourceInsertionRange resourceInsertionRangeForSourceSelection(
        const QString& bodySourceText,
        const int sourceSelectionStart,
        const int sourceSelectionEnd)
    {
        const int boundedStart = clampedPosition(sourceSelectionStart, bodySourceText.size());
        const int boundedEnd = clampedPosition(sourceSelectionEnd, bodySourceText.size());
        if (boundedStart != boundedEnd)
        {
            return {std::min(boundedStart, boundedEnd), std::max(boundedStart, boundedEnd)};
        }

        const int currentLineStart = sourceLineStartForPosition(bodySourceText, boundedStart);
        const int currentLineEnd = sourceLineEndForStart(bodySourceText, currentLineStart);
        if (sourceLineIsBlank(bodySourceText, currentLineStart, currentLineEnd))
        {
            return {currentLineStart, currentLineEnd};
        }

        if (boundedStart == currentLineStart && currentLineStart > 0)
        {
            const int previousLineEnd = currentLineStart - 1;
            const int previousLineStart = sourceLineStartForPosition(bodySourceText, previousLineEnd);
            if (sourceLineIsBlank(bodySourceText, previousLineStart, previousLineEnd))
            {
                return {previousLineStart, previousLineEnd};
            }
        }

        return {boundedStart, boundedEnd};
    }

    bool isStandaloneResourceSourceLine(const QString& line)
    {
        static const QRegularExpression resourcePattern(
            QStringLiteral(R"(^\s*<\s*resource\b[^>]*?/?>\s*$)"),
            QRegularExpression::CaseInsensitiveOption);
        return resourcePattern.match(line).hasMatch();
    }

    QString canonicalResourceSourceLine(QString line)
    {
        line = line.trimmed();
        if (!isStandaloneResourceSourceLine(line))
        {
            return {};
        }
        if (!line.endsWith(QStringLiteral("/>")))
        {
            line.chop(1);
            line = line.trimmed() + QStringLiteral(" />");
        }
        return line;
    }

    QString hubRootPathForNoteDirectory(const QString& noteDirectoryPath)
    {
        QFileInfo currentInfo(normalizePath(noteDirectoryPath));
        if (!currentInfo.exists())
        {
            return {};
        }

        QDir current = currentInfo.isDir() ? QDir(currentInfo.absoluteFilePath()) : QDir(currentInfo.absolutePath());
        while (!current.path().isEmpty())
        {
            const QFileInfo directoryInfo(current.absolutePath());
            if (directoryInfo.isDir()
                && directoryInfo.fileName().endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive))
            {
                return normalizePath(directoryInfo.absoluteFilePath());
            }
            if (!current.cdUp())
            {
                break;
            }
        }
        return {};
    }

    QVariantMap resourceDescriptorFromSourceTag(const QString& resourceTag)
    {
        const QString canonicalTag = canonicalResourceSourceLine(resourceTag);
        if (canonicalTag.isEmpty())
        {
            return {};
        }

        const QString parseableDocument = QStringLiteral("<contents><body>%1</body></contents>").arg(canonicalTag);
        const iiXml::Parser::TagDocumentResult parsedDocument = IiXml::parseDocument(parseableDocument);
        if (parsedDocument.Status != iiXml::Parser::TagTreeParseStatus::Parsed
            || !parsedDocument.Document.has_value())
        {
            return {};
        }

        const iiXml::Parser::TagDocument& document = parsedDocument.Document.value();
        const iiXml::Parser::TagNode* resourceNode =
            IiXml::findFirstDescendant(document.Nodes, QStringLiteral("resource"));
        if (resourceNode == nullptr)
        {
            return {};
        }

        QString resourcePath = WhatSon::Resources::normalizePath(
            IiXml::attributeValue(
                document,
                resourceNode,
                QStringList{
                    QStringLiteral("resourcePath"),
                    QStringLiteral("path"),
                    QStringLiteral("src"),
                    QStringLiteral("href"),
                    QStringLiteral("url")
                }));
        QString format = WhatSon::Resources::normalizeFormat(
            IiXml::attributeValue(
                document,
                resourceNode,
                QStringList{
                    QStringLiteral("format"),
                    QStringLiteral("resourceFormat"),
                    QStringLiteral("ext"),
                    QStringLiteral("extension")
                }));
        if (format.isEmpty())
        {
            format = WhatSon::Resources::normalizeFormat(
                WhatSon::Resources::formatFromAssetFilePath(resourcePath));
        }

        const QString type = WhatSon::Resources::normalizedTypeFromBucketAndFormat(
            IiXml::attributeValue(
                document,
                resourceNode,
                QStringList{
                    QStringLiteral("type"),
                    QStringLiteral("resourceType"),
                    QStringLiteral("mime"),
                    QStringLiteral("kind")
                }),
            IiXml::attributeValue(document, resourceNode, QStringLiteral("bucket")),
            format);

        QVariantMap descriptor;
        descriptor.insert(QStringLiteral("resourcePath"), resourcePath);
        descriptor.insert(QStringLiteral("id"), IiXml::attributeValue(document, resourceNode, QStringLiteral("id")));
        descriptor.insert(QStringLiteral("type"), type.isEmpty() ? QStringLiteral("other") : type);
        descriptor.insert(QStringLiteral("format"), format.isEmpty() ? QStringLiteral(".bin") : format);
        return descriptor;
    }

    QString resolvedResourceAssetPath(const QVariantMap& descriptor, const QString& noteDirectoryPath)
    {
        const QString resourcePath = descriptor.value(QStringLiteral("resourcePath")).toString().trimmed();
        if (resourcePath.isEmpty())
        {
            return {};
        }

        QStringList basePaths = WhatSon::Resources::resourceReferenceBasePathsForContext(
            noteDirectoryPath,
            QString(),
            hubRootPathForNoteDirectory(noteDirectoryPath));
        return WhatSon::Resources::resolveAssetLocationFromReference(resourcePath, std::move(basePaths));
    }

    WhatSon::EditorComponent::ResourceFrameDescriptor resourceFrameDescriptorFromSourceTag(
        const QString& resourceTag,
        const QString& noteDirectoryPath,
        const int editorViewportWidth,
        const int lockedFrameDisplayHeight)
    {
        const QString canonicalTag = canonicalResourceSourceLine(resourceTag);
        const QVariantMap descriptor = resourceDescriptorFromSourceTag(canonicalTag);

        WhatSon::EditorComponent::ResourceFrameDescriptor frameDescriptor;
        frameDescriptor.sourceTag = canonicalTag;
        frameDescriptor.resourcePath = descriptor.value(QStringLiteral("resourcePath")).toString().trimmed();
        frameDescriptor.resourceId = descriptor.value(QStringLiteral("id")).toString().trimmed();
        frameDescriptor.type = descriptor.value(QStringLiteral("type")).toString().trimmed();
        frameDescriptor.format = descriptor.value(QStringLiteral("format")).toString().trimmed();
        frameDescriptor.resolvedAssetPath = noteDirectoryPath.trimmed().isEmpty()
            ? QString()
            : resolvedResourceAssetPath(descriptor, noteDirectoryPath);
        frameDescriptor.editorViewportWidth = editorViewportWidth;
        frameDescriptor.lockedFrameDisplayHeight = qMax(0, lockedFrameDisplayHeight);
        return frameDescriptor;
    }

    QString resourceFrameHtml(
        const QString& resourceTag,
        const QString& noteDirectoryPath,
        const int editorViewportWidth,
        const int lockedFrameDisplayHeight)
    {
        const QString canonicalTag = canonicalResourceSourceLine(resourceTag);
        if (canonicalTag.isEmpty())
        {
            return WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(QStringLiteral("note"), resourceTag);
        }

        return WhatSon::EditorComponent::ResourceFrame::renderHtml(
            resourceFrameDescriptorFromSourceTag(
                canonicalTag,
                noteDirectoryPath,
                editorViewportWidth,
                lockedFrameDisplayHeight));
    }

    QString editorHtmlFromBodySourceForNoteContext(
        const QString& noteId,
        const QString& bodySourceText,
        const QString& noteDirectoryPath,
        const int editorViewportWidth,
        const QHash<QString, int>* lockedFrameDisplayHeightsBySourceTag = nullptr)
    {
        const QString normalizedSourceText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(bodySourceText);
        const QStringList sourceLines = normalizedSourceText.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
        QStringList htmlLines;
        htmlLines.reserve(sourceLines.size());
        QStringList pendingSourceLines;
        pendingSourceLines.reserve(sourceLines.size());

        const auto flushPendingSourceLines = [&]()
        {
            if (pendingSourceLines.isEmpty())
            {
                return;
            }
            htmlLines.push_back(
                WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(
                    noteId,
                    pendingSourceLines.join(QLatin1Char('\n')),
                    editorViewportWidth));
            pendingSourceLines.clear();
        };

        for (const QString& sourceLine : sourceLines)
        {
            if (WhatSon::EditorComponent::Break::isSourceLine(sourceLine))
            {
                pendingSourceLines.push_back(WhatSon::EditorComponent::Break::sourceToken());
                continue;
            }

            const QString resourceLine = canonicalResourceSourceLine(sourceLine);
            if (resourceLine.isEmpty())
            {
                pendingSourceLines.push_back(sourceLine);
                continue;
            }

            flushPendingSourceLines();
            const int lockedFrameDisplayHeight = lockedFrameDisplayHeightsBySourceTag == nullptr
                ? 0
                : lockedFrameDisplayHeightsBySourceTag->value(resourceLine, 0);
            htmlLines.push_back(resourceFrameHtml(
                resourceLine,
                noteDirectoryPath,
                editorViewportWidth,
                lockedFrameDisplayHeight));
        }
        flushPendingSourceLines();
        return htmlLines.join(QStringLiteral("<br/>"));
    }

    QHash<QString, int> resourceFrameDisplayHeightsBySourceTag(const QString& editorDocumentText)
    {
        QHash<QString, int> heightsBySourceTag;
        if (editorDocumentText.isEmpty())
        {
            return heightsBySourceTag;
        }

        static const QRegularExpression markerPattern(
            QStringLiteral(
                R"(<!--whatson-resource-source:([0-9a-fA-F]+)-->([\s\S]*?)<!--\/whatson-resource-source-->)"));
        static const QRegularExpression displayHeightPattern(
            QStringLiteral(R"rx(data-frame-display-height\s*=\s*"([0-9]+)")rx"),
            QRegularExpression::CaseInsensitiveOption);

        QRegularExpressionMatchIterator matchIterator = markerPattern.globalMatch(editorDocumentText);
        while (matchIterator.hasNext())
        {
            const QRegularExpressionMatch match = matchIterator.next();
            const QString sourceTag = canonicalResourceSourceLine(
                QString::fromUtf8(QByteArray::fromHex(match.captured(1).toLatin1())).trimmed());
            if (sourceTag.isEmpty())
            {
                continue;
            }

            const QRegularExpressionMatch displayHeightMatch = displayHeightPattern.match(match.captured(2));
            if (!displayHeightMatch.hasMatch())
            {
                continue;
            }

            bool parsed = false;
            const int displayHeight = displayHeightMatch.captured(1).toInt(&parsed);
            if (parsed && displayHeight > 0)
            {
                heightsBySourceTag.insert(sourceTag, displayHeight);
            }
        }
        return heightsBySourceTag;
    }

    struct ActiveResourceSourceLine final
    {
        QString sourceTag;
        bool hasBlankBefore = false;
        bool hasBlankAfter = false;
    };

    QVector<ActiveResourceSourceLine> activeResourceSourceLines(const QString& activeBodySourceText)
    {
        const QStringList activeLines =
            WhatSon::NoteBodyPersistence::normalizeBodyPlainText(activeBodySourceText)
                .split(QLatin1Char('\n'), Qt::KeepEmptyParts);
        QVector<ActiveResourceSourceLine> resourceLines;
        resourceLines.reserve(activeLines.size());
        for (int index = 0; index < activeLines.size(); ++index)
        {
            const QString resourceLine = canonicalResourceSourceLine(activeLines.at(index));
            if (resourceLine.isEmpty())
            {
                continue;
            }

            resourceLines.push_back({
                resourceLine,
                index > 0 && activeLines.at(index - 1).trimmed().isEmpty(),
                index + 1 < activeLines.size() && activeLines.at(index + 1).trimmed().isEmpty()
            });
        }
        return resourceLines;
    }

    bool lineContainsRichTextObjectReplacement(const QString& line)
    {
        return line.contains(QChar::ObjectReplacementCharacter);
    }

    bool editorDocumentContainsLiveRenderedResourceFrame(const QString& editorDocumentText)
    {
        const QString foldedEditorDocumentText = editorDocumentText.toCaseFolded();
        return foldedEditorDocumentText.contains(QStringLiteral("whatson-resource-frame"))
            || foldedEditorDocumentText.contains(QStringLiteral("data-resource-preview"))
            || foldedEditorDocumentText.contains(QStringLiteral("<img"));
    }

    QString compactRestoredResourcePadding(
        const QStringList& restoredLines,
        const QVector<ActiveResourceSourceLine>& resourceLines)
    {
        if (resourceLines.isEmpty())
        {
            return WhatSon::NoteBodyPersistence::normalizeBodyPlainText(restoredLines.join(QLatin1Char('\n')));
        }

        QStringList compactedLines;
        compactedLines.reserve(restoredLines.size());
        int resourceIndex = 0;
        for (int index = 0; index < restoredLines.size(); ++index)
        {
            const QString resourceLine = canonicalResourceSourceLine(restoredLines.at(index));
            if (!resourceLine.isEmpty()
                && resourceIndex < resourceLines.size()
                && resourceLine == resourceLines.at(resourceIndex).sourceTag)
            {
                const ActiveResourceSourceLine& activeResourceLine = resourceLines.at(resourceIndex);
                if (!activeResourceLine.hasBlankBefore)
                {
                    while (!compactedLines.isEmpty() && compactedLines.constLast().trimmed().isEmpty())
                    {
                        compactedLines.removeLast();
                    }
                }

                compactedLines.push_back(activeResourceLine.sourceTag);
                if (!activeResourceLine.hasBlankAfter)
                {
                    while (index + 1 < restoredLines.size()
                           && restoredLines.at(index + 1).trimmed().isEmpty())
                    {
                        ++index;
                    }
                }
                ++resourceIndex;
                continue;
            }

            compactedLines.push_back(restoredLines.at(index));
        }

        return WhatSon::NoteBodyPersistence::normalizeBodyPlainText(compactedLines.join(QLatin1Char('\n')));
    }

    QStringList removeDeletedResourceObjectPaddingLines(
        QStringList restoredLines,
        const QString& activeBodySourceText)
    {
        const QStringList activeLines =
            WhatSon::NoteBodyPersistence::normalizeBodyPlainText(activeBodySourceText)
                .split(QLatin1Char('\n'), Qt::KeepEmptyParts);
        for (int activeIndex = 0; activeIndex < activeLines.size(); ++activeIndex)
        {
            if (canonicalResourceSourceLine(activeLines.at(activeIndex)).isEmpty())
            {
                continue;
            }
            if (activeIndex <= 0 || activeIndex + 1 >= activeLines.size())
            {
                continue;
            }
            if (activeLines.at(activeIndex - 1).trimmed().isEmpty()
                || activeLines.at(activeIndex + 1).trimmed().isEmpty())
            {
                continue;
            }

            const QString previousLine = activeLines.at(activeIndex - 1).trimmed();
            const QString nextLine = activeLines.at(activeIndex + 1).trimmed();
            for (int lineIndex = 0; lineIndex + 2 < restoredLines.size(); ++lineIndex)
            {
                if (restoredLines.at(lineIndex).trimmed() != previousLine)
                {
                    continue;
                }

                int blankEndIndex = lineIndex + 1;
                while (blankEndIndex < restoredLines.size()
                       && restoredLines.at(blankEndIndex).trimmed().isEmpty())
                {
                    ++blankEndIndex;
                }
                if (blankEndIndex == lineIndex + 1
                    || blankEndIndex >= restoredLines.size()
                    || restoredLines.at(blankEndIndex).trimmed() != nextLine)
                {
                    continue;
                }

                while (blankEndIndex - lineIndex > 1)
                {
                    restoredLines.removeAt(lineIndex + 1);
                    --blankEndIndex;
                }
                break;
            }
        }
        return restoredLines;
    }

    QString restoreResourceObjectPlaceholdersFromActiveSource(
        const QString& editorSourceText,
        const QString& activeBodySourceText)
    {
        const QVector<ActiveResourceSourceLine> resourceLines =
            activeResourceSourceLines(activeBodySourceText);
        if (resourceLines.isEmpty())
        {
            return editorSourceText;
        }

        const QStringList editorLines =
            WhatSon::NoteBodyPersistence::normalizeBodyPlainText(editorSourceText)
                .split(QLatin1Char('\n'), Qt::KeepEmptyParts);
        QStringList restoredLines;
        restoredLines.reserve(editorLines.size());

        int resourceIndex = 0;
        bool skipNextPaddingLineAfterResource = false;
        for (int index = 0; index < editorLines.size(); ++index)
        {
            const QString& editorLine = editorLines.at(index);
            const bool lineIsBlank = editorLine.trimmed().isEmpty();
            const bool lineHasResourceObject = lineContainsRichTextObjectReplacement(editorLine);

            if (skipNextPaddingLineAfterResource && lineIsBlank)
            {
                skipNextPaddingLineAfterResource = false;
                continue;
            }
            skipNextPaddingLineAfterResource = false;

            const bool nextLineIsResourceObject =
                index + 1 < editorLines.size()
                && lineContainsRichTextObjectReplacement(editorLines.at(index + 1));
            if (lineIsBlank
                && nextLineIsResourceObject
                && resourceIndex < resourceLines.size()
                && !resourceLines.at(resourceIndex).hasBlankBefore)
            {
                continue;
            }

            if (lineHasResourceObject && resourceIndex < resourceLines.size())
            {
                const ActiveResourceSourceLine& resourceLine = resourceLines.at(resourceIndex);
                restoredLines.push_back(resourceLine.sourceTag);
                skipNextPaddingLineAfterResource = !resourceLine.hasBlankAfter;
                ++resourceIndex;
                continue;
            }

            restoredLines.push_back(editorLine);
        }

        return compactRestoredResourcePadding(
            removeDeletedResourceObjectPaddingLines(restoredLines, activeBodySourceText),
            resourceLines);
    }

    QVariantMap invalidImportedResourcesInsertionResult(
        const QString& noteId,
        const QString& bodySourceText,
        const int sourceSelectionStart,
        const int sourceSelectionLength,
        const int editorCursorPosition,
        const int editorSelectionStart,
        const int editorSelectionLength,
        const QString& errorMessage)
    {
        const QString normalizedSourceText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(bodySourceText);
        const int sourceSize = normalizedSourceText.size();
        const int boundedSelectionStart = clampedPosition(sourceSelectionStart, sourceSize);
        const int boundedSelectionEnd =
            clampedPosition(sourceSelectionStart + sourceSelectionLength, sourceSize);

        QVariantMap result;
        result.insert(QStringLiteral("valid"), false);
        result.insert(QStringLiteral("changed"), false);
        result.insert(QStringLiteral("bodySourceText"), normalizedSourceText);
        result.insert(
            QStringLiteral("editorDocumentText"),
            WhatSon::NoteBodyPersistence::editorHtmlFromBodySource(noteId, normalizedSourceText));
        result.insert(QStringLiteral("cursorPosition"), editorCursorPosition);
        result.insert(QStringLiteral("sourceCursorPosition"), boundedSelectionStart);
        result.insert(QStringLiteral("selectionStart"), boundedSelectionStart);
        result.insert(QStringLiteral("selectionLength"), boundedSelectionEnd - boundedSelectionStart);
        result.insert(QStringLiteral("editorSelectionStart"), editorSelectionStart);
        result.insert(QStringLiteral("editorSelectionLength"), editorSelectionLength);
        result.insert(QStringLiteral("insertedText"), QString());
        result.insert(QStringLiteral("insertedCount"), 0);
        result.insert(QStringLiteral("errorMessage"), errorMessage);
        return result;
    }

} // namespace

NoteEditorDocumentSession::NoteEditorDocumentSession(QObject* parent)
    : QObject(parent)
    , m_noteManagementCoordinator(this)
    , m_rawPullController(this)
    , m_rawPushController(this)
{
    m_rawPullController.setRawPullCallback(
        [this](
            const QString& noteId,
            const QString& noteDirectoryPath,
            const QString&,
            QString* errorMessage) -> quint64
        {
            const quint64 sequence = m_noteManagementCoordinator.loadNoteBodyTextForNote(
                noteId,
                noteDirectoryPath);
            if (sequence == 0 && errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to enqueue note body load.");
            }
            return sequence;
        });

    m_rawPushController.setRawPushCallback(
        [this](
            const QString& editorFilePath,
            const QString& editorDocumentText,
            const bool hasEditorDocumentText,
            const QString&,
            QString* errorMessage) -> bool
        {
            const bool pushed = hasEditorDocumentText
                ? persistEditorDocumentText(editorFilePath, editorDocumentText)
                : persistEditorFile(editorFilePath);
            if (!pushed && errorMessage != nullptr)
            {
                *errorMessage = m_lastError.trimmed().isEmpty()
                    ? QStringLiteral("Failed to push editor surface to RAW.")
                    : m_lastError.trimmed();
            }
            return pushed;
        });

    connect(
        &m_rawPullController,
        &WhatSonEditorRawPullController::rawPullFinished,
        this,
        [this](
            const QString& noteId,
            const QString& noteDirectoryPath,
            const QString& reason,
            const quint64 sequence,
            const bool success,
            const QString&)
        {
            if (reason != QStringLiteral("idle") || !success || sequence == 0)
            {
                return;
            }

            m_pendingIdlePullSequence = sequence;
            m_pendingIdlePullNoteId = noteId.trimmed();
            m_pendingIdlePullNoteDirectoryPath = normalizePath(noteDirectoryPath);
        });
    connect(
        &m_noteManagementCoordinator,
        &ContentsNoteManagementCoordinator::noteBodyTextLoaded,
        this,
        &NoteEditorDocumentSession::handleNoteBodyTextLoaded);
    connect(
        &m_noteManagementCoordinator,
        &ContentsNoteManagementCoordinator::editorTextPersistenceFinished,
        this,
        &NoteEditorDocumentSession::handleEditorTextPersistenceFinished);
    connect(
        &m_noteManagementCoordinator,
        &ContentsNoteManagementCoordinator::hubFilesystemMutated,
        this,
        &NoteEditorDocumentSession::hubFilesystemMutated);
}

NoteEditorDocumentSession::~NoteEditorDocumentSession() = default;

QObject* NoteEditorDocumentSession::noteActiveState() const noexcept
{
    return m_noteActiveState;
}

void NoteEditorDocumentSession::setNoteActiveState(QObject* noteActiveState)
{
    auto* typedState = qobject_cast<NoteActiveStateTracker*>(noteActiveState);
    if (m_noteActiveState == typedState)
    {
        return;
    }

    if (m_noteActiveState != nullptr)
    {
        disconnect(m_noteActiveState, nullptr, this, nullptr);
    }

    m_noteActiveState = typedState;
    if (m_noteActiveState != nullptr)
    {
        connect(
            m_noteActiveState,
            &NoteActiveStateTracker::activeNoteStateChanged,
            this,
            &NoteEditorDocumentSession::refreshFromActiveNoteState);
        connect(
            m_noteActiveState,
            &NoteActiveStateTracker::activeHierarchyControllerChanged,
            this,
            &NoteEditorDocumentSession::refreshContentController);
        connect(
            m_noteActiveState,
            &QObject::destroyed,
            this,
            &NoteEditorDocumentSession::handleNoteActiveStateDestroyed);
    }

    emit noteActiveStateChanged();
    refreshContentController();
    refreshFromActiveNoteState();
}

QString NoteEditorDocumentSession::editorFilePath() const
{
    return m_editorFilePath;
}

QString NoteEditorDocumentSession::activeNoteId() const
{
    return m_activeNoteId;
}

QString NoteEditorDocumentSession::activeNoteDirectoryPath() const
{
    return m_activeNoteDirectoryPath;
}

int NoteEditorDocumentSession::parsedLineCount() const noexcept
{
    return m_parsedLineCount;
}

int NoteEditorDocumentSession::editorViewportWidth() const noexcept
{
    return m_editorViewportWidth;
}

bool NoteEditorDocumentSession::hasActiveNote() const noexcept
{
    return !m_activeNoteId.trimmed().isEmpty();
}

bool NoteEditorDocumentSession::loading() const noexcept
{
    return m_loading;
}

bool NoteEditorDocumentSession::readOnly() const noexcept
{
    return m_readOnly;
}

QString NoteEditorDocumentSession::lastError() const
{
    return m_lastError;
}

void NoteEditorDocumentSession::setSessionRootPathForTests(const QString& sessionRootPath)
{
    m_sessionRootPathForTests = normalizePath(sessionRootPath);
}

void NoteEditorDocumentSession::setEditorViewportWidth(const int editorViewportWidth)
{
    const int normalizedEditorViewportWidth = qMax(0, editorViewportWidth);
    if (m_editorViewportWidth == normalizedEditorViewportWidth)
    {
        return;
    }

    m_editorViewportWidth = normalizedEditorViewportWidth;
    emit editorViewportWidthChanged();
}

bool NoteEditorDocumentSession::openNoteForEditing(
    const QString& noteId,
    const QString& noteDirectoryPath)
{
    const QString normalizedNoteId = noteId.trimmed();
    const QString normalizedNoteDirectoryPath = normalizePath(noteDirectoryPath);
    if (normalizedNoteId.isEmpty() || normalizedNoteDirectoryPath.isEmpty())
    {
        return clearEditor();
    }

    if (!m_activeNoteId.trimmed().isEmpty()
        && (m_activeNoteId != normalizedNoteId
            || m_activeNoteDirectoryPath != normalizedNoteDirectoryPath))
    {
        pushActiveEditorBeforeNoteDeparture();
    }

    if (m_pendingLoadSequence == 0
        && m_activeNoteId == normalizedNoteId
        && m_activeNoteDirectoryPath == normalizedNoteDirectoryPath
        && !m_editorFilePath.trimmed().isEmpty()
        && QFileInfo::exists(m_editorFilePath))
    {
        setLoading(false);
        setReadOnly(false);
        setLastError(QString());
        return true;
    }

    setReadOnly(true);
    setLoading(true);
    setLastError(QString());
    m_activeBodySourceText.clear();

    m_pendingLoadNoteId = normalizedNoteId;
    m_pendingLoadNoteDirectoryPath = normalizedNoteDirectoryPath;
    m_pendingLoadSequence = m_rawPullController.requestNoteOpenPull(
        normalizedNoteId,
        normalizedNoteDirectoryPath);
    if (m_pendingLoadSequence == 0)
    {
        setLoading(false);
        switchToBlankEditorFile();
        setActiveNoteContext(QString(), QString());
        setLastError(QStringLiteral("Failed to enqueue note body load."));
        return false;
    }
    return true;
}

bool NoteEditorDocumentSession::clearEditor()
{
    if (hasActiveNote())
    {
        pushActiveEditorBeforeNoteDeparture();
    }

    m_rawPullController.clearActiveNoteForIdlePull();
    m_pendingLoadSequence = 0;
    m_pendingLoadNoteId.clear();
    m_pendingLoadNoteDirectoryPath.clear();
    m_pendingIdlePullSequence = 0;
    m_pendingIdlePullNoteId.clear();
    m_pendingIdlePullNoteDirectoryPath.clear();
    m_activeBodySourceText.clear();
    setActiveNoteContext(QString(), QString());
    setParsedLineCount(0);
    setLoading(false);
    setReadOnly(true);
    setLastError(QString());
    switchToBlankEditorFile();
    return true;
}

bool NoteEditorDocumentSession::persistEditorFile(const QString& editorFilePath)
{
    const QString normalizedEditorFilePath = normalizePath(editorFilePath);
    if (normalizedEditorFilePath.isEmpty())
    {
        setLastError(QStringLiteral("Editor file path is empty."));
        return false;
    }

    QString editorDocumentText;
    QString readError;
    if (!readEditorSourceFile(normalizedEditorFilePath, &editorDocumentText, &readError))
    {
        setLastError(readError);
        return false;
    }

    return persistEditorDocumentText(normalizedEditorFilePath, editorDocumentText);
}

void NoteEditorDocumentSession::requestEditorIdleRawPush(
    const QString& editorFilePath,
    const QString& editorDocumentText)
{
    m_rawPushController.requestIdlePush(editorFilePath, editorDocumentText);
}

void NoteEditorDocumentSession::requestEditorModifiedCountRawPush(
    const QString& editorFilePath,
    const int modifiedCount,
    const QString& editorDocumentText)
{
    recordEditorUserActivity();
    m_rawPushController.requestModifiedCountPush(
        editorFilePath,
        modifiedCount,
        editorDocumentText);
}

void NoteEditorDocumentSession::recordEditorUserActivity()
{
    m_rawPullController.recordUserActivity();
}

quint64 NoteEditorDocumentSession::requestActiveNoteIdleRawPull()
{
    return m_rawPullController.requestActiveIdlePull();
}

bool NoteEditorDocumentSession::persistEditorDocumentText(
    const QString& editorFilePath,
    const QString& editorDocumentText)
{
    const QString normalizedEditorFilePath = normalizePath(editorFilePath);
    if (normalizedEditorFilePath.isEmpty())
    {
        setLastError(QStringLiteral("Editor file path is empty."));
        return false;
    }

    const auto contextIterator = m_editorFileContexts.constFind(normalizedEditorFilePath);
    if (contextIterator == m_editorFileContexts.constEnd()
        || contextIterator->noteId.trimmed().isEmpty()
        || contextIterator->noteDirectoryPath.trimmed().isEmpty())
    {
        setLastError(QStringLiteral("Editor file context is unavailable."));
        return false;
    }

    QString sourceText = WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
        contextIterator->noteId,
        editorDocumentText);
    const QString activeSourceTextForContext =
        contextIterator->noteId == m_activeNoteId
        && contextIterator->noteDirectoryPath == m_activeNoteDirectoryPath
            ? m_activeBodySourceText
            : QString();
    sourceText = restoreResourceObjectPlaceholdersFromActiveSource(
        sourceText,
        activeSourceTextForContext);
    setParsedLineCount(lineCountForEditorSource(sourceText));

    emit editorSourcePersistRequested(contextIterator->noteId, normalizedEditorFilePath);
    const bool enqueued = m_noteManagementCoordinator.persistEditorTextForNoteAtPath(
        contextIterator->noteId,
        contextIterator->noteDirectoryPath,
        sourceText,
        contextIterator->loadedLastModifiedAt);
    if (!enqueued)
    {
        const QString error = QStringLiteral("Failed to enqueue note body persistence.");
        setLastError(error);
        emit editorSourcePersistFinished(contextIterator->noteId, false, error);
    }
    else
    {
        m_activeBodySourceText = sourceText;
    }
    return enqueued;
}

bool NoteEditorDocumentSession::pushActiveEditorBeforeNoteDeparture()
{
    const QString normalizedEditorFilePath = normalizePath(m_editorFilePath);
    if (normalizedEditorFilePath.isEmpty())
    {
        return true;
    }

    return m_rawPushController.pushBeforeNoteDeparture(normalizedEditorFilePath);
}

QVariantMap NoteEditorDocumentSession::reprojectResourceFramesForEditorWidth(
    const QString& editorDocumentText,
    const int editorViewportWidth)
{
    setEditorViewportWidth(editorViewportWidth);

    const QString noteId = m_activeNoteId.trimmed().isEmpty()
        ? QStringLiteral("note")
        : m_activeNoteId.trimmed();
    const QString sourceText = bodySourceTextForEditorDocument(noteId, editorDocumentText);
    const bool hasResourceFrame =
        editorDocumentText.contains(QStringLiteral("whatson-resource-frame"))
        || sourceText.contains(QRegularExpression(QStringLiteral("<\\s*resource\\b")));
    const bool hasCalloutFrame =
        editorDocumentText.contains(QStringLiteral("whatson-callout"))
        || sourceText.contains(QRegularExpression(QStringLiteral("<\\s*callout\\b")));
    if (!hasResourceFrame && !hasCalloutFrame)
    {
        QVariantMap result;
        result.insert(QStringLiteral("valid"), true);
        result.insert(QStringLiteral("changed"), false);
        result.insert(QStringLiteral("bodySourceText"), sourceText);
        result.insert(QStringLiteral("editorDocumentText"), editorDocumentText);
        result.insert(QStringLiteral("editorViewportWidth"), m_editorViewportWidth);
        result.insert(QStringLiteral("errorMessage"), QString());
        return result;
    }

    const QHash<QString, int> lockedFrameDisplayHeights =
        resourceFrameDisplayHeightsBySourceTag(editorDocumentText);
    const QString reprojectedEditorDocumentText = editorHtmlFromBodySourceForNoteContext(
        noteId,
        sourceText,
        m_activeNoteDirectoryPath,
        m_editorViewportWidth,
        &lockedFrameDisplayHeights);
    const bool changed = reprojectedEditorDocumentText != editorDocumentText;
    QString errorMessage;
    bool valid = true;

    if (changed
        && !m_editorFilePath.trimmed().isEmpty()
        && !writeEditorSourceFile(m_editorFilePath, reprojectedEditorDocumentText, &errorMessage))
    {
        valid = false;
        setLastError(errorMessage);
    }
    else if (valid)
    {
        if (hasActiveNote())
        {
            m_activeBodySourceText = sourceText;
        }
        setParsedLineCount(lineCountForEditorSource(sourceText));
        setLastError(QString());
    }

    QVariantMap result;
    result.insert(QStringLiteral("valid"), valid);
    result.insert(QStringLiteral("changed"), valid && changed);
    result.insert(QStringLiteral("bodySourceText"), sourceText);
    result.insert(QStringLiteral("editorDocumentText"), valid ? reprojectedEditorDocumentText : editorDocumentText);
    result.insert(QStringLiteral("editorViewportWidth"), m_editorViewportWidth);
    result.insert(QStringLiteral("errorMessage"), errorMessage);
    return result;
}

QVariantMap NoteEditorDocumentSession::insertImportedResourcesIntoSource(
    const QString& editorDocumentText,
    const int cursorPosition,
    const int selectionLength,
    const QVariantList& importedEntries)
{
    const QString noteId = m_activeNoteId.trimmed().isEmpty()
        ? QStringLiteral("note")
        : m_activeNoteId.trimmed();
    const QString activeSourceText =
        WhatSon::NoteBodyPersistence::normalizeBodyPlainText(m_activeBodySourceText);
    const QString sourceText = hasActiveNote() && !activeSourceText.isEmpty()
        ? activeSourceText
        : bodySourceTextForEditorDocument(noteId, editorDocumentText);
    const QVector<SourceVisibleCharacter> visibleCharacters = visibleCharactersForSourceText(sourceText);
    const int editorTextSize = visibleCharacters.size();
    const int editorAnchor = clampedPosition(cursorPosition, editorTextSize);
    const int editorActive = clampedPosition(cursorPosition + selectionLength, editorTextSize);
    const int editorSelectionStart = std::min(editorAnchor, editorActive);
    const int editorSelectionEnd = std::max(editorAnchor, editorActive);
    const int sourceSelectionStart = sourcePositionForEditorSelectionStart(sourceText, editorSelectionStart);
    const int sourceSelectionEnd = editorSelectionStart == editorSelectionEnd
        ? sourceSelectionStart
        : sourcePositionForEditorSelectionEnd(sourceText, editorSelectionEnd);

    QStringList resourceTags;
    resourceTags.reserve(importedEntries.size());

    for (const QVariant& importedEntry : importedEntries)
    {
        const QVariantMap importedResource = importedEntry.toMap();
        if (importedResource.isEmpty())
        {
            continue;
        }

        const QString resourceTag =
            WhatSon::NoteBodyResourceTagGenerator::buildCanonicalResourceTag(importedResource).trimmed();
        if (!resourceTag.isEmpty())
        {
            resourceTags.push_back(resourceTag);
        }
    }

    if (resourceTags.isEmpty())
    {
        const QString errorMessage = QStringLiteral("Imported resources did not produce note resource tags.");
        setLastError(errorMessage);
        return invalidImportedResourcesInsertionResult(
            noteId,
            sourceText,
            sourceSelectionStart,
            sourceSelectionEnd - sourceSelectionStart,
            editorAnchor,
            editorSelectionStart,
            editorSelectionEnd - editorSelectionStart,
            errorMessage);
    }

    const SourceInsertionRange insertionRange = resourceInsertionRangeForSourceSelection(
        sourceText,
        sourceSelectionStart,
        sourceSelectionEnd);
    const QString beforeSelection = sourceText.left(insertionRange.sourceStart);
    const QString afterSelection = sourceText.mid(insertionRange.sourceEnd);
    const QString insertedBlock = resourceTags.join(QLatin1Char('\n'));
    const QString prefix = !beforeSelection.isEmpty() && !beforeSelection.endsWith(QLatin1Char('\n'))
        ? QStringLiteral("\n")
        : QString();
    const QString suffix = !afterSelection.isEmpty() && !afterSelection.startsWith(QLatin1Char('\n'))
        ? QStringLiteral("\n")
        : QString();
    const QString insertedText = prefix + insertedBlock + suffix;
    const QString mutatedSourceText = beforeSelection + insertedText + afterSelection;
    const QString projectedEditorDocumentText = editorHtmlFromBodySourceForNoteContext(
        noteId,
        mutatedSourceText,
        m_activeNoteDirectoryPath,
        m_editorViewportWidth);
    const int sourceCursorPosition = beforeSelection.size() + prefix.size() + insertedBlock.size();

    setParsedLineCount(lineCountForEditorSource(mutatedSourceText));
    if (hasActiveNote())
    {
        m_activeBodySourceText = mutatedSourceText;
    }
    setLastError(QString());

    QVariantMap result;
    result.insert(QStringLiteral("valid"), true);
    result.insert(QStringLiteral("changed"), true);
    result.insert(QStringLiteral("bodySourceText"), mutatedSourceText);
    result.insert(QStringLiteral("editorDocumentText"), projectedEditorDocumentText);
    result.insert(
        QStringLiteral("cursorPosition"),
        editorCursorPositionForSourcePosition(mutatedSourceText, sourceCursorPosition));
    result.insert(QStringLiteral("sourceCursorPosition"), sourceCursorPosition);
    result.insert(QStringLiteral("selectionStart"), insertionRange.sourceStart);
    result.insert(QStringLiteral("selectionLength"), insertionRange.sourceEnd - insertionRange.sourceStart);
    result.insert(QStringLiteral("editorSelectionStart"), editorSelectionStart);
    result.insert(QStringLiteral("editorSelectionLength"), editorSelectionEnd - editorSelectionStart);
    result.insert(QStringLiteral("insertedText"), insertedText);
    result.insert(QStringLiteral("insertedCount"), resourceTags.size());
    result.insert(QStringLiteral("errorMessage"), QString());
    return result;
}

QVariantMap NoteEditorDocumentSession::insertFormatTagIntoSource(
    const QString& tagName,
    const QString& editorDocumentText,
    const int cursorPosition,
    const int selectionLength,
    const QString& selectedText)
{
    const QString noteId = m_activeNoteId.trimmed().isEmpty()
        ? QStringLiteral("note")
        : m_activeNoteId.trimmed();
    const QString activeSourceText =
        WhatSon::NoteBodyPersistence::normalizeBodyPlainText(m_activeBodySourceText);
    const QString sourceText = hasActiveNote() && !activeSourceText.isEmpty()
        ? activeSourceText
        : bodySourceTextForEditorDocument(noteId, editorDocumentText);
    const EditorSelectionRange editorSelectionRange = resolvedEditorSelectionRange(
        sourceText,
        cursorPosition,
        selectionLength,
        selectedText);
    const int sourceSelectionStart = sourcePositionForEditorSelectionStart(
        sourceText,
        editorSelectionRange.editorStart);
    const int sourceSelectionEnd = editorSelectionRange.editorStart == editorSelectionRange.editorEnd
        ? sourceSelectionStart
        : sourcePositionForEditorSelectionEnd(sourceText, editorSelectionRange.editorEnd);

    SetTag tagInput;
    QVariantMap result = tagInput.insertNamedTagIntoSource(
        tagName,
        sourceText,
        sourceSelectionStart,
        sourceSelectionEnd - sourceSelectionStart);

    const QString resultSourceText = result.value(QStringLiteral("bodySourceText")).toString();
    const QString editorHtml = editorHtmlFromBodySourceForNoteContext(
        noteId,
        resultSourceText,
        m_activeNoteDirectoryPath,
        m_editorViewportWidth);
    result.insert(QStringLiteral("editorDocumentText"), editorHtml);
    result.insert(QStringLiteral("sourceCursorPosition"), result.value(QStringLiteral("cursorPosition")).toInt());
    result.insert(QStringLiteral("editorSelectionStart"), editorSelectionRange.editorStart);
    result.insert(
        QStringLiteral("editorSelectionLength"),
        editorSelectionRange.editorEnd - editorSelectionRange.editorStart);

    if (!result.value(QStringLiteral("valid")).toBool())
    {
        setLastError(result.value(QStringLiteral("errorMessage")).toString());
        return result;
    }

    const int sourceCursorPosition = result.value(QStringLiteral("sourceCursorPosition")).toInt();
    const QString normalizedTagName = tagName.trimmed().toCaseFolded();
    const int editorCursorPosition = normalizedTagName == QStringLiteral("callout")
        ? WhatSon::EditorComponent::Callout::decoratedContentStartForVisibleCursor(
            editorHtml,
            visibleCursorMappingTextForSourceText(resultSourceText),
            editorCursorPositionForSourcePosition(resultSourceText, sourceCursorPosition))
        : editorCursorPositionForSourcePosition(resultSourceText, sourceCursorPosition);
    result.insert(QStringLiteral("cursorPosition"), editorCursorPosition);
    setParsedLineCount(lineCountForEditorSource(resultSourceText));
    if (hasActiveNote())
    {
        m_activeBodySourceText = resultSourceText;
    }
    setLastError(QString());
    return result;
}

QVariantMap NoteEditorDocumentSession::handleCalloutBoundaryKeyInSource(
    const QString& editorDocumentText,
    const int cursorPosition,
    const int selectionLength,
    const int key)
{
    const QString noteId = m_activeNoteId.trimmed().isEmpty()
        ? QStringLiteral("note")
        : m_activeNoteId.trimmed();
    const QString activeSourceText =
        WhatSon::NoteBodyPersistence::normalizeBodyPlainText(m_activeBodySourceText);
    const QString sourceText = hasActiveNote() && !activeSourceText.isEmpty()
        ? activeSourceText
        : bodySourceTextForEditorDocument(noteId, editorDocumentText);
    const QString sourceVisibleText = visibleCursorMappingTextForSourceText(sourceText);
    const int boundedDecoratedCursorPosition =
        clampedPosition(cursorPosition, plainTextForEditorDocumentText(editorDocumentText).size());
    const int boundedCursorPosition =
        clampedPosition(
            WhatSon::EditorComponent::Callout::sourceVisibleCursorForDecoratedCursor(
                editorDocumentText,
                sourceVisibleText,
                cursorPosition),
            sourceVisibleText.size());
    const int boundedSelectionLength = qMax(0, selectionLength);
    const auto sourceToVisibleCursor =
        [&sourceText](const int sourcePosition) -> int
        {
            return editorCursorPositionForSourcePosition(sourceText, sourcePosition);
        };

    const auto buildResult =
        [this, &noteId, &editorDocumentText](
            const bool handled,
            const bool changed,
            const QString& bodySourceText,
            const int sourceCursorPosition,
            const int editorCursorPosition,
            const QString& errorMessage = QString()) -> QVariantMap
        {
            const QString projectedEditorDocumentText = handled
                ? editorHtmlFromBodySourceForNoteContext(
                    noteId,
                    bodySourceText,
                    m_activeNoteDirectoryPath,
                    m_editorViewportWidth)
                : editorDocumentText;
            const int resolvedEditorCursorPosition = handled
                ? decoratedEditorCursorPositionForVisibleCursor(
                    projectedEditorDocumentText,
                    editorCursorPosition)
                : editorCursorPosition;

            QVariantMap result;
            result.insert(QStringLiteral("valid"), errorMessage.isEmpty());
            result.insert(QStringLiteral("handled"), handled);
            result.insert(QStringLiteral("changed"), handled && changed);
            result.insert(QStringLiteral("bodySourceText"), bodySourceText);
            result.insert(QStringLiteral("editorDocumentText"), projectedEditorDocumentText);
            result.insert(QStringLiteral("cursorPosition"), resolvedEditorCursorPosition);
            result.insert(QStringLiteral("sourceCursorPosition"), sourceCursorPosition);
            result.insert(QStringLiteral("selectionStart"), sourceCursorPosition);
            result.insert(QStringLiteral("selectionLength"), 0);
            result.insert(QStringLiteral("editorSelectionStart"), resolvedEditorCursorPosition);
            result.insert(QStringLiteral("editorSelectionLength"), 0);
            result.insert(QStringLiteral("errorMessage"), errorMessage);
            return result;
        };
    const auto applyCalloutEdit =
        [this, &buildResult](const WhatSon::EditorComponent::CalloutBoundaryEdit& edit) -> QVariantMap
        {
            if (edit.changed)
            {
                setParsedLineCount(lineCountForEditorSource(edit.bodySourceText));
                if (hasActiveNote())
                {
                    m_activeBodySourceText = edit.bodySourceText;
                }
            }
            setLastError(QString());
            return buildResult(
                true,
                edit.changed,
                edit.bodySourceText,
                edit.sourceCursorPosition,
                editorCursorPositionForSourcePosition(edit.bodySourceText, edit.sourceCursorPosition));
        };

    if (boundedSelectionLength > 0
        || (key != Qt::Key_Backspace && key != Qt::Key_Return && key != Qt::Key_Enter))
    {
        return buildResult(
            false,
            false,
            sourceText,
            sourcePositionForEditorSelectionStart(sourceText, boundedCursorPosition),
            boundedDecoratedCursorPosition);
    }

    if (key == Qt::Key_Backspace)
    {
        const auto edit = WhatSon::EditorComponent::Callout::backspaceAtVisibleContentStart(
            sourceText,
            boundedCursorPosition,
            sourceToVisibleCursor);
        if (!edit.has_value())
        {
            return buildResult(
                false,
                false,
                sourceText,
                sourcePositionForEditorSelectionStart(sourceText, boundedCursorPosition),
                boundedDecoratedCursorPosition);
        }
        return applyCalloutEdit(*edit);
    }

    const auto chromeBeforeContentEdit = WhatSon::EditorComponent::Callout::enterBeforeContentChrome(
        sourceText,
        editorDocumentText,
        sourceVisibleText,
        boundedDecoratedCursorPosition,
        sourceToVisibleCursor);
    if (chromeBeforeContentEdit.has_value())
    {
        return applyCalloutEdit(*chromeBeforeContentEdit);
    }

    const auto enterEdit = WhatSon::EditorComponent::Callout::enterInsideVisibleCursor(
        sourceText,
        boundedCursorPosition,
        sourceToVisibleCursor);
    if (!enterEdit.has_value())
    {
        return buildResult(
            false,
            false,
            sourceText,
            sourcePositionForEditorSelectionStart(sourceText, boundedCursorPosition),
            boundedDecoratedCursorPosition);
    }

    return applyCalloutEdit(*enterEdit);
}

void NoteEditorDocumentSession::refreshFromActiveNoteState()
{
    if (m_noteActiveState == nullptr || !m_noteActiveState->hasActiveNote())
    {
        clearEditor();
        return;
    }

    openNoteForEditing(
        m_noteActiveState->activeNoteId(),
        m_noteActiveState->activeNoteDirectoryPath());
}

void NoteEditorDocumentSession::refreshContentController()
{
    m_noteManagementCoordinator.setContentController(
        m_noteActiveState != nullptr ? m_noteActiveState->activeHierarchyController() : nullptr);
}

void NoteEditorDocumentSession::handleNoteBodyTextLoaded(
    const quint64 sequence,
    const QString& noteId,
    const QString& text,
    const bool success,
    const QString& errorMessage,
    const QString& lastModifiedAt)
{
    if (sequence == 0)
    {
        return;
    }

    if (sequence == m_pendingIdlePullSequence)
    {
        handleIdleNoteBodyTextLoaded(
            sequence,
            noteId,
            text,
            success,
            errorMessage,
            lastModifiedAt);
        return;
    }

    if (sequence != m_pendingLoadSequence)
    {
        return;
    }

    const QString loadedNoteId = noteId.trimmed();
    const QString loadedNoteDirectoryPath = m_pendingLoadNoteDirectoryPath;
    m_pendingLoadSequence = 0;
    m_pendingLoadNoteId.clear();
    m_pendingLoadNoteDirectoryPath.clear();
    setLoading(false);

    if (!success)
    {
        switchToBlankEditorFile();
        setActiveNoteContext(QString(), QString());
        setParsedLineCount(0);
        m_activeBodySourceText.clear();
        setReadOnly(true);
        setLastError(errorMessage.trimmed().isEmpty()
                         ? QStringLiteral("Failed to load note body text.")
                         : errorMessage.trimmed());
        return;
    }

    const QString bodySourceText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(text);
    if (!applyLoadedBodyTextToEditorSession(
            loadedNoteId,
            loadedNoteDirectoryPath,
            bodySourceText,
            lastModifiedAt,
            true,
            false))
    {
        switchToBlankEditorFile();
        setActiveNoteContext(QString(), QString());
        m_activeBodySourceText.clear();
        setReadOnly(true);
        return;
    }

    m_rawPullController.setActiveNoteForIdlePull(loadedNoteId, loadedNoteDirectoryPath);
    m_rawPullController.recordUserActivity();
}

bool NoteEditorDocumentSession::applyLoadedBodyTextToEditorSession(
    const QString& noteId,
    const QString& noteDirectoryPath,
    const QString& bodySourceText,
    const QString& loadedLastModifiedAt,
    const bool bindSelectedNote,
    const bool emitPulledDocumentText)
{
    const QString loadedNoteId = noteId.trimmed();
    const QString loadedNoteDirectoryPath = normalizePath(noteDirectoryPath);
    const QString sessionFilePath = editorFilePathForNote(loadedNoteId, loadedNoteDirectoryPath);
    QString writeError;
    const QString normalizedBodySourceText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(bodySourceText);
    const QString editorDocumentText = editorHtmlFromBodySourceForNoteContext(
        loadedNoteId,
        normalizedBodySourceText,
        loadedNoteDirectoryPath,
        m_editorViewportWidth);
    if (!writeEditorSourceFile(sessionFilePath, editorDocumentText, &writeError))
    {
        setLastError(writeError);
        return false;
    }

    m_editorFileContexts.insert(
        sessionFilePath,
        {loadedNoteId, loadedNoteDirectoryPath, loadedLastModifiedAt.trimmed()});
    setActiveNoteContext(loadedNoteId, loadedNoteDirectoryPath);
    m_activeBodySourceText = normalizedBodySourceText;
    setParsedLineCount(lineCountForEditorSource(normalizedBodySourceText));
    setLastError(QString());
    setReadOnly(false);
    setEditorFilePath(sessionFilePath);
    if (bindSelectedNote)
    {
        m_noteManagementCoordinator.bindSelectedNote(loadedNoteId, loadedNoteDirectoryPath);
    }
    if (emitPulledDocumentText)
    {
        emit editorDocumentTextPulled(loadedNoteId, editorDocumentText);
    }
    else
    {
        emit editorSourceLoaded(loadedNoteId, sessionFilePath);
    }
    return true;
}

void NoteEditorDocumentSession::handleIdleNoteBodyTextLoaded(
    const quint64 sequence,
    const QString& noteId,
    const QString& text,
    const bool success,
    const QString& errorMessage,
    const QString& lastModifiedAt)
{
    if (sequence == 0 || sequence != m_pendingIdlePullSequence)
    {
        return;
    }

    const QString loadedNoteId = noteId.trimmed();
    const QString loadedNoteDirectoryPath = m_pendingIdlePullNoteDirectoryPath;
    m_pendingIdlePullSequence = 0;
    m_pendingIdlePullNoteId.clear();
    m_pendingIdlePullNoteDirectoryPath.clear();

    if (!success)
    {
        setLastError(errorMessage.trimmed());
        emit editorFilesystemPullIgnored(loadedNoteId, QStringLiteral("load-failed"));
        return;
    }

    if (loadedNoteId != m_activeNoteId
        || loadedNoteDirectoryPath != m_activeNoteDirectoryPath)
    {
        emit editorFilesystemPullIgnored(loadedNoteId, QStringLiteral("inactive-note"));
        return;
    }

    const auto contextIterator = m_editorFileContexts.constFind(normalizePath(m_editorFilePath));
    const QString loadedContextTimestamp =
        contextIterator == m_editorFileContexts.constEnd()
            ? QString()
            : contextIterator->loadedLastModifiedAt;
    WhatSonTimestampConflictResolver timestampResolver;
    if (!timestampResolver.isTimestampNewer(lastModifiedAt, loadedContextTimestamp))
    {
        emit editorFilesystemPullIgnored(loadedNoteId, QStringLiteral("not-newer"));
        return;
    }

    applyLoadedBodyTextToEditorSession(
        loadedNoteId,
        loadedNoteDirectoryPath,
        text,
        lastModifiedAt,
        false,
        true);
}

void NoteEditorDocumentSession::handleEditorTextPersistenceFinished(
    const QString& noteId,
    const QString& text,
    const bool success,
    const QString& errorMessage)
{
    setLastError(success ? QString() : errorMessage);
    if (success && noteId == m_activeNoteId)
    {
        const QString canonicalSourceText = WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(
            WhatSon::NoteBodyPersistence::serializeBodyDocument(noteId, text));
        m_activeBodySourceText = canonicalSourceText;
        setParsedLineCount(lineCountForEditorSource(canonicalSourceText));
    }
    emit editorSourcePersistFinished(noteId, success, errorMessage);
}

void NoteEditorDocumentSession::handleNoteActiveStateDestroyed()
{
    m_noteActiveState = nullptr;
    emit noteActiveStateChanged();
    refreshContentController();
    clearEditor();
}

QString NoteEditorDocumentSession::sessionRootPath() const
{
    if (!m_sessionRootPathForTests.trimmed().isEmpty())
    {
        return m_sessionRootPathForTests;
    }

    QString basePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (basePath.trimmed().isEmpty())
    {
        basePath = QDir::tempPath() + QStringLiteral("/WhatSon");
    }
    return QDir(basePath).filePath(QStringLiteral("note-editor-sessions"));
}

QString NoteEditorDocumentSession::blankEditorFilePath() const
{
    return QDir(sessionRootPath()).filePath(QStringLiteral("_blank.wsnsource"));
}

QString NoteEditorDocumentSession::editorFilePathForNote(
    const QString& noteId,
    const QString& noteDirectoryPath) const
{
    const QString stem = sanitizeFileStem(noteId)
        + QLatin1Char('-')
        + shortHash(noteDirectoryPath);
    return QDir(sessionRootPath()).filePath(stem + QStringLiteral(".wsnsource"));
}

bool NoteEditorDocumentSession::ensureSessionRoot(QString* errorMessage) const
{
    const QString rootPath = sessionRootPath();
    if (rootPath.trimmed().isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Editor session root path is empty.");
        }
        return false;
    }

    QDir directory;
    if (!directory.mkpath(rootPath))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to create editor session directory: %1").arg(rootPath);
        }
        return false;
    }
    return true;
}

bool NoteEditorDocumentSession::writeEditorSourceFile(
    const QString& filePath,
    const QString& text,
    QString* errorMessage) const
{
    QString ensureError;
    if (!ensureSessionRoot(&ensureError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = ensureError;
        }
        return false;
    }

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to open editor source file for write: %1").arg(filePath);
        }
        return false;
    }

    const QByteArray encodedText = text.toUtf8();
    if (file.write(encodedText) != encodedText.size())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to write complete editor source file: %1").arg(filePath);
        }
        return false;
    }

    if (!file.commit())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to commit editor source file: %1").arg(filePath);
        }
        return false;
    }
    return true;
}

bool NoteEditorDocumentSession::readEditorSourceFile(
    const QString& filePath,
    QString* outText,
    QString* errorMessage) const
{
    if (outText != nullptr)
    {
        outText->clear();
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to open editor source file for read: %1").arg(filePath);
        }
        return false;
    }

    if (outText != nullptr)
    {
        *outText = QString::fromUtf8(file.readAll());
    }
    return true;
}

QString NoteEditorDocumentSession::bodySourceTextForEditorDocument(
    const QString& noteId,
    const QString& editorDocumentText) const
{
    const QString editorSourceText = WhatSon::NoteBodyPersistence::sourceTextFromEditorDocument(
        noteId,
        editorDocumentText);
    const QString activeSourceText =
        WhatSon::NoteBodyPersistence::normalizeBodyPlainText(m_activeBodySourceText);
    if (hasActiveNote() && !activeSourceText.isEmpty())
    {
        const QString restoredEditorSourceText = restoreResourceObjectPlaceholdersFromActiveSource(
            editorSourceText,
            activeSourceText);
        if (restoredEditorSourceText != editorSourceText
            && editorDocumentContainsLiveRenderedResourceFrame(editorDocumentText))
        {
            return restoredEditorSourceText;
        }
        if (visibleTextForSourceText(activeSourceText) == visibleTextForSourceText(editorSourceText))
        {
            return activeSourceText;
        }
    }
    return editorSourceText;
}

void NoteEditorDocumentSession::setEditorFilePath(const QString& editorFilePath)
{
    const QString normalizedEditorFilePath = normalizePath(editorFilePath);
    if (m_editorFilePath == normalizedEditorFilePath)
    {
        return;
    }

    m_editorFilePath = normalizedEditorFilePath;
    emit editorFilePathChanged();
}

void NoteEditorDocumentSession::setActiveNoteContext(
    const QString& noteId,
    const QString& noteDirectoryPath)
{
    const QString normalizedNoteId = noteId.trimmed();
    const QString normalizedNoteDirectoryPath = normalizePath(noteDirectoryPath);
    if (m_activeNoteId == normalizedNoteId
        && m_activeNoteDirectoryPath == normalizedNoteDirectoryPath)
    {
        return;
    }

    m_activeNoteId = normalizedNoteId;
    m_activeNoteDirectoryPath = normalizedNoteDirectoryPath;
    emit activeNoteChanged();
}

void NoteEditorDocumentSession::setParsedLineCount(const int parsedLineCount)
{
    const int normalizedLineCount = qMax(0, parsedLineCount);
    if (m_parsedLineCount == normalizedLineCount)
    {
        return;
    }

    m_parsedLineCount = normalizedLineCount;
    emit parsedLineCountChanged();
}

void NoteEditorDocumentSession::setLoading(const bool loading)
{
    if (m_loading == loading)
    {
        return;
    }

    m_loading = loading;
    emit loadingChanged();
}

void NoteEditorDocumentSession::setReadOnly(const bool readOnly)
{
    if (m_readOnly == readOnly)
    {
        return;
    }

    m_readOnly = readOnly;
    emit readOnlyChanged();
}

void NoteEditorDocumentSession::setLastError(const QString& lastError)
{
    if (m_lastError == lastError)
    {
        return;
    }

    m_lastError = lastError;
    emit lastErrorChanged();
}

void NoteEditorDocumentSession::switchToBlankEditorFile()
{
    const QString blankPath = blankEditorFilePath();
    QString writeError;
    if (!QFileInfo::exists(blankPath) && !writeEditorSourceFile(blankPath, QString(), &writeError))
    {
        setEditorFilePath(QString());
        if (!writeError.trimmed().isEmpty())
        {
            setLastError(writeError);
        }
        return;
    }
    setEditorFilePath(blankPath);
}
