#include "app/models/file/note/WhatSonLocalNoteFileStore.hpp"

#include "app/models/file/note/WhatSonNoteBodyPersistence.hpp"
#include "app/models/file/note/WhatSonIiXmlDocumentSupport.hpp"
#include "app/models/file/note/WhatSonNoteHeaderCreator.hpp"
#include "app/models/file/diff/WhatSonLocalNoteVersionStore.hpp"
#include "app/models/file/statistic/WhatSonNoteFileStatSupport.hpp"
#include "app/models/file/note/WhatSonNoteHeaderParser.hpp"
#include "app/models/file/hierarchy/resources/WhatSonResourcePackageSupport.hpp"
#include "app/models/file/hierarchy/tags/WhatSonTagsHierarchyParser.hpp"
#include "app/models/file/hierarchy/tags/WhatSonTagsHierarchyStore.hpp"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>
#include <QUrl>

#include <utility>

namespace
{
    namespace IiXml = WhatSon::IiXmlDocumentSupport;

    constexpr auto kNoteTimestampFormat = "yyyy-MM-dd-hh-mm-ss";
    constexpr auto kNoteVersionSchema = "whatson.note.version.store";

    QString createEmptyVersionDocumentText(const QString& noteId)
    {
        QJsonObject root;
        root.insert(QStringLiteral("version"), 1);
        root.insert(QStringLiteral("schema"), QString::fromLatin1(kNoteVersionSchema));
        root.insert(QStringLiteral("noteId"), noteId.trimmed());
        root.insert(QStringLiteral("currentSnapshotId"), QString());
        root.insert(QStringLiteral("headSnapshotId"), QString());
        root.insert(QStringLiteral("snapshots"), QJsonArray{});
        return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
    }

