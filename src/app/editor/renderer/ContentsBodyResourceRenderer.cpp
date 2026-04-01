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
        nextRenderedResources = buildRenderedResources(noteDirectoryPath);
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

QVariantList ContentsBodyResourceRenderer::buildRenderedResources(const QString& noteDirectoryPath) const
{
    const QString normalizedNoteDirectoryPath = normalizePath(noteDirectoryPath);
    if (normalizedNoteDirectoryPath.isEmpty())
    {
        return {};
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

    const QString hubRootPath = findAncestorDirectoryWithSuffix(normalizedNoteDirectoryPath, QStringLiteral(".wshub"));
    const QStringList basePaths = WhatSon::Resources::resourceReferenceBasePathsForContext(
        normalizedNoteDirectoryPath,
        QString(),
        hubRootPath);

    static const QRegularExpression resourceTagPattern(
        QStringLiteral(R"(<resource\b[^>]*?/?>)"),
        QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator iterator = resourceTagPattern.globalMatch(bodyInnerText);

    QVariantList resources;
    int resourceIndex = 0;
    while (iterator.hasNext())
    {
        const QRegularExpressionMatch match = iterator.next();
        const QString resourceTagText = match.captured(0);

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
        const QString resolvedResourceLocation = WhatSon::Resources::resolveAssetLocationFromReference(
            resourcePath,
            basePaths);
        const QString sourceUrl = normalizeSourceUrl(resolvedResourceLocation);
        const bool imageResource = isImageResource(type, format, resolvedResourceLocation, resourcePath);

        QVariantMap resource;
        resource.insert(QStringLiteral("index"), resourceIndex);
        resource.insert(QStringLiteral("type"), type);
        resource.insert(QStringLiteral("format"), format);
        resource.insert(QStringLiteral("resourcePath"), resourcePath);
        resource.insert(QStringLiteral("resolvedPath"), resolvedResourceLocation);
        resource.insert(QStringLiteral("source"), sourceUrl);
        resource.insert(
            QStringLiteral("renderMode"),
            imageResource && !sourceUrl.isEmpty() ? QStringLiteral("image") : QStringLiteral("unsupported"));
        resources.push_back(resource);
        ++resourceIndex;
    }

    return resources;
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
