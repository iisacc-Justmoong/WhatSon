#include "WhatSonNoteBodyPersistence.hpp"

#include "WhatSonLocalNoteFileStore.hpp"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QXmlStreamReader>

namespace
{
    struct BodyDocumentTextFragments final
    {
        QString fallbackText;
        QStringList blockLines;
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
        text.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
        return text;
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

                if (elementName.compare(QStringLiteral("br"), Qt::CaseInsensitive) == 0)
                {
                    if (blockDepth > 0)
                    {
                        currentBlockText += QLatin1Char('\n');
                    }
                    else if (!encounteredBlockElement)
                    {
                        fragments.fallbackText += QLatin1Char('\n');
                    }
                    continue;
                }

                if (isTextBlockElement(elementName))
                {
                    encounteredBlockElement = true;
                    if (blockDepth == 0)
                    {
                        currentBlockText.clear();
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
                if (blockDepth > 0)
                {
                    currentBlockText += text;
                }
                else if (!encounteredBlockElement || !text.trimmed().isEmpty())
                {
                    fragments.fallbackText += text;
                }
                continue;
            }

            if (!reader.isEndElement())
            {
                continue;
            }

            const QString elementName = reader.name().toString();
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
                currentBlockText.clear();
            }
        }

        fragments.fallbackText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(fragments.fallbackText);
        return fragments;
    }
} // namespace

namespace WhatSon::NoteBodyPersistence
{
    QString normalizeBodyPlainText(QString text)
    {
        text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
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

        const QString normalizedBodyText = normalizeBodyPlainText(bodyPlainText);
        if (normalizedBodyText == document.bodyPlainText)
        {
            if (outNormalizedBodyText != nullptr)
            {
                *outNormalizedBodyText = normalizedBodyText;
            }
            if (outLastModifiedAt != nullptr)
            {
                *outLastModifiedAt = document.headerStore.lastModifiedAt();
            }
            return true;
        }

        document.bodyPlainText = normalizedBodyText;

        WhatSonLocalNoteFileStore::UpdateRequest updateRequest;
        updateRequest.document = document;
        updateRequest.persistHeader = true;
        updateRequest.persistBody = true;
        updateRequest.touchLastModified = true;

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
        if (outLastModifiedAt != nullptr)
        {
            *outLastModifiedAt = document.headerStore.lastModifiedAt();
        }
        return true;
    }
} // namespace WhatSon::NoteBodyPersistence
