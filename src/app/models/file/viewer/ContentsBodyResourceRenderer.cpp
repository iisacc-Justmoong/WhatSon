#include "ContentsBodyResourceRenderer.hpp"

#include "models/file/WhatSonDebugTrace.hpp"
#include "models/file/hierarchy/resources/WhatSonResourcePackageSupport.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaObject>
#include <QUrl>

#include <algorithm>

namespace
{
    constexpr auto kNoteDirectoryPathForNoteIdSignature = "noteDirectoryPathForNoteId(QString)";

    QString normalizePath(QString path)
    {
        path = path.trimmed();
        if (path.isEmpty())
        {
            return {};
        }
        return QDir::cleanPath(path);
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

    QString resolvePrimaryHubChildDirectory(
        const QString& hubRootPath,
        const QString& fixedDirectoryName,
        const QString& wildcardPattern)
    {
        const QString normalizedHubRootPath = normalizePath(hubRootPath);
        if (normalizedHubRootPath.isEmpty() || !QFileInfo(normalizedHubRootPath).isDir())
        {
            return {};
        }

        const QDir hubDir(normalizedHubRootPath);
        const QString fixedPath = hubDir.filePath(fixedDirectoryName);
        if (QFileInfo(fixedPath).isDir())
        {
            return normalizePath(fixedPath);
        }

        const QStringList candidates = hubDir.entryList(
            QStringList{wildcardPattern},
            QDir::Dirs | QDir::NoDotAndDotDot,
            QDir::Name);
        if (candidates.isEmpty())
        {
            return {};
        }

        return normalizePath(hubDir.filePath(candidates.constFirst()));
    }

    void appendExistingDirectoryPath(QStringList* candidateBasePaths, const QString& path)
    {
        if (candidateBasePaths == nullptr)
        {
            return;
        }

        const QString normalizedPath = normalizePath(path);
        if (normalizedPath.isEmpty() || !QFileInfo(normalizedPath).isDir())
        {
            return;
        }

        if (!candidateBasePaths->contains(normalizedPath))
        {
            candidateBasePaths->push_back(normalizedPath);
        }
    }

    QStringList resourceReferenceBasePathsForNoteDirectory(const QString& noteDirectoryPath)
    {
        QStringList candidateBasePaths;

        const QString normalizedNoteDirectoryPath = normalizePath(noteDirectoryPath);
        if (!normalizedNoteDirectoryPath.isEmpty())
        {
            appendExistingDirectoryPath(&candidateBasePaths, normalizedNoteDirectoryPath);
        }

        const QString hubRootPath = findAncestorDirectoryWithSuffix(
            normalizedNoteDirectoryPath,
            QStringLiteral(".wshub"));
        candidateBasePaths.append(
            WhatSon::Resources::resourceReferenceBasePathsForContext(
                normalizedNoteDirectoryPath,
                QString(),
                hubRootPath));
        candidateBasePaths.removeAll(QString());
        candidateBasePaths.removeDuplicates();

        QFileInfo noteLocationInfo(normalizedNoteDirectoryPath);
        QDir currentDirectory = noteLocationInfo.isDir()
            ? QDir(noteLocationInfo.absoluteFilePath())
            : noteLocationInfo.absoluteDir();
        const QString normalizedHubRootPath = normalizePath(hubRootPath);
        while (currentDirectory.exists())
        {
            appendExistingDirectoryPath(&candidateBasePaths, currentDirectory.absolutePath());
            if (!normalizedHubRootPath.isEmpty()
                && normalizePath(currentDirectory.absolutePath()) == normalizedHubRootPath)
            {
                break;
            }
            if (!currentDirectory.cdUp())
            {
                break;
            }
        }

        if (!normalizedHubRootPath.isEmpty())
        {
            appendExistingDirectoryPath(
                &candidateBasePaths,
                resolvePrimaryHubChildDirectory(
                    normalizedHubRootPath,
                    QStringLiteral(".wscontents"),
                    QStringLiteral("*.wscontents")));

            const QStringList resourceRootDirectories =
                WhatSon::Resources::resolveResourceRootDirectories(normalizedHubRootPath);
            for (const QString& resourceRootDirectory : resourceRootDirectories)
            {
                appendExistingDirectoryPath(&candidateBasePaths, resourceRootDirectory);
            }
        }

        candidateBasePaths.removeAll(QString());
        candidateBasePaths.removeDuplicates();
        return candidateBasePaths;
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

    QVariantList buildRenderedResourcesFromDocumentBlocks(
        const QVariantList& documentBlocks,
        const QStringList& basePaths)
    {
        QVariantList resources;
        int nextFallbackResourceIndex = 0;
        // Resource payload now follows the parser-owned block stream instead of reparsing note RAW separately.
        for (const QVariant& blockValue : documentBlocks)
        {
            const QVariantMap block = blockValue.toMap();
            const QString blockType =
                block.value(QStringLiteral("type")).toString().trimmed().toCaseFolded();
            if (blockType != QStringLiteral("resource"))
            {
                continue;
            }

            bool resourceIndexOk = false;
            const int parsedResourceIndex =
                block.value(QStringLiteral("resourceIndex")).toInt(&resourceIndexOk);
            const int resourceIndex = std::max(
                0,
                resourceIndexOk ? parsedResourceIndex : nextFallbackResourceIndex);
            QString effectiveType =
                block.value(QStringLiteral("resourceType")).toString().trimmed().toCaseFolded();
            QString effectiveFormat =
                normalizedFormat(block.value(QStringLiteral("resourceFormat")).toString());
            QString effectiveResourceId =
                block.value(QStringLiteral("resourceId")).toString().trimmed();
            QString effectiveResourcePath =
                block.value(QStringLiteral("resourcePath")).toString().trimmed();
            QString resolvedResourceLocation = WhatSon::Resources::resolveAssetLocationFromReference(
                effectiveResourcePath,
                basePaths);

            const QString resolvedPackagePath = WhatSon::Resources::resolvePackageDirectoryFromReference(
                effectiveResourcePath,
                basePaths);
            if (!resolvedPackagePath.isEmpty())
            {
                WhatSon::Resources::ResourcePackageMetadata metadata;
                QString metadataError;
                if (WhatSon::Resources::loadResourcePackageMetadata(
                    resolvedPackagePath,
                    &metadata,
                    &metadataError))
                {
                    if (!metadata.resourceId.trimmed().isEmpty())
                    {
                        effectiveResourceId = metadata.resourceId.trimmed();
                    }
                    if (!metadata.resourcePath.trimmed().isEmpty())
                    {
                        effectiveResourcePath = normalizePath(metadata.resourcePath);
                    }

                    const QString metadataType =
                        WhatSon::Resources::normalizedTypeFromBucketAndFormat(
                            metadata.type,
                            metadata.bucket,
                            metadata.format);
                    const QString metadataFormat = normalizedFormat(metadata.format);
                    if (!metadataType.isEmpty())
                    {
                        effectiveType = metadataType;
                    }
                    if (!metadataFormat.isEmpty())
                    {
                        effectiveFormat = metadataFormat;
                    }

                    if (resolvedResourceLocation.isEmpty())
                    {
                        resolvedResourceLocation = normalizePath(
                            WhatSon::Resources::resolveAssetPathFromMetadata(
                                resolvedPackagePath,
                                metadata));
                    }
                }
            }

            const QString sourceUrl = normalizeSourceUrl(resolvedResourceLocation);
            resources.push_back(
                buildRenderedResourceMap(
                    resourceIndex,
                    effectiveType,
                    effectiveFormat,
                    effectiveResourcePath,
                    resolvedResourceLocation,
                    sourceUrl,
                    effectiveResourceId,
                    std::max(0, block.value(QStringLiteral("sourceStart")).toInt()),
                    std::max(
                        std::max(0, block.value(QStringLiteral("sourceStart")).toInt()),
                        block.value(QStringLiteral("sourceEnd")).toInt())));
            nextFallbackResourceIndex = std::max(nextFallbackResourceIndex, resourceIndex + 1);
        }

        return resources;
    }
} // namespace

ContentsBodyResourceRenderer::ContentsBodyResourceRenderer(QObject* parent)
    : QObject(parent)
{
    WhatSon::Debug::traceEditorSelf(this, QStringLiteral("bodyResourceRenderer"), QStringLiteral("ctor"));
}

ContentsBodyResourceRenderer::~ContentsBodyResourceRenderer()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("bodyResourceRenderer"),
        QStringLiteral("dtor"),
        QStringLiteral("noteId=%1 resourceCount=%2").arg(m_noteId).arg(m_renderedResources.size()));
}

