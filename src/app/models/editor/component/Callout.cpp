#include "app/models/editor/component/Callout.h"

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
    constexpr int kFrameChromeHeight = kFramePaddingVertical + kLeadingBarHeight + kFramePaddingVertical;
    constexpr auto kTransparentLeftInsetPng =
        "iVBORw0KGgoAAAANSUhEUgAAAAQAAAABCAYAAAD5PA/NAAAACXBIWXMAAA9hAAAPYQGoP6dp"
        "AAAAC0lEQVQImWNgQAMAABEAAfYtn/gAAAAASUVORK5CYII=";
    constexpr auto kLeadingBarPng =
        "iVBORw0KGgoAAAANSUhEUgAAAAMAAAAWCAYAAAAFMyaXAAAAFklEQVR42mNgIAPcvHnzPwwPHQ4ZAAAEPZSl4QZxvwAAAABJRU5ErkJggg==";
    constexpr auto kTransparentContentGapPng =
        "iVBORw0KGgoAAAANSUhEUgAAAAwAAAABCAYAAADq6085AAAACXBIWXMAAA9hAAAPYQGoP6dp"
        "AAAADElEQVQImWNgIBEAAAAxAAHiKCvqAAAAAElFTkSuQmCC";
    constexpr auto kCalloutRenderVersion = "figma-280-7897-inline-frame-v3";

    QString normalizedContentHtml(const QString& contentHtml)
    {
        return contentHtml.trimmed().isEmpty() ? QStringLiteral("&nbsp;") : contentHtml;
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
            "data-callout-bar-width=\"%6\" data-callout-bar-height=\"%7\" "
            "data-callout-bar-radius=\"%8\" data-callout-content-gap=\"%9\" "
            "data-callout-frame-chrome-height=\"%10\" "
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
            .arg(QString::number(kFrameChromeHeight));
        html += QStringLiteral(
                    "<img class=\"whatson-callout-left-inset\" data-callout-frame-chrome=\"true\" "
                    "src=\"data:image/png;base64,%1\" width=\"%2\" height=\"1\" "
                    "style=\"border:0;vertical-align:middle;\"/>")
                    .arg(
                        QString::fromLatin1(kTransparentLeftInsetPng),
                        QString::number(kFramePaddingLeft));
        html += QStringLiteral(
                    "<img class=\"whatson-callout-leading-bar\" data-callout-frame-chrome=\"true\" "
                    "src=\"data:image/png;base64,%1\" width=\"%2\" height=\"%3\" "
                    "style=\"border:0;vertical-align:middle;\"/>")
                    .arg(
                        QString::fromLatin1(kLeadingBarPng),
                        QString::number(kLeadingBarWidth),
                        QString::number(kFrameChromeHeight));
        html += QStringLiteral(
                    "<img class=\"whatson-callout-content-gap\" data-callout-frame-chrome=\"true\" "
                    "src=\"data:image/png;base64,%1\" width=\"%2\" height=\"1\" "
                    "style=\"border:0;vertical-align:middle;\"/>")
                    .arg(
                        QString::fromLatin1(kTransparentContentGapPng),
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
