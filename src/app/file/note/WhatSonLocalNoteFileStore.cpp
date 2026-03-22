#include "WhatSonLocalNoteFileStore.hpp"

#include "WhatSonDebugTrace.hpp"
#include "WhatSonNoteBodyPersistence.hpp"
#include "WhatSonNoteHeaderCreator.hpp"
#include "WhatSonNoteHeaderParser.hpp"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QUrl>

#include <utility>

namespace
{
    constexpr auto kNoteTimestampFormat = "yyyy-MM-dd-hh-mm-ss";

    QString escapeXmlText(QString value)
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

    QString normalizeBodyLine(QString text)
    {
        text.replace(QRegularExpression(QStringLiteral(R"(\s+)")), QStringLiteral(" "));
        return text.trimmed();
    }

    QString serializeBodyDocument(const QString& noteId, const QString& plainText)
    {
        const QString normalizedId = noteId.trimmed().isEmpty()
                                         ? QStringLiteral("note")
                                         : escapeXmlText(noteId.trimmed());
        const QStringList lines = plainText.split(QLatin1Char('\n'), Qt::KeepEmptyParts);

        QString text;
        text += QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        text += QStringLiteral("<!DOCTYPE WHATSONNOTE>\n");
        text += QStringLiteral("<contents id=\"") + normalizedId + QStringLiteral("\">\n");
        text += QStringLiteral("  <body>\n");
        if (plainText.isEmpty())
        {
            text += QStringLiteral("  </body>\n");
            text += QStringLiteral("</contents>\n");
            return text;
        }

        for (const QString& line : lines)
        {
            text += QStringLiteral("    <paragraph>") + escapeXmlText(line) + QStringLiteral("</paragraph>\n");
        }
        text += QStringLiteral("  </body>\n");
        text += QStringLiteral("</contents>\n");
        return text;
    }

    QString findAncestorDirectoryWithSuffix(const QString& startPath, const QString& suffix)
    {
        if (startPath.trimmed().isEmpty())
        {
            return {};
        }

        const QFileInfo startInfo(startPath);
        QDir currentDirectory = startInfo.isDir() ? QDir(startInfo.absoluteFilePath()) : startInfo.absoluteDir();
        while (currentDirectory.exists())
        {
            if (currentDirectory.dirName().endsWith(suffix, Qt::CaseInsensitive))
            {
                return QDir::cleanPath(currentDirectory.absolutePath());
            }
            if (!currentDirectory.cdUp())
            {
                break;
            }
        }
        return {};
    }