    QString createEmptyPaintDocumentText(const QString& noteId)
    {
        return QStringLiteral(
                   "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                   "<!DOCTYPE WHATSONNOTEPAINT>\n"
                   "<contents id=\"%1\">\n"
                   "  <body>\n"
                   "  </body>\n"
                   "</contents>\n")
            .arg(noteId.trimmed());
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

    iiXml::Parser::TagDocumentResult parseWsnXmlDocument(const QString& sourceText)
    {
        return IiXml::parseDocument(prepareWsnXmlForIiXml(sourceText));
    }

    QString serializeBodyDocument(const QString& noteId, const QString& plainText)
    {
        return WhatSon::NoteBodyPersistence::serializeBodyDocument(noteId, plainText);
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

    QString resolveResourceLocation(const QString& rawResourcePath, const WhatSonLocalNoteDocument& document)
    {
        const QString hubRootPath = findAncestorDirectoryWithSuffix(
            !document.noteDirectoryPath.trimmed().isEmpty() ? document.noteDirectoryPath : document.noteHeaderPath,
            QStringLiteral(".wshub"));
        return WhatSon::Resources::resolveAssetLocationFromReference(
            rawResourcePath,
            WhatSon::Resources::resourceReferenceBasePathsForContext(
                document.noteDirectoryPath,
                document.noteHeaderPath,
                hubRootPath));
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

    QString normalizedTagKey(const QString& tag)
    {
        return tag.trimmed().toCaseFolded();
    }

    QStringList mergeTagsPreservingOrder(const QStringList& existingTags, const QStringList& additionalTags)
    {
        QStringList mergedTags;
        mergedTags.reserve(existingTags.size() + additionalTags.size());

        QSet<QString> seenTags;
        seenTags.reserve(existingTags.size() + additionalTags.size());

        const auto appendTag = [&mergedTags, &seenTags](const QString& rawTag)
        {
            const QString trimmedTag = rawTag.trimmed();
            const QString tagKey = normalizedTagKey(trimmedTag);
            if (trimmedTag.isEmpty() || seenTags.contains(tagKey))
            {
                return;
            }

            seenTags.insert(tagKey);
            mergedTags.push_back(trimmedTag);
        };

        for (const QString& existingTag : existingTags)
        {
            appendTag(existingTag);
        }
        for (const QString& additionalTag : additionalTags)
        {
            appendTag(additionalTag);
        }

        return mergedTags;
    }

    QString resolveTagsFilePathForNoteDirectory(const QString& noteDirectoryPath)
    {
        const QString contentsDirectoryPath = findAncestorDirectoryWithSuffix(
            noteDirectoryPath,
            QStringLiteral(".wscontents"));
        if (contentsDirectoryPath.isEmpty())
        {
            return {};
        }

        return QDir(contentsDirectoryPath).filePath(QStringLiteral("Tags.wstags"));
    }

    bool ensureTagsHierarchyContainsTags(
        const QString& noteDirectoryPath,
        const QStringList& tags,
        QString* errorMessage)
    {
        QStringList requestedTags;
        requestedTags.reserve(tags.size());

        QSet<QString> requestedTagKeys;
        requestedTagKeys.reserve(tags.size());
        for (const QString& rawTag : tags)
        {
            const QString trimmedTag = rawTag.trimmed();
            const QString tagKey = normalizedTagKey(trimmedTag);
            if (trimmedTag.isEmpty() || requestedTagKeys.contains(tagKey))
            {
                continue;
            }

            requestedTagKeys.insert(tagKey);
            requestedTags.push_back(trimmedTag);
        }

        if (requestedTags.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                errorMessage->clear();
            }
            return true;
        }

        const QString tagsFilePath = resolveTagsFilePathForNoteDirectory(noteDirectoryPath);
        if (tagsFilePath.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral(
                    "Failed to resolve Tags.wstags path from note directory: %1").arg(noteDirectoryPath);
            }
            return false;
        }

        WhatSonTagsHierarchyStore hierarchyStore;
        if (QFileInfo(tagsFilePath).isFile())
        {
            QString rawTagsText;
            QString readError;
            if (!WhatSonSystemIoGateway().readUtf8File(tagsFilePath, &rawTagsText, &readError))
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = readError;
                }
                return false;
            }

            WhatSonTagsHierarchyParser parser;
            QString parseError;
            if (!parser.parse(rawTagsText, &hierarchyStore, &parseError))
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = parseError;
                }
                return false;
            }
        }

        QVector<WhatSonTagDepthEntry> stagedEntries = hierarchyStore.tagEntries();
        QSet<QString> existingTagKeys;
        existingTagKeys.reserve(stagedEntries.size() * 2);
        for (const WhatSonTagDepthEntry& entry : stagedEntries)
        {
            const QString idKey = normalizedTagKey(entry.id);
            const QString labelKey = normalizedTagKey(entry.label);
            if (!idKey.isEmpty())
            {
                existingTagKeys.insert(idKey);
            }
            if (!labelKey.isEmpty())
            {
                existingTagKeys.insert(labelKey);
            }
        }

        bool changed = false;
        for (const QString& requestedTag : requestedTags)
        {
            const QString requestedTagKey = normalizedTagKey(requestedTag);
            if (existingTagKeys.contains(requestedTagKey))
            {
                continue;
            }

            WhatSonTagDepthEntry entry;
            entry.id = requestedTag;
            entry.label = requestedTag;
            entry.depth = 0;
            stagedEntries.push_back(entry);
            existingTagKeys.insert(requestedTagKey);
            changed = true;
        }

        if (!changed)
        {
            if (errorMessage != nullptr)
            {
                errorMessage->clear();
            }
            return true;
        }

        hierarchyStore.setTagEntries(std::move(stagedEntries));
        return hierarchyStore.writeToFile(tagsFilePath, errorMessage);
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

QString WhatSonLocalNoteFileStore::versionPathForDirectory(const QString& noteId, const QString& noteDirectoryPath) const
{
    const QString normalizedDirectoryPath = normalizePath(noteDirectoryPath);
    if (normalizedDirectoryPath.isEmpty())
    {
        return {};
    }
    return QDir(normalizedDirectoryPath).filePath(
        resolveNoteStem(noteId, normalizedDirectoryPath) + QStringLiteral(".wsnversion"));
}

