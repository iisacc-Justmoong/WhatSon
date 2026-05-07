#include "app/models/editor/resource/ContentsInlineResourcePresentationController.hpp"

#include "app/models/editor/resource/ContentsEditorDynamicObjectSupport.hpp"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QImage>
#include <QImageReader>
#include <QPainter>
#include <QPainterPath>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStringList>
#include <QUrl>

#include <algorithm>
#include <cmath>

using namespace WhatSon::Editor::DynamicObjectSupport;

namespace
{
    constexpr int kResourceFrameDesignWidth = 480;
    constexpr int kResourceFrameCornerRadius = 12;
    constexpr int kResourceFrameHorizontalPadding = 8;
    constexpr int kResourceFrameBarHeight = 19;
    constexpr int kResourceFrameMenuSize = 16;
    constexpr int kResourceFrameMediaWidth = 338;
    constexpr int kResourceFrameMediaHeight = 352;
    constexpr int kResourceFrameDesignHeight = kResourceFrameBarHeight * 2 + kResourceFrameMediaHeight;
    constexpr auto kResourceFrameBorderColor = "#2C2E2F";
    constexpr auto kResourceFrameCacheVersion = "figma-292-50-v3";

    QString htmlText(const QString& text)
    {
        return text.toHtmlEscaped();
    }

    QString firstPresentResourceEntryLabel(const QVariantMap& entry)
    {
        const QStringList candidateKeys{
            QStringLiteral("displayName"),
            QStringLiteral("resourcePath"),
            QStringLiteral("resourceId"),
            QStringLiteral("resolvedPath"),
            QStringLiteral("source"),
        };
        for (const QString& key : candidateKeys)
        {
            const QString value = entry.value(key).toString().trimmed();
            if (!value.isEmpty())
            {
                return value;
            }
        }
        return QStringLiteral("Image Resource");
    }

    QString localImagePathForResourceEntry(const QVariantMap& entry)
    {
        const QString resolvedPath = entry.value(QStringLiteral("resolvedPath")).toString().trimmed();
        if (!resolvedPath.isEmpty() && QFileInfo(resolvedPath).isFile())
        {
            return QDir::cleanPath(QFileInfo(resolvedPath).absoluteFilePath());
        }

        const QUrl sourceUrl(entry.value(QStringLiteral("source")).toString().trimmed());
        if (sourceUrl.isValid() && sourceUrl.isLocalFile())
        {
            const QString localPath = sourceUrl.toLocalFile();
            if (!localPath.isEmpty() && QFileInfo(localPath).isFile())
            {
                return QDir::cleanPath(QFileInfo(localPath).absoluteFilePath());
            }
        }
        return {};
    }

    QSize resourceEntryImageSize(const QVariantMap& entry, const QString& imagePath)
    {
        const QSize entrySize(
            entry.value(QStringLiteral("imageWidth")).toInt(),
            entry.value(QStringLiteral("imageHeight")).toInt());
        if (entrySize.isValid() && !entrySize.isEmpty())
        {
            return entrySize;
        }

        QImageReader reader(imagePath);
        reader.setAutoTransform(true);
        const QSize readerSize = reader.size();
        if (readerSize.isValid() && !readerSize.isEmpty())
        {
            return readerSize;
        }

        const QImage image(imagePath);
        if (!image.isNull() && image.size().isValid() && !image.size().isEmpty())
        {
            return image.size();
        }
        return {};
    }

    QString frameCacheDirectoryPath()
    {
        QString cacheRoot = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        if (cacheRoot.trimmed().isEmpty())
        {
            cacheRoot = QDir::tempPath();
        }

        const QString frameCachePath = QDir(cacheRoot).filePath(QStringLiteral("inline-resource-frames"));
        QDir().mkpath(frameCachePath);
        return frameCachePath;
    }

    QSize resourceFrameSizeForWidth(const int targetFrameWidth)
    {
        const int frameWidth = std::max(120, targetFrameWidth);
        const int frameHeight = std::max(
            1,
            static_cast<int>(std::lround(
                static_cast<double>(frameWidth) * kResourceFrameDesignHeight / kResourceFrameDesignWidth)));
        return QSize(frameWidth, frameHeight);
    }

