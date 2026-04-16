#include "ContentsEditorPresentationProjection.hpp"

#include "editor/renderer/ContentsTextFormatRenderer.hpp"
#include "file/WhatSonDebugTrace.hpp"
#include "viewmodel/content/ContentsLogicalTextBridge.hpp"

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
        QStringLiteral("source=%1 previewEnabled=%2 overrideActive=%3")
            .arg(WhatSon::Debug::summarizeText(sourceText()))
            .arg(previewEnabled())
            .arg(m_hasRichTextSurfaceHtmlOverride));
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

QString ContentsEditorPresentationProjection::editorSurfaceHtml() const
{
    return m_textFormatRenderer != nullptr ? m_textFormatRenderer->editorSurfaceHtml() : QString();
}

QString ContentsEditorPresentationProjection::richTextSurfaceHtml() const
{
    return m_hasRichTextSurfaceHtmlOverride ? m_richTextSurfaceHtmlOverride : editorSurfaceHtml();
}

QString ContentsEditorPresentationProjection::renderedHtml() const
{
    return m_textFormatRenderer != nullptr ? m_textFormatRenderer->renderedHtml() : QString();
}

QString ContentsEditorPresentationProjection::logicalText() const
{
    return m_logicalTextBridge != nullptr ? m_logicalTextBridge->logicalText() : QString();
}

QVariantList ContentsEditorPresentationProjection::logicalLineStartOffsets() const
{
    return m_logicalTextBridge != nullptr ? m_logicalTextBridge->logicalLineStartOffsets() : QVariantList{0};
}

int ContentsEditorPresentationProjection::logicalLineCount() const noexcept
{
    return m_logicalTextBridge != nullptr ? m_logicalTextBridge->logicalLineCount() : 1;
}

QString ContentsEditorPresentationProjection::renderRichText(const QString& sourceText) const
{
    return m_textFormatRenderer != nullptr ? m_textFormatRenderer->renderRichText(sourceText) : QString();
}

QString ContentsEditorPresentationProjection::normalizeInlineStyleAliasesForEditor(
    const QString& sourceText) const
{
    return m_textFormatRenderer != nullptr
               ? m_textFormatRenderer->normalizeInlineStyleAliasesForEditor(sourceText)
               : sourceText;
}

QString ContentsEditorPresentationProjection::plainTextFromEditorSurfaceHtml(
    const QString& richTextHtml) const
{
    return m_textFormatRenderer != nullptr
               ? m_textFormatRenderer->plainTextFromEditorSurfaceHtml(richTextHtml)
               : QString();
}

QString ContentsEditorPresentationProjection::applyPlainTextReplacementToSource(
    const QString& sourceText,
    const int sourceStart,
    const int sourceEnd,
    const QString& replacementText) const
{
    return m_textFormatRenderer != nullptr
               ? m_textFormatRenderer->applyPlainTextReplacementToSource(
                     sourceText,
                     sourceStart,
                     sourceEnd,
                     replacementText)
               : sourceText;
}

QString ContentsEditorPresentationProjection::applyInlineStyleToLogicalSelectionSource(
    const QString& sourceText,
    const int selectionStart,
    const int selectionEnd,
    const QString& styleTag) const
{
    return m_textFormatRenderer != nullptr
               ? m_textFormatRenderer->applyInlineStyleToLogicalSelectionSource(
                     sourceText,
                     selectionStart,
                     selectionEnd,
                     styleTag)
               : sourceText;
}

int ContentsEditorPresentationProjection::logicalLineNumberForOffset(const int offset) const noexcept
{
    return m_logicalTextBridge != nullptr ? m_logicalTextBridge->logicalLineNumberForOffset(offset) : 1;
}

int ContentsEditorPresentationProjection::logicalLineStartOffsetAt(const int index) const noexcept
{
    return m_logicalTextBridge != nullptr ? m_logicalTextBridge->logicalLineStartOffsetAt(index) : 0;
}

