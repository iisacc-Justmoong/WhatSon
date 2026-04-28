#include "app/viewmodel/editor/display/ContentsDisplaySurfacePolicy.hpp"

ContentsDisplaySurfacePolicy::ContentsDisplaySurfacePolicy(QObject* parent)
    : QObject(parent)
{
}

ContentsDisplaySurfacePolicy::~ContentsDisplaySurfacePolicy() = default;

bool ContentsDisplaySurfacePolicy::hasSelectedNote() const noexcept
{
    return m_hasSelectedNote;
}

bool ContentsDisplaySurfacePolicy::resourceViewerRequested() const noexcept
{
    return m_resourceViewerRequested;
}

bool ContentsDisplaySurfacePolicy::formattedPreviewRequested() const noexcept
{
    return m_formattedPreviewRequested;
}

bool ContentsDisplaySurfacePolicy::structuredDocumentSurfaceRequested() const noexcept
{
    return structuredSurfaceActive();
}

bool ContentsDisplaySurfacePolicy::structuredDocumentFlowVisible() const noexcept
{
    return structuredSurfaceActive();
}

bool ContentsDisplaySurfacePolicy::dedicatedResourceViewerVisible() const noexcept
{
    return m_hasSelectedNote && m_resourceViewerRequested;
}

bool ContentsDisplaySurfacePolicy::formattedTextRendererVisible() const noexcept
{
    return m_hasSelectedNote && m_formattedPreviewRequested && !m_resourceViewerRequested;
}

bool ContentsDisplaySurfacePolicy::documentPresentationProjectionEnabled() const noexcept
{
    return formattedTextRendererVisible();
}

bool ContentsDisplaySurfacePolicy::inlineHtmlImageRenderingEnabled() const noexcept
{
    return false;
}

bool ContentsDisplaySurfacePolicy::resourceBlocksRenderedInlineByHtmlProjection() const noexcept
{
    return false;
}

bool ContentsDisplaySurfacePolicy::nativeInputPriority() const noexcept
{
    return structuredSurfaceActive();
}

QString ContentsDisplaySurfacePolicy::activeSurfaceKind() const
{
    if (dedicatedResourceViewerVisible())
    {
        return QStringLiteral("resourceViewer");
    }
    if (formattedTextRendererVisible())
    {
        return QStringLiteral("formattedPreview");
    }
    if (structuredSurfaceActive())
    {
        return QStringLiteral("structured");
    }
    return QStringLiteral("none");
}

void ContentsDisplaySurfacePolicy::setHasSelectedNote(const bool selected)
{
    if (m_hasSelectedNote == selected)
    {
        return;
    }

    m_hasSelectedNote = selected;
    emitPolicyChanged();
}

void ContentsDisplaySurfacePolicy::setResourceViewerRequested(const bool requested)
{
    if (m_resourceViewerRequested == requested)
    {
        return;
    }

    m_resourceViewerRequested = requested;
    emitPolicyChanged();
}

void ContentsDisplaySurfacePolicy::setFormattedPreviewRequested(const bool requested)
{
    if (m_formattedPreviewRequested == requested)
    {
        return;
    }

    m_formattedPreviewRequested = requested;
    emitPolicyChanged();
}

void ContentsDisplaySurfacePolicy::emitPolicyChanged()
{
    emit inputsChanged();
    emit outputsChanged();
}

bool ContentsDisplaySurfacePolicy::structuredSurfaceActive() const noexcept
{
    return m_hasSelectedNote
        && !m_resourceViewerRequested
        && !m_formattedPreviewRequested;
}