    QString extractXmlAttributeValue(const QString& tagText, const QStringList& attributeNames)
    {
        for (const QString& attributeName : attributeNames)
        {
            const QRegularExpression attributePattern(
                QStringLiteral(R"ATTR(\b%1\s*=\s*(?:"([^"]*)"|'([^']*)'|([^\s/>]+)))ATTR")
                .arg(QRegularExpression::escape(attributeName)),
                QRegularExpression::CaseInsensitiveOption);
            const QRegularExpressionMatch match = attributePattern.match(tagText);
            if (!match.hasMatch())
            {
                continue;
            }

            for (int captureIndex = 1; captureIndex <= 3; ++captureIndex)
            {
                const QString value = decodeXmlEntities(match.captured(captureIndex)).trimmed();
                if (!value.isEmpty())
                {
                    return value;
                }
            }
        }

        return {};
    }

    QString resolveResourceLocation(const QString& rawResourcePath, const WhatSonLocalNoteDocument& document)
    {
        const QString normalizedPath = decodeXmlEntities(rawResourcePath).trimmed();
        if (normalizedPath.isEmpty())
        {
            return {};
        }

        if (QFileInfo(normalizedPath).isAbsolute())
        {
            const QString absolutePath = QDir::cleanPath(normalizedPath);
            return QFileInfo(absolutePath).isFile() ? absolutePath : QString();
        }

        const QUrl resourceUrl(normalizedPath);
        if (resourceUrl.isValid() && !resourceUrl.scheme().isEmpty())
        {
            if (!resourceUrl.isLocalFile())
            {
                return resourceUrl.toString();
            }

            const QString localFilePath = QDir::cleanPath(resourceUrl.toLocalFile());
            return QFileInfo(localFilePath).isFile() ? localFilePath : QString();
        }

        QStringList candidates;
        if (!document.noteDirectoryPath.trimmed().isEmpty())
        {
            candidates.push_back(QDir(document.noteDirectoryPath).absoluteFilePath(normalizedPath));
        }
        if (!document.noteHeaderPath.trimmed().isEmpty())
        {
            candidates.push_back(
                QDir(QFileInfo(document.noteHeaderPath).absolutePath()).absoluteFilePath(normalizedPath));
        }

        const QString hubRootPath = findAncestorDirectoryWithSuffix(
            !document.noteDirectoryPath.trimmed().isEmpty() ? document.noteDirectoryPath : document.noteHeaderPath,
            QStringLiteral(".wshub"));
        if (!hubRootPath.isEmpty())
        {
            candidates.push_back(QDir(hubRootPath).absoluteFilePath(normalizedPath));
            candidates.push_back(QDir(QFileInfo(hubRootPath).absolutePath()).absoluteFilePath(normalizedPath));
        }

        candidates.removeDuplicates();
        for (const QString& candidate : std::as_const(candidates))
        {
            const QString cleanedCandidate = QDir::cleanPath(candidate);
            if (QFileInfo(cleanedCandidate).isFile())
            {
                return cleanedCandidate;
            }
        }

        return {};
    }

    bool isPreviewableImageResource(const QString& resolvedResourceLocation, const QString& resourceFormat)
    {
        QString suffix;
        if (QFileInfo(resolvedResourceLocation).isAbsolute())
        {
            suffix = QFileInfo(resolvedResourceLocation).suffix().toCaseFolded();
        }
        else
        {
            const QUrl resourceUrl(resolvedResourceLocation);
            if (resourceUrl.isValid() && !resourceUrl.scheme().isEmpty())
            {
                suffix = QFileInfo(resourceUrl.path()).suffix().toCaseFolded();
            }
            else
            {
                suffix = QFileInfo(resolvedResourceLocation).suffix().toCaseFolded();
            }
        }

        if (suffix.isEmpty())
        {
            suffix = resourceFormat.trimmed().toCaseFolded();
            if (suffix.startsWith(QLatin1Char('.')))
            {
                suffix.remove(0, 1);
            }
        }

        static const QStringList kPreviewableImageExtensions = {
            QStringLiteral("png"),
            QStringLiteral("jpg"),
            QStringLiteral("jpeg"),
            QStringLiteral("webp"),
            QStringLiteral("bmp"),
            QStringLiteral("gif"),
            QStringLiteral("svg")
        };
        return !suffix.isEmpty() && kPreviewableImageExtensions.contains(suffix);
    }

    QString normalizeThumbnailSource(const QString& resolvedResourceLocation)
    {
        if (resolvedResourceLocation.isEmpty())
        {
            return {};
        }

        if (QFileInfo(resolvedResourceLocation).isAbsolute())
        {
            return QUrl::fromLocalFile(QDir::cleanPath(resolvedResourceLocation)).toString();
        }

        const QUrl resourceUrl(resolvedResourceLocation);
        if (resourceUrl.isValid() && !resourceUrl.scheme().isEmpty())
        {
            if (!resourceUrl.isLocalFile())
            {
                return resourceUrl.toString();
            }
            return QUrl::fromLocalFile(QDir::cleanPath(resourceUrl.toLocalFile())).toString();
        }
        return {};
    }
} // namespace

WhatSonLocalNoteFileStore::WhatSonLocalNoteFileStore() = default;

WhatSonLocalNoteFileStore::~WhatSonLocalNoteFileStore() = default;

QString WhatSonLocalNoteFileStore::normalizePath(QString path) const
{
    path = path.trimmed();
    if (path.isEmpty())
    {
        return {};
    }
    return QDir::cleanPath(path);
}

QString WhatSonLocalNoteFileStore::resolveNoteStem(const QString& noteId, const QString& noteDirectoryPath) const
{
    QString stem = QFileInfo(normalizePath(noteDirectoryPath)).completeBaseName().trimmed();
    if (stem.isEmpty())
    {
        stem = QFileInfo(noteId.trimmed()).completeBaseName().trimmed();
    }
    if (stem.isEmpty())
    {
        stem = QFileInfo(noteId.trimmed()).fileName().trimmed();
    }
    if (stem.isEmpty())
    {
        stem = QStringLiteral("note");
    }
    return stem;
}

QString WhatSonLocalNoteFileStore::resolveNoteId(
    const QString& noteId,
    const QString& noteDirectoryPath,
    const WhatSonNoteHeaderStore* headerStore) const
{
    QString resolvedNoteId = noteId.trimmed();
    if (resolvedNoteId.isEmpty() && headerStore != nullptr)
    {
        resolvedNoteId = headerStore->noteId().trimmed();
    }
    if (resolvedNoteId.isEmpty())
    {
        resolvedNoteId = resolveNoteStem(QString(), noteDirectoryPath);
    }
    return resolvedNoteId;
}

QString WhatSonLocalNoteFileStore::resolveDirectoryPath(
    const QString& noteDirectoryPath,
    const QString& noteHeaderPath,
    const QString& noteBodyPath) const
{
    const QString directPath = normalizePath(noteDirectoryPath);
    if (!directPath.isEmpty())
    {
        return directPath;
    }

    const QString normalizedHeaderPath = normalizePath(noteHeaderPath);
    if (!normalizedHeaderPath.isEmpty())
    {
        return QFileInfo(normalizedHeaderPath).absolutePath();
    }

    const QString normalizedBodyPath = normalizePath(noteBodyPath);
    if (!normalizedBodyPath.isEmpty())
    {
        return QFileInfo(normalizedBodyPath).absolutePath();
    }

    return {};
}

QString WhatSonLocalNoteFileStore::headerPathForDirectory(const QString& noteId, const QString& noteDirectoryPath) const
{
    const QString normalizedDirectoryPath = normalizePath(noteDirectoryPath);
    if (normalizedDirectoryPath.isEmpty())
    {
        return {};
    }
    return QDir(normalizedDirectoryPath).filePath(resolveNoteStem(noteId, normalizedDirectoryPath) + QStringLiteral(".wsnhead"));
}

QString WhatSonLocalNoteFileStore::bodyPathForDirectory(const QString& noteId, const QString& noteDirectoryPath) const
{
    const QString normalizedDirectoryPath = normalizePath(noteDirectoryPath);
    if (normalizedDirectoryPath.isEmpty())
    {
        return {};
    }
    return QDir(normalizedDirectoryPath).filePath(resolveNoteStem(noteId, normalizedDirectoryPath) + QStringLiteral(".wsnbody"));
}

QString WhatSonLocalNoteFileStore::currentNoteTimestamp() const
{
    return QDateTime::currentDateTime().toString(QString::fromLatin1(kNoteTimestampFormat));
}

bool WhatSonLocalNoteFileStore::loadHeaderStore(
    const QString& headerPath,
    WhatSonNoteHeaderStore* outHeaderStore,
    QString* errorMessage) const
{
    if (outHeaderStore == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("outHeaderStore must not be null.");
        }
        return false;
    }

