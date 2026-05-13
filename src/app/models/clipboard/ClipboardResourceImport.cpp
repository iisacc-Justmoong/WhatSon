#include "app/models/clipboard/ClipboardResourceImport.h"

#include "app/models/file/hub/WhatSonHubPathUtils.hpp"
#include "app/models/hierarchy/resources/WhatSonResourcePackageSupport.hpp"

#include <QFileInfo>
#include <QHash>

namespace
{
    QString normalizedMimeType(QString value)
    {
        value = value.trimmed().toCaseFolded();
        const int parameterIndex = value.indexOf(QLatin1Char(';'));
        if (parameterIndex >= 0)
        {
            value = value.left(parameterIndex).trimmed();
        }
        return value;
    }

    QString normalizedFileNameOrDefault(const QString& fileName, const QString& format)
    {
        const QString trimmedFileName = QFileInfo(fileName.trimmed()).fileName().trimmed();
        if (!trimmedFileName.isEmpty())
        {
            return trimmedFileName;
        }
        return WhatSon::Clipboard::defaultClipboardResourceFileName(format);
    }

    QString normalizedFormatForFileType(const QString& fileName, const QString& mimeType)
    {
        QString format = WhatSon::Resources::formatFromAssetFilePath(fileName);
        if (format.trimmed().isEmpty())
        {
            format = WhatSon::Clipboard::formatFromMimeType(mimeType);
        }
        format = WhatSon::Resources::normalizeFormat(format);
        return format.trimmed().isEmpty() ? QStringLiteral(".bin") : format.toCaseFolded();
    }

    void applyResourceTaxonomy(WhatSon::Clipboard::ClipboardResourceImport* resourceImport)
    {
        if (resourceImport == nullptr)
        {
            return;
        }

        resourceImport->format = WhatSon::Resources::normalizeFormat(resourceImport->format).toCaseFolded();
        if (resourceImport->format.trimmed().isEmpty())
        {
            resourceImport->format = QStringLiteral(".bin");
        }
        resourceImport->type = WhatSon::Resources::inferTypeFromFormat(resourceImport->format);
        resourceImport->bucket = WhatSon::Resources::inferBucket(resourceImport->type, resourceImport->format);
        resourceImport->fileName = normalizedFileNameOrDefault(resourceImport->fileName, resourceImport->format);
    }
} // namespace

namespace WhatSon::Clipboard
{
    bool ClipboardResourceImport::valid() const noexcept
    {
        return !format.trimmed().isEmpty()
            && !type.trimmed().isEmpty();
    }

    bool ClipboardResourceImport::hasLocalFile() const noexcept
    {
        return !localFilePath.trimmed().isEmpty();
    }

    bool ClipboardResourceImport::hasMemoryPayload() const noexcept
    {
        return !payloadBytes.isEmpty() || !image.isNull();
    }

    QVariantMap ClipboardResourceImport::toVariantMap() const
    {
        QVariantMap entry;
        entry.insert(QStringLiteral("fileName"), fileName.trimmed());
        entry.insert(QStringLiteral("localFilePath"), localFilePath.trimmed());
        entry.insert(QStringLiteral("mimeType"), mimeType.trimmed());
        entry.insert(QStringLiteral("format"), Resources::normalizeFormat(format).toCaseFolded());
        entry.insert(QStringLiteral("type"), type.trimmed().toCaseFolded());
        entry.insert(QStringLiteral("bucket"), bucket.trimmed());
        entry.insert(QStringLiteral("hasPayloadBytes"), !payloadBytes.isEmpty());
        entry.insert(QStringLiteral("hasImage"), !image.isNull());
        return entry;
    }

