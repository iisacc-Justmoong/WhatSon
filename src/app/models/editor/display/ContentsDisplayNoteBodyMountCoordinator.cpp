#include "app/models/editor/display/ContentsDisplayNoteBodyMountCoordinator.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"

namespace
{
    QString normalizedOptionalNoteId(const QString& value)
    {
        return value.trimmed();
    }
}

ContentsDisplayNoteBodyMountCoordinator::ContentsDisplayNoteBodyMountCoordinator(QObject* parent)
    : QObject(parent)
{
    WhatSon::Debug::traceEditorSelf(this, QStringLiteral("noteBodyMountCoordinator"), QStringLiteral("ctor"));
}

ContentsDisplayNoteBodyMountCoordinator::~ContentsDisplayNoteBodyMountCoordinator()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("dtor"),
        QStringLiteral("selectedNoteId=%1 bodyNoteId=%2 bodyResolved=%3 pendingMount=%4 refreshAttempted=%5")
            .arg(m_selectedNoteId)
            .arg(m_selectedNoteBodyNoteId)
            .arg(m_selectedNoteBodyResolved)
            .arg(m_pendingMountNoteId)
            .arg(m_snapshotRefreshAttemptedNoteId));
}

bool ContentsDisplayNoteBodyMountCoordinator::visible() const noexcept
{
    return m_visible;
}

void ContentsDisplayNoteBodyMountCoordinator::setVisible(const bool visible)
{
    if (m_visible == visible)
    {
        return;
    }

    m_visible = visible;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setVisible"),
        QStringLiteral("visible=%1").arg(visible));
    emit visibleChanged();
    emit mountStateChanged();
}

QString ContentsDisplayNoteBodyMountCoordinator::selectedNoteId() const
{
    return m_selectedNoteId;
}

void ContentsDisplayNoteBodyMountCoordinator::setSelectedNoteId(const QString& noteId)
{
    const QString normalized = normalizeNoteId(noteId);
    if (m_selectedNoteId == normalized)
    {
        return;
    }

    m_selectedNoteId = normalized;
    resetCurrentSelectionMountTracking();
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setSelectedNoteId"),
        QStringLiteral("noteId=%1").arg(normalized));
    emit selectedNoteIdChanged();
    emit mountStateChanged();
}

QString ContentsDisplayNoteBodyMountCoordinator::selectedNoteBodyNoteId() const
{
    return m_selectedNoteBodyNoteId;
}

void ContentsDisplayNoteBodyMountCoordinator::setSelectedNoteBodyNoteId(const QString& noteId)
{
    const QString normalized = normalizeNoteId(noteId);
    if (m_selectedNoteBodyNoteId == normalized)
    {
        return;
    }

    m_selectedNoteBodyNoteId = normalized;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setSelectedNoteBodyNoteId"),
        QStringLiteral("noteId=%1").arg(normalized));
    emit selectedNoteBodyNoteIdChanged();
    emit mountStateChanged();
}

QString ContentsDisplayNoteBodyMountCoordinator::selectedNoteBodyText() const
{
    return m_selectedNoteBodyText;
}

void ContentsDisplayNoteBodyMountCoordinator::setSelectedNoteBodyText(const QString& text)
{
    if (m_selectedNoteBodyText == text)
    {
        return;
    }

    m_selectedNoteBodyText = text;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setSelectedNoteBodyText"),
        WhatSon::Debug::summarizeText(text));
    emit selectedNoteBodyTextChanged();
    emit mountStateChanged();
}

bool ContentsDisplayNoteBodyMountCoordinator::selectedNoteBodyResolved() const noexcept
{
    return m_selectedNoteBodyResolved;
}

void ContentsDisplayNoteBodyMountCoordinator::setSelectedNoteBodyResolved(const bool resolved)
{
    if (m_selectedNoteBodyResolved == resolved)
    {
        return;
    }

    m_selectedNoteBodyResolved = resolved;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setSelectedNoteBodyResolved"),
        QStringLiteral("resolved=%1").arg(resolved));
    emit selectedNoteBodyResolvedChanged();
    emit mountStateChanged();
}

