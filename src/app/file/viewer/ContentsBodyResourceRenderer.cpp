#include "ContentsBodyResourceRenderer.hpp"

#include "file/hierarchy/resources/WhatSonResourcePackageSupport.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaObject>
#include <QRegularExpression>
#include <QUrl>

#include <algorithm>

namespace
{
    constexpr auto kNoteDirectoryPathForNoteIdSignature = "noteDirectoryPathForNoteId(QString)";

    QString readUtf8File(const QString& filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return {};
        }
        return QString::fromUtf8(file.readAll());
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

    QString normalizePath(QString path)
    {
        path = path.trimmed();
        if (path.isEmpty())
        {
            return {};
        }
        return QDir::cleanPath(path);
    }

    QString resolveBodyPathFromNoteDirectory(const QString& noteDirectoryPath)
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

        return {};
    }

    QString extractBodyInnerText(const QString& bodyDocumentText)
    {
        if (bodyDocumentText.isEmpty())
        {
            return {};
        }

        static const QRegularExpression bodyPattern(
            QStringLiteral(R"(<body\b[^>]*>([\s\S]*?)</body>)"),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch bodyMatch = bodyPattern.match(bodyDocumentText);
        if (!bodyMatch.hasMatch())
        {
            return {};
        }

        QString bodyInnerText = bodyMatch.captured(1);
        bodyInnerText.remove(QRegularExpression(QStringLiteral(R"(<!--[\s\S]*?-->)")));
        return bodyInnerText;
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
                const QString value = decodeXmlEntities(match.captured(captureIndex)).trimmed();
                if (!value.isEmpty())
                {
                    return value;
                }
            }
        }
        return {};
    }

    QString normalizedFormat(QString format)
    {
        format = format.trimmed().toCaseFolded();
        if (format.isEmpty())
        {
            return {};
        }
        if (!format.startsWith(QLatin1Char('.')))
        {
            format.prepend(QLatin1Char('.'));
        }
        return format;
    }

    QString extensionFromFormat(QString format)
    {
        format = normalizedFormat(format);
        if (format.startsWith(QLatin1Char('.')))
        {
            format.remove(0, 1);
        }
        return format;
    }

    QString normalizeSourceUrl(const QString& resolvedResourceLocation)
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

    QString extensionFromResourceLocation(const QString& resourceLocation)
    {
        if (resourceLocation.isEmpty())
        {
            return {};
        }

        if (QFileInfo(resourceLocation).isAbsolute())
        {
            return QFileInfo(resourceLocation).suffix().toCaseFolded();
        }

        const QUrl resourceUrl(resourceLocation);
        if (resourceUrl.isValid() && !resourceUrl.scheme().isEmpty())
        {
            return QFileInfo(resourceUrl.path()).suffix().toCaseFolded();
        }

        return QFileInfo(resourceLocation).suffix().toCaseFolded();
    }

    bool isImageResource(
        const QString& type,
        const QString& format,
        const QString& resolvedResourceLocation,
        const QString& rawResourcePath)
    {
        const QString normalizedType = type.trimmed().toCaseFolded();
        if (normalizedType == QStringLiteral("image"))
        {
            return true;
        }

        QString extension = extensionFromFormat(format);
        if (extension.isEmpty())
        {
            extension = extensionFromResourceLocation(resolvedResourceLocation);
        }
        if (extension.isEmpty())
        {
            extension = extensionFromResourceLocation(rawResourcePath);
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
        return !extension.isEmpty() && kPreviewableImageExtensions.contains(extension);
    }

    QString effectiveExtension(
        const QString& format,
        const QString& resolvedResourceLocation,
        const QString& rawResourcePath)
    {
        QString extension = extensionFromFormat(format);
        if (extension.isEmpty())
        {
            extension = extensionFromResourceLocation(resolvedResourceLocation);
        }
        if (extension.isEmpty())
        {
            extension = extensionFromResourceLocation(rawResourcePath);
        }
        return extension.toCaseFolded();
    }

    bool isVideoResource(
        const QString& type,
        const QString& format,
        const QString& resolvedResourceLocation,
        const QString& rawResourcePath)
    {
        const QString normalizedType = type.trimmed().toCaseFolded();
        if (normalizedType == QStringLiteral("video"))
        {
            return true;
        }

        static const QStringList kPreviewableVideoExtensions = {
            QStringLiteral("mp4"),
            QStringLiteral("mov"),
            QStringLiteral("m4v"),
            QStringLiteral("avi"),
            QStringLiteral("mkv"),
            QStringLiteral("webm")
        };
        const QString extension = effectiveExtension(format, resolvedResourceLocation, rawResourcePath);
        return !extension.isEmpty() && kPreviewableVideoExtensions.contains(extension);
    }

    bool isAudioResource(
        const QString& type,
        const QString& format,
        const QString& resolvedResourceLocation,
        const QString& rawResourcePath)
    {
        const QString normalizedType = type.trimmed().toCaseFolded();
        if (normalizedType == QStringLiteral("audio") || normalizedType == QStringLiteral("music"))
        {
            return true;
        }

        static const QStringList kPreviewableAudioExtensions = {
            QStringLiteral("mp3"),
            QStringLiteral("aac"),
            QStringLiteral("m4a"),
            QStringLiteral("flac"),
            QStringLiteral("alac"),
            QStringLiteral("aiff"),
            QStringLiteral("wav"),
            QStringLiteral("ogg"),
            QStringLiteral("opus"),
            QStringLiteral("caf"),
            QStringLiteral("wma"),
            QStringLiteral("amr")
        };
        const QString extension = effectiveExtension(format, resolvedResourceLocation, rawResourcePath);
        return !extension.isEmpty() && kPreviewableAudioExtensions.contains(extension);
    }

    bool isPdfResource(
        const QString& format,
        const QString& resolvedResourceLocation,
        const QString& rawResourcePath)
    {
        return effectiveExtension(format, resolvedResourceLocation, rawResourcePath) == QStringLiteral("pdf");
    }

    bool isTextResource(
        const QString& type,
        const QString& format,
        const QString& resolvedResourceLocation,
        const QString& rawResourcePath)
    {
        const QString normalizedType = type.trimmed().toCaseFolded();
        if (normalizedType == QStringLiteral("text"))
        {
            return true;
        }

        static const QStringList kTextExtensions = {
            QStringLiteral("txt"),
            QStringLiteral("md"),
            QStringLiteral("csv"),
            QStringLiteral("json"),
            QStringLiteral("xml"),
            QStringLiteral("yaml"),
            QStringLiteral("yml"),
            QStringLiteral("ini"),
            QStringLiteral("log"),
            QStringLiteral("rtf")
        };
        const QString extension = effectiveExtension(format, resolvedResourceLocation, rawResourcePath);
        return !extension.isEmpty() && kTextExtensions.contains(extension);
    }

    QString renderModeForResource(
        const QString& type,
        const QString& format,
        const QString& resolvedResourceLocation,
        const QString& rawResourcePath,
        const QString& sourceUrl)
    {
        if (isImageResource(type, format, resolvedResourceLocation, rawResourcePath) && !sourceUrl.isEmpty())
        {
            return QStringLiteral("image");
        }
        if (isVideoResource(type, format, resolvedResourceLocation, rawResourcePath) && !sourceUrl.isEmpty())
        {
            return QStringLiteral("video");
        }
        if (isAudioResource(type, format, resolvedResourceLocation, rawResourcePath) && !sourceUrl.isEmpty())
        {
            return QStringLiteral("audio");
        }
        if (isPdfResource(format, resolvedResourceLocation, rawResourcePath))
        {
            return QStringLiteral("pdf");
        }
        if (isTextResource(type, format, resolvedResourceLocation, rawResourcePath))
        {
            return QStringLiteral("text");
        }
        return QStringLiteral("document");
    }

    QString previewTextForResource(const QString& renderMode, const QString& resolvedResourceLocation)
    {
        if (renderMode != QStringLiteral("text"))
        {
            return {};
        }

        const QString normalizedResourceLocation = normalizePath(resolvedResourceLocation);
        if (normalizedResourceLocation.isEmpty() || !QFileInfo(normalizedResourceLocation).isFile())
        {
            return {};
        }

        QFile file(normalizedResourceLocation);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return {};
        }

        QString preview = QString::fromUtf8(file.read(1024));
        preview.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        preview.replace(QLatin1Char('\r'), QLatin1Char('\n'));
        preview = preview.trimmed();
        if (preview.size() > 400)
        {
            preview = preview.left(400) + QStringLiteral("...");
        }
        return preview;
    }

    QString displayNameFromResourcePaths(
        const QString& resolvedResourceLocation,
        const QString& resourcePath,
        const QString& fallbackResourceId)
    {
        const QString assetName = QFileInfo(resolvedResourceLocation).fileName().trimmed();
        if (!assetName.isEmpty())
        {
            return assetName;
        }

        const QString resourceName = QFileInfo(resourcePath).completeBaseName().trimmed();
        if (!resourceName.isEmpty())
        {
            return resourceName;
        }

        return fallbackResourceId.trimmed();
    }

    QVariantMap buildRenderedResourceMap(
        int resourceIndex,
        const QString& type,
        const QString& format,
        const QString& resourcePath,
        const QString& resolvedResourceLocation,
        const QString& sourceUrl,
        const QString& resourceId = QString(),
        const int sourceStart = 0,
        const int sourceEnd = 0)
    {
        const QString renderMode = renderModeForResource(type, format, resolvedResourceLocation, resourcePath, sourceUrl);
        QVariantMap resource;
        resource.insert(QStringLiteral("index"), resourceIndex);
        resource.insert(QStringLiteral("type"), type);
        resource.insert(QStringLiteral("format"), format);
        resource.insert(QStringLiteral("resourcePath"), resourcePath);
        resource.insert(QStringLiteral("resolvedPath"), resolvedResourceLocation);
        resource.insert(QStringLiteral("source"), sourceUrl);
        resource.insert(QStringLiteral("renderMode"), renderMode);
        resource.insert(QStringLiteral("resourceId"), resourceId);
        resource.insert(QStringLiteral("sourceStart"), std::max(0, sourceStart));
        resource.insert(QStringLiteral("sourceEnd"), std::max(std::max(0, sourceStart), sourceEnd));
        resource.insert(QStringLiteral("focusSourceOffset"), std::max(std::max(0, sourceStart), sourceEnd));
        resource.insert(
            QStringLiteral("displayName"),
            displayNameFromResourcePaths(resolvedResourceLocation, resourcePath, resourceId));
        resource.insert(
            QStringLiteral("previewText"),
            previewTextForResource(renderMode, resolvedResourceLocation));
        return resource;
    }

    QVariantList buildRenderedResourcesFromSourceText(
        const QString& sourceText,
        const QStringList& basePaths)
    {
        static const QRegularExpression resourceTokenPattern(
            QStringLiteral(R"(<!--[\s\S]*?-->|<resource\b[^>]*?/?>)"),
            QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator iterator = resourceTokenPattern.globalMatch(sourceText);

        QVariantList resources;
        int resourceIndex = 0;
        while (iterator.hasNext())
        {
            const QRegularExpressionMatch match = iterator.next();
            const QString resourceTagText = match.captured(0);
            if (resourceTagText.startsWith(QStringLiteral("<!--")))
            {
                continue;
            }

            const QString type = extractXmlAttributeValue(
                resourceTagText,
                {QStringLiteral("type"), QStringLiteral("kind"), QStringLiteral("mime")}).toCaseFolded();
            const QString format = normalizedFormat(extractXmlAttributeValue(
                resourceTagText,
                {QStringLiteral("format"), QStringLiteral("ext"), QStringLiteral("extension")}));
            const QString resourcePath = extractXmlAttributeValue(
                resourceTagText,
                {
                    QStringLiteral("resourcePath"),
                    QStringLiteral("path"),
                    QStringLiteral("src"),
                    QStringLiteral("href"),
                    QStringLiteral("url")
                });
            const QString resourceId = extractXmlAttributeValue(
                resourceTagText,
                {QStringLiteral("id"), QStringLiteral("resourceId")});
            const QString resolvedResourceLocation = WhatSon::Resources::resolveAssetLocationFromReference(
                resourcePath,
                basePaths);
            const QString sourceUrl = normalizeSourceUrl(resolvedResourceLocation);
            resources.push_back(
                buildRenderedResourceMap(
                    resourceIndex,
                    type,
                    format,
                    resourcePath,
                    resolvedResourceLocation,
                    sourceUrl,
                    resourceId,
                    std::max(0, static_cast<int>(match.capturedStart(0))),
                    std::max(0, static_cast<int>(match.capturedEnd(0)))));
            ++resourceIndex;
        }

        return resources;
    }
} // namespace

