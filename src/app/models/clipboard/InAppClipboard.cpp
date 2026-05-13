#include "app/models/clipboard/InAppClipboard.h"

#include <QByteArray>
#include <QClipboard>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QMimeData>
#include <QPixmap>
#include <QRegularExpression>
#include <QUrl>
#include <QVariant>

#include <utility>

namespace
{
    QString normalizedMimeFormat(QString value)
    {
        value = value.trimmed().toCaseFolded();
        const int parameterIndex = value.indexOf(QLatin1Char(';'));
        if (parameterIndex >= 0)
        {
            value = value.left(parameterIndex).trimmed();
        }
        return value;
    }

    bool mimeFormatLooksLikeSupportedResource(const QString& mimeFormat)
    {
        return !WhatSon::Clipboard::formatFromMimeType(mimeFormat).trimmed().isEmpty();
    }

    QByteArray payloadForMimeFormat(const QMimeData* mimeData, const QString& mimeFormat)
    {
        if (mimeData == nullptr)
        {
            return {};
        }

        QByteArray payload = mimeData->data(mimeFormat);
        if (!payload.isEmpty())
        {
            return payload;
        }

        const QString normalized = normalizedMimeFormat(mimeFormat);
        if ((normalized == QStringLiteral("text/plain")
             || normalized == QStringLiteral("public.plain-text")
             || normalized == QStringLiteral("text/markdown")
             || normalized == QStringLiteral("text/csv"))
            && mimeData->hasText())
        {
            return mimeData->text().toUtf8();
        }

        if ((normalized == QStringLiteral("text/html")
             || normalized == QStringLiteral("public.html"))
            && mimeData->hasHtml())
        {
            return mimeData->html().toUtf8();
        }

        return {};
    }

