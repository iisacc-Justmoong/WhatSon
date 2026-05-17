#include "app/models/editor/component/Callout.h"

namespace
{
    constexpr int kFigmaNodeWidth = 295;
    constexpr auto kCalloutRenderVersion = "figma-280-7897-wrapping-callout-v1";

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
            "<span class=\"whatson-callout\" data-figma-node-id=\"280:7897\" "
            "data-frame-design-width=\"%1\" data-callout-render=\"%2\" "
            "data-frame-width-mode=\"fill\" data-frame-height-mode=\"hug-contents\" "
            "data-callout-content=\"true\" width=\"100%\" "
            "style=\"width:100%;max-width:100%;height:auto;min-height:0;"
            "box-sizing:border-box;background-color:#262728;"
            "padding:16px 4px;padding-left:12px;border-left:3px solid #d9d9d9;"
            "font-family:Pretendard;font-size:12px;font-weight:500;line-height:12px;"
            "color:#FFFFFF;white-space:normal;word-break:break-word;\">")
            .arg(QString::number(kFigmaNodeWidth), QString::fromLatin1(kCalloutRenderVersion));
        html += contentHtml;
        html += QStringLiteral(
            "</span>"
            "<!--/whatson-callout-source-->");
        return html;
    }
} // namespace WhatSon::EditorComponent