ContentsBodyResourceRenderer::ContentsBodyResourceRenderer(QObject* parent)
    : QObject(parent)
{
}

ContentsBodyResourceRenderer::~ContentsBodyResourceRenderer() = default;

QObject* ContentsBodyResourceRenderer::contentViewModel() const noexcept
{
    return m_contentViewModel;
}

void ContentsBodyResourceRenderer::setContentViewModel(QObject* model)
{
    if (m_contentViewModel == model)
    {
        return;
    }

    disconnectContentViewModel();
    m_contentViewModel = model;

    if (m_contentViewModel != nullptr)
    {
        m_contentViewModelDestroyedConnection = connect(
            m_contentViewModel,
            &QObject::destroyed,
            this,
            &ContentsBodyResourceRenderer::handleContentViewModelDestroyed);

        if (m_contentViewModel->metaObject()->indexOfSignal("hubFilesystemMutated()") >= 0)
        {
            m_hubFilesystemMutatedConnection = connect(
                m_contentViewModel,
                SIGNAL(hubFilesystemMutated()),
                this,
                SLOT(handleContentFilesystemMutated()));
        }
    }

    emit contentViewModelChanged();
    refreshRenderedResources();
}

QString ContentsBodyResourceRenderer::noteId() const
{
    return m_noteId;
}

