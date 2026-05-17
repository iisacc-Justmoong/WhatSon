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
        html.reserve(contentHtml.size() + 1800);
        html += QStringLiteral("<!--whatson-callout-source:");
        html += sourceMarker(descriptor.sourceText);
        html += QStringLiteral("-->");
        html += QStringLiteral(
            "<table class=\"whatson-callout\" data-figma-node-id=\"280:7897\" "
            "data-frame-design-width=\"%1\" data-callout-render=\"%2\" "
            "data-frame-width-mode=\"fill\" data-frame-height-mode=\"hug-contents\" "
            "width=\"100%\" cellspacing=\"0\" cellpadding=\"0\" "
            "style=\"width:100%;max-width:100%;height:auto;min-height:0;"
            "border-spacing:0;border-collapse:separate;"
            "background-color:#262728;padding:4px;\">"
            "<tr>"
            "<td width=\"3\" class=\"whatson-callout-bar\" "
            "style=\"width:3px;min-width:3px;max-width:3px;height:100%;min-height:14px;"
            "background-color:#d9d9d9;border-radius:3px;padding:0;margin:0;"
            "font-size:0;line-height:0;vertical-align:top;\">&nbsp;</td>"
            "<td width=\"12\" class=\"whatson-callout-gap\" "
            "style=\"width:12px;min-width:12px;max-width:12px;padding:0;margin:0;"
            "font-size:0;line-height:0;\">&nbsp;</td>"
            "<td class=\"whatson-callout-content\" data-callout-content=\"true\" "
            "style=\"padding:0;margin:0;width:100%;font-family:Pretendard;"
            "font-size:12px;font-weight:500;line-height:12px;color:#FFFFFF;"
            "white-space:normal;word-break:break-word;vertical-align:middle;\">")
            .arg(QString::number(kFigmaNodeWidth), QString::fromLatin1(kCalloutRenderVersion));
        html += contentHtml;
        html += QStringLiteral(
            "</td>"
            "</tr>"
            "</table>"
            "<!--/whatson-callout-source-->");
        return html;
    }
} // namespace WhatSon::EditorComponent