QObject* ContentsBodyResourceRenderer::contentViewModel() const noexcept
{
    return m_contentViewModel;
}

QObject* ContentsBodyResourceRenderer::fallbackContentViewModel() const noexcept
{
    return m_fallbackContentViewModel;
}

void ContentsBodyResourceRenderer::setContentViewModel(QObject* model)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("bodyResourceRenderer"),
        QStringLiteral("setContentViewModel"),
        QStringLiteral("next=0x%1").arg(QString::number(reinterpret_cast<quintptr>(model), 16)));
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

void ContentsBodyResourceRenderer::setFallbackContentViewModel(QObject* model)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("bodyResourceRenderer"),
        QStringLiteral("setFallbackContentViewModel"),
        QStringLiteral("next=0x%1").arg(QString::number(reinterpret_cast<quintptr>(model), 16)));
    if (m_fallbackContentViewModel == model)
    {
        return;
    }

    disconnectFallbackContentViewModel();
    m_fallbackContentViewModel = model;

    if (m_fallbackContentViewModel != nullptr)
    {
        m_fallbackContentViewModelDestroyedConnection = connect(
            m_fallbackContentViewModel,
            &QObject::destroyed,
            this,
            &ContentsBodyResourceRenderer::handleFallbackContentViewModelDestroyed);

        if (m_fallbackContentViewModel->metaObject()->indexOfSignal("hubFilesystemMutated()") >= 0)
        {
            m_fallbackHubFilesystemMutatedConnection = connect(
                m_fallbackContentViewModel,
                SIGNAL(hubFilesystemMutated()),
                this,
                SLOT(handleContentFilesystemMutated()));
        }
    }

    emit fallbackContentViewModelChanged();
    refreshRenderedResources();
}

