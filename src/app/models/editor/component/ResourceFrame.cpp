#include "app/models/editor/component/ResourceFrame.h"

#include "app/models/hierarchy/resources/WhatSonResourcePackageSupport.hpp"

#include <algorithm>
#include <cmath>

#include <QFileInfo>
#include <QImageReader>
#include <QUrl>

namespace
{
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

    QSize sourceImageSize(const QString& imagePath)
    {
        QImageReader reader(imagePath);
        reader.setAutoTransform(true);
        if (reader.size().isValid() && !reader.size().isEmpty())
        {
            return reader.size();
        }
        return {};
    }

    bool isImageDescriptor(const WhatSon::EditorComponent::ResourceFrameDescriptor& descriptor)
    {
        const QString normalizedType = WhatSon::Resources::normalizedType(descriptor.type);
        const QString normalizedFormat = WhatSon::Resources::normalizeFormat(descriptor.format).toCaseFolded();
        return normalizedType == QStringLiteral("image")
            || WhatSon::Resources::inferTypeFromFormat(normalizedFormat) == QStringLiteral("image");
    }

    QString mediaHtml(const WhatSon::EditorComponent::ResourceFrameDescriptor& descriptor)
    {
        if (isImageDescriptor(descriptor) && QFileInfo(descriptor.resolvedAssetPath).isFile())
        {
            const QSize displaySize = WhatSon::EditorComponent::ResourceFrame::imageDisplaySize(
                sourceImageSize(descriptor.resolvedAssetPath));
            return QStringLiteral(
                       "<img src=\"%1\" width=\"%2\" height=\"%3\" style=\"vertical-align:middle;\" />")
                .arg(
                    htmlAttribute(QUrl::fromLocalFile(descriptor.resolvedAssetPath).toString()),
                    QString::number(displaySize.width()),
                    QString::number(displaySize.height()));
        }

        return QStringLiteral(
                   "<span style=\"font-family:Pretendard;font-size:11px;line-height:11px;color:#7F7F7F;\">%1</span>")
            .arg(htmlAttribute(displayFileName(descriptor)));
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
        return QSize(338, 352);
    }

    QSize ResourceFrame::imageDisplaySize(const QSize& sourceSize)
    {
        const QSize viewportSize = previewViewportSize();
        if (!sourceSize.isValid() || sourceSize.isEmpty())
        {
            return viewportSize;
        }

        const double scale = std::min(
            static_cast<double>(viewportSize.width()) / std::max(1, sourceSize.width()),
            static_cast<double>(viewportSize.height()) / std::max(1, sourceSize.height()));
        return QSize(
            std::max(1, static_cast<int>(std::lround(sourceSize.width() * scale))),
            std::max(1, static_cast<int>(std::lround(sourceSize.height() * scale))));
    }

    QString ResourceFrame::sourceMarker(const QString& sourceTag)
    {
        return QString::fromLatin1(sourceTag.toUtf8().toHex());
    }

    QString ResourceFrame::renderHtml(const ResourceFrameDescriptor& descriptor)
    {
        const QString typeLabel = displayTypeLabel(descriptor);
        const QString fileName = displayFileName(descriptor);
        const QSize viewportSize = previewViewportSize();

        QString html;
        html.reserve(1800);
        html += QStringLiteral("<!--whatson-resource-source:");
        html += sourceMarker(descriptor.sourceTag);
        html += QStringLiteral("-->");
        html += QStringLiteral(
            "<p class=\"whatson-resource-frame\" data-figma-node-id=\"292:50\" "
            "style=\"margin-top:0px;margin-bottom:0px;line-height:1;\">");
        html += QStringLiteral(
            "<table width=\"480\" border=\"1\" cellspacing=\"0\" cellpadding=\"0\" style=\""
            "border:1px solid #2C2E2F;border-color:#2C2E2F;border-radius:12px;"
            "background-color:#1E1F20;\">");
        html += QStringLiteral("<tr class=\"resourceHeader\" data-name=\"resourceHeader\">");
        html += QStringLiteral(
            "<td width=\"480\" height=\"20\" bgcolor=\"#1E1F20\" "
            "style=\"border-bottom:1px solid #2C2E2F;padding:4px 8px;\">");
        html += QStringLiteral(
            "<span style=\"font-family:Pretendard;font-size:11px;font-weight:400;"
            "line-height:11px;color:#7F7F7F;\">");
        html += htmlAttribute(typeLabel);
        html += QStringLiteral("</span>");
        html += QStringLiteral(
            "<span class=\"whatson-resource-more\" style=\"font-family:Pretendard;"
            "font-size:11px;line-height:11px;color:#CED0D6;\">&nbsp;&nbsp;&nbsp;...</span>");
        html += QStringLiteral("</td></tr>");
        html += QStringLiteral("<tr>");
        html += QStringLiteral(
            "<td width=\"480\" height=\"%1\" align=\"center\" valign=\"middle\" "
            "bgcolor=\"#1E1F20\" style=\"padding:0px;\">")
            .arg(QString::number(viewportSize.height()));
        html += mediaHtml(descriptor);
        html += QStringLiteral("</td></tr>");
        html += QStringLiteral("<tr class=\"resourceToolbar\" data-name=\"resourceToolbar\">");
        html += QStringLiteral(
            "<td width=\"480\" height=\"20\" bgcolor=\"#1E1F20\" "
            "style=\"border-top:1px solid #2C2E2F;padding:4px 8px;\">");
        html += QStringLiteral(
            "<span style=\"font-family:Pretendard;font-size:11px;font-weight:400;"
            "line-height:11px;color:#7F7F7F;\">");
        html += htmlAttribute(fileName);
        html += QStringLiteral("</span></td></tr>");
        html += QStringLiteral("</table></p><!--/whatson-resource-source-->");
        return html;
    }

    QStringList ResourceFrame::renderedTextLines(const ResourceFrameDescriptor& descriptor)
    {
        QStringList lines;
        const QString typeLabel = displayTypeLabel(descriptor);
        appendUniqueTextLine(&lines, typeLabel);
        appendUniqueTextLine(&lines, QStringLiteral("..."));
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