    QString rawHeaderText;
    QString readError;
    if (!m_ioGateway.readUtf8File(headerPath, &rawHeaderText, &readError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = readError;
        }
        return false;
    }

    WhatSonNoteHeaderParser parser;
    QString parseError;
    if (!parser.parse(rawHeaderText, outHeaderStore, &parseError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = parseError;
        }
        return false;
    }

    return true;
}

void WhatSonLocalNoteFileStore::applyBodyDocumentText(
    const QString& bodyDocumentText,
    WhatSonLocalNoteDocument* document) const
{
    if (document == nullptr)
    {
        return;
    }

    document->bodyHasResource = false;
    document->bodyFirstResourceThumbnailUrl.clear();
    document->bodyPlainText.clear();
    document->bodyFirstLine.clear();

    static const QRegularExpression bodyPattern(
        QStringLiteral(R"(<body\b[^>]*>([\s\S]*?)</body>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch bodyMatch = bodyPattern.match(bodyDocumentText);
    if (!bodyMatch.hasMatch())
    {
        return;
    }

    QString innerText = bodyMatch.captured(1);
    innerText.remove(QRegularExpression(QStringLiteral(R"(<!--[\s\S]*?-->)")));

    static const QRegularExpression resourcePattern(
        QStringLiteral(R"(<resource\b[^>]*?/?>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch resourceMatch = resourcePattern.match(innerText);
    if (resourceMatch.hasMatch())
    {
        document->bodyHasResource = true;

        const QString resourceTag = resourceMatch.captured(0);
        const QString resourcePath = extractXmlAttributeValue(
            resourceTag,
            {
                QStringLiteral("resourcePath"),
                QStringLiteral("path"),
                QStringLiteral("src"),
                QStringLiteral("href"),
                QStringLiteral("url")
            });
        const QString resourceFormat = extractXmlAttributeValue(
            resourceTag,
            {QStringLiteral("format"), QStringLiteral("type"), QStringLiteral("mime"), QStringLiteral("kind")});
        const QString resolvedResourceLocation = resolveResourceLocation(resourcePath, *document);
        if (isPreviewableImageResource(resolvedResourceLocation, resourceFormat))
        {
            document->bodyFirstResourceThumbnailUrl = normalizeThumbnailSource(resolvedResourceLocation);
        }
    }

    innerText.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    innerText.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    innerText.replace(
        QRegularExpression(QStringLiteral(R"(<br\s*/?>)"), QRegularExpression::CaseInsensitiveOption),
        QStringLiteral("\n"));
    innerText.replace(
        QRegularExpression(
            QStringLiteral(R"(</(?:p|paragraph|div|li|h[1-6]|section|article|blockquote|ul|ol|tr|table|pre)>)"),
            QRegularExpression::CaseInsensitiveOption),
        QStringLiteral("\n"));
    innerText.replace(
        QRegularExpression(
            QStringLiteral(
                R"(<(?:p|paragraph|div|li|h[1-6]|section|article|blockquote|ul|ol|tr|table|pre|hr)\b[^>]*>)"),
            QRegularExpression::CaseInsensitiveOption),
        QStringLiteral("\n"));
    innerText.replace(QRegularExpression(QStringLiteral(R"(<[^>]+>)")), QString());
    innerText = decodeXmlEntities(std::move(innerText));

    const QStringList rawLines = innerText.split(QLatin1Char('\n'));
    QStringList lines;
    lines.reserve(rawLines.size());
    for (QString line : rawLines)
    {
        line = normalizeBodyLine(std::move(line));
        if (!line.isEmpty())
        {
            lines.push_back(std::move(line));
        }
    }

    if (lines.isEmpty())
    {
        return;
    }

    document->bodyFirstLine = lines.constFirst();
    document->bodyPlainText = lines.join(QLatin1Char('\n'));
}

bool WhatSonLocalNoteFileStore::createNote(
    CreateRequest request,
    WhatSonLocalNoteDocument* outDocument,
    QString* errorMessage) const
{
    const QString noteDirectoryPath = normalizePath(request.noteDirectoryPath);
    if (noteDirectoryPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("noteDirectoryPath must not be empty.");
        }
        return false;
    }

    const QString resolvedNoteId = resolveNoteId(request.noteId, noteDirectoryPath, &request.headerStore);
    const QString headerPath = headerPathForDirectory(resolvedNoteId, noteDirectoryPath);
    const QString bodyPath = bodyPathForDirectory(resolvedNoteId, noteDirectoryPath);
    if (headerPath.isEmpty() || bodyPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to resolve local note paths.");
        }
        return false;
    }

    if (QFileInfo(headerPath).exists() || QFileInfo(bodyPath).exists())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Local note files already exist for: %1").arg(noteDirectoryPath);
        }
        return false;
    }

    QString ensureError;
    if (!m_ioGateway.ensureDirectory(noteDirectoryPath, &ensureError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = ensureError;
        }
        return false;
    }

    const QString metaDirectoryPath = QDir(noteDirectoryPath).filePath(QStringLiteral(".meta"));
    if (!m_ioGateway.ensureDirectory(metaDirectoryPath, &ensureError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = ensureError;
        }
        return false;
    }

    if (request.headerStore.noteId().trimmed().isEmpty())
    {
        request.headerStore.setNoteId(resolvedNoteId);
    }
    const QString normalizedBodyText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(request.bodyPlainText);

    WhatSonNoteHeaderCreator headerCreator(noteDirectoryPath, QString());
    QString writeError;
    if (!m_ioGateway.writeUtf8File(headerPath, headerCreator.createHeaderText(request.headerStore), &writeError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = writeError;
        }
        return false;
    }

    if (!m_ioGateway.writeUtf8File(bodyPath, serializeBodyDocument(request.headerStore.noteId(), normalizedBodyText), &writeError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = writeError;
        }
        return false;
    }

    if (outDocument != nullptr)
    {
        outDocument->noteDirectoryPath = noteDirectoryPath;
        outDocument->noteHeaderPath = headerPath;
        outDocument->noteBodyPath = bodyPath;
        outDocument->headerStore = request.headerStore;
        outDocument->bodyPlainText = normalizedBodyText;
        outDocument->bodyFirstLine = WhatSon::NoteBodyPersistence::firstLineFromBodyPlainText(normalizedBodyText);
        outDocument->bodyHasResource = false;
        outDocument->bodyFirstResourceThumbnailUrl.clear();
    }

    return true;
}