    QString frameCacheFilePath(
        const QString& imagePath,
        const QString& frameLabel,
        const QSize& metadataSize,
        const QSize& frameSize)
    {
        const QFileInfo imageInfo(imagePath);
        QByteArray fingerprint;
        fingerprint.append(kResourceFrameCacheVersion);
        fingerprint.append('\n');
        fingerprint.append(imageInfo.absoluteFilePath().toUtf8());
        fingerprint.append('\n');
        fingerprint.append(frameLabel.toUtf8());
        fingerprint.append('\n');
        fingerprint.append(QByteArray::number(imageInfo.size()));
        fingerprint.append('\n');
        fingerprint.append(QByteArray::number(imageInfo.lastModified().toMSecsSinceEpoch()));
        fingerprint.append('\n');
        fingerprint.append(QByteArray::number(metadataSize.width()));
        fingerprint.append('x');
        fingerprint.append(QByteArray::number(metadataSize.height()));
        fingerprint.append('\n');
        fingerprint.append(QByteArray::number(frameSize.width()));
        fingerprint.append('x');
        fingerprint.append(QByteArray::number(frameSize.height()));
        const QString digest =
            QString::fromLatin1(QCryptographicHash::hash(fingerprint, QCryptographicHash::Sha256).toHex().left(32));
        return QDir(frameCacheDirectoryPath()).filePath(QStringLiteral("whatson-resource-frame-%1.png").arg(digest));
    }

    QImage coverCroppedImage(const QImage& sourceImage, const QSize& targetSize)
    {
        if (sourceImage.isNull() || !targetSize.isValid() || targetSize.isEmpty())
        {
            return {};
        }

        const QImage scaled = sourceImage.scaled(
            targetSize,
            Qt::KeepAspectRatioByExpanding,
            Qt::SmoothTransformation);
        const QRect cropRect(
            std::max(0, (scaled.width() - targetSize.width()) / 2),
            std::max(0, (scaled.height() - targetSize.height()) / 2),
            targetSize.width(),
            targetSize.height());
        return scaled.copy(cropRect);
    }

    void drawCaptionText(QPainter* painter, const QRectF& rect, const QString& text, const Qt::TextElideMode elideMode)
    {
        if (painter == nullptr)
        {
            return;
        }

        QFont captionFont(QStringLiteral("Pretendard"));
        captionFont.setPixelSize(11);
        captionFont.setWeight(QFont::Normal);
        painter->setFont(captionFont);
        painter->setPen(QColor(255, 255, 255, 128));

        const QFontMetrics metrics(captionFont);
        const QString elidedText = metrics.elidedText(text, elideMode, std::max(0, static_cast<int>(rect.width())));
        painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, elidedText);
    }

    bool renderFigmaResourceFrameImage(
        const QString& imagePath,
        const QString& frameLabel,
        const QString& framePath,
        const QSize& frameSize)
    {
        if (!frameSize.isValid() || frameSize.isEmpty())
        {
            return false;
        }

        QImageReader reader(imagePath);
        reader.setAutoTransform(true);
        const QImage sourceImage = reader.read();
        if (sourceImage.isNull())
        {
            return false;
        }

        const QSize mediaSize(kResourceFrameMediaWidth, kResourceFrameMediaHeight);
        const QImage mediaImage = coverCroppedImage(
            sourceImage.convertToFormat(QImage::Format_ARGB32_Premultiplied),
            mediaSize);
        if (mediaImage.isNull())
        {
            return false;
        }

        QImage frameImage(
            frameSize,
            QImage::Format_ARGB32_Premultiplied);
        frameImage.fill(Qt::transparent);

        QPainter painter(&frameImage);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.setRenderHint(QPainter::TextAntialiasing, true);
        const qreal frameScale = static_cast<qreal>(frameSize.width()) / kResourceFrameDesignWidth;
        painter.scale(frameScale, frameScale);

        const QRectF outerRect(0.5, 0.5, kResourceFrameDesignWidth - 1.0, kResourceFrameDesignHeight - 1.0);
        QPainterPath outerPath;
        outerPath.addRoundedRect(outerRect, kResourceFrameCornerRadius, kResourceFrameCornerRadius);
        painter.setClipPath(outerPath);

        const QRectF mediaRect(
            (kResourceFrameDesignWidth - kResourceFrameMediaWidth) / 2.0,
            kResourceFrameBarHeight,
            kResourceFrameMediaWidth,
            kResourceFrameMediaHeight);
        painter.drawImage(mediaRect, mediaImage);

        const QColor borderColor(QString::fromLatin1(kResourceFrameBorderColor));
        QPen dividerPen(borderColor);
        dividerPen.setWidthF(1.0);
        painter.setPen(dividerPen);
        painter.drawLine(
            QPointF(0.5, kResourceFrameBarHeight - 0.5),
            QPointF(kResourceFrameDesignWidth - 0.5, kResourceFrameBarHeight - 0.5));
        painter.drawLine(
            QPointF(0.5, kResourceFrameBarHeight + kResourceFrameMediaHeight + 0.5),
            QPointF(kResourceFrameDesignWidth - 0.5, kResourceFrameBarHeight + kResourceFrameMediaHeight + 0.5));

        drawCaptionText(
            &painter,
            QRectF(
                kResourceFrameHorizontalPadding,
                0,
                kResourceFrameDesignWidth - kResourceFrameHorizontalPadding * 3 - kResourceFrameMenuSize,
                kResourceFrameBarHeight),
            QStringLiteral("Image"),
            Qt::ElideRight);
        drawCaptionText(
            &painter,
            QRectF(
                kResourceFrameHorizontalPadding,
                kResourceFrameBarHeight + kResourceFrameMediaHeight,
                kResourceFrameDesignWidth - kResourceFrameHorizontalPadding * 2,
                kResourceFrameBarHeight),
            frameLabel,
            Qt::ElideMiddle);

        painter.setBrush(QColor(255, 255, 255, 128));
        painter.setPen(Qt::NoPen);
        const qreal menuLeft = kResourceFrameDesignWidth - kResourceFrameHorizontalPadding - kResourceFrameMenuSize;
        const qreal menuTop = (kResourceFrameBarHeight - kResourceFrameMenuSize) / 2.0;
        for (const qreal dotLeft : {2.0, 7.0, 12.0})
        {
            painter.drawEllipse(QRectF(menuLeft + dotLeft, menuTop + 7.0, 2.0, 2.0));
        }

        painter.setClipping(false);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(dividerPen);
        painter.drawPath(outerPath);
        painter.end();

        QDir().mkpath(QFileInfo(framePath).absolutePath());
        return frameImage.save(framePath, "PNG");
    }
}

