#include "app/models/editor/projection/ContentsEditorPresentationProjection.hpp"

#include "app/models/editor/format/ContentsTextFormatRenderer.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/models/editor/text/ContentsLogicalTextBridge.hpp"

ContentsEditorPresentationProjection::ContentsEditorPresentationProjection(QObject* parent)
    : QObject(parent)
    , m_textFormatRenderer(new ContentsTextFormatRenderer(this))
    , m_logicalTextBridge(new ContentsLogicalTextBridge(this))
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPresentationProjection"),
        QStringLiteral("ctor"));
    connectSignals();
}

ContentsEditorPresentationProjection::~ContentsEditorPresentationProjection()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPresentationProjection"),
        QStringLiteral("dtor"),
        QStringLiteral("source=%1 previewEnabled=%2")
            .arg(WhatSon::Debug::summarizeText(sourceText()))
            .arg(previewEnabled()));
}

QString ContentsEditorPresentationProjection::sourceText() const
{
    return m_textFormatRenderer != nullptr ? m_textFormatRenderer->sourceText() : QString();
}

void ContentsEditorPresentationProjection::setSourceText(const QString& sourceText)
{
    const QString previousSourceText = this->sourceText();
    if (previousSourceText == sourceText)
    {
        return;
    }

    if (m_textFormatRenderer != nullptr)
    {
        m_textFormatRenderer->setSourceText(sourceText);
    }
    if (m_logicalTextBridge != nullptr)
    {
        m_logicalTextBridge->setText(sourceText);
    }

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPresentationProjection"),
        QStringLiteral("setSourceText"),
        QStringLiteral("previous=%1 next=%2")
            .arg(WhatSon::Debug::summarizeText(previousSourceText))
            .arg(WhatSon::Debug::summarizeText(sourceText)));
    emit sourceTextChanged();
}

bool ContentsEditorPresentationProjection::previewEnabled() const noexcept
{
    return m_textFormatRenderer != nullptr && m_textFormatRenderer->previewEnabled();
}

void ContentsEditorPresentationProjection::setPreviewEnabled(const bool enabled)
{
    if (previewEnabled() == enabled)
    {
        return;
    }

    if (m_textFormatRenderer != nullptr)
    {
        m_textFormatRenderer->setPreviewEnabled(enabled);
    }

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPresentationProjection"),
        QStringLiteral("setPreviewEnabled"),
        QStringLiteral("enabled=%1").arg(enabled));
    emit previewEnabledChanged();
}

bool ContentsEditorPresentationProjection::paperPaletteEnabled() const noexcept
{
    return m_textFormatRenderer != nullptr && m_textFormatRenderer->paperPaletteEnabled();
}

void ContentsEditorPresentationProjection::setPaperPaletteEnabled(const bool enabled)
{
    if (paperPaletteEnabled() == enabled)
    {
        return;
    }

    if (m_textFormatRenderer != nullptr)
    {
        m_textFormatRenderer->setPaperPaletteEnabled(enabled);
    }

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPresentationProjection"),
        QStringLiteral("setPaperPaletteEnabled"),
        QStringLiteral("enabled=%1").arg(enabled));
    emit paperPaletteEnabledChanged();
}

QString ContentsEditorPresentationProjection::editorSurfaceHtml() const
{
    return m_textFormatRenderer != nullptr ? m_textFormatRenderer->editorSurfaceHtml() : QString();
}

QString ContentsEditorPresentationProjection::renderedHtml() const
{
    return m_textFormatRenderer != nullptr ? m_textFormatRenderer->renderedHtml() : QString();
}

QVariantList ContentsEditorPresentationProjection::htmlTokens() const
{
    return m_textFormatRenderer != nullptr ? m_textFormatRenderer->htmlTokens() : QVariantList();
}

QVariantList ContentsEditorPresentationProjection::normalizedHtmlBlocks() const
{
    return m_textFormatRenderer != nullptr ? m_textFormatRenderer->normalizedHtmlBlocks() : QVariantList();
}

QString ContentsEditorPresentationProjection::logicalText() const
{
    return m_logicalTextBridge != nullptr ? m_logicalTextBridge->logicalText() : QString();
}

int ContentsEditorPresentationProjection::logicalLineCount() const noexcept
{
    return m_logicalTextBridge != nullptr ? m_logicalTextBridge->logicalLineCount() : 1;
}

int ContentsEditorPresentationProjection::logicalLengthForSourceText(const QString& text) const
{
    return m_logicalTextBridge != nullptr ? m_logicalTextBridge->logicalLengthForSourceText(text) : text.size();
}

QVariantList ContentsEditorPresentationProjection::logicalToSourceOffsets() const
{
    return m_logicalTextBridge != nullptr ? m_logicalTextBridge->logicalToSourceOffsets()
                                          : QVariantList{0};
}

int ContentsEditorPresentationProjection::sourceOffsetForLogicalOffset(const int logicalOffset) const noexcept
{
    return m_logicalTextBridge != nullptr
               ? m_logicalTextBridge->sourceOffsetForLogicalOffset(logicalOffset)
               : logicalOffset;
}

void ContentsEditorPresentationProjection::connectSignals()
{
    if (m_textFormatRenderer != nullptr)
    {
        connect(
            m_textFormatRenderer,
            &ContentsTextFormatRenderer::editorSurfaceHtmlChanged,
            this,
            &ContentsEditorPresentationProjection::editorSurfaceHtmlChanged);
        connect(
            m_textFormatRenderer,
            &ContentsTextFormatRenderer::renderedHtmlChanged,
            this,
            &ContentsEditorPresentationProjection::renderedHtmlChanged);
        connect(
            m_textFormatRenderer,
            &ContentsTextFormatRenderer::htmlTokensChanged,
            this,
            &ContentsEditorPresentationProjection::htmlTokensChanged);
        connect(
            m_textFormatRenderer,
            &ContentsTextFormatRenderer::normalizedHtmlBlocksChanged,
            this,
            &ContentsEditorPresentationProjection::normalizedHtmlBlocksChanged);
    }

    if (m_logicalTextBridge != nullptr)
    {
        connect(
            m_logicalTextBridge,
            &ContentsLogicalTextBridge::logicalTextChanged,
            this,
            &ContentsEditorPresentationProjection::logicalTextChanged);
        connect(
            m_logicalTextBridge,
            &ContentsLogicalTextBridge::logicalLineCountChanged,
            this,
            &ContentsEditorPresentationProjection::logicalLineCountChanged);
    }
}
