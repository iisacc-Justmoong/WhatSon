#include "ContentsStructuredBlockRenderer.hpp"

ContentsStructuredBlockRenderer::ContentsStructuredBlockRenderer(QObject* parent)
    : QObject(parent)
{
}

ContentsStructuredBlockRenderer::~ContentsStructuredBlockRenderer() = default;

QString ContentsStructuredBlockRenderer::sourceText() const
{
    return m_sourceText;
}

void ContentsStructuredBlockRenderer::setSourceText(const QString& sourceText)
{
    if (m_sourceText == sourceText)
    {
        return;
    }

    m_sourceText = sourceText;
    emit sourceTextChanged();
    refreshRenderedBlocks();
}

QVariantList ContentsStructuredBlockRenderer::renderedAgendas() const
{
    return m_renderedAgendas;
}

QVariantList ContentsStructuredBlockRenderer::renderedCallouts() const
{
    return m_renderedCallouts;
}

int ContentsStructuredBlockRenderer::agendaCount() const noexcept
{
    return m_renderedAgendas.size();
}

int ContentsStructuredBlockRenderer::calloutCount() const noexcept
{
    return m_renderedCallouts.size();
}

bool ContentsStructuredBlockRenderer::hasRenderedBlocks() const noexcept
{
    return !m_renderedAgendas.isEmpty() || !m_renderedCallouts.isEmpty();
}

void ContentsStructuredBlockRenderer::requestRenderRefresh()
{
    refreshRenderedBlocks();
}

void ContentsStructuredBlockRenderer::refreshRenderedBlocks()
{
    const QVariantList nextRenderedAgendas = m_agendaBackend.parseAgendas(m_sourceText);
    const QVariantList nextRenderedCallouts = m_calloutBackend.parseCallouts(m_sourceText);
    if (m_renderedAgendas == nextRenderedAgendas
        && m_renderedCallouts == nextRenderedCallouts)
    {
        return;
    }

    m_renderedAgendas = nextRenderedAgendas;
    m_renderedCallouts = nextRenderedCallouts;
    emit renderedBlocksChanged();
}
