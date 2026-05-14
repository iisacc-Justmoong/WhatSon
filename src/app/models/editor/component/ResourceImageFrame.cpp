#include "app/models/editor/component/ResourceImageFrame.h"

#include "app/models/hierarchy/resources/WhatSonResourcePackageSupport.hpp"

#include <QBuffer>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QImage>
#include <QImageReader>
#include <QPainter>
#include <QStandardPaths>
#include <QUrl>

namespace
{
    constexpr int kFigmaFrameWidth = 480;
    constexpr int kFrameHeaderHeight = 24;
    constexpr int kFrameToolbarHeight = 19;
    constexpr int kFrameHorizontalPadding = 8;
    constexpr int kFrameRadius = 12;
    constexpr int kFrameTextPixelSize = 11;
    constexpr int kFrameTextLineHeight = 11;
    constexpr int kMoreIconSize = 16;
    constexpr int kMoreDotSize = 2;
    constexpr int kMoreFirstDotOffset = 2;
    constexpr int kMoreDotStep = 5;
    constexpr auto kFrameRenderVersion = "figma-292-50-fixed-chrome-v1";

    QString htmlAttribute(QString value)
    {
        return value.toHtmlEscaped();
    }

    QString displayTypeLabel(const WhatSon::EditorComponent::ResourceFrameDescriptor& descriptor)
    {
        QString type = WhatSon::Resources::normalizedType(descriptor.type);
        if (type.isEmpty())
        {
            type = WhatSon::Resources::inferTypeFromFormat(descriptor.format);
        }
        if (type.isEmpty())
        {
            type = QStringLiteral("resource");
        }

        type[0] = type.at(0).toUpper();
        return type;
    }

    QString displayFileName(const WhatSon::EditorComponent::ResourceFrameDescriptor& descriptor)
    {
        const QString fileName = QFileInfo(descriptor.resourcePath.trimmed()).fileName().trimmed();
        if (!fileName.isEmpty())
        {
            return fileName;
        }
        if (!descriptor.resourceId.trimmed().isEmpty())
        {
            return descriptor.resourceId.trimmed();
        }
        if (!descriptor.resourcePath.trimmed().isEmpty())
        {
            return descriptor.resourcePath.trimmed();
        }
        return QStringLiteral("filename");
    }

    QString displayResourceLabel(const WhatSon::EditorComponent::ResourceFrameDescriptor& descriptor)
    {
        if (!descriptor.resourceId.trimmed().isEmpty())
        {
            return descriptor.resourceId.trimmed();
        }

        const QString completeBaseName = QFileInfo(descriptor.resourcePath.trimmed()).completeBaseName().trimmed();
        if (!completeBaseName.isEmpty())
        {
            return completeBaseName;
        }
        return displayFileName(descriptor);
    }

    bool isImageDescriptor(const WhatSon::EditorComponent::ResourceFrameDescriptor& descriptor)
    {
        const QString normalizedType = WhatSon::Resources::normalizedType(descriptor.type);
        const QString normalizedFormat = WhatSon::Resources::normalizeFormat(descriptor.format).toCaseFolded();
        return normalizedType == QStringLiteral("image")
            || WhatSon::Resources::inferTypeFromFormat(normalizedFormat) == QStringLiteral("image");
    }

    QSize imageSourceSize(const WhatSon::EditorComponent::ResourceFrameDescriptor& descriptor)
    {
        if (!QFileInfo(descriptor.resolvedAssetPath).isFile())
        {
            return {};
        }

        QImageReader imageReader(descriptor.resolvedAssetPath);
        imageReader.setAutoTransform(true);
        const QSize imageSize = imageReader.size();
        return imageSize.isValid() && !imageSize.isEmpty() ? imageSize : QSize();
    }

    QSize fallbackMediaDisplaySize()
    {
        return QSize(640, 360);
    }

    QSize mediaDisplaySizeForSource(const QSize& sourceSize)
    {
        const QSize displaySize = WhatSon::EditorComponent::ResourceFrame::imageDisplaySize(sourceSize);
        return displaySize.isValid() && !displaySize.isEmpty()
            ? displaySize
            : fallbackMediaDisplaySize();
    }

    QSize mediaRasterSizeForSource(const QSize& sourceSize)
    {
        const QSize displaySize = mediaDisplaySizeForSource(sourceSize);
        const qreal mediaScale =
            displaySize.width() > 0
            ? static_cast<qreal>(kFigmaFrameWidth) / static_cast<qreal>(displaySize.width())
            : 1.0;
        return QSize(
            kFigmaFrameWidth,
            qMax(1, qRound(static_cast<qreal>(displaySize.height()) * mediaScale)));
    }