QString ContentsBodyResourceRenderer::noteId() const
{
    return m_noteId;
}

void ContentsBodyResourceRenderer::setNoteId(const QString& noteId)
{
    const QString normalizedNoteId = noteId.trimmed();
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("bodyResourceRenderer"),
        QStringLiteral("setNoteId"),
        QStringLiteral("previous=%1 next=%2").arg(m_noteId).arg(normalizedNoteId));
    if (m_noteId == normalizedNoteId)
    {
        return;
    }

    m_noteId = normalizedNoteId;
    emit noteIdChanged();
    refreshRenderedResources();
}

QString ContentsBodyResourceRenderer::noteDirectoryPath() const
{
    return m_noteDirectoryPath;
}

void ContentsBodyResourceRenderer::setNoteDirectoryPath(const QString& noteDirectoryPath)
{
    const QString normalizedNoteDirectoryPath = normalizePath(noteDirectoryPath);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("bodyResourceRenderer"),
        QStringLiteral("setNoteDirectoryPath"),
        QStringLiteral("previous=%1 next=%2").arg(m_noteDirectoryPath).arg(normalizedNoteDirectoryPath));
    if (m_noteDirectoryPath == normalizedNoteDirectoryPath)
    {
        return;
    }

    m_noteDirectoryPath = normalizedNoteDirectoryPath;
    emit noteDirectoryPathChanged();
    refreshRenderedResources();
}

QVariantList ContentsBodyResourceRenderer::documentBlocks() const
{
    return m_documentBlocks;
}

void ContentsBodyResourceRenderer::setDocumentBlocks(const QVariantList& documentBlocks)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("bodyResourceRenderer"),
        QStringLiteral("setDocumentBlocks"),
        QStringLiteral("previousCount=%1 nextCount=%2")
            .arg(m_documentBlocks.size())
            .arg(documentBlocks.size()));
    if (m_documentBlocks == documentBlocks)
    {
        return;
    }

    m_documentBlocks = documentBlocks;
    emit documentBlocksChanged();
    refreshRenderedResources();
}

int ContentsBodyResourceRenderer::maxRenderCount() const noexcept
{
    return m_maxRenderCount;
}

void ContentsBodyResourceRenderer::setMaxRenderCount(int maxRenderCount)
{
    const int normalizedMaxRenderCount = std::max(0, maxRenderCount);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("bodyResourceRenderer"),
        QStringLiteral("setMaxRenderCount"),
        QStringLiteral("previous=%1 next=%2").arg(m_maxRenderCount).arg(normalizedMaxRenderCount));
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
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("bodyResourceRenderer"),
        QStringLiteral("requestRenderRefresh"),
        QStringLiteral("noteId=%1 blockCount=%2").arg(m_noteId).arg(m_documentBlocks.size()));
    refreshRenderedResources();
}