bool WhatSonLocalNoteFileStore::readNote(
    ReadRequest request,
    WhatSonLocalNoteDocument* outDocument,
    QString* errorMessage) const
{
    if (outDocument == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("outDocument must not be null.");
        }
        return false;
    }

    const QString noteDirectoryPath = resolveDirectoryPath(
        request.noteDirectoryPath,
        request.noteHeaderPath,
        request.noteBodyPath);
    const QString resolvedHeaderPath = normalizePath(
        WhatSon::NoteBodyPersistence::resolveHeaderPath(request.noteHeaderPath, noteDirectoryPath));
    if (resolvedHeaderPath.isEmpty() || !QFileInfo(resolvedHeaderPath).isFile())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to resolve existing local note header.");
        }
        return false;
    }

    WhatSonLocalNoteDocument document;
    document.noteDirectoryPath = noteDirectoryPath.isEmpty() ? QFileInfo(resolvedHeaderPath).absolutePath() : noteDirectoryPath;
    document.noteHeaderPath = resolvedHeaderPath;

    QString headerError;
    if (!loadHeaderStore(resolvedHeaderPath, &document.headerStore, &headerError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = headerError;
        }
        return false;
    }

    const QString resolvedNoteId = resolveNoteId(request.noteId, document.noteDirectoryPath, &document.headerStore);
    const QString bodyCandidatePath = normalizePath(request.noteBodyPath);
    const QString resolvedBodyPath = bodyCandidatePath.isEmpty()
                                         ? normalizePath(WhatSon::NoteBodyPersistence::resolveBodyPath(document.noteDirectoryPath))
                                         : bodyCandidatePath;
    document.noteBodyPath = resolvedBodyPath;

    if (!resolvedBodyPath.isEmpty() && QFileInfo(resolvedBodyPath).isFile())
    {
        QString rawBodyText;
        QString readError;
        if (!m_ioGateway.readUtf8File(resolvedBodyPath, &rawBodyText, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            return false;
        }

        applyBodyDocumentText(rawBodyText, &document);
    }

    if (document.headerStore.noteId().trimmed().isEmpty())
    {
        document.headerStore.setNoteId(resolvedNoteId);
    }
    *outDocument = std::move(document);
    return true;
}

