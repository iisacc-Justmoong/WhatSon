#include "app/models/editor/component/Callout.h"

#include <QBuffer>
#include <QImage>
#include <QIODevice>
#include <QTextDocument>

#include <cmath>

namespace
{
    constexpr int kFigmaNodeWidth = 295;
    constexpr int kFramePaddingVertical = 4;
    constexpr int kFramePaddingRight = 4;
    constexpr int kFramePaddingLeft = 4;
    constexpr int kLeadingBarWidth = 3;
    constexpr int kLeadingBarHeight = 14;
    constexpr int kLeadingBarRadius = 3;
    constexpr int kContentGap = 12;
    constexpr int kFrameMinHeight = kFramePaddingVertical + kLeadingBarHeight + kFramePaddingVertical;
    constexpr auto kCalloutRenderVersion = "figma-280-7897-floated-frame-v5";

    QString normalizedContentHtml(const QString& contentHtml)
    {
        return contentHtml.trimmed().isEmpty() ? QStringLiteral("&nbsp;") : contentHtml;
    }

    int effectiveFrameWidth(const int editorViewportWidth)
    {
        return editorViewportWidth > 0 ? editorViewportWidth : kFigmaNodeWidth;
    }

    int estimatedContentHeight(const QString& contentHtml, const int editorViewportWidth)
    {
        QTextDocument document;
        document.setDocumentMargin(0);
        document.setTextWidth(qMax(
            1,
            effectiveFrameWidth(editorViewportWidth)
                - kFramePaddingLeft
                - kFramePaddingRight
                - kLeadingBarWidth
                - kContentGap));
        document.setHtml(QStringLiteral(
            "<span style=\"font-family:Pretendard;font-size:12px;font-weight:500;"
            "line-height:12px;white-space:normal;word-break:break-word;\">%1</span>")
            .arg(contentHtml));
        return qMax(kLeadingBarHeight, static_cast<int>(std::ceil(document.size().height())));
    }

    QString leadingBarPngBase64(const int contentHeight)
    {
        const int imageHeight = qMax(kLeadingBarHeight, contentHeight);
        QImage image(kLeadingBarWidth, imageHeight, QImage::Format_ARGB32);
        image.fill(Qt::transparent);

        const QRgb barColor = qRgba(217, 217, 217, 255);
        const int barBottom = qMin(imageHeight, contentHeight);
        for (int y = 0; y < barBottom; ++y)
        {
            for (int x = 0; x < kLeadingBarWidth; ++x)
            {
                image.setPixel(x, y, barColor);
            }
        }

        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        return QString::fromLatin1(bytes.toBase64());
    }
} // namespace

namespace WhatSon::EditorComponent
{
    int Callout::designWidth()
    {
        return kFigmaNodeWidth;
    }

    QString Callout::sourceMarker(const QString& sourceText)
    {
        return QString::fromLatin1(sourceText.toUtf8().toHex());
    }

    QString Callout::renderHtml(const CalloutDescriptor& descriptor)
    {
        const QString contentHtml = normalizedContentHtml(descriptor.contentHtml);
        const int contentHeight = estimatedContentHeight(contentHtml, descriptor.editorViewportWidth);
        const int frameChromeHeight = qMax(kLeadingBarHeight, contentHeight);
        const QString leadingBarPng = leadingBarPngBase64(contentHeight);

        QString html;
        html.reserve(contentHtml.size() + 1200);
        html += QStringLiteral("<!--whatson-callout-source:");
        html += sourceMarker(descriptor.sourceText);
        html += QStringLiteral("-->");
        html += QStringLiteral(
            "<div class=\"whatson-callout\" data-figma-node-id=\"280:7897\" "
            "data-frame-design-width=\"%1\" data-callout-render=\"%2\" "
            "data-frame-width-mode=\"fill\" data-frame-height-mode=\"hug-contents\" "
            "data-frame-padding-top=\"%3\" data-frame-padding-right=\"%4\" "
            "data-frame-padding-bottom=\"%3\" data-frame-padding-left=\"%5\" "
            "data-callout-bar-width=\"%6\" data-callout-bar-design-height=\"%7\" "
            "data-callout-bar-height-mode=\"match-content\" "
            "data-callout-bar-radius=\"%8\" data-callout-content-gap=\"%9\" "
            "data-callout-frame-min-height=\"%10\" "
            "data-callout-frame-chrome-height=\"%11\" "
            "width=\"100%\" "
            "style=\"width:100%;max-width:100%;height:auto;min-height:0;"
            "box-sizing:border-box;background-color:#262728;"
            "padding:%3px %4px;"
            "font-family:Pretendard;font-size:12px;font-weight:500;line-height:12px;"
            "color:#FFFFFF;white-space:normal;word-break:break-word;\">")
            .arg(
                QString::number(kFigmaNodeWidth),
                QString::fromLatin1(kCalloutRenderVersion),
                QString::number(kFramePaddingVertical),
                QString::number(kFramePaddingRight),
                QString::number(kFramePaddingLeft),
                QString::number(kLeadingBarWidth),
                QString::number(kLeadingBarHeight),
                QString::number(kLeadingBarRadius),
                QString::number(kContentGap))
            .arg(QString::number(kFrameMinHeight))
            .arg(QString::number(frameChromeHeight));
        html += QStringLiteral(
            "<img class=\"whatson-callout-leading-bar\" data-callout-frame-chrome=\"true\" "
            "src=\"data:image/png;base64,%1\" width=\"%2\" height=\"%3\" align=\"left\" "
            "style=\"border:0;float:left;margin-right:%4px;\"/>")
            .arg(
                leadingBarPng,
                QString::number(kLeadingBarWidth),
                QString::number(frameChromeHeight),
                QString::number(kContentGap));
        html += QStringLiteral(
            "<span data-callout-content=\"true\" "
            "style=\"background-color:#262728;font-family:Pretendard;font-size:12px;"
            "font-weight:500;line-height:12px;color:#FFFFFF;white-space:normal;"
            "word-break:break-word;\">"
            "<!--whatson-callout-content-->");
        html += contentHtml;
        html += QStringLiteral("<!--/whatson-callout-content--></span>");
        html += QStringLiteral(
            "</div>"
            "<!--/whatson-callout-source-->");
        return html;
    }
} // namespace WhatSon::EditorComponent
