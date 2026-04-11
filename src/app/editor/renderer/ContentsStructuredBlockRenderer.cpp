#include "ContentsStructuredBlockRenderer.hpp"

#include "file/validator/WhatSonStructuredTagLinter.hpp"

#include <QVariantMap>

namespace
{
    QVariantMap defaultParseVerification(const QString& tagName)
    {
        return QVariantMap {
            {QStringLiteral("tagName"), tagName},
            {QStringLiteral("wellFormed"), true},
            {QStringLiteral("sourceLength"), 0},
            {QStringLiteral("issues"), QVariantList()}
        };
    }
} // namespace

ContentsStructuredBlockRenderer::ContentsStructuredBlockRenderer(QObject* parent)
    : QObject(parent)
    , m_agendaParseVerification(defaultParseVerification(QStringLiteral("agenda")))
    , m_calloutParseVerification(defaultParseVerification(QStringLiteral("callout")))
{
    connect(
        &m_agendaBackend,
        &ContentsAgendaBackend::parseVerificationReported,
        this,
        &ContentsStructuredBlockRenderer::handleAgendaParseVerificationReported);
    connect(
        &m_calloutBackend,
        &ContentsCalloutBackend::parseVerificationReported,
        this,
        &ContentsStructuredBlockRenderer::handleCalloutParseVerificationReported);
    refreshStructuredParseVerification();
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

QVariantMap ContentsStructuredBlockRenderer::agendaParseVerification() const
{
    return m_agendaParseVerification;
}

QVariantMap ContentsStructuredBlockRenderer::calloutParseVerification() const
{
    return m_calloutParseVerification;
}

QVariantMap ContentsStructuredBlockRenderer::structuredParseVerification() const
{
    return m_structuredParseVerification;
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

void ContentsStructuredBlockRenderer::refreshStructuredParseVerification()
{
    const WhatSonStructuredTagLinter tagLinter;
    const QVariantMap nextStructuredVerification = tagLinter.buildStructuredVerification(
        m_agendaParseVerification,
        m_calloutParseVerification,
        m_sourceText);
    if (m_structuredParseVerification == nextStructuredVerification)
    {
        return;
    }

    m_structuredParseVerification = nextStructuredVerification;
    emit structuredParseVerificationChanged();
    emit structuredParseVerificationReported(m_structuredParseVerification);
}

void ContentsStructuredBlockRenderer::handleAgendaParseVerificationReported(const QVariantMap& verification)
{
    if (m_agendaParseVerification != verification)
    {
        m_agendaParseVerification = verification;
        emit agendaParseVerificationChanged();
    }
    emit agendaParseVerificationReported(verification);
    refreshStructuredParseVerification();
}

void ContentsStructuredBlockRenderer::handleCalloutParseVerificationReported(const QVariantMap& verification)
{
    if (m_calloutParseVerification != verification)
    {
        m_calloutParseVerification = verification;
        emit calloutParseVerificationChanged();
    }
    emit calloutParseVerificationReported(verification);
    refreshStructuredParseVerification();
}
