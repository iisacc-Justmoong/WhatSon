#include "app/models/file/viewer/ImageFormatCompatibilityLayer.hpp"

#include <QFileInfo>
#include <QHash>
#include <QImageReader>
#include <QRegularExpression>
#include <QSet>
#include <QString>
#include <QUrl>

namespace
{
    QString normalizeExtension(QString value)
    {
        value = value.trimmed().toCaseFolded();
        if (value.isEmpty())
        {
            return {};
        }

        if (!value.startsWith(QLatin1Char('.')))
        {
            value.prepend(QLatin1Char('.'));
        }

        if (value == QStringLiteral(".jpeg") || value == QStringLiteral(".jpe"))
        {
            return QStringLiteral(".jpg");
        }
        if (value == QStringLiteral(".tif"))
        {
            return QStringLiteral(".tiff");
        }
        return value;
    }

    QString extensionFromPathLikeValue(const QString& value)
    {
        const QString trimmedValue = value.trimmed();
        if (trimmedValue.isEmpty())
        {
            return {};
        }

        QUrl url(trimmedValue);
        if (!url.isValid() || url.isEmpty())
        {
            url = QUrl::fromUserInput(trimmedValue);
        }

        if (url.isValid() && !url.scheme().isEmpty())
        {
            QString path;
            if (url.isLocalFile())
            {
                path = url.toLocalFile();
            }
            else
            {
                path = url.path();
            }
            return normalizeExtension(QFileInfo(path).suffix());
        }

        return normalizeExtension(QFileInfo(trimmedValue).suffix());
    }

    QString formatFromMimeValue(const QString& value)
    {
        QString lowered = value.trimmed().toCaseFolded();
        if (lowered.isEmpty())
        {
            return {};
        }

        const int separatorIndex = lowered.indexOf(QLatin1Char(';'));
        if (separatorIndex > 0)
        {
            lowered = lowered.left(separatorIndex).trimmed();
        }

        static const QHash<QString, QString> kMimeToFormat = {
            {QStringLiteral("image/png"), QStringLiteral(".png")},
            {QStringLiteral("image/jpeg"), QStringLiteral(".jpg")},
            {QStringLiteral("image/jpg"), QStringLiteral(".jpg")},
            {QStringLiteral("image/bmp"), QStringLiteral(".bmp")},
            {QStringLiteral("image/gif"), QStringLiteral(".gif")},
            {QStringLiteral("image/webp"), QStringLiteral(".webp")},
            {QStringLiteral("image/tiff"), QStringLiteral(".tiff")},
            {QStringLiteral("image/x-tiff"), QStringLiteral(".tiff")},
            {QStringLiteral("image/heic"), QStringLiteral(".heic")},
            {QStringLiteral("image/heif"), QStringLiteral(".heif")},
            {QStringLiteral("image/avif"), QStringLiteral(".avif")},
            {QStringLiteral("image/x-icon"), QStringLiteral(".ico")},
            {QStringLiteral("image/vnd.microsoft.icon"), QStringLiteral(".ico")}
        };

        return kMimeToFormat.value(lowered);
    }

    QString normalizedFormatFromValue(const QString& value)
    {
        const QString trimmedValue = value.trimmed();
        if (trimmedValue.isEmpty())
        {
            return {};
        }

        if (trimmedValue.startsWith(QStringLiteral("image/"), Qt::CaseInsensitive))
        {
            const QString mimeFormat = formatFromMimeValue(trimmedValue);
            if (!mimeFormat.isEmpty())
            {
                return normalizeExtension(mimeFormat);
            }
        }

        const QString pathFormat = extensionFromPathLikeValue(trimmedValue);
        if (!pathFormat.isEmpty())
        {
            return pathFormat;
        }

        static const QRegularExpression kSimpleFormatPattern(QStringLiteral("^[A-Za-z0-9]+$"));
        if (kSimpleFormatPattern.match(trimmedValue).hasMatch())
        {
            return normalizeExtension(trimmedValue);
        }

        return normalizeExtension(trimmedValue);
    }

    QSet<QString> supportedBitmapFormats()
    {
        static const QSet<QString> kFormats = []
        {
            QSet<QString> formats;
            const QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats();
            for (const QByteArray& format : supportedFormats)
            {
                const QString normalized = normalizeExtension(QString::fromLatin1(format));
                if (!normalized.isEmpty())
                {
                    formats.insert(normalized);
                }
            }

            if (formats.contains(QStringLiteral(".jpg")))
            {
                formats.insert(QStringLiteral(".jpeg"));
            }
            if (formats.contains(QStringLiteral(".jpeg")))
            {
                formats.insert(QStringLiteral(".jpg"));
            }
            if (formats.contains(QStringLiteral(".tiff")))
            {
                formats.insert(QStringLiteral(".tif"));
            }
            if (formats.contains(QStringLiteral(".tif")))
            {
                formats.insert(QStringLiteral(".tiff"));
            }
            return formats;
        }();
        return kFormats;
    }
} // namespace

namespace WhatSon::Viewer::ImageFormatCompatibilityLayer
{
    QString normalizedBitmapFormat(const QString& value)
    {
        return normalizedFormatFromValue(value);
    }

    bool isBitmapFormatCompatible(const QString& value)
    {
        const QString normalizedFormat = normalizedFormatFromValue(value);
        if (normalizedFormat.isEmpty())
        {
            return false;
        }

        return supportedBitmapFormats().contains(normalizedFormat);
    }

    QString unsupportedBitmapFormatMessage(const QString& value)
    {
        const QString normalizedFormat = normalizedFormatFromValue(value);
        if (normalizedFormat.isEmpty())
        {
            return QStringLiteral("Missing image format metadata for bitmap preview.");
        }
        if (isBitmapFormatCompatible(normalizedFormat))
        {
            return {};
        }
        return QStringLiteral("Unsupported bitmap format for in-app preview: %1").arg(normalizedFormat);
    }
} // namespace WhatSon::Viewer::ImageFormatCompatibilityLayer
