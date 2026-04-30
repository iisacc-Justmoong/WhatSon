#include "app/models/editor/resource/ContentsInlineResourcePresentationController.hpp"

#include "app/models/editor/resource/ContentsEditorDynamicObjectSupport.hpp"

#include <QRegularExpression>

using namespace WhatSon::Editor::DynamicObjectSupport;

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
    const QString sourceUrl = entry.value(QStringLiteral("source")).toString().trimmed();
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

QString ContentsInlineResourcePresentationController::inlineResourceBlockHtml(const QVariant& resourceEntry) const
{
    const QVariantMap entry = resourceEntry.toMap();
    const QString renderMode = entry.value(QStringLiteral("renderMode")).toString().trimmed().toLower();
    const QString sourceUrl = resourceEntryOpenTarget(resourceEntry);
    const QString encodedSourceUrl = m_view
        ? invokeString(m_view, "encodeXmlAttributeValue", { sourceUrl }, sourceUrl)
        : sourceUrl;
    if (renderMode == QStringLiteral("image") && !encodedSourceUrl.isEmpty())
    {
        return QStringLiteral(
                   "<table width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\" style=\"margin-top:0px;margin-bottom:0px;\">"
                   "<tr><td align=\"center\"><img src=\"%1\" width=\"%2\" /></td></tr></table>")
            .arg(encodedSourceUrl, QString::number(inlineResourcePreviewWidth()));
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
    const QString& editorHtml) const
{
    const QString baseEditorHtml = editorHtml;
    QVariant renderedResources;
    if (m_bodyResourceRenderer)
    {
        renderedResources = m_bodyResourceRenderer->property("renderedResources");
    }
    const QVariantList normalizedResources = invokeVariant(
        m_resourceTagController,
        "normalizedImportedResourceEntries",
        { renderedResources })
                                               .toList();

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
                ? inlineResourceBlockHtml(entry)
                : resourcePlaceholderBlockHtml());
        lastOffset = currentMatch.capturedEnd();
    }
    renderedHtml.append(baseEditorHtml.mid(lastOffset));
    return renderedHtml;
}