int ContentsEditorPresentationProjection::logicalLineCharacterCountAt(const int index) const noexcept
{
    return m_logicalTextBridge != nullptr ? m_logicalTextBridge->logicalLineCharacterCountAt(index) : 0;
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

void ContentsEditorPresentationProjection::adoptIncrementalState(
    const QString& sourceText,
    const QString& logicalText,
    const QVariantList& logicalLineStartOffsets,
    const QVariantList& logicalToSourceOffsets)
{
    if (m_logicalTextBridge == nullptr)
    {
        return;
    }

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPresentationProjection"),
        QStringLiteral("adoptIncrementalState"),
        QStringLiteral("source=%1 logical=%2 lineOffsets=%3 sourceOffsets=%4")
            .arg(WhatSon::Debug::summarizeText(sourceText))
            .arg(WhatSon::Debug::summarizeText(logicalText))
            .arg(logicalLineStartOffsets.size())
            .arg(logicalToSourceOffsets.size()));

    m_logicalTextBridge->adoptIncrementalState(
        sourceText,
        logicalText,
        logicalLineStartOffsets,
        logicalToSourceOffsets);
}

void ContentsEditorPresentationProjection::setRichTextSurfaceHtml(
    const QString& richTextSurfaceHtml)
{
    const QString previousSurfaceHtml = this->richTextSurfaceHtml();
    if (m_hasRichTextSurfaceHtmlOverride
        && m_richTextSurfaceHtmlOverride == richTextSurfaceHtml)
    {
        return;
    }

    m_richTextSurfaceHtmlOverride = richTextSurfaceHtml;
    m_hasRichTextSurfaceHtmlOverride = true;

    if (previousSurfaceHtml == this->richTextSurfaceHtml())
    {
        return;
    }

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPresentationProjection"),
        QStringLiteral("setRichTextSurfaceHtml"),
        QStringLiteral("surface=%1")
            .arg(WhatSon::Debug::summarizeText(this->richTextSurfaceHtml())));
    emit richTextSurfaceHtmlChanged();
}

void ContentsEditorPresentationProjection::clearRichTextSurfaceHtml()
{
    const QString previousSurfaceHtml = richTextSurfaceHtml();
    if (!m_hasRichTextSurfaceHtmlOverride)
    {
        return;
    }

    m_hasRichTextSurfaceHtmlOverride = false;
    m_richTextSurfaceHtmlOverride.clear();

    if (previousSurfaceHtml == richTextSurfaceHtml())
    {
        return;
    }

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorPresentationProjection"),
        QStringLiteral("clearRichTextSurfaceHtml"),
        QStringLiteral("surface=%1")
            .arg(WhatSon::Debug::summarizeText(richTextSurfaceHtml())));
    emit richTextSurfaceHtmlChanged();
}

void ContentsEditorPresentationProjection::connectSignals()
{
    if (m_textFormatRenderer != nullptr)
    {
        connect(
            m_textFormatRenderer,
            &ContentsTextFormatRenderer::editorSurfaceHtmlChanged,
            this,
            [this]() {
                emit editorSurfaceHtmlChanged();
                if (!m_hasRichTextSurfaceHtmlOverride)
                {
                    emit richTextSurfaceHtmlChanged();
                }
            });
        connect(
            m_textFormatRenderer,
            &ContentsTextFormatRenderer::renderedHtmlChanged,
            this,
            &ContentsEditorPresentationProjection::renderedHtmlChanged);
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
            &ContentsLogicalTextBridge::logicalLineStartOffsetsChanged,
            this,
            &ContentsEditorPresentationProjection::logicalLineStartOffsetsChanged);
        connect(
            m_logicalTextBridge,
            &ContentsLogicalTextBridge::logicalLineCountChanged,
            this,
            &ContentsEditorPresentationProjection::logicalLineCountChanged);
    }
}