void ContentsBodyResourceRenderer::setNoteId(const QString& noteId)
{
    const QString normalizedNoteId = noteId.trimmed();
    if (m_noteId == normalizedNoteId)
    {
        return;
    }

    m_noteId = normalizedNoteId;
    emit noteIdChanged();
    refreshRenderedResources();
}

QString ContentsBodyResourceRenderer::bodySourceText() const
{
    return m_bodySourceText;
}

void ContentsBodyResourceRenderer::setBodySourceText(const QString& bodySourceText)
{
    if (m_bodySourceText == bodySourceText)
    {
        return;
    }

    m_bodySourceText = bodySourceText;
    emit bodySourceTextChanged();
    refreshRenderedResources();
}

int ContentsBodyResourceRenderer::maxRenderCount() const noexcept
{
    return m_maxRenderCount;
}

void ContentsBodyResourceRenderer::setMaxRenderCount(int maxRenderCount)
{
    const int normalizedMaxRenderCount = std::max(0, maxRenderCount);
    if (m_maxRenderCount == normalizedMaxRenderCount)
    {
        return;
    }

    m_maxRenderCount = normalizedMaxRenderCount;
    emit maxRenderCountChanged();
    refreshRenderedResources();
}

QVariantList ContentsBodyResourceRenderer::renderedResources() const
{
    return m_renderedResources;
}

