#include "app/models/editor/format/ContentsInlineStyleOverlayRenderer.hpp"

#include "app/models/editor/format/ContentsTextFormatRenderer.hpp"

ContentsInlineStyleOverlayRenderer::ContentsInlineStyleOverlayRenderer(QObject* parent)
    : QObject(parent)
    , m_delegateRenderer(new ContentsTextFormatRenderer(this))
{
    connect(
        m_delegateRenderer,
        &ContentsTextFormatRenderer::sourceTextChanged,
        this,
        &ContentsInlineStyleOverlayRenderer::sourceTextChanged);
    connect(
        m_delegateRenderer,
        &ContentsTextFormatRenderer::editorSurfaceHtmlChanged,
        this,
        &ContentsInlineStyleOverlayRenderer::editorSurfaceHtmlChanged);
    connect(
        m_delegateRenderer,
        &ContentsTextFormatRenderer::htmlOverlayVisibleChanged,
        this,
        &ContentsInlineStyleOverlayRenderer::htmlOverlayVisibleChanged);
    connect(
        m_delegateRenderer,
        &ContentsTextFormatRenderer::paperPaletteEnabledChanged,
        this,
        &ContentsInlineStyleOverlayRenderer::paperPaletteEnabledChanged);
}

ContentsInlineStyleOverlayRenderer::~ContentsInlineStyleOverlayRenderer() = default;

QString ContentsInlineStyleOverlayRenderer::sourceText() const
{
    return m_delegateRenderer != nullptr ? m_delegateRenderer->sourceText() : QString();
}

void ContentsInlineStyleOverlayRenderer::setSourceText(const QString& sourceText)
{
    if (m_delegateRenderer != nullptr)
    {
        m_delegateRenderer->setSourceText(sourceText);
    }
}

QString ContentsInlineStyleOverlayRenderer::editorSurfaceHtml() const
{
    return m_delegateRenderer != nullptr ? m_delegateRenderer->editorSurfaceHtml() : QString();
}

bool ContentsInlineStyleOverlayRenderer::htmlOverlayVisible() const noexcept
{
    return m_delegateRenderer != nullptr && m_delegateRenderer->htmlOverlayVisible();
}

bool ContentsInlineStyleOverlayRenderer::paperPaletteEnabled() const noexcept
{
    return m_delegateRenderer != nullptr && m_delegateRenderer->paperPaletteEnabled();
}

void ContentsInlineStyleOverlayRenderer::setPaperPaletteEnabled(const bool enabled)
{
    if (m_delegateRenderer != nullptr)
    {
        m_delegateRenderer->setPaperPaletteEnabled(enabled);
    }
}

void ContentsInlineStyleOverlayRenderer::requestRenderRefresh()
{
    if (m_delegateRenderer != nullptr)
    {
        m_delegateRenderer->requestRenderRefresh();
    }
}