void ContentsBodyResourceRenderer::handleContentViewModelDestroyed()
{
    WhatSon::Debug::traceEditorSelf(this, QStringLiteral("bodyResourceRenderer"), QStringLiteral("handleContentViewModelDestroyed"));
    disconnectContentViewModel();
    m_contentViewModel = nullptr;
    emit contentViewModelChanged();
    refreshRenderedResources();
}

void ContentsBodyResourceRenderer::handleFallbackContentViewModelDestroyed()
{
    WhatSon::Debug::traceEditorSelf(this, QStringLiteral("bodyResourceRenderer"), QStringLiteral("handleFallbackContentViewModelDestroyed"));
    disconnectFallbackContentViewModel();
    m_fallbackContentViewModel = nullptr;
    emit fallbackContentViewModelChanged();
    refreshRenderedResources();
}

void ContentsBodyResourceRenderer::handleContentFilesystemMutated()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("bodyResourceRenderer"),
        QStringLiteral("handleContentFilesystemMutated"),
        QStringLiteral("noteId=%1").arg(m_noteId));
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
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("bodyResourceRenderer"),
        QStringLiteral("refreshRenderedResources"),
        QStringLiteral("noteId=%1 blockCount=%2 maxRenderCount=%3")
            .arg(m_noteId)
            .arg(m_documentBlocks.size())
            .arg(m_maxRenderCount));
    QVariantList nextRenderedResources;
    const QString noteDirectoryPath = resolveNoteDirectoryPath();
    if (!noteDirectoryPath.isEmpty())
    {
        nextRenderedResources = buildRenderedResources(noteDirectoryPath, m_documentBlocks);
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
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("bodyResourceRenderer"),
        QStringLiteral("renderedResourcesChanged"),
        QStringLiteral("resourceCount=%1 noteDirectoryPath=%2")
            .arg(m_renderedResources.size())
            .arg(noteDirectoryPath));
}

QVariantList ContentsBodyResourceRenderer::buildRenderedResources(
    const QString& noteDirectoryPath,
    const QVariantList& documentBlocks) const
{
    const QString normalizedNoteDirectoryPath = normalizePath(noteDirectoryPath);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("bodyResourceRenderer"),
        QStringLiteral("buildRenderedResources"),
        QStringLiteral("noteDirectoryPath=%1 blockCount=%2")
            .arg(normalizedNoteDirectoryPath)
            .arg(documentBlocks.size()));
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

    const QStringList basePaths = resourceReferenceBasePathsForNoteDirectory(normalizedNoteDirectoryPath);
    return buildRenderedResourcesFromDocumentBlocks(documentBlocks, basePaths);
}

QString ContentsBodyResourceRenderer::resolveNoteDirectoryPathFromViewModel(QObject* viewModel) const
{
    if (viewModel == nullptr || m_noteId.trimmed().isEmpty())
    {
        return {};
    }

    if (!hasInvokableMethod(viewModel, kNoteDirectoryPathForNoteIdSignature))
    {
        return {};
    }

    QString noteDirectoryPath;
    const bool invoked = QMetaObject::invokeMethod(
        viewModel,
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

QString ContentsBodyResourceRenderer::resolveNoteDirectoryPath() const
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("bodyResourceRenderer"),
        QStringLiteral("resolveNoteDirectoryPath"),
        QStringLiteral("noteId=%1 explicitPath=%2").arg(m_noteId).arg(m_noteDirectoryPath));
    if (!m_noteDirectoryPath.trimmed().isEmpty())
    {
        return normalizePath(m_noteDirectoryPath);
    }

    if (m_noteId.trimmed().isEmpty())
    {
        return {};
    }

    const QString primaryPath = resolveNoteDirectoryPathFromViewModel(m_contentViewModel);
    if (!primaryPath.isEmpty())
    {
        return primaryPath;
    }

    if (m_fallbackContentViewModel != nullptr
        && m_fallbackContentViewModel != m_contentViewModel)
    {
        return resolveNoteDirectoryPathFromViewModel(m_fallbackContentViewModel);
    }
    return {};
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

void ContentsBodyResourceRenderer::disconnectFallbackContentViewModel()
{
    if (m_fallbackContentViewModelDestroyedConnection)
    {
        disconnect(m_fallbackContentViewModelDestroyedConnection);
        m_fallbackContentViewModelDestroyedConnection = QMetaObject::Connection();
    }
    if (m_fallbackHubFilesystemMutatedConnection)
    {
        disconnect(m_fallbackHubFilesystemMutatedConnection);
        m_fallbackHubFilesystemMutatedConnection = QMetaObject::Connection();
    }
}