ContentsInlineResourcePresentationController::ContentsInlineResourcePresentationController(QObject* parent)
    : QObject(parent)
{
}

ContentsInlineResourcePresentationController::~ContentsInlineResourcePresentationController() = default;

int ContentsInlineResourcePresentationController::inlineResourcePreviewWidth() const
{
    if (m_showPrintEditorLayout)
    {
        return std::max(120, static_cast<int>(std::floor(m_printPaperTextWidth)));
    }

    const qreal editorWidth = realProperty(m_contentEditor, "width");
    const qreal viewportWidth = realProperty(m_editorViewport, "width");
    const qreal availableWidth = std::max(editorWidth, viewportWidth) - (m_editorHorizontalInset * 2);
    return std::max(120, static_cast<int>(std::floor(std::max<qreal>(120.0, availableWidth))));
}

QString ContentsInlineResourcePresentationController::resourceEntryOpenTarget(const QVariant& resourceEntry) const
{
    const QVariantMap entry = resourceEntry.toMap();
    QString sourceUrl = entry.value(QStringLiteral("source")).toString().trimmed();
    if (!sourceUrl.isEmpty())
    {
        return sourceUrl;
    }
    return entry.value(QStringLiteral("resolvedPath")).toString().trimmed();
}

QString ContentsInlineResourcePresentationController::paragraphHtml(const QString& innerHtml) const
{
    return QStringLiteral("<p style=\"margin-top:0px;margin-bottom:0px;\">%1</p>")
        .arg(innerHtml.isEmpty() ? QStringLiteral("&nbsp;") : innerHtml);
}

QString ContentsInlineResourcePresentationController::inlineResourcePlaceholderHtml(const int lineCount) const
{
    QStringList lines;
    lines.reserve(std::max(0, lineCount));
    for (int index = 0; index < std::max(0, lineCount); ++index)
    {
        lines.append(paragraphHtml(QStringLiteral("&nbsp;")));
    }
    return lines.join(QString());
}

QString ContentsInlineResourcePresentationController::resourcePlaceholderBlockHtml() const
{
    return inlineResourcePlaceholderHtml(m_resourceEditorPlaceholderLineCount);
}

QString ContentsInlineResourcePresentationController::resourceEntryFrameLabel(
    const QVariant& resourceEntry) const
{
    const QVariantMap entry = resourceEntry.toMap();
    return firstPresentResourceEntryLabel(entry);
}

QString ContentsInlineResourcePresentationController::resourceEntryFrameImageSource(
    const QVariant& resourceEntry,
    const int targetFrameWidth) const
{
    const QVariantMap entry = resourceEntry.toMap();
    const QString renderMode = entry.value(QStringLiteral("renderMode")).toString().trimmed().toLower();
    if (renderMode != QStringLiteral("image"))
    {
        return {};
    }

    const QString imagePath = localImagePathForResourceEntry(entry);
    if (imagePath.isEmpty())
    {
        return {};
    }

    const QString frameLabel = resourceEntryFrameLabel(resourceEntry);
    const QSize metadataSize = resourceEntryImageSize(entry, imagePath);
    const QSize frameSize =
        resourceFrameSizeForWidth(targetFrameWidth > 0 ? targetFrameWidth : inlineResourcePreviewWidth());
    const QString framePath = frameCacheFilePath(imagePath, frameLabel, metadataSize, frameSize);
    if (!QFileInfo(framePath).isFile()
        && !renderFigmaResourceFrameImage(imagePath, frameLabel, framePath, frameSize))
    {
        return {};
    }

    return QUrl::fromLocalFile(framePath).toString();
}