    int frameDisplayHeightForSource(const QSize& sourceSize)
    {
        return mediaRasterSizeForSource(sourceSize).height() + kFrameHeaderHeight + kFrameToolbarHeight;
    }

    QSize frameRasterSizeForSource(const QSize& sourceSize)
    {
        return QSize(kFigmaFrameWidth, frameDisplayHeightForSource(sourceSize));
    }

    QString frameMetricAttributes(const QSize& sourceSize)
    {
        QString attributes = QStringLiteral(
                                 " data-frame-chrome-width=\"%1\""
                                 " data-frame-header-height=\"%2\""
                                 " data-frame-toolbar-height=\"%3\""
                                 " data-frame-text-pixel-size=\"%4\""
                                 " data-frame-text-line-height=\"%5\""
                                 " data-frame-more-icon-size=\"%6\""
                                 " data-frame-more-dot-size=\"%7\"")
            .arg(
                QString::number(kFigmaFrameWidth),
                QString::number(kFrameHeaderHeight),
                QString::number(kFrameToolbarHeight),
                QString::number(kFrameTextPixelSize),
                QString::number(kFrameTextLineHeight),
                QString::number(kMoreIconSize),
                QString::number(kMoreDotSize));

        if (!sourceSize.isValid() || sourceSize.isEmpty())
        {
            return attributes;
        }

        const QSize displaySize = mediaRasterSizeForSource(sourceSize);
        attributes += QStringLiteral(
                          " data-source-width=\"%1\" data-source-height=\"%2\""
                          " data-display-width=\"%3\" data-display-height=\"%4\""
                          " data-frame-display-height=\"%5\"")
            .arg(
                QString::number(sourceSize.width()),
                QString::number(sourceSize.height()),
                QString::number(displaySize.width()),
                QString::number(displaySize.height()),
                QString::number(frameDisplayHeightForSource(sourceSize)));
        return attributes;
    }

    QImage readSourceImage(const WhatSon::EditorComponent::ResourceFrameDescriptor& descriptor)
    {
        if (!QFileInfo(descriptor.resolvedAssetPath).isFile())
        {
            return {};
        }

        QImageReader imageReader(descriptor.resolvedAssetPath);
        imageReader.setAutoTransform(true);
        QImage image = imageReader.read();
        if (image.isNull())
        {
            return {};
        }
        if (image.format() != QImage::Format_ARGB32_Premultiplied)
        {
            image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        }
        return image;
    }

    QString elidedFrameText(QPainter* painter, const QString& text, const int width)
    {
        if (painter == nullptr)
        {
            return text;
        }

        const QFontMetrics metrics(painter->font());
        return metrics.elidedText(text, Qt::ElideRight, qMax(1, width));
    }

    void paintFrameText(
        QPainter* painter,
        const QRect& rect,
        const QString& text,
        const Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter)
    {
        if (painter == nullptr)
        {
            return;
        }

        const QString elidedText = elidedFrameText(
            painter,
            text,
            rect.width() - kFrameHorizontalPadding);
        painter->drawText(rect, alignment, elidedText);
    }