int ContentsBodyResourceRenderer::resourceCount() const noexcept
{
    return m_renderedResources.size();
}

bool ContentsBodyResourceRenderer::hasRenderableResource() const noexcept
{
    return !m_renderedResources.isEmpty();
}

void ContentsBodyResourceRenderer::requestRenderRefresh()
{
    refreshRenderedResources();
}

void ContentsBodyResourceRenderer::handleContentViewModelDestroyed()
{
    disconnectContentViewModel();
    m_contentViewModel = nullptr;
    emit contentViewModelChanged();
    refreshRenderedResources();
}

void ContentsBodyResourceRenderer::handleContentFilesystemMutated()
{
    refreshRenderedResources();
}

bool ContentsBodyResourceRenderer::hasInvokableMethod(const QObject* object, const char* methodSignature)
{
    if (object == nullptr || methodSignature == nullptr)
    {
        return false;
    }

    return object->metaObject()->indexOfMethod(QMetaObject::normalizedSignature(methodSignature)) >= 0;
}

void ContentsBodyResourceRenderer::refreshRenderedResources()
{
    QVariantList nextRenderedResources;
    const QString noteDirectoryPath = resolveNoteDirectoryPath();
    if (!noteDirectoryPath.isEmpty())
    {
        nextRenderedResources = buildRenderedResources(noteDirectoryPath, m_bodySourceText);
    }

    if (m_maxRenderCount > 0 && nextRenderedResources.size() > m_maxRenderCount)
    {
        nextRenderedResources = nextRenderedResources.mid(0, m_maxRenderCount);
    }

    if (m_renderedResources == nextRenderedResources)
    {
        return;
    }

    m_renderedResources = nextRenderedResources;
    emit renderedResourcesChanged();
}

