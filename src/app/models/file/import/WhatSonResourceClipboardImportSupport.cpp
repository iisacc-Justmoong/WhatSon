#include "app/models/file/import/WhatSonResourceClipboardImportSupport.hpp"

#include <QByteArray>
#include <QClipboard>
#include <QGuiApplication>
#include <QImage>
#include <QMimeData>
#include <QPixmap>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QVariant>

namespace
{
    bool mimeFormatLooksLikeImage(const QString& mimeFormat)
    {
        const QString normalizedFormat = mimeFormat.trimmed().toCaseFolded();
        return normalizedFormat.startsWith(QStringLiteral("image/"))
            || normalizedFormat.contains(QStringLiteral("png"))
            || normalizedFormat.contains(QStringLiteral("jpeg"))
            || normalizedFormat.contains(QStringLiteral("jpg"))
            || normalizedFormat.contains(QStringLiteral("gif"))
            || normalizedFormat.contains(QStringLiteral("bmp"))
            || normalizedFormat.contains(QStringLiteral("webp"))
            || normalizedFormat.contains(QStringLiteral("tiff"))
            || normalizedFormat.contains(QStringLiteral("heic"))
            || normalizedFormat.contains(QStringLiteral("heif"));
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
            if (match.hasMatch())
            {
                return match.captured(1).trimmed();
            }
            return {};
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

        const QStringList formats = mimeData->formats();
        for (const QString& mimeFormat : formats)
        {
            if (!mimeFormat.startsWith(QStringLiteral("text/"), Qt::CaseInsensitive))
            {
                continue;
            }

            const QString payloadText = QString::fromUtf8(mimeData->data(mimeFormat));
            const QString dataUrl = extractFromText(payloadText);
            if (!dataUrl.isEmpty())
            {
                return dataUrl;
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
} // namespace

namespace WhatSon::Resources::ClipboardImportSupport
{
    bool extractClipboardImage(const QMimeData* mimeData, QImage* outImage)
    {
        if (outImage != nullptr)
        {
            *outImage = QImage();
        }

        if (mimeData == nullptr)
        {
            return false;
        }

        const auto tryLoadImageData = [&](const QByteArray& imageData)
        {
            if (imageData.isEmpty())
            {
                return false;
            }

            QImage image;
            if (!image.loadFromData(imageData) || image.isNull())
            {
                return false;
            }

            if (outImage != nullptr)
            {
                *outImage = image;
            }
            return true;
        };

        const QStringList formats = mimeData->formats();
        for (const QString& mimeFormat : formats)
        {
            if (!mimeFormatLooksLikeImage(mimeFormat))
            {
                continue;
            }
            if (tryLoadImageData(mimeData->data(mimeFormat)))
            {
                return true;
            }
        }

        const QString clipboardImageDataUrl = firstClipboardImageDataUrl(mimeData);
        if (!clipboardImageDataUrl.isEmpty()
            && tryLoadImageData(decodedClipboardImageDataUrlPayload(clipboardImageDataUrl)))
        {
            return true;
        }

        if (!mimeData->hasImage())
        {
            return false;
        }

        const QVariant imageVariant = mimeData->imageData();
        QImage image;
        if (imageVariant.canConvert<QImage>())
        {
            image = qvariant_cast<QImage>(imageVariant);
        }
        else if (imageVariant.canConvert<QPixmap>())
        {
            image = qvariant_cast<QPixmap>(imageVariant).toImage();
        }

        if (!image.isNull())
        {
            if (outImage != nullptr)
            {
                *outImage = image;
            }
            return true;
        }

        return false;
    }

    bool extractClipboardImage(const QClipboard* clipboard, QImage* outImage)
    {
        if (outImage != nullptr)
        {
            *outImage = QImage();
        }

        if (clipboard == nullptr)
        {
            return false;
        }

        if (extractClipboardImage(clipboard->mimeData(), outImage))
        {
            return true;
        }

        const QImage clipboardImage = clipboard->image();
        if (!clipboardImage.isNull())
        {
            if (outImage != nullptr)
            {
                *outImage = clipboardImage;
            }
            return true;
        }

        const QPixmap clipboardPixmap = clipboard->pixmap();
        if (!clipboardPixmap.isNull())
        {
            if (outImage != nullptr)
            {
                *outImage = clipboardPixmap.toImage();
            }
            return true;
        }

        return false;
    }

    bool clipboardContainsImportableImage()
    {
        const QClipboard* clipboard = QGuiApplication::clipboard();
        if (clipboard == nullptr)
        {
            return false;
        }

        return extractClipboardImage(clipboard, nullptr);
    }
} // namespace WhatSon::Resources::ClipboardImportSupport