    QString formatFromMimeType(const QString& mimeType)
    {
        static const QHash<QString, QString> kFormatsByMimeType = {
            {QStringLiteral("image/png"), QStringLiteral(".png")},
            {QStringLiteral("image/jpeg"), QStringLiteral(".jpg")},
            {QStringLiteral("image/jpg"), QStringLiteral(".jpg")},
            {QStringLiteral("image/tiff"), QStringLiteral(".tiff")},
            {QStringLiteral("image/x-tiff"), QStringLiteral(".tiff")},
            {QStringLiteral("image/webp"), QStringLiteral(".webp")},
            {QStringLiteral("image/gif"), QStringLiteral(".gif")},
            {QStringLiteral("image/bmp"), QStringLiteral(".bmp")},
            {QStringLiteral("image/heic"), QStringLiteral(".heic")},
            {QStringLiteral("image/heif"), QStringLiteral(".heic")},
            {QStringLiteral("image/avif"), QStringLiteral(".avif")},
            {QStringLiteral("public.png"), QStringLiteral(".png")},
            {QStringLiteral("public.jpeg"), QStringLiteral(".jpg")},
            {QStringLiteral("public.tiff"), QStringLiteral(".tiff")},
            {QStringLiteral("video/mp4"), QStringLiteral(".mp4")},
            {QStringLiteral("video/quicktime"), QStringLiteral(".mov")},
            {QStringLiteral("video/x-msvideo"), QStringLiteral(".avi")},
            {QStringLiteral("video/x-matroska"), QStringLiteral(".mkv")},
            {QStringLiteral("video/webm"), QStringLiteral(".webm")},
            {QStringLiteral("application/pdf"), QStringLiteral(".pdf")},
            {QStringLiteral("application/msword"), QStringLiteral(".doc")},
            {QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document"), QStringLiteral(".docx")},
            {QStringLiteral("text/plain"), QStringLiteral(".txt")},
            {QStringLiteral("text/markdown"), QStringLiteral(".md")},
            {QStringLiteral("text/html"), QStringLiteral(".html")},
            {QStringLiteral("application/rtf"), QStringLiteral(".rtf")},
            {QStringLiteral("application/vnd.ms-powerpoint"), QStringLiteral(".ppt")},
            {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.presentation"), QStringLiteral(".pptx")},
            {QStringLiteral("application/vnd.ms-excel"), QStringLiteral(".xls")},
            {QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"), QStringLiteral(".xlsx")},
            {QStringLiteral("text/csv"), QStringLiteral(".csv")},
            {QStringLiteral("model/gltf+json"), QStringLiteral(".gltf")},
            {QStringLiteral("model/gltf-binary"), QStringLiteral(".glb")},
            {QStringLiteral("model/obj"), QStringLiteral(".obj")},
            {QStringLiteral("audio/mpeg"), QStringLiteral(".mp3")},
            {QStringLiteral("audio/aac"), QStringLiteral(".aac")},
            {QStringLiteral("audio/mp4"), QStringLiteral(".m4a")},
            {QStringLiteral("audio/flac"), QStringLiteral(".flac")},
            {QStringLiteral("audio/wav"), QStringLiteral(".wav")},
            {QStringLiteral("audio/x-wav"), QStringLiteral(".wav")},
            {QStringLiteral("audio/ogg"), QStringLiteral(".ogg")},
            {QStringLiteral("audio/opus"), QStringLiteral(".opus")},
            {QStringLiteral("application/zip"), QStringLiteral(".zip")},
            {QStringLiteral("application/x-zip-compressed"), QStringLiteral(".zip")},
            {QStringLiteral("application/x-7z-compressed"), QStringLiteral(".7z")},
            {QStringLiteral("application/x-rar-compressed"), QStringLiteral(".rar")},
            {QStringLiteral("application/gzip"), QStringLiteral(".gz")},
            {QStringLiteral("application/x-tar"), QStringLiteral(".tar")}
        };

        const QString normalized = normalizedMimeType(mimeType);
        if (kFormatsByMimeType.contains(normalized))
        {
            return kFormatsByMimeType.value(normalized);
        }

        if (normalized.startsWith(QStringLiteral("image/")))
        {
            return QStringLiteral(".png");
        }
        if (normalized == QStringLiteral("application/octet-stream"))
        {
            return QStringLiteral(".bin");
        }
        return {};
    }

    QString defaultClipboardResourceFileName(const QString& format)
    {
        const QString normalizedFormat = Resources::normalizeFormat(format).toCaseFolded();
        return QStringLiteral("clipboard-resource%1").arg(
            normalizedFormat.trimmed().isEmpty() ? QStringLiteral(".bin") : normalizedFormat);
    }

    ClipboardResourceImport resourceImportForFileType(const QString& fileName, const QString& mimeType)
    {
        ClipboardResourceImport resourceImport;
        resourceImport.fileName = fileName;
        resourceImport.mimeType = normalizedMimeType(mimeType);
        resourceImport.format = normalizedFormatForFileType(fileName, mimeType);
        applyResourceTaxonomy(&resourceImport);
        return resourceImport;
    }

    ClipboardResourceImport resourceImportForLocalFile(const QString& localFilePath, const QString& mimeType)
    {
        ClipboardResourceImport resourceImport = resourceImportForFileType(QFileInfo(localFilePath).fileName(), mimeType);
        resourceImport.localFilePath = WhatSon::HubPath::normalizeAbsolutePath(localFilePath);
        return resourceImport;
    }

    ClipboardResourceImport resourceImportForImage(const QImage& image, const QString& mimeType)
    {
        ClipboardResourceImport resourceImport = resourceImportForFileType(QString(), mimeType);
        if (resourceImport.type != QStringLiteral("image"))
        {
            resourceImport.mimeType = QStringLiteral("image/png");
            resourceImport.format = QStringLiteral(".png");
            applyResourceTaxonomy(&resourceImport);
        }
        resourceImport.image = image;
        return resourceImport;
    }

    ClipboardResourceImport resourceImportForBytes(
        const QByteArray& bytes,
        const QString& fileName,
        const QString& mimeType)
    {
        ClipboardResourceImport resourceImport = resourceImportForFileType(fileName, mimeType);
        resourceImport.payloadBytes = bytes;
        return resourceImport;
    }
} // namespace WhatSon::Clipboard