QVariantList ContentsBodyResourceRenderer::buildRenderedResources(
    const QString& noteDirectoryPath,
    const QString& bodySourceText) const
{
    const QString normalizedNoteDirectoryPath = normalizePath(noteDirectoryPath);
    if (normalizedNoteDirectoryPath.isEmpty())
    {
        return {};
    }

    const QFileInfo noteDirectoryInfo(normalizedNoteDirectoryPath);
    if (noteDirectoryInfo.isDir()
        && WhatSon::Resources::isResourcePackageDirectoryName(noteDirectoryInfo.fileName()))
    {
        WhatSon::Resources::ResourcePackageMetadata metadata;
        QString metadataError;
        if (!WhatSon::Resources::loadResourcePackageMetadata(
            normalizedNoteDirectoryPath,
            &metadata,
            &metadataError))
        {
            return {};
        }

        const QString resourcePath = metadata.resourcePath.trimmed().isEmpty()
                                         ? WhatSon::Resources::resourcePathForPackageDirectory(normalizedNoteDirectoryPath)
                                         : normalizePath(metadata.resourcePath);
        const QString resolvedResourceLocation = normalizePath(
            WhatSon::Resources::resolveAssetPathFromMetadata(normalizedNoteDirectoryPath, metadata));
        const QString sourceUrl = normalizeSourceUrl(resolvedResourceLocation);
        const QString type = WhatSon::Resources::normalizedTypeFromBucketAndFormat(
            metadata.type,
            metadata.bucket,
            metadata.format);
        QString format = normalizedFormat(metadata.format);
        if (format.isEmpty())
        {
            format = normalizedFormat(WhatSon::Resources::formatFromAssetFilePath(resolvedResourceLocation));
        }

        return {
            buildRenderedResourceMap(
                0,
                type,
                format,
                resourcePath,
                resolvedResourceLocation,
                sourceUrl,
                metadata.resourceId)
        };
    }

    const QString hubRootPath = findAncestorDirectoryWithSuffix(normalizedNoteDirectoryPath, QStringLiteral(".wshub"));
    const QStringList basePaths = WhatSon::Resources::resourceReferenceBasePathsForContext(
        normalizedNoteDirectoryPath,
        QString(),
        hubRootPath);

    const QString normalizedBodySourceText = bodySourceText.trimmed();
    if (!normalizedBodySourceText.isEmpty())
    {
        return buildRenderedResourcesFromSourceText(normalizedBodySourceText, basePaths);
    }

    const QString bodyPath = resolveBodyPathFromNoteDirectory(normalizedNoteDirectoryPath);
    if (bodyPath.isEmpty())
    {
        return {};
    }

    const QString bodyDocumentText = readUtf8File(bodyPath);
    if (bodyDocumentText.isEmpty())
    {
        return {};
    }

    return buildRenderedResourcesFromSourceText(extractBodyInnerText(bodyDocumentText), basePaths);
}

QString ContentsBodyResourceRenderer::resolveNoteDirectoryPath() const
{
    if (m_contentViewModel == nullptr || m_noteId.trimmed().isEmpty())
    {
        return {};
    }

    if (!hasInvokableMethod(m_contentViewModel, kNoteDirectoryPathForNoteIdSignature))
    {
        return {};
    }

    QString noteDirectoryPath;
    const bool invoked = QMetaObject::invokeMethod(
        m_contentViewModel,
        "noteDirectoryPathForNoteId",
        Qt::DirectConnection,
        Q_RETURN_ARG(QString, noteDirectoryPath),
        Q_ARG(QString, m_noteId));
    if (!invoked)
    {
        return {};
    }

    return normalizePath(noteDirectoryPath);
}

void ContentsBodyResourceRenderer::disconnectContentViewModel()
{
    if (m_contentViewModelDestroyedConnection)
    {
        disconnect(m_contentViewModelDestroyedConnection);
        m_contentViewModelDestroyedConnection = QMetaObject::Connection();
    }
    if (m_hubFilesystemMutatedConnection)
    {
        disconnect(m_hubFilesystemMutatedConnection);
        m_hubFilesystemMutatedConnection = QMetaObject::Connection();
    }
}
