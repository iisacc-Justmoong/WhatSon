#include "app/models/editor/NoteEditorDocumentSession.hpp"

#include "app/models/editor/component/Break.h"
#include "app/models/editor/component/ResourceImageFrame.h"
#include "app/models/editor/SetTag.h"
#include "app/models/file/conflict/WhatSonTimestampConflictResolver.hpp"
#include "app/models/file/diff/WhatSonNoteVersionDiffBuilder.hpp"
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

    int visibleEditorCursorPositionForDecoratedCursor(
        const QString& editorDocumentText,
        const int decoratedCursorPosition)
    {
        const QString plainText = plainTextForEditorDocumentText(editorDocumentText);
        const int boundedCursorPosition = clampedPosition(decoratedCursorPosition, plainText.size());

        int visibleCursorPosition = 0;
        for (int index = 0; index < boundedCursorPosition; ++index)
        {
            if (plainText.at(index) != QChar::ObjectReplacementCharacter)
            {
                ++visibleCursorPosition;
            }
        }
        return visibleCursorPosition;
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

    int decoratedEditorContentStartForVisibleCursor(
        const QString& editorDocumentText,
        const int visibleCursorPosition)
    {
        const QString plainText = plainTextForEditorDocumentText(editorDocumentText);
        int decoratedPosition =
            decoratedEditorCursorPositionForVisibleCursor(editorDocumentText, visibleCursorPosition);
        while (decoratedPosition < plainText.size()
               && plainText.at(decoratedPosition) == QChar::ObjectReplacementCharacter)
        {
            ++decoratedPosition;
        }
        return decoratedPosition;
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

    struct CalloutSourceRange final
    {
        int openingStart = -1;
        int openingEnd = -1;
        int contentStart = -1;
        int contentEnd = -1;
        int closingStart = -1;
        int closingEnd = -1;

        bool isValid() const noexcept
        {
            return openingStart >= 0
                && openingEnd >= openingStart
                && contentStart == openingEnd
                && contentEnd >= contentStart
                && closingStart == contentEnd
                && closingEnd >= closingStart;
        }
    };

    QVector<CalloutSourceRange> calloutSourceRanges(const QString& bodySourceText)
    {
        struct OpeningCalloutTag final
        {
            int start = -1;
            int end = -1;
        };

        static const QRegularExpression calloutTagPattern(
            QStringLiteral(R"(<\s*(/?)\s*callout\b[^>]*>)"),
            QRegularExpression::CaseInsensitiveOption);

        QVector<OpeningCalloutTag> openingStack;
        QVector<CalloutSourceRange> ranges;
        QRegularExpressionMatchIterator matchIterator = calloutTagPattern.globalMatch(bodySourceText);
        while (matchIterator.hasNext())
        {
            const QRegularExpressionMatch match = matchIterator.next();
            const QString token = match.captured(0).trimmed();
            const bool closingTag = !match.captured(1).isEmpty();
            const bool selfClosingTag = token.endsWith(QStringLiteral("/>"));
            if (!closingTag && !selfClosingTag)
            {
                openingStack.push_back({
                    static_cast<int>(match.capturedStart(0)),
                    static_cast<int>(match.capturedEnd(0))
                });
                continue;
            }
            if (!closingTag || openingStack.isEmpty())
            {
                continue;
            }

            const OpeningCalloutTag opening = openingStack.takeLast();
            ranges.push_back({
                opening.start,
                opening.end,
                opening.end,
                static_cast<int>(match.capturedStart(0)),
                static_cast<int>(match.capturedStart(0)),
                static_cast<int>(match.capturedEnd(0))
            });
        }

        std::sort(
            ranges.begin(),
            ranges.end(),
            [](const CalloutSourceRange& left, const CalloutSourceRange& right)
            {
                return left.openingStart < right.openingStart;
            });
        return ranges;
    }

    bool calloutRangeContainsEditorCursor(
        const QString& bodySourceText,
        const CalloutSourceRange& range,
        const int editorCursorPosition)
    {
        if (!range.isValid())
        {
            return false;
        }

        const int contentEditorStart = editorCursorPositionForSourcePosition(bodySourceText, range.contentStart);
        const int contentEditorEnd = editorCursorPositionForSourcePosition(bodySourceText, range.contentEnd);
        return editorCursorPosition >= contentEditorStart && editorCursorPosition <= contentEditorEnd;
    }

    bool calloutRangeStartsAtEditorCursor(
        const QString& bodySourceText,
        const CalloutSourceRange& range,
        const int editorCursorPosition)
    {
        return range.isValid()
            && editorCursorPosition == editorCursorPositionForSourcePosition(bodySourceText, range.contentStart);
    }

    bool findCalloutRangeForEditorCursor(
        const QString& bodySourceText,
        const int editorCursorPosition,
        const bool requireInitCursor,
        CalloutSourceRange* outRange)
    {
        const QVector<CalloutSourceRange> ranges = calloutSourceRanges(bodySourceText);
        for (const CalloutSourceRange& range : ranges)
        {
            const bool matches = requireInitCursor
                ? calloutRangeStartsAtEditorCursor(bodySourceText, range, editorCursorPosition)
                : calloutRangeContainsEditorCursor(bodySourceText, range, editorCursorPosition);
            if (!matches)
            {
                continue;
            }
            if (outRange != nullptr)
            {
                *outRange = range;
            }
            return true;
        }
        return false;
    }

    bool calloutChromeBeforeContentContainsDecoratedCursor(
        const QString& editorDocumentText,
        const QString& bodySourceText,
        const CalloutSourceRange& range,
        const int decoratedCursorPosition)
    {
        if (!range.isValid())
        {
            return false;
        }

        const int contentVisibleCursorPosition =
            editorCursorPositionForSourcePosition(bodySourceText, range.contentStart);
        const int decoratedFrameStart =
            decoratedEditorCursorPositionForVisibleCursor(editorDocumentText, contentVisibleCursorPosition);
        const int decoratedContentStart =
            decoratedEditorContentStartForVisibleCursor(editorDocumentText, contentVisibleCursorPosition);
        return decoratedContentStart > decoratedFrameStart
            && decoratedCursorPosition >= decoratedFrameStart
            && decoratedCursorPosition < decoratedContentStart;
    }

    bool findCalloutRangeForEditorChromeBeforeContent(
        const QString& editorDocumentText,
        const QString& bodySourceText,
        const int decoratedCursorPosition,
        CalloutSourceRange* outRange)
    {
        const QVector<CalloutSourceRange> ranges = calloutSourceRanges(bodySourceText);
        for (const CalloutSourceRange& range : ranges)
        {
            if (!calloutChromeBeforeContentContainsDecoratedCursor(
                    editorDocumentText,
                    bodySourceText,
                    range,
                    decoratedCursorPosition))
            {
                continue;
            }
            if (outRange != nullptr)
            {
                *outRange = range;
            }
            return true;
        }
        return false;
    }

    void expandEmptyCalloutRemovalToSourceLine(
        const QString& bodySourceText,
        int* removeStart,
        int* removeEnd)
    {
        if (removeStart == nullptr || removeEnd == nullptr)
        {
            return;
        }

        const bool hasPreviousLineBreak =
            *removeStart > 0 && bodySourceText.at(*removeStart - 1) == QLatin1Char('\n');
        const bool hasNextLineBreak =
            *removeEnd < bodySourceText.size() && bodySourceText.at(*removeEnd) == QLatin1Char('\n');
        if (hasPreviousLineBreak && hasNextLineBreak)
        {
            ++(*removeEnd);
            return;
        }
        if (hasPreviousLineBreak)
        {
            --(*removeStart);
            return;
        }
        if (hasNextLineBreak)
        {
            ++(*removeEnd);
        }
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
                WhatSon::NoteBodyPersistence::htmlProjectionFromBodyDocument(
                    WhatSon::NoteBodyPersistence::serializeBodyDocument(
                        noteId,
                        pendingSourceLines.join(QLatin1Char('\n'))),
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
        return WhatSon::NoteBodyPersistence::editorHtmlDocumentFromProjection(
            htmlLines.join(QStringLiteral("<br/>")));
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

    void appendRestoredResourceObjectLineParts(
        QStringList* restoredLines,
        const QString& editorLine,
        const QVector<ActiveResourceSourceLine>& resourceLines,
        int* resourceIndex,
        bool* skipNextPaddingLineAfterResource)
    {
        if (restoredLines == nullptr
            || resourceIndex == nullptr
            || skipNextPaddingLineAfterResource == nullptr)
        {
            return;
        }

        int segmentStart = 0;
        while (segmentStart <= editorLine.size())
        {
            const int objectIndex = editorLine.indexOf(QChar::ObjectReplacementCharacter, segmentStart);
            if (objectIndex < 0)
            {
                const QString trailingText = editorLine.mid(segmentStart);
                if (!trailingText.trimmed().isEmpty())
                {
                    restoredLines->push_back(trailingText);
                    *skipNextPaddingLineAfterResource = false;
                }
                return;
            }

            const QString leadingText = editorLine.mid(segmentStart, objectIndex - segmentStart);
            if (!leadingText.trimmed().isEmpty())
            {
                restoredLines->push_back(leadingText);
            }

            if (*resourceIndex < resourceLines.size())
            {
                const ActiveResourceSourceLine& resourceLine = resourceLines.at(*resourceIndex);
                restoredLines->push_back(resourceLine.sourceTag);
                *skipNextPaddingLineAfterResource = !resourceLine.hasBlankAfter;
                ++(*resourceIndex);
            }

            segmentStart = objectIndex + 1;
        }
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
                appendRestoredResourceObjectLineParts(
                    &restoredLines,
                    editorLine,
                    resourceLines,
                    &resourceIndex,
                    &skipNextPaddingLineAfterResource);
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
            const QString& bodySourceText,
            const bool hasBodySourceText,
            const QString&,
            QString* errorMessage) -> bool
        {
            const bool pushed = hasBodySourceText
                ? persistBodySourceTextForEditorFile(editorFilePath, bodySourceText)
                : persistBodySourceTextForEditorFile(editorFilePath, m_activeBodySourceText);
            if (!pushed && errorMessage != nullptr)
            {
                *errorMessage = m_lastError.trimmed().isEmpty()
                    ? QStringLiteral("Failed to push RAW source.")
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
    m_activeBodySourceDirtyForEditorSession = false;

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
    m_activeBodySourceDirtyForEditorSession = false;
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

bool NoteEditorDocumentSession::markEditorSessionFileReadyForRawPush(const QString& editorFilePath)
{
    const QString normalizedEditorFilePath = normalizePath(editorFilePath);
    if (normalizedEditorFilePath.isEmpty())
    {
        return false;
    }

    auto contextIterator = m_editorFileContexts.find(normalizedEditorFilePath);
    if (contextIterator == m_editorFileContexts.end()
        || contextIterator->noteId.trimmed().isEmpty()
        || contextIterator->noteDirectoryPath.trimmed().isEmpty())
    {
        return false;
    }

    contextIterator->readyForRawPush = true;
    return true;
}

void NoteEditorDocumentSession::requestEditorIdleRawPush(
    const QString& editorFilePath,
    const QString& editorDocumentText)
{
    QString bodySourceText;
    if (!prepareBodySourceTextForRawPush(
            editorFilePath,
            editorDocumentText,
            QStringLiteral("idle"),
            &bodySourceText))
    {
        return;
    }
    m_rawPushController.requestIdlePush(editorFilePath, bodySourceText);
}

void NoteEditorDocumentSession::requestEditorModifiedCountRawPush(
    const QString& editorFilePath,
    const int modifiedCount,
    const QString& editorDocumentText)
{
    recordEditorUserActivity();
    QString bodySourceText;
    if (!prepareBodySourceTextForRawPush(
            editorFilePath,
            editorDocumentText,
            QStringLiteral("modified-count"),
            &bodySourceText))
    {
        return;
    }
    m_rawPushController.requestModifiedCountPush(
        editorFilePath,
        modifiedCount,
        bodySourceText);
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

    const QString sourceText = bodySourceTextForEditorDocument(
        contextIterator->noteId,
        editorDocumentText);
    return persistBodySourceTextForEditorFile(normalizedEditorFilePath, sourceText);
}

bool NoteEditorDocumentSession::persistBodySourceTextForEditorFile(
    const QString& editorFilePath,
    const QString& bodySourceText)
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

    const QString sourceText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(bodySourceText);
    setParsedLineCount(lineCountForEditorSource(sourceText));

    emit editorSourcePersistRequested(contextIterator->noteId, normalizedEditorFilePath);
    const bool enqueued = m_noteManagementCoordinator.persistEditorTextForNoteAtPath(
        contextIterator->noteId,
        contextIterator->noteDirectoryPath,
        sourceText,
        contextIterator->loadedLastModifiedAt,
        contextIterator->loadedBodySourceText,
        true);
    if (!enqueued)
    {
        const QString error = QStringLiteral("Failed to enqueue note body persistence.");
        setLastError(error);
        emit editorSourcePersistFinished(contextIterator->noteId, false, error);
    }
    else
    {
        if (contextIterator->noteId == m_activeNoteId
            && contextIterator->noteDirectoryPath == m_activeNoteDirectoryPath)
        {
            m_activeBodySourceText = sourceText;
            m_activeBodySourceDirtyForEditorSession = true;
        }
    }
    return enqueued;
}

bool NoteEditorDocumentSession::prepareBodySourceTextForRawPush(
    const QString& editorFilePath,
    const QString& editorDocumentText,
    const QString& reason,
    QString* bodySourceText)
{
    if (bodySourceText != nullptr)
    {
        bodySourceText->clear();
    }

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

    if (!contextIterator->readyForRawPush)
    {
        setLastError(QStringLiteral("Editor session file is not ready for RAW push."));
        return false;
    }

    const QString sourceText = bodySourceTextForEditorDocument(
        contextIterator->noteId,
        editorDocumentText);
    const bool activeContext =
        contextIterator->noteId == m_activeNoteId
        && contextIterator->noteDirectoryPath == m_activeNoteDirectoryPath;
    const QString activeSourceText = activeContext
        ? WhatSon::NoteBodyPersistence::normalizeBodyPlainText(m_activeBodySourceText)
        : QString();
    if (sourceText.trimmed().isEmpty()
        && !activeSourceText.trimmed().isEmpty()
        && (editorDocumentText.trimmed().isEmpty()
            || (reason == QStringLiteral("idle")
                && plainTextForEditorDocumentText(editorDocumentText).trimmed().isEmpty())))
    {
        setLastError(
            QStringLiteral("Ignored transient empty editor payload for RAW push (%1).").arg(reason));
        return false;
    }

    if (activeContext)
    {
        m_activeBodySourceText = sourceText;
        m_activeBodySourceDirtyForEditorSession = true;
        setParsedLineCount(lineCountForEditorSource(sourceText));
    }
    setLastError(QString());

    if (bodySourceText != nullptr)
    {
        *bodySourceText = sourceText;
    }
    return true;
}

bool NoteEditorDocumentSession::stageActiveSourceMutationForCurrentEditorFile(
    const QString& bodySourceText,
    const QString& editorDocumentText,
    QString* errorMessage)
{
    if (!hasActiveNote())
    {
        return true;
    }

    const QString normalizedEditorFilePath = normalizePath(m_editorFilePath);
    if (normalizedEditorFilePath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Editor file path is empty.");
        }
        return false;
    }

    if (!writeEditorSourceFile(normalizedEditorFilePath, editorDocumentText, errorMessage))
    {
        return false;
    }

    m_rawPushController.discardPendingPushForFile(normalizedEditorFilePath);
    m_activeBodySourceText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(bodySourceText);
    m_activeBodySourceDirtyForEditorSession = true;
    return true;
}

bool NoteEditorDocumentSession::pushActiveEditorBeforeNoteDeparture()
{
    const QString normalizedEditorFilePath = normalizePath(m_editorFilePath);
    if (normalizedEditorFilePath.isEmpty())
    {
        return true;
    }

    if (m_activeBodySourceDirtyForEditorSession && hasActiveNote())
    {
        const QString activeSourceText =
            WhatSon::NoteBodyPersistence::normalizeBodyPlainText(m_activeBodySourceText);
        const QString editorDocumentText = editorHtmlFromBodySourceForNoteContext(
            m_activeNoteId,
            activeSourceText,
            m_activeNoteDirectoryPath,
            m_editorViewportWidth);
        QString writeError;
        if (!writeEditorSourceFile(normalizedEditorFilePath, editorDocumentText, &writeError))
        {
            setLastError(writeError);
            return false;
        }

        m_rawPushController.discardPendingPushForFile(normalizedEditorFilePath);
        return persistBodySourceTextForEditorFile(normalizedEditorFilePath, activeSourceText);
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
    const QString sourceText = bodySourceTextForEditorDocument(noteId, editorDocumentText);
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

    const QString beforeSelection = sourceText.left(sourceSelectionStart);
    const QString afterSelection = sourceText.mid(sourceSelectionEnd);
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

    QString stageError;
    if (!stageActiveSourceMutationForCurrentEditorFile(
            mutatedSourceText,
            projectedEditorDocumentText,
            &stageError))
    {
        const QString errorMessage = stageError.trimmed().isEmpty()
            ? QStringLiteral("Failed to stage imported resources in the editor session file.")
            : stageError.trimmed();
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

    setParsedLineCount(lineCountForEditorSource(mutatedSourceText));
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
    result.insert(QStringLiteral("selectionStart"), sourceSelectionStart);
    result.insert(QStringLiteral("selectionLength"), sourceSelectionEnd - sourceSelectionStart);
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

    result.insert(
        QStringLiteral("cursorPosition"),
        editorCursorPositionForSourcePosition(
            resultSourceText,
            result.value(QStringLiteral("sourceCursorPosition")).toInt()));
    setParsedLineCount(lineCountForEditorSource(resultSourceText));
    if (hasActiveNote())
    {
        m_activeBodySourceText = resultSourceText;
        m_activeBodySourceDirtyForEditorSession = true;
        m_rawPushController.discardPendingPushForFile(m_editorFilePath);
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
    const int boundedDecoratedCursorPosition =
        clampedPosition(cursorPosition, plainTextForEditorDocumentText(editorDocumentText).size());
    const int boundedCursorPosition =
        clampedPosition(
            visibleEditorCursorPositionForDecoratedCursor(editorDocumentText, cursorPosition),
            visibleCharactersForSourceText(sourceText).size());
    const int boundedSelectionLength = qMax(0, selectionLength);

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

    CalloutSourceRange range;
    if (key == Qt::Key_Backspace)
    {
        if (!findCalloutRangeForEditorCursor(sourceText, boundedCursorPosition, true, &range))
        {
            return buildResult(
                false,
                false,
                sourceText,
                sourcePositionForEditorSelectionStart(sourceText, boundedCursorPosition),
                boundedDecoratedCursorPosition);
        }

        QString mutatedSourceText;
        int sourceCursorPosition = range.openingStart;
        if (range.contentStart == range.contentEnd)
        {
            int removeStart = range.openingStart;
            int removeEnd = range.closingEnd;
            expandEmptyCalloutRemovalToSourceLine(sourceText, &removeStart, &removeEnd);
            mutatedSourceText = sourceText.left(removeStart) + sourceText.mid(removeEnd);
            sourceCursorPosition = removeStart;
        }
        else
        {
            mutatedSourceText =
                sourceText.left(range.openingStart)
                + sourceText.mid(range.contentStart, range.contentEnd - range.contentStart)
                + sourceText.mid(range.closingEnd);
        }

        sourceCursorPosition = clampedPosition(sourceCursorPosition, mutatedSourceText.size());
        const int editorCursorPosition = editorCursorPositionForSourcePosition(
            mutatedSourceText,
            sourceCursorPosition);
        setParsedLineCount(lineCountForEditorSource(mutatedSourceText));
        if (hasActiveNote())
        {
            m_activeBodySourceText = mutatedSourceText;
            m_activeBodySourceDirtyForEditorSession = true;
            m_rawPushController.discardPendingPushForFile(m_editorFilePath);
        }
        setLastError(QString());
        return buildResult(true, mutatedSourceText != sourceText, mutatedSourceText, sourceCursorPosition, editorCursorPosition);
    }

    if (findCalloutRangeForEditorChromeBeforeContent(
            editorDocumentText,
            sourceText,
            boundedDecoratedCursorPosition,
            &range))
    {
        QString mutatedSourceText = sourceText;
        int sourceCursorPosition = range.openingStart;
        mutatedSourceText.insert(sourceCursorPosition, QLatin1Char('\n'));
        ++sourceCursorPosition;

        setParsedLineCount(lineCountForEditorSource(mutatedSourceText));
        if (hasActiveNote())
        {
            m_activeBodySourceText = mutatedSourceText;
            m_activeBodySourceDirtyForEditorSession = true;
            m_rawPushController.discardPendingPushForFile(m_editorFilePath);
        }
        setLastError(QString());
        return buildResult(
            true,
            true,
            mutatedSourceText,
            sourceCursorPosition,
            editorCursorPositionForSourcePosition(mutatedSourceText, sourceCursorPosition));
    }

    if (!findCalloutRangeForEditorCursor(sourceText, boundedCursorPosition, false, &range))
    {
        return buildResult(
            false,
            false,
            sourceText,
            sourcePositionForEditorSelectionStart(sourceText, boundedCursorPosition),
            boundedDecoratedCursorPosition);
    }

    QString mutatedSourceText = sourceText;
    int sourceCursorPosition = range.closingEnd;
    if (sourceCursorPosition < mutatedSourceText.size()
        && mutatedSourceText.at(sourceCursorPosition) == QLatin1Char('\n'))
    {
        ++sourceCursorPosition;
    }
    else
    {
        mutatedSourceText.insert(sourceCursorPosition, QLatin1Char('\n'));
        ++sourceCursorPosition;
    }

    const bool changed = mutatedSourceText != sourceText;
    if (changed)
    {
        setParsedLineCount(lineCountForEditorSource(mutatedSourceText));
        if (hasActiveNote())
        {
            m_activeBodySourceText = mutatedSourceText;
            m_activeBodySourceDirtyForEditorSession = true;
            m_rawPushController.discardPendingPushForFile(m_editorFilePath);
        }
    }
    setLastError(QString());
    return buildResult(
        true,
        changed,
        mutatedSourceText,
        sourceCursorPosition,
        editorCursorPositionForSourcePosition(mutatedSourceText, sourceCursorPosition));
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
        m_activeBodySourceDirtyForEditorSession = false;
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

    const auto existingContextIterator = m_editorFileContexts.constFind(sessionFilePath);
    const bool readyForRawPush =
        emitPulledDocumentText
        && existingContextIterator != m_editorFileContexts.constEnd()
        && existingContextIterator->readyForRawPush;
    m_editorFileContexts.insert(
        sessionFilePath,
        {
            loadedNoteId,
            loadedNoteDirectoryPath,
            loadedLastModifiedAt.trimmed(),
            normalizedBodySourceText,
            readyForRawPush
        });
    setActiveNoteContext(loadedNoteId, loadedNoteDirectoryPath);
    m_activeBodySourceText = normalizedBodySourceText;
    m_activeBodySourceDirtyForEditorSession = false;
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

    const QString normalizedEditorFilePath = normalizePath(m_editorFilePath);
    const auto contextIterator = m_editorFileContexts.constFind(normalizedEditorFilePath);
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

    const bool hadLocalDirty =
        m_activeBodySourceDirtyForEditorSession
        || m_rawPushController.hasPendingPushForFile(normalizedEditorFilePath);
    const QString baseBodySourceText =
        contextIterator == m_editorFileContexts.constEnd()
            ? WhatSon::NoteBodyPersistence::normalizeBodyPlainText(m_activeBodySourceText)
            : WhatSon::NoteBodyPersistence::normalizeBodyPlainText(contextIterator->loadedBodySourceText);
    const QString filesystemBodySourceText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(text);
    const QString activeBodySourceText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(m_activeBodySourceText);

    WhatSonNoteVersionDiffBuilder diffBuilder;
    const WhatSonNoteVersionDiffSegment pullDiff = diffBuilder.diffSegment(
        baseBodySourceText,
        filesystemBodySourceText,
        QStringLiteral("body.wsnbody"));
    bool diffApplied = false;
    QString diffApplyError;
    const QString mergedBodySourceText = diffBuilder.applyDiffSegmentOntoCurrent(
        baseBodySourceText,
        activeBodySourceText,
        pullDiff,
        &diffApplied,
        &diffApplyError);
    if (!diffApplied)
    {
        setLastError(diffApplyError);
        emit editorFilesystemPullIgnored(loadedNoteId, QStringLiteral("diff-conflict"));
        return;
    }

    const QString editorDocumentText = editorHtmlFromBodySourceForNoteContext(
        loadedNoteId,
        mergedBodySourceText,
        loadedNoteDirectoryPath,
        m_editorViewportWidth);
    QString writeError;
    if (!writeEditorSourceFile(normalizedEditorFilePath, editorDocumentText, &writeError))
    {
        setLastError(writeError);
        emit editorFilesystemPullIgnored(loadedNoteId, QStringLiteral("write-failed"));
        return;
    }

    m_rawPushController.discardPendingPushForFile(normalizedEditorFilePath);
    auto mutableContextIterator = m_editorFileContexts.find(normalizedEditorFilePath);
    if (mutableContextIterator != m_editorFileContexts.end())
    {
        mutableContextIterator->loadedLastModifiedAt = lastModifiedAt.trimmed();
        mutableContextIterator->loadedBodySourceText = filesystemBodySourceText;
    }
    m_activeBodySourceText = mergedBodySourceText;
    m_activeBodySourceDirtyForEditorSession =
        hadLocalDirty && mergedBodySourceText != filesystemBodySourceText;
    setParsedLineCount(lineCountForEditorSource(mergedBodySourceText));
    setLastError(QString());
    emit editorDocumentTextPulled(loadedNoteId, editorDocumentText);
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
        m_activeBodySourceDirtyForEditorSession = false;
        auto contextIterator = m_editorFileContexts.find(normalizePath(m_editorFilePath));
        if (contextIterator != m_editorFileContexts.end())
        {
            contextIterator->loadedBodySourceText = canonicalSourceText;
        }
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
        if (visibleTextForSourceText(activeSourceText) == visibleTextForSourceText(editorSourceText))
        {
            return activeSourceText;
        }
        return restoreResourceObjectPlaceholdersFromActiveSource(
            editorSourceText,
            activeSourceText);
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