QString ContentsInlineResourcePresentationController::inlineResourceBlockHtml(
    const QVariant& resourceEntry,
    const int targetFrameWidth) const
{
    const QVariantMap entry = resourceEntry.toMap();
    const QString renderMode = entry.value(QStringLiteral("renderMode")).toString().trimmed().toLower();
    const QString sourceUrl = resourceEntryOpenTarget(resourceEntry);
    const QString encodedSourceUrl = m_view
        ? invokeString(m_view, "encodeXmlAttributeValue", { sourceUrl }, sourceUrl)
        : sourceUrl;
    if (renderMode == QStringLiteral("image") && !encodedSourceUrl.isEmpty())
    {
        const QSize frameSize =
            resourceFrameSizeForWidth(targetFrameWidth > 0 ? targetFrameWidth : inlineResourcePreviewWidth());
        const QString frameImageSource = resourceEntryFrameImageSource(resourceEntry, frameSize.width());
        if (!frameImageSource.isEmpty())
        {
            const QString encodedFrameImageSource = m_view
                ? invokeString(m_view, "encodeXmlAttributeValue", { frameImageSource }, frameImageSource)
                : htmlText(frameImageSource);
            return QStringLiteral(
                       "<p style=\"margin-top:0px;margin-bottom:0px;line-height:0px;\">"
                       "<a href=\"%1\"><img src=\"%2\" width=\"%3\" height=\"%4\" style=\"vertical-align:top;\" /></a>"
                       "</p>")
                .arg(
                    encodedSourceUrl,
                    encodedFrameImageSource,
                    QString::number(frameSize.width()),
                    QString::number(frameSize.height()));
        }
    }
    return resourcePlaceholderBlockHtml();
}

bool ContentsInlineResourcePresentationController::resourceEntryCanRenderInlineInHtmlProjection(
    const QVariant& resourceEntry) const
{
    const QVariantMap entry = resourceEntry.toMap();
    const QString renderMode = entry.value(QStringLiteral("renderMode")).toString().trimmed().toLower();
    return m_inlineHtmlImageRenderingEnabled && renderMode == QStringLiteral("image")
        && !resourceEntryOpenTarget(resourceEntry).isEmpty();
}

QString ContentsInlineResourcePresentationController::renderEditorSurfaceHtmlWithInlineResources(
    const QString& editorHtml,
    const QVariant& renderedResourcesOverride,
    const int targetFrameWidth) const
{
    const QString& baseEditorHtml = editorHtml;
    QVariant renderedResources = renderedResourcesOverride;
    if ((!renderedResources.isValid() || renderedResources.isNull()) && m_bodyResourceRenderer)
    {
        renderedResources = m_bodyResourceRenderer->property("renderedResources");
    }
    QVariantList normalizedResources = invokeVariant(
        m_resourceTagController,
        "normalizedImportedResourceEntries",
        { renderedResources })
                                               .toList();
    if (normalizedResources.isEmpty() && renderedResources.isValid() && !renderedResources.isNull())
    {
        normalizedResources = normalizeSequentialVariant(renderedResources);
    }

    static const QRegularExpression blockExpression(
        QStringLiteral(R"(<!--whatson-resource-block:(\d+)-->[\s\S]*?<!--\/whatson-resource-block:\1-->)"));

    QString renderedHtml;
    renderedHtml.reserve(baseEditorHtml.size() + 64);
    int lastOffset = 0;
    auto match = blockExpression.globalMatch(baseEditorHtml);
    while (match.hasNext())
    {
        const QRegularExpressionMatch currentMatch = match.next();
        renderedHtml.append(baseEditorHtml.mid(lastOffset, currentMatch.capturedStart() - lastOffset));
        const int resourceIndex = currentMatch.captured(1).toInt();
        const QVariant entry = resourceIndex >= 0 && resourceIndex < normalizedResources.size()
            ? normalizedResources.at(resourceIndex)
            : QVariant{};
        renderedHtml.append(
            resourceEntryCanRenderInlineInHtmlProjection(entry)
                ? inlineResourceBlockHtml(entry, targetFrameWidth)
                : resourcePlaceholderBlockHtml());
        lastOffset = currentMatch.capturedEnd();
    }
    renderedHtml.append(baseEditorHtml.mid(lastOffset));
    return renderedHtml;
}