    void paintMoreIcon(QPainter* painter, const QRect& rect)
    {
        if (painter == nullptr)
        {
            return;
        }

        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(QStringLiteral("#CED0D6")));
        const int dotY = rect.y() + ((rect.height() - kMoreDotSize) / 2);
        for (int dotIndex = 0; dotIndex < 3; ++dotIndex)
        {
            const int dotX = rect.x() + kMoreFirstDotOffset + (dotIndex * kMoreDotStep);
            painter->drawEllipse(QRect(dotX, dotY, kMoreDotSize, kMoreDotSize));
        }
    }

    QImage renderFramePreviewImage(
        const WhatSon::EditorComponent::ResourceFrameDescriptor& descriptor,
        const QSize& sourceSize,
        const QString& typeLabel,
        const QString& fileName)
    {
        const QSize frameSize = frameRasterSizeForSource(sourceSize);
        if (!frameSize.isValid() || frameSize.isEmpty())
        {
            return {};
        }

        QImage frame(frameSize, QImage::Format_ARGB32_Premultiplied);
        frame.fill(Qt::transparent);

        QPainter painter(&frame);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const QColor backgroundColor(QStringLiteral("#1E1F20"));
        const QColor strokeColor(QStringLiteral("#2C2E2F"));
        const QColor captionTextColor(255, 255, 255, 128);

        const QRectF frameRect(0.5, 0.5, frame.width() - 1.0, frame.height() - 1.0);
        painter.setPen(QPen(strokeColor, 1));
        painter.setBrush(backgroundColor);
        painter.drawRoundedRect(frameRect, kFrameRadius, kFrameRadius);

        painter.setPen(QPen(strokeColor, 1));
        painter.drawLine(1, kFrameHeaderHeight, frame.width() - 2, kFrameHeaderHeight);
        painter.drawLine(
            1,
            frame.height() - kFrameToolbarHeight,
            frame.width() - 2,
            frame.height() - kFrameToolbarHeight);

        QFont textFont(QStringLiteral("Pretendard"));
        textFont.setPixelSize(kFrameTextPixelSize);
        textFont.setWeight(QFont::Normal);
        painter.setFont(textFont);
        painter.setPen(captionTextColor);

        const QRect headerTextRect(
            kFrameHorizontalPadding,
            0,
            frame.width() - (kFrameHorizontalPadding * 2) - kMoreIconSize,
            kFrameHeaderHeight);
        paintFrameText(&painter, headerTextRect, typeLabel);

        const QRect moreRect(
            frame.width() - kFrameHorizontalPadding - kMoreIconSize,
            (kFrameHeaderHeight - kMoreIconSize) / 2,
            kMoreIconSize,
            kMoreIconSize);
        paintMoreIcon(&painter, moreRect);

        const QRect mediaRect(
            1,
            kFrameHeaderHeight + 1,
            frame.width() - 2,
            frame.height() - kFrameHeaderHeight - kFrameToolbarHeight - 2);
        if (isImageDescriptor(descriptor))
        {
            const QImage sourceImage = readSourceImage(descriptor);
            if (!sourceImage.isNull())
            {
                const QImage scaledImage = sourceImage.scaled(
                    mediaRect.size(),
                    Qt::KeepAspectRatio,
                    Qt::SmoothTransformation);
                const QPoint mediaTopLeft(
                    mediaRect.x() + ((mediaRect.width() - scaledImage.width()) / 2),
                    mediaRect.y() + ((mediaRect.height() - scaledImage.height()) / 2));
                painter.drawImage(mediaTopLeft, scaledImage);
            }
        }

        painter.setPen(captionTextColor);
        const QRect toolbarTextRect(
            kFrameHorizontalPadding,
            frame.height() - kFrameToolbarHeight,
            frame.width() - (kFrameHorizontalPadding * 2),
            kFrameToolbarHeight);
        paintFrameText(&painter, toolbarTextRect, fileName);

        return frame;
    }

    QString dataUriForFrameImage(const QImage& frameImage)
    {
        if (frameImage.isNull())
        {
            return {};
        }

        QByteArray encodedImage;
        QBuffer buffer(&encodedImage);
        if (!buffer.open(QIODevice::WriteOnly) || !frameImage.save(&buffer, "PNG"))
        {
            return {};
        }
        return QStringLiteral("data:image/png;base64,%1")
            .arg(QString::fromLatin1(encodedImage.toBase64()));
    }

    QString framePreviewCachePath(
        const WhatSon::EditorComponent::ResourceFrameDescriptor& descriptor,
        const QSize& sourceSize)
    {
        QString cacheRoot = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        if (cacheRoot.trimmed().isEmpty())
        {
            cacheRoot = QDir(QDir::tempPath()).filePath(QStringLiteral("WhatSon"));
        }

        QByteArray key;
        key += kFrameRenderVersion;
        key += '\0';
        key += QByteArray::number(kFigmaFrameWidth);
        key += 'x';
        key += QByteArray::number(kFrameHeaderHeight);
        key += 'x';
        key += QByteArray::number(kFrameToolbarHeight);
        key += 'x';
        key += QByteArray::number(kFrameTextPixelSize);
        key += 'x';
        key += QByteArray::number(kMoreIconSize);
        key += 'x';
        key += QByteArray::number(kMoreDotSize);
        key += '\0';
        key += descriptor.sourceTag.toUtf8();
        key += '\0';
        key += descriptor.resourcePath.toUtf8();
        key += '\0';
        key += descriptor.resolvedAssetPath.toUtf8();
        key += '\0';
        key += QByteArray::number(sourceSize.width());
        key += 'x';
        key += QByteArray::number(sourceSize.height());

        const QFileInfo assetInfo(descriptor.resolvedAssetPath);
        if (assetInfo.isFile())
        {
            key += '\0';
            key += QByteArray::number(assetInfo.size());
            key += '\0';
            key += QByteArray::number(assetInfo.lastModified().toMSecsSinceEpoch());
        }

        const QString cacheFileName =
            QString::fromLatin1(QCryptographicHash::hash(key, QCryptographicHash::Sha256).toHex().left(32))
            + QStringLiteral(".png");
        return QDir(cacheRoot).filePath(QStringLiteral("resource-frames/%1").arg(cacheFileName));
    }

    QString framePreviewImageUrl(
        const WhatSon::EditorComponent::ResourceFrameDescriptor& descriptor,
        const QSize& sourceSize,
        const QString& typeLabel,
        const QString& fileName)
    {
        const QImage frameImage = renderFramePreviewImage(descriptor, sourceSize, typeLabel, fileName);
        if (frameImage.isNull())
        {
            return {};
        }

        const QString cachePath = framePreviewCachePath(descriptor, sourceSize);
        const QFileInfo cacheInfo(cachePath);
        QDir cacheDir(cacheInfo.absolutePath());
        if ((cacheDir.exists() || QDir().mkpath(cacheInfo.absolutePath()))
            && (cacheInfo.isFile() || frameImage.save(cachePath, "PNG")))
        {
            return QUrl::fromLocalFile(cachePath).toString();
        }

        return dataUriForFrameImage(frameImage);
    }

    void appendUniqueTextLine(QStringList* lines, const QString& line)
    {
        const QString normalizedLine = line.trimmed().simplified();
        if (normalizedLine.isEmpty() || lines->contains(normalizedLine, Qt::CaseInsensitive))
        {
            return;
        }
        lines->push_back(normalizedLine);
    }
} // namespace

