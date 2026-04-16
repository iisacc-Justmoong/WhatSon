#include "ContentsDisplayStructuredFlowCoordinator.hpp"

#include "file/WhatSonDebugTrace.hpp"

ContentsDisplayStructuredFlowCoordinator::ContentsDisplayStructuredFlowCoordinator(QObject* parent)
    : QObject(parent)
{
    WhatSon::Debug::traceEditorSelf(this, QStringLiteral("structuredFlowCoordinator"), QStringLiteral("ctor"));
}

ContentsDisplayStructuredFlowCoordinator::~ContentsDisplayStructuredFlowCoordinator()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredFlowCoordinator"),
        QStringLiteral("dtor"),
        QStringLiteral("selectedNoteId=%1 activatedNoteId=%2 renderPending=%3 parsedRequested=%4")
            .arg(m_selectedNoteId)
            .arg(m_activatedNoteId)
            .arg(m_renderPending)
            .arg(m_parsedStructuredFlowRequested));
}

QString ContentsDisplayStructuredFlowCoordinator::selectedNoteId() const
{
    return m_selectedNoteId;
}

void ContentsDisplayStructuredFlowCoordinator::setSelectedNoteId(const QString& noteId)
{
    const QString normalized = normalizeNoteId(noteId);
    if (m_selectedNoteId == normalized)
    {
        return;
    }

    m_selectedNoteId = normalized;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredFlowCoordinator"),
        QStringLiteral("setSelectedNoteId"),
        QStringLiteral("noteId=%1").arg(normalized));
    emit selectedNoteIdChanged();
    refreshActivatedNoteId();
}

bool ContentsDisplayStructuredFlowCoordinator::editorSessionBoundToSelectedNote() const noexcept
{
    return m_editorSessionBoundToSelectedNote;
}

void ContentsDisplayStructuredFlowCoordinator::setEditorSessionBoundToSelectedNote(const bool bound)
{
    if (m_editorSessionBoundToSelectedNote == bound)
    {
        return;
    }

    m_editorSessionBoundToSelectedNote = bound;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredFlowCoordinator"),
        QStringLiteral("setEditorSessionBoundToSelectedNote"),
        QStringLiteral("bound=%1").arg(bound));
    emit editorSessionBoundToSelectedNoteChanged();
    refreshActivatedNoteId();
}

bool ContentsDisplayStructuredFlowCoordinator::parsedStructuredFlowRequested() const noexcept
{
    return m_parsedStructuredFlowRequested;
}

void ContentsDisplayStructuredFlowCoordinator::setParsedStructuredFlowRequested(const bool requested)
{
    if (m_parsedStructuredFlowRequested == requested)
    {
        return;
    }

    m_parsedStructuredFlowRequested = requested;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredFlowCoordinator"),
        QStringLiteral("setParsedStructuredFlowRequested"),
        QStringLiteral("requested=%1").arg(requested));
    emit parsedStructuredFlowRequestedChanged();
    refreshActivatedNoteId();
}

bool ContentsDisplayStructuredFlowCoordinator::renderPending() const noexcept
{
    return m_renderPending;
}

void ContentsDisplayStructuredFlowCoordinator::setRenderPending(const bool pending)
{
    if (m_renderPending == pending)
    {
        return;
    }

    m_renderPending = pending;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredFlowCoordinator"),
        QStringLiteral("setRenderPending"),
        QStringLiteral("pending=%1").arg(pending));
    emit renderPendingChanged();
    refreshActivatedNoteId();
}

QString ContentsDisplayStructuredFlowCoordinator::activatedNoteId() const
{
    return m_activatedNoteId;
}

void ContentsDisplayStructuredFlowCoordinator::refreshActivatedNoteId()
{
    const QString normalizedSelectedNoteId = normalizeNoteId(m_selectedNoteId);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredFlowCoordinator"),
        QStringLiteral("refreshActivatedNoteId"),
        QStringLiteral("selectedNoteId=%1 sessionBound=%2 parsedRequested=%3 renderPending=%4 activatedNoteId=%5")
            .arg(normalizedSelectedNoteId)
            .arg(m_editorSessionBoundToSelectedNote)
            .arg(m_parsedStructuredFlowRequested)
            .arg(m_renderPending)
            .arg(m_activatedNoteId));

    if (normalizedSelectedNoteId.isEmpty())
    {
        setActivatedNoteId(QString());
        return;
    }

    if (!m_editorSessionBoundToSelectedNote)
    {
        if (!m_renderPending)
        {
            setActivatedNoteId(QString());
        }
        return;
    }

    if (m_parsedStructuredFlowRequested)
    {
        setActivatedNoteId(normalizedSelectedNoteId);
        return;
    }

    if (m_renderPending)
    {
        return;
    }

    if (m_activatedNoteId == normalizedSelectedNoteId)
    {
        setActivatedNoteId(QString());
    }
}

QString ContentsDisplayStructuredFlowCoordinator::normalizeNoteId(const QString& noteId)
{
    return noteId.trimmed();
}

void ContentsDisplayStructuredFlowCoordinator::setActivatedNoteId(const QString& noteId)
{
    const QString normalized = normalizeNoteId(noteId);
    if (m_activatedNoteId == normalized)
    {
        return;
    }

    m_activatedNoteId = normalized;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("structuredFlowCoordinator"),
        QStringLiteral("setActivatedNoteId"),
        QStringLiteral("noteId=%1").arg(normalized));
    emit activatedNoteIdChanged();
}
