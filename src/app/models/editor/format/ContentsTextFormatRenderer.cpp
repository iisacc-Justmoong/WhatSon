#include "app/models/editor/format/ContentsTextFormatRenderer.hpp"
#include "app/models/editor/format/ContentsTextFormatRendererInternal.hpp"
#include "app/models/editor/renderer/ContentsHtmlBlockRenderPipeline.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"

ContentsTextFormatRenderer::ContentsTextFormatRenderer(QObject* parent)
    : QObject(parent)
{
    WhatSon::Debug::traceEditorSelf(this, QStringLiteral("textFormatRenderer"), QStringLiteral("ctor"));
}

ContentsTextFormatRenderer::~ContentsTextFormatRenderer()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("textFormatRenderer"),
        QStringLiteral("dtor"),
        QStringLiteral("preview=%1 sourceSummary=%2")
            .arg(m_previewEnabled)
            .arg(WhatSon::Debug::summarizeText(m_sourceText)));
}

QString ContentsTextFormatRenderer::sourceText() const
{
    return m_sourceText;
}

void ContentsTextFormatRenderer::setSourceText(const QString& sourceText)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("textFormatRenderer"),
        QStringLiteral("setSourceText"),
        QStringLiteral("changed=%1 %2").arg(m_sourceText != sourceText).arg(WhatSon::Debug::summarizeText(sourceText)));
    if (m_sourceText == sourceText)
    {
        return;
    }

    m_sourceText = sourceText;
    emit sourceTextChanged();
    refreshRenderedOutputs();
}

QString ContentsTextFormatRenderer::editorSurfaceHtml() const
{
    return m_editorSurfaceHtml;
}

QString ContentsTextFormatRenderer::renderedHtml() const
{
    return m_renderedHtml;
}

QVariantList ContentsTextFormatRenderer::htmlTokens() const
{
    return m_htmlTokens;
}

QVariantList ContentsTextFormatRenderer::normalizedHtmlBlocks() const
{
    return m_normalizedHtmlBlocks;
}

bool ContentsTextFormatRenderer::htmlOverlayVisible() const noexcept
{
    return m_htmlOverlayVisible;
}

bool ContentsTextFormatRenderer::previewEnabled() const noexcept
{
    return m_previewEnabled;
}

void ContentsTextFormatRenderer::setPreviewEnabled(const bool enabled)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("textFormatRenderer"),
        QStringLiteral("setPreviewEnabled"),
        QStringLiteral("previous=%1 next=%2").arg(m_previewEnabled).arg(enabled));
    if (m_previewEnabled == enabled)
    {
        return;
    }

    m_previewEnabled = enabled;
    emit previewEnabledChanged();
    refreshRenderedOutputs();
}

bool ContentsTextFormatRenderer::paperPaletteEnabled() const noexcept
{
    return m_paperPaletteEnabled;
}

void ContentsTextFormatRenderer::setPaperPaletteEnabled(const bool enabled)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("textFormatRenderer"),
        QStringLiteral("setPaperPaletteEnabled"),
        QStringLiteral("previous=%1 next=%2").arg(m_paperPaletteEnabled).arg(enabled));
    if (m_paperPaletteEnabled == enabled)
    {
        return;
    }

    m_paperPaletteEnabled = enabled;
    emit paperPaletteEnabledChanged();
    refreshRenderedOutputs();
}

void ContentsTextFormatRenderer::requestRenderRefresh()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("textFormatRenderer"),
        QStringLiteral("requestRenderRefresh"),
        QStringLiteral("preview=%1").arg(m_previewEnabled));
    refreshRenderedOutputs();
}

void ContentsTextFormatRenderer::refreshRenderedOutputs()
{
    using namespace WhatSon::ContentsTextFormatRendererInternal;

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("textFormatRenderer"),
        QStringLiteral("refreshRenderedOutputs"),
        QStringLiteral("preview=%1 %2").arg(m_previewEnabled).arg(WhatSon::Debug::summarizeText(m_sourceText)));
    const ContentsHtmlBlockRenderPipeline renderPipeline;
    const ContentsHtmlBlockRenderPipeline::RenderResult editorRenderResult =
        renderPipeline.renderEditorDocument(m_sourceText);
    const QString presentationSourceText = editorRenderResult.correctedSourceText.isEmpty()
        ? m_sourceText
        : editorRenderResult.correctedSourceText;

    const QVariantList nextHtmlTokens = m_paperPaletteEnabled
        ? applyPaperPaletteToHtmlField(editorRenderResult.htmlTokens, QStringLiteral("html"))
        : editorRenderResult.htmlTokens;
    if (m_htmlTokens != nextHtmlTokens)
    {
        m_htmlTokens = nextHtmlTokens;
        emit htmlTokensChanged();
    }

    const QVariantList nextNormalizedHtmlBlocks = m_paperPaletteEnabled
        ? applyPaperPaletteToHtmlField(editorRenderResult.normalizedHtmlBlocks, QStringLiteral("htmlBlockHtml"))
        : editorRenderResult.normalizedHtmlBlocks;
    if (m_normalizedHtmlBlocks != nextNormalizedHtmlBlocks)
    {
        m_normalizedHtmlBlocks = nextNormalizedHtmlBlocks;
        emit normalizedHtmlBlocksChanged();
    }

    if (m_htmlOverlayVisible != editorRenderResult.htmlOverlayVisible)
    {
        m_htmlOverlayVisible = editorRenderResult.htmlOverlayVisible;
        emit htmlOverlayVisibleChanged();
    }

    const QString baseEditorSurfaceHtml = editorRenderResult.requiresLegacyDocumentComposition
        ? renderInlineStyleEditingSurfaceHtml(presentationSourceText)
        : editorRenderResult.documentHtml;
    const QString nextEditorSurfaceHtml = m_paperPaletteEnabled
        ? applyPaperPaletteToHtml(baseEditorSurfaceHtml)
        : baseEditorSurfaceHtml;
    if (m_editorSurfaceHtml != nextEditorSurfaceHtml)
    {
        m_editorSurfaceHtml = nextEditorSurfaceHtml;
        emit editorSurfaceHtmlChanged();
    }

    const QString baseRenderedHtml = m_previewEnabled
        ? renderMarkdownAwareTextToHtml(presentationSourceText)
        : QString();
    const QString nextRenderedHtml = m_paperPaletteEnabled
        ? applyPaperPaletteToHtml(baseRenderedHtml)
        : baseRenderedHtml;
    if (m_renderedHtml == nextRenderedHtml)
    {
        return;
    }

    m_renderedHtml = nextRenderedHtml;
    emit renderedHtmlChanged();
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("textFormatRenderer"),
        QStringLiteral("renderedHtmlChanged"),
        QStringLiteral("editorSurfaceLen=%1 renderedLen=%2 htmlTokens=%3 htmlBlocks=%4")
            .arg(m_editorSurfaceHtml.size())
            .arg(m_renderedHtml.size())
            .arg(m_htmlTokens.size())
            .arg(m_normalizedHtmlBlocks.size()));
}