bool ContentsDisplayNoteBodyMountCoordinator::selectedNoteBodyLoading() const noexcept
{
    return m_selectedNoteBodyLoading;
}

void ContentsDisplayNoteBodyMountCoordinator::setSelectedNoteBodyLoading(const bool loading)
{
    if (m_selectedNoteBodyLoading == loading)
    {
        return;
    }

    m_selectedNoteBodyLoading = loading;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setSelectedNoteBodyLoading"),
        QStringLiteral("loading=%1").arg(loading));
    emit selectedNoteBodyLoadingChanged();
    emit mountStateChanged();
}

QString ContentsDisplayNoteBodyMountCoordinator::editorBoundNoteId() const
{
    return m_editorBoundNoteId;
}

void ContentsDisplayNoteBodyMountCoordinator::setEditorBoundNoteId(const QString& noteId)
{
    const QString normalized = normalizeNoteId(noteId);
    if (m_editorBoundNoteId == normalized)
    {
        return;
    }

    m_editorBoundNoteId = normalized;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setEditorBoundNoteId"),
        QStringLiteral("noteId=%1").arg(normalized));
    emit editorBoundNoteIdChanged();
    emit mountStateChanged();
}

bool ContentsDisplayNoteBodyMountCoordinator::editorSessionBoundToSelectedNote() const noexcept
{
    return m_editorSessionBoundToSelectedNote;
}

void ContentsDisplayNoteBodyMountCoordinator::setEditorSessionBoundToSelectedNote(const bool bound)
{
    if (m_editorSessionBoundToSelectedNote == bound)
    {
        return;
    }

    m_editorSessionBoundToSelectedNote = bound;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setEditorSessionBoundToSelectedNote"),
        QStringLiteral("bound=%1").arg(bound));
    emit editorSessionBoundToSelectedNoteChanged();
    emit mountStateChanged();
}

bool ContentsDisplayNoteBodyMountCoordinator::pendingBodySave() const noexcept
{
    return m_pendingBodySave;
}

void ContentsDisplayNoteBodyMountCoordinator::setPendingBodySave(const bool pending)
{
    if (m_pendingBodySave == pending)
    {
        return;
    }

    m_pendingBodySave = pending;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setPendingBodySave"),
        QStringLiteral("pending=%1").arg(pending));
    emit pendingBodySaveChanged();
    emit mountStateChanged();
}

QString ContentsDisplayNoteBodyMountCoordinator::editorText() const
{
    return m_editorText;
}

void ContentsDisplayNoteBodyMountCoordinator::setEditorText(const QString& text)
{
    if (m_editorText == text)
    {
        return;
    }

    m_editorText = text;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setEditorText"),
        WhatSon::Debug::summarizeText(text));
    emit editorTextChanged();
    emit mountStateChanged();
}

bool ContentsDisplayNoteBodyMountCoordinator::structuredDocumentSurfaceRequested() const noexcept
{
    return m_structuredDocumentSurfaceRequested;
}

void ContentsDisplayNoteBodyMountCoordinator::setStructuredDocumentSurfaceRequested(const bool requested)
{
    if (m_structuredDocumentSurfaceRequested == requested)
    {
        return;
    }

    m_structuredDocumentSurfaceRequested = requested;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("noteBodyMountCoordinator"),
        QStringLiteral("setStructuredDocumentSurfaceRequested"),
        QStringLiteral("requested=%1").arg(requested));
    emit structuredDocumentSurfaceRequestedChanged();
    emit mountStateChanged();
}

QString ContentsDisplayNoteBodyMountCoordinator::normalizeNoteId(const QString& noteId)
{
    return normalizedOptionalNoteId(noteId);
}