namespace WhatSon::EditorComponent
{
    QSize ResourceFrame::previewViewportSize()
    {
        return {};
    }

    QSize ResourceFrame::imageDisplaySize(const QSize& sourceSize)
    {
        if (!sourceSize.isValid() || sourceSize.isEmpty())
        {
            return {};
        }

        const int sourceWidth = qMax(1, sourceSize.width());
        const int sourceHeight = qMax(1, sourceSize.height());
        return QSize(sourceWidth, sourceHeight > sourceWidth ? sourceWidth : sourceHeight);
    }

    QString ResourceFrame::sourceMarker(const QString& sourceTag)
    {
        return QString::fromLatin1(sourceTag.toUtf8().toHex());
    }

    QString ResourceFrame::renderHtml(const ResourceFrameDescriptor& descriptor)
    {
        const QString typeLabel = displayTypeLabel(descriptor);
        const QString fileName = displayFileName(descriptor);
        const QSize sourceImageSize =
            isImageDescriptor(descriptor) ? imageSourceSize(descriptor) : QSize();
        const QString previewImageUrl = framePreviewImageUrl(
            descriptor,
            sourceImageSize,
            typeLabel,
            fileName);

        QString html;
        html.reserve(1400);
        html += QStringLiteral("<!--whatson-resource-source:");
        html += sourceMarker(descriptor.sourceTag);
        html += QStringLiteral("-->");
        html += QStringLiteral(
            "<img src=\"%1\" class=\"whatson-resource-frame\" data-figma-node-id=\"292:50\" "
            "data-resource-preview=\"single-object-raster\" data-resource-type-label=\"%2\" "
            "data-resource-file-name=\"%3\" data-max-width-height-ratio=\"1:1\"%4 "
            "width=\"100%\" style=\"display:block;width:100%;max-width:100%;height:auto;"
            "max-height:100%;vertical-align:middle;object-fit:contain;border:0;\" />")
            .arg(
                htmlAttribute(previewImageUrl),
                htmlAttribute(typeLabel),
                htmlAttribute(fileName),
                frameMetricAttributes(sourceImageSize));
        html += QStringLiteral("<!--/whatson-resource-source-->");
        return html;
    }

    QStringList ResourceFrame::renderedTextLines(const ResourceFrameDescriptor& descriptor)
    {
        QStringList lines;
        const QString typeLabel = displayTypeLabel(descriptor);
        appendUniqueTextLine(&lines, typeLabel);
        appendUniqueTextLine(&lines, QStringLiteral("..."));
        appendUniqueTextLine(&lines, QStringLiteral("%1...").arg(typeLabel));
        appendUniqueTextLine(&lines, QStringLiteral("%1 ...").arg(typeLabel));
        appendUniqueTextLine(&lines, displayFileName(descriptor));

        const QString label = displayResourceLabel(descriptor);
        const QString type = WhatSon::Resources::normalizedType(descriptor.type);
        const QString format = WhatSon::Resources::normalizeFormat(descriptor.format);
        appendUniqueTextLine(&lines, label);
        appendUniqueTextLine(&lines, QStringLiteral("%1 %2").arg(type, format));
        appendUniqueTextLine(&lines, QStringLiteral("%1 %2 %3").arg(label, type, format));
        appendUniqueTextLine(&lines, descriptor.resourcePath);
        return lines;
    }
} // namespace WhatSon::EditorComponent
