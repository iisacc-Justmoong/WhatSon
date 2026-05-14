#include "app/models/clipboard/ClipboardResourceImport.h"

#include "app/models/clipboard/FiletypeCapture.h"
#include "app/models/file/hub/WhatSonHubPathUtils.hpp"
#include "app/models/hierarchy/resources/WhatSonResourcePackageSupport.hpp"

#include <QFileInfo>

namespace
{
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
        resourceImport->fileName =
            WhatSon::Clipboard::FiletypeCapture::normalizedFileNameOrDefault(
                resourceImport->fileName,
                resourceImport->format);
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

    ClipboardResourceImport resourceImportForFileType(const QString& fileName, const QString& mimeType)
    {
        ClipboardResourceImport resourceImport;
        resourceImport.fileName = fileName;
        resourceImport.mimeType = FiletypeCapture::normalizeMimeType(mimeType);
        resourceImport.format = FiletypeCapture::normalizedFormatForFileType(fileName, mimeType);
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