bool WhatSonLocalNoteFileStore::updateNote(
    UpdateRequest request,
    WhatSonLocalNoteDocument* outDocument,
    QString* errorMessage) const
{
    QString noteDirectoryPath = resolveDirectoryPath(
        request.document.noteDirectoryPath,
        request.document.noteHeaderPath,
        request.document.noteBodyPath);
    if (noteDirectoryPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to resolve local note directory for update.");
        }
        return false;
    }

    const QString resolvedNoteId = resolveNoteId(
        request.document.headerStore.noteId(),
        noteDirectoryPath,
        &request.document.headerStore);
    if (resolvedNoteId.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("noteId must not be empty.");
        }
        return false;
    }

    const bool persistHeader = request.persistHeader || request.touchLastModified;
    const bool persistBody = request.persistBody;
    if (!persistHeader && !persistBody)
    {
        if (outDocument != nullptr)
        {
            *outDocument = std::move(request.document);
        }
        return true;
    }

    const QString headerPath = normalizePath(request.document.noteHeaderPath).isEmpty()
                                   ? normalizePath(WhatSon::NoteBodyPersistence::resolveHeaderPath(QString(), noteDirectoryPath))
                                   : normalizePath(request.document.noteHeaderPath);
    const QString bodyPath = normalizePath(request.document.noteBodyPath).isEmpty()
                                 ? normalizePath(WhatSon::NoteBodyPersistence::resolveBodyPath(noteDirectoryPath))
                                 : normalizePath(request.document.noteBodyPath);

    QString ensureError;
    if (!m_ioGateway.ensureDirectory(noteDirectoryPath, &ensureError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = ensureError;
        }
        return false;
    }

    request.document.noteDirectoryPath = noteDirectoryPath;
    request.document.noteHeaderPath = headerPath;
    request.document.noteBodyPath = bodyPath;
    if (request.document.headerStore.noteId().trimmed().isEmpty())
    {
        request.document.headerStore.setNoteId(resolvedNoteId);
    }
    request.document.bodyPlainText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(request.document.bodyPlainText);
    request.document.bodyFirstLine = WhatSon::NoteBodyPersistence::firstLineFromBodyPlainText(request.document.bodyPlainText);

    if (request.touchLastModified)
    {
        request.document.headerStore.setLastModifiedAt(currentNoteTimestamp());
    }

    QString writeError;
    if (persistBody)
    {
        if (!m_ioGateway.writeUtf8File(
            bodyPath,
            serializeBodyDocument(request.document.headerStore.noteId(), request.document.bodyPlainText),
            &writeError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = writeError;
            }
            return false;
        }
    }

    if (persistHeader)
    {
        WhatSonNoteHeaderCreator headerCreator(noteDirectoryPath, QString());
        if (!m_ioGateway.writeUtf8File(
            headerPath,
            headerCreator.createHeaderText(request.document.headerStore),
            &writeError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = writeError;
            }
            return false;
        }
    }

    if (outDocument != nullptr)
    {
        *outDocument = std::move(request.document);
    }
    return true;
}

bool WhatSonLocalNoteFileStore::deleteNote(DeleteRequest request, QString* errorMessage) const
{
    const QString noteDirectoryPath = resolveDirectoryPath(
        request.noteDirectoryPath,
        request.noteHeaderPath,
        request.noteBodyPath);
    if (noteDirectoryPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to resolve local note directory for deletion.");
        }
        return false;
    }

    if (!QFileInfo(noteDirectoryPath).exists())
    {
        return true;
    }

    QString removeError;
    if (!m_ioGateway.removeDirectoryRecursively(noteDirectoryPath, &removeError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = removeError;
        }
        return false;
    }

    return true;
}