    QString firstClipboardImageDataUrl(const QMimeData* mimeData)
    {
        if (mimeData == nullptr)
        {
            return {};
        }

        const auto extractFromText = [](const QString& text) -> QString
        {
            const QString trimmedText = text.trimmed();
            if (trimmedText.startsWith(QStringLiteral("data:image/"), Qt::CaseInsensitive))
            {
                return trimmedText;
            }

            static const QRegularExpression quotedImageSrcPattern(
                QStringLiteral(R"(src\s*=\s*["'](data:image\/[^"']+)["'])"),
                QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch match = quotedImageSrcPattern.match(text);
            if (match.hasMatch())
            {
                return match.captured(1).trimmed();
            }

            static const QRegularExpression bareDataUrlPattern(
                QStringLiteral(R"((data:image\/[^\s"'<>]+))"),
                QRegularExpression::CaseInsensitiveOption);
            match = bareDataUrlPattern.match(text);
            return match.hasMatch() ? match.captured(1).trimmed() : QString();
        };

        if (mimeData->hasHtml())
        {
            const QString htmlDataUrl = extractFromText(mimeData->html());
            if (!htmlDataUrl.isEmpty())
            {
                return htmlDataUrl;
            }
        }
        if (mimeData->hasText())
        {
            const QString textDataUrl = extractFromText(mimeData->text());
            if (!textDataUrl.isEmpty())
            {
                return textDataUrl;
            }
        }
        return {};
    }

    QByteArray decodedClipboardImageDataUrlPayload(const QString& dataUrl)
    {
        const QString normalizedDataUrl = dataUrl.trimmed();
        if (!normalizedDataUrl.startsWith(QStringLiteral("data:image/"), Qt::CaseInsensitive))
        {
            return {};
        }

        const int commaIndex = normalizedDataUrl.indexOf(QLatin1Char(','));
        if (commaIndex <= 0)
        {
            return {};
        }

        const QString header = normalizedDataUrl.left(commaIndex);
        QString payload = normalizedDataUrl.mid(commaIndex + 1);
        if (payload.isEmpty())
        {
            return {};
        }

        if (header.contains(QStringLiteral(";base64"), Qt::CaseInsensitive))
        {
            payload.remove(QRegularExpression(QStringLiteral("\\s+")));
            return QByteArray::fromBase64(payload.toUtf8());
        }

        return QByteArray::fromPercentEncoding(payload.toUtf8());
    }

    QImage imageFromVariant(const QVariant& imageVariant)
    {
        if (imageVariant.canConvert<QImage>())
        {
            return qvariant_cast<QImage>(imageVariant);
        }
        if (imageVariant.canConvert<QPixmap>())
        {
            return qvariant_cast<QPixmap>(imageVariant).toImage();
        }
        return {};
    }
} // namespace

InAppClipboard::InAppClipboard(QObject* parent)
    : QObject(parent)
{
}

InAppClipboard::~InAppClipboard() = default;

bool InAppClipboard::hasResource() const noexcept
{
    return m_resourceImport.valid();
}

QString InAppClipboard::resourceFileName() const
{
    return m_resourceImport.fileName.trimmed();
}

QString InAppClipboard::resourceFormat() const
{
    return m_resourceImport.format.trimmed();
}

QString InAppClipboard::resourceType() const
{
    return m_resourceImport.type.trimmed();
}

QString InAppClipboard::resourceBucket() const
{
    return m_resourceImport.bucket.trimmed();
}

QString InAppClipboard::resourceMimeType() const
{
    return m_resourceImport.mimeType.trimmed();
}

QVariantMap InAppClipboard::resourceEntry() const
{
    return m_resourceImport.toVariantMap();
}

const WhatSon::Clipboard::ClipboardResourceImport& InAppClipboard::resourceImport() const noexcept
{
    return m_resourceImport;
}

WhatSon::Clipboard::ClipboardResourceImport InAppClipboard::takeResourceImport()
{
    WhatSon::Clipboard::ClipboardResourceImport resourceImport = m_resourceImport;
    clear();
    return resourceImport;
}

bool InAppClipboard::captureSystemClipboardResource()
{
    return captureResourceFromClipboard(QGuiApplication::clipboard());
}

bool InAppClipboard::captureResourceFromClipboard(const QClipboard* clipboard)
{
    if (clipboard == nullptr)
    {
        clear();
        return false;
    }

    if (captureResourceFromMimeData(clipboard->mimeData()))
    {
        return true;
    }

    const QImage clipboardImage = clipboard->image();
    if (!clipboardImage.isNull())
    {
        return setImageResource(clipboardImage);
    }

    const QPixmap clipboardPixmap = clipboard->pixmap();
    if (!clipboardPixmap.isNull())
    {
        return setImageResource(clipboardPixmap.toImage());
    }

    clear();
    return false;
}

bool InAppClipboard::captureResourceFromMimeData(const QMimeData* mimeData)
{
    if (mimeData == nullptr)
    {
        clear();
        return false;
    }

    const QList<QUrl> urls = mimeData->urls();
    for (const QUrl& url : urls)
    {
        if (url.isValid() && url.isLocalFile())
        {
            if (setResourceLocalFile(url.toLocalFile()))
            {
                return true;
            }
        }
    }

    const QStringList formats = mimeData->formats();
    for (const QString& mimeFormat : formats)
    {
        if (!mimeFormatLooksLikeSupportedResource(mimeFormat))
        {
            continue;
        }

        const QByteArray payload = payloadForMimeFormat(mimeData, mimeFormat);
        QImage image;
        if (payload.isEmpty() || image.loadFromData(payload))
        {
            if (!image.isNull())
            {
                return setImageResource(image, mimeFormat);
            }
        }
        if (!payload.isEmpty())
        {
            return setResourceImport(WhatSon::Clipboard::resourceImportForBytes(
                payload,
                QString(),
                mimeFormat));
        }
    }

    const QString imageDataUrl = firstClipboardImageDataUrl(mimeData);
    if (!imageDataUrl.isEmpty())
    {
        QImage image;
        const QByteArray imageBytes = decodedClipboardImageDataUrlPayload(imageDataUrl);
        if (!imageBytes.isEmpty() && image.loadFromData(imageBytes) && !image.isNull())
        {
            return setImageResource(image);
        }
    }

    if (mimeData->hasImage())
    {
        const QImage image = imageFromVariant(mimeData->imageData());
        if (!image.isNull())
        {
            return setImageResource(image);
        }
    }

    clear();
    return false;
}

bool InAppClipboard::setResourceFileType(const QString& fileName, const QString& mimeType)
{
    WhatSon::Clipboard::ClipboardResourceImport resourceImport =
        WhatSon::Clipboard::resourceImportForFileType(fileName, mimeType);
    return setResourceImport(std::move(resourceImport));
}

bool InAppClipboard::setResourceLocalFile(const QString& localFilePath, const QString& mimeType)
{
    const QString trimmedLocalFilePath = localFilePath.trimmed();
    if (trimmedLocalFilePath.isEmpty() || !QFileInfo(trimmedLocalFilePath).isFile())
    {
        clear();
        return false;
    }

    WhatSon::Clipboard::ClipboardResourceImport resourceImport =
        WhatSon::Clipboard::resourceImportForLocalFile(trimmedLocalFilePath, mimeType);
    return setResourceImport(std::move(resourceImport));
}

bool InAppClipboard::setResourceBytes(
    const QByteArray& bytes,
    const QString& fileName,
    const QString& mimeType)
{
    if (bytes.isEmpty())
    {
        clear();
        return false;
    }

    WhatSon::Clipboard::ClipboardResourceImport resourceImport =
        WhatSon::Clipboard::resourceImportForBytes(bytes, fileName, mimeType);
    return setResourceImport(std::move(resourceImport));
}

bool InAppClipboard::setResourceText(
    const QString& text,
    const QString& fileName,
    const QString& mimeType)
{
    if (text.isEmpty())
    {
        clear();
        return false;
    }

    return setResourceBytes(text.toUtf8(), fileName, mimeType);
}

bool InAppClipboard::setImageResource(const QImage& image, const QString& mimeType)
{
    if (image.isNull())
    {
        clear();
        return false;
    }

    return setResourceImport(WhatSon::Clipboard::resourceImportForImage(image, mimeType));
}

bool InAppClipboard::setResourceImport(WhatSon::Clipboard::ClipboardResourceImport resourceImport)
{
    if (!resourceImport.valid())
    {
        clear();
        return false;
    }

    m_resourceImport = std::move(resourceImport);
    emit resourceChanged();
    return true;
}

void InAppClipboard::clear()
{
    if (!m_resourceImport.valid()
        && m_resourceImport.fileName.trimmed().isEmpty()
        && m_resourceImport.mimeType.trimmed().isEmpty())
    {
        return;
    }

    m_resourceImport = {};
    emit resourceChanged();
}