QString WhatSonLocalNoteFileStore::paintPathForDirectory(const QString& noteId, const QString& noteDirectoryPath) const
{
    const QString normalizedDirectoryPath = normalizePath(noteDirectoryPath);
    if (normalizedDirectoryPath.isEmpty())
    {
        return {};
    }
    return QDir(normalizedDirectoryPath).filePath(
        resolveNoteStem(noteId, normalizedDirectoryPath) + QStringLiteral(".wsnpaint"));
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
    document->bodySourceText.clear();
    document->bodyFirstLine.clear();

    const iiXml::Parser::TagDocumentResult parsedBodyDocument = parseWsnXmlDocument(bodyDocumentText);
    if (parsedBodyDocument.Status != iiXml::Parser::TagTreeParseStatus::Parsed
        || !parsedBodyDocument.Document.has_value())
    {
        return;
    }

    const iiXml::Parser::TagDocument& xmlDocument = parsedBodyDocument.Document.value();
    const iiXml::Parser::TagNode* bodyNode = IiXml::findFirstDescendant(xmlDocument.Nodes, QStringLiteral("body"));
    if (bodyNode == nullptr)
    {
        return;
    }

    const iiXml::Parser::TagNode* resourceNode =
        IiXml::findFirstDescendant(bodyNode->Children, QStringLiteral("resource"));
    if (resourceNode != nullptr)
    {
        document->bodyHasResource = true;

        const QString resolvedResourcePath = IiXml::attributeValue(
            xmlDocument,
            resourceNode,
            {
                QStringLiteral("resourcePath"),
                QStringLiteral("path"),
                QStringLiteral("src"),
                QStringLiteral("href"),
                QStringLiteral("url")
            });
        const QString resolvedResourceFormat = IiXml::attributeValue(
            xmlDocument,
            resourceNode,
            {QStringLiteral("format"), QStringLiteral("type"), QStringLiteral("mime"), QStringLiteral("kind")});
        const QString resourceLocation = resolveResourceLocation(resolvedResourcePath, *document);
        if (isPreviewableImageResource(resourceLocation, resolvedResourceFormat))
        {
            document->bodyFirstResourceThumbnailUrl = normalizeThumbnailSource(resourceLocation);
        }
    }

    document->bodyPlainText = WhatSon::NoteBodyPersistence::plainTextFromBodyDocument(bodyDocumentText);
    document->bodySourceText = WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(bodyDocumentText);
    document->normalizeBodyFields();
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
    const QString versionPath = versionPathForDirectory(resolvedNoteId, noteDirectoryPath);
    const QString paintPath = paintPathForDirectory(resolvedNoteId, noteDirectoryPath);
    if (headerPath.isEmpty() || bodyPath.isEmpty() || versionPath.isEmpty() || paintPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to resolve local note paths.");
        }
        return false;
    }

    if (QFileInfo(headerPath).exists() || QFileInfo(bodyPath).exists() || QFileInfo(versionPath).exists()
        || QFileInfo(paintPath).exists())
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

    if (request.headerStore.noteId().trimmed().isEmpty())
    {
        request.headerStore.setNoteId(resolvedNoteId);
    }
    const QString normalizedBodyText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(request.bodyPlainText);
    const QString bodyDocumentText = serializeBodyDocument(request.headerStore.noteId(), normalizedBodyText);

    QString statsError;
    if (!WhatSon::NoteFileStatSupport::applyTrackedStatistics(
            &request.headerStore,
            noteDirectoryPath,
            normalizedBodyText,
            bodyDocumentText,
            &statsError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = statsError;
        }
        return false;
    }
    const QStringList trackedBacklinkTargets = WhatSon::NoteFileStatSupport::extractBacklinkTargets(
        normalizedBodyText,
        bodyDocumentText);

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

    if (!m_ioGateway.writeUtf8File(bodyPath, bodyDocumentText, &writeError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = writeError;
        }
        return false;
    }

    if (!m_ioGateway.writeUtf8File(versionPath, createEmptyVersionDocumentText(request.headerStore.noteId()), &writeError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = writeError;
        }
        return false;
    }

    if (!m_ioGateway.writeUtf8File(paintPath, createEmptyPaintDocumentText(request.headerStore.noteId()), &writeError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = writeError;
        }
        return false;
    }

    for (const QString& backlinkTarget : trackedBacklinkTargets)
    {
        if (backlinkTarget.trimmed().isEmpty() || backlinkTarget.trimmed() == request.headerStore.noteId().trimmed())
        {
            continue;
        }

        QString targetRefreshError;
        if (!WhatSon::NoteFileStatSupport::refreshTrackedStatisticsForNoteId(
                backlinkTarget,
                noteDirectoryPath,
                &targetRefreshError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = targetRefreshError;
            }
            return false;
        }
    }

    if (outDocument != nullptr)
    {
        outDocument->noteDirectoryPath = noteDirectoryPath;
        outDocument->noteHeaderPath = headerPath;
        outDocument->noteBodyPath = bodyPath;
        outDocument->noteVersionPath = versionPath;
        outDocument->notePaintPath = paintPath;
        outDocument->headerStore = request.headerStore;
        outDocument->bodyPlainText = normalizedBodyText;
        outDocument->bodySourceText = normalizedBodyText;
        outDocument->normalizeBodyFields();
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
    const QString resolvedVersionPath = versionPathForDirectory(resolvedNoteId, document.noteDirectoryPath);
    const QString resolvedPaintPath = paintPathForDirectory(resolvedNoteId, document.noteDirectoryPath);
    document.noteBodyPath = resolvedBodyPath;
    document.noteVersionPath = resolvedVersionPath;
    document.notePaintPath = resolvedPaintPath;

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

    const bool persistHeader = request.persistHeader || request.touchLastModified || request.persistBody;
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
    const QString versionPath = normalizePath(request.document.noteVersionPath).isEmpty()
                                    ? versionPathForDirectory(resolvedNoteId, noteDirectoryPath)
                                    : normalizePath(request.document.noteVersionPath);
    const QString paintPath = normalizePath(request.document.notePaintPath).isEmpty()
                                  ? paintPathForDirectory(resolvedNoteId, noteDirectoryPath)
                                  : normalizePath(request.document.notePaintPath);

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
    request.document.noteVersionPath = versionPath;
    request.document.notePaintPath = paintPath;
    if (request.document.headerStore.noteId().trimmed().isEmpty())
    {
        request.document.headerStore.setNoteId(resolvedNoteId);
    }
    const WhatSonLocalNoteDocument unchangedDocument = request.document;
    QString serializedBodyDocument;
    QString bodyDocumentForStats;
    QString existingBodyDocument;
    QStringList previousBacklinkTargets;
    if ((persistHeader || persistBody) && !bodyPath.isEmpty() && QFileInfo(bodyPath).isFile())
    {
        QString existingBodyReadError;
        existingBodyDocument = [this, &bodyPath, &existingBodyReadError]()
        {
            QString rawText;
            if (!m_ioGateway.readUtf8File(bodyPath, &rawText, &existingBodyReadError))
            {
                return QString();
            }
            return rawText;
        }();
        if (!existingBodyReadError.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = existingBodyReadError;
            }
            return false;
        }

        if (persistBody)
        {
            previousBacklinkTargets = WhatSon::NoteFileStatSupport::extractBacklinkTargets(
                WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(existingBodyDocument),
                existingBodyDocument);
        }
        else
        {
            bodyDocumentForStats = existingBodyDocument;
        }
    }

    if (persistBody)
    {
        const QString bodyText = request.document.effectiveBodyText();
        serializedBodyDocument = serializeBodyDocument(request.document.headerStore.noteId(), bodyText);
        request.document.bodyPlainText = WhatSon::NoteBodyPersistence::plainTextFromBodyDocument(serializedBodyDocument);
        request.document.bodySourceText = bodyText;
        request.document.normalizeBodyFields();
        bodyDocumentForStats = serializedBodyDocument;

        if (!request.persistHeader
            && !existingBodyDocument.isEmpty()
            && existingBodyDocument == serializedBodyDocument)
        {
            if (outDocument != nullptr)
            {
                WhatSonLocalNoteDocument normalizedDocument = unchangedDocument;
                normalizedDocument.normalizeBodyFields();
                *outDocument = std::move(normalizedDocument);
            }
            return true;
        }
    }
    else
    {
        request.document.normalizeBodyFields();
    }

    if (persistBody)
    {
        const QStringList inlineTags = WhatSon::NoteBodyPersistence::extractedInlineTagValues(
            request.document.bodySourceText);
        if (!inlineTags.isEmpty())
        {
            request.document.headerStore.setTags(
                mergeTagsPreservingOrder(request.document.headerStore.tags(), inlineTags));

            QString tagsMutationError;
            if (!ensureTagsHierarchyContainsTags(noteDirectoryPath, inlineTags, &tagsMutationError))
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = tagsMutationError;
                }
                return false;
            }
        }
    }

    QString bodySourceTextForStats = request.document.effectiveBodyText();
    if (bodySourceTextForStats.isEmpty() && !bodyDocumentForStats.isEmpty())
    {
        bodySourceTextForStats = WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(bodyDocumentForStats);
        if (bodySourceTextForStats.isEmpty())
        {
            bodySourceTextForStats = WhatSon::NoteBodyPersistence::plainTextFromBodyDocument(bodyDocumentForStats);
        }
    }

    QString statsError;
    if (persistHeader && request.refreshIncomingBacklinkStatistics
        && !WhatSon::NoteFileStatSupport::applyTrackedStatistics(
            &request.document.headerStore,
            noteDirectoryPath,
            bodySourceTextForStats,
            bodyDocumentForStats,
            &statsError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = statsError;
        }
        return false;
    }
    if (persistHeader && !request.refreshIncomingBacklinkStatistics)
    {
        WhatSon::NoteFileStatSupport::applyBodyDerivedStatistics(
            &request.document.headerStore,
            bodySourceTextForStats,
            bodyDocumentForStats);
    }

    const int modifiedCountBeforeUpdate = request.document.headerStore.modifiedCount();
    if (request.touchLastModified)
    {
        request.document.headerStore.setLastModifiedAt(currentNoteTimestamp());
    }
    if (request.incrementModifiedCount && (persistHeader || persistBody))
    {
        request.document.headerStore.incrementModifiedCount();
    }
    const int modifiedCountAfterUpdate = request.document.headerStore.modifiedCount();
    const bool shouldCaptureVersionCommit = request.incrementModifiedCount
        && (persistHeader || persistBody)
        && modifiedCountAfterUpdate == modifiedCountBeforeUpdate + 1;

    QString writeError;
    if (!versionPath.isEmpty() && !m_ioGateway.exists(versionPath))
    {
        if (!m_ioGateway.writeUtf8File(
            versionPath,
            createEmptyVersionDocumentText(request.document.headerStore.noteId()),
            &writeError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = writeError;
            }
            return false;
        }
    }

    if (!paintPath.isEmpty() && !m_ioGateway.exists(paintPath))
    {
        if (!m_ioGateway.writeUtf8File(
            paintPath,
            createEmptyPaintDocumentText(request.document.headerStore.noteId()),
            &writeError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = writeError;
            }
            return false;
        }
    }

    if (persistBody)
    {
        if (!m_ioGateway.writeUtf8File(
            bodyPath,
            serializedBodyDocument,
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

    if (persistBody && request.refreshAffectedBacklinkTargets)
    {
        const QStringList currentBacklinkTargets = WhatSon::NoteFileStatSupport::extractBacklinkTargets(
            bodySourceTextForStats,
            bodyDocumentForStats);
        QSet<QString> affectedBacklinkTargets(previousBacklinkTargets.begin(), previousBacklinkTargets.end());
        for (const QString& target : currentBacklinkTargets)
        {
            affectedBacklinkTargets.insert(target);
        }

        for (const QString& backlinkTarget : affectedBacklinkTargets)
        {
            if (backlinkTarget.trimmed().isEmpty()
                || backlinkTarget.trimmed() == request.document.headerStore.noteId().trimmed())
            {
                continue;
            }

            QString targetRefreshError;
            if (!WhatSon::NoteFileStatSupport::refreshTrackedStatisticsForNoteId(
                    backlinkTarget,
                    noteDirectoryPath,
                    &targetRefreshError))
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = targetRefreshError;
                }
                return false;
            }
        }
    }

    if (shouldCaptureVersionCommit)
    {
        WhatSonLocalNoteVersionStore versionStore;
        WhatSonLocalNoteVersionStore::CaptureRequest captureRequest;
        captureRequest.document = request.document;
        captureRequest.label = QStringLiteral("commit:%1").arg(modifiedCountAfterUpdate);
        captureRequest.commitModifiedCount = modifiedCountAfterUpdate;

        QString captureError;
        if (!versionStore.captureSnapshot(
                std::move(captureRequest),
                nullptr,
                nullptr,
                &captureError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = captureError;
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
