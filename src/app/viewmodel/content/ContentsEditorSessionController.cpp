#include "ContentsEditorSessionController.hpp"

#include "agenda/ContentsAgendaBackend.hpp"
#include "file/WhatSonDebugTrace.hpp"
#include "viewmodel/content/ContentsEditorSelectionBridge.hpp"

#include <QDateTime>
#include <QRegularExpression>
#include <QTimer>

#include <algorithm>
#include <cmath>

namespace
{
    const QRegularExpression kEmptyTaskPattern(
        QStringLiteral(R"(<task\b([^>]*)>\s*</task>)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression kEmptyCalloutPattern(
        QStringLiteral(R"(<callout\b([^>]*)>\s*</callout>)"),
        QRegularExpression::CaseInsensitiveOption);
} // namespace

ContentsEditorSessionController::ContentsEditorSessionController(QObject* parent)
    : QObject(parent)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorSession"),
        QStringLiteral("ctor"));
}

ContentsEditorSessionController::~ContentsEditorSessionController()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorSession"),
        QStringLiteral("dtor"),
        QStringLiteral("noteId=%1 pendingBodySave=%2")
            .arg(m_editorBoundNoteId)
            .arg(m_pendingBodySave));
}

QString ContentsEditorSessionController::editorBoundNoteId() const
{
    return m_editorBoundNoteId;
}

void ContentsEditorSessionController::setEditorBoundNoteId(const QString& noteId)
{
    if (m_editorBoundNoteId == noteId)
    {
        return;
    }

    m_editorBoundNoteId = noteId;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorSession"),
        QStringLiteral("editorBoundNoteIdChanged"),
        QStringLiteral("noteId=%1").arg(m_editorBoundNoteId));
    emit editorBoundNoteIdChanged();
}

QString ContentsEditorSessionController::editorText() const
{
    return m_editorText;
}

void ContentsEditorSessionController::setEditorText(const QString& text)
{
    if (m_editorText == text)
    {
        return;
    }

    m_editorText = text;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorSession"),
        QStringLiteral("editorTextChanged"),
        WhatSon::Debug::summarizeText(m_editorText));
    emit editorTextChanged();
}

bool ContentsEditorSessionController::localEditorAuthority() const noexcept
{
    return m_localEditorAuthority;
}

void ContentsEditorSessionController::setLocalEditorAuthority(const bool localEditorAuthority)
{
    if (m_localEditorAuthority == localEditorAuthority)
    {
        return;
    }

    m_localEditorAuthority = localEditorAuthority;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorSession"),
        QStringLiteral("localEditorAuthorityChanged"),
        QStringLiteral("localEditorAuthority=%1 lastEditMs=%2")
            .arg(m_localEditorAuthority)
            .arg(m_lastLocalEditTimestampMs));
    emit localEditorAuthorityChanged();
}

double ContentsEditorSessionController::lastLocalEditTimestampMs() const noexcept
{
    return m_lastLocalEditTimestampMs;
}

void ContentsEditorSessionController::setLastLocalEditTimestampMs(const double timestampMs)
{
    if (m_lastLocalEditTimestampMs == timestampMs)
    {
        return;
    }

    m_lastLocalEditTimestampMs = timestampMs;
    emit lastLocalEditTimestampMsChanged();
}

bool ContentsEditorSessionController::pendingBodySave() const noexcept
{
    return m_pendingBodySave;
}

void ContentsEditorSessionController::setPendingBodySave(const bool pendingBodySave)
{
    if (m_pendingBodySave == pendingBodySave)
    {
        return;
    }

    m_pendingBodySave = pendingBodySave;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorSession"),
        QStringLiteral("pendingBodySaveChanged"),
        QStringLiteral("pendingBodySave=%1").arg(m_pendingBodySave));
    emit pendingBodySaveChanged();
}

int ContentsEditorSessionController::typingIdleThresholdMs() const noexcept
{
    return m_typingIdleThresholdMs;
}

void ContentsEditorSessionController::setTypingIdleThresholdMs(const int thresholdMs)
{
    const int normalizedThresholdMs = std::max(0, thresholdMs);
    if (m_typingIdleThresholdMs == normalizedThresholdMs)
    {
        return;
    }

    m_typingIdleThresholdMs = normalizedThresholdMs;
    emit typingIdleThresholdMsChanged();
}

QObject* ContentsEditorSessionController::selectionBridge() const noexcept
{
    return m_selectionBridge;
}

void ContentsEditorSessionController::setSelectionBridge(QObject* selectionBridgeObject)
{
    auto* const nextSelectionBridge =
        qobject_cast<ContentsEditorSelectionBridge*>(selectionBridgeObject);
    if (m_selectionBridge == nextSelectionBridge)
    {
        return;
    }

    if (m_selectionBridge != nullptr)
    {
        disconnect(
            m_selectionBridge,
            &ContentsEditorSelectionBridge::editorTextPersistenceFinished,
            this,
            &ContentsEditorSessionController::handleEditorTextPersistenceFinished);
    }

    m_selectionBridge = nextSelectionBridge;
    if (m_selectionBridge != nullptr)
    {
        connect(
            m_selectionBridge,
            &ContentsEditorSelectionBridge::editorTextPersistenceFinished,
            this,
            &ContentsEditorSessionController::handleEditorTextPersistenceFinished);
    }

    emit selectionBridgeChanged();
}

QObject* ContentsEditorSessionController::agendaBackend() const noexcept
{
    return m_agendaBackend;
}

void ContentsEditorSessionController::setAgendaBackend(QObject* agendaBackendObject)
{
    auto* const nextAgendaBackend = qobject_cast<ContentsAgendaBackend*>(agendaBackendObject);
    if (m_agendaBackend == nextAgendaBackend)
    {
        return;
    }

    m_agendaBackend = nextAgendaBackend;
    emit agendaBackendChanged();
}

bool ContentsEditorSessionController::syncingEditorTextFromModel() const noexcept
{
    return m_syncingEditorTextFromModel;
}

void ContentsEditorSessionController::setSyncingEditorTextFromModel(const bool syncing)
{
    if (m_syncingEditorTextFromModel == syncing)
    {
        return;
    }

    m_syncingEditorTextFromModel = syncing;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorSession"),
        QStringLiteral("syncingEditorTextFromModelChanged"),
        QStringLiteral("syncingEditorTextFromModel=%1").arg(m_syncingEditorTextFromModel));
    emit syncingEditorTextFromModelChanged();
}

bool ContentsEditorSessionController::flushPendingEditorText()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorSession"),
        QStringLiteral("flushPendingEditorText"),
        QStringLiteral("pendingBodySave=%1 noteId=%2").arg(m_pendingBodySave).arg(m_editorBoundNoteId));
    if (!m_pendingBodySave)
    {
        return true;
    }

    const QString noteId = normalizedNoteId(m_editorBoundNoteId);
    if (noteId.isEmpty())
    {
        setPendingBodySave(false);
        return false;
    }

    return queueCurrentEditorTextForPersistence(true, m_editorText);
}

bool ContentsEditorSessionController::isTypingSessionActive() const
{
    if (!m_localEditorAuthority)
    {
        return false;
    }

    const int thresholdMs = std::max(0, m_typingIdleThresholdMs);
    if (thresholdMs <= 0)
    {
        return true;
    }

    const double elapsedMs = currentTimestampMs() - std::max(0.0, m_lastLocalEditTimestampMs);
    if (!std::isfinite(elapsedMs) || elapsedMs < 0)
    {
        return true;
    }

    return elapsedMs < static_cast<double>(thresholdMs);
}

bool ContentsEditorSessionController::requestSyncEditorTextFromSelection(
    const QString& noteId,
    const QString& text,
    const QString& bodyNoteId)
{
    const QString nextNoteId = normalizedNoteId(noteId);
    const QString nextBodyNoteId = normalizedNoteId(bodyNoteId);
    const QString nextText = normalizedEditorText(text);
    const QString currentNoteId = normalizedNoteId(m_editorBoundNoteId);
    const QString currentText = m_editorText;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorSession"),
        QStringLiteral("requestSyncEditorTextFromSelection"),
        QStringLiteral("nextNoteId=%1 bodyNoteId=%2 currentNoteId=%3 %4")
            .arg(nextNoteId)
            .arg(nextBodyNoteId)
            .arg(currentNoteId)
            .arg(WhatSon::Debug::summarizeText(nextText)));

    if (nextNoteId.isEmpty() || nextBodyNoteId != nextNoteId)
    {
        return false;
    }

    if (currentNoteId != nextNoteId && m_pendingBodySave)
    {
        if (!flushPendingEditorText())
        {
            return false;
        }
    }

    if (currentNoteId == nextNoteId && currentText == nextText)
    {
        return false;
    }

    if (currentNoteId == nextNoteId && !shouldAcceptModelBodyText(nextNoteId, nextText))
    {
        WhatSon::Debug::traceEditorSelf(
            this,
            QStringLiteral("editorSession"),
            QStringLiteral("requestSyncEditorTextFromSelection.rejectedProtectedSameNote"),
            QStringLiteral("noteId=%1 pendingBodySave=%2 typingActive=%3 current=%4 incoming=%5")
                .arg(nextNoteId)
                .arg(m_pendingBodySave)
                .arg(isTypingSessionActive())
                .arg(WhatSon::Debug::summarizeText(currentText))
                .arg(WhatSon::Debug::summarizeText(nextText)));
        return false;
    }

    syncEditorTextFromSelection(nextNoteId, nextText);
    return true;
}

void ContentsEditorSessionController::markLocalEditorAuthority()
{
    setLocalEditorAuthority(true);
    setLastLocalEditTimestampMs(currentTimestampMs());
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorSession"),
        QStringLiteral("markLocalEditorAuthority"),
        QStringLiteral("timestampMs=%1").arg(m_lastLocalEditTimestampMs));
}

bool ContentsEditorSessionController::scheduleEditorPersistence()
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorSession"),
        QStringLiteral("scheduleEditorPersistence"),
        QStringLiteral("noteId=%1 pendingBodySave(before)=%2 %3")
            .arg(normalizedNoteId(m_editorBoundNoteId))
            .arg(m_pendingBodySave)
            .arg(WhatSon::Debug::summarizeText(m_editorText)));
    return queueCurrentEditorTextForPersistence(false, m_editorText);
}

bool ContentsEditorSessionController::persistEditorTextImmediately()
{
    return persistEditorTextImmediatelyWithText(m_editorText);
}

bool ContentsEditorSessionController::persistEditorTextImmediatelyWithText(const QString& text)
{
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorSession"),
        QStringLiteral("persistEditorTextImmediately"),
        QStringLiteral("noteId=%1 %2")
            .arg(normalizedNoteId(m_editorBoundNoteId))
            .arg(WhatSon::Debug::summarizeText(text)));
    return queueCurrentEditorTextForPersistence(true, text);
}

void ContentsEditorSessionController::handleEditorTextPersistenceFinished(
    const QString& noteId,
    const QString& text,
    const bool success,
    const QString& errorMessage)
{
    Q_UNUSED(errorMessage)

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorSession"),
        QStringLiteral("handleEditorPersistenceFinished"),
        QStringLiteral("success=%1 noteId=%2 %3")
            .arg(success)
            .arg(noteId)
            .arg(WhatSon::Debug::summarizeText(text)));
    if (!success)
    {
        return;
    }

    if (m_editorBoundNoteId == noteId && m_editorText == text)
    {
        setPendingBodySave(false);
    }
}

QString ContentsEditorSessionController::normalizedNoteId(const QString& noteId)
{
    return noteId.trimmed();
}

bool ContentsEditorSessionController::enqueueEditorPersistence(
    const QString& noteId,
    const QString& bodyText,
    const bool immediateFlush)
{
    if (noteId.isEmpty() || m_selectionBridge == nullptr)
    {
        return false;
    }

    if (immediateFlush)
    {
        return m_selectionBridge->flushEditorTextForNote(noteId, bodyText);
    }
    return m_selectionBridge->stageEditorTextForIdleSync(noteId, bodyText);
}

QString ContentsEditorSessionController::normalizeAgendaPlaceholderDates(const QString& text) const
{
    if (text.isEmpty() || m_agendaBackend == nullptr)
    {
        return text;
    }

    return m_agendaBackend->normalizeAgendaModifiedDate(text);
}

QString ContentsEditorSessionController::normalizeStructuredEmptyBlockAnchors(const QString& text) const
{
    if (text.isEmpty())
    {
        return text;
    }

    QString normalizedText = text;
    normalizedText.replace(kEmptyTaskPattern, QStringLiteral("<task\\1> </task>"));
    normalizedText.replace(kEmptyCalloutPattern, QStringLiteral("<callout\\1> </callout>"));
    return normalizedText;
}

QString ContentsEditorSessionController::normalizedEditorText(const QString& text) const
{
    return normalizeAgendaPlaceholderDates(normalizeStructuredEmptyBlockAnchors(text));
}

QString ContentsEditorSessionController::normalizeModifiedEditorText(const QString& text)
{
    const QString normalizedText = normalizedEditorText(text);
    if (m_editorText != normalizedText)
    {
        setEditorText(normalizedText);
    }
    return normalizedText;
}

bool ContentsEditorSessionController::shouldAcceptModelBodyText(
    const QString& noteId,
    const QString& text) const
{
    const QString nextNoteId = normalizedNoteId(noteId);
    const QString currentNoteId = normalizedNoteId(m_editorBoundNoteId);
    if (nextNoteId != currentNoteId)
    {
        return true;
    }

    if (m_editorText == text)
    {
        return true;
    }

    if (isTypingSessionActive())
    {
        return false;
    }

    return !m_pendingBodySave;
}

bool ContentsEditorSessionController::queueCurrentEditorTextForPersistence(
    const bool immediateFlush,
    const QString& rawBodyText)
{
    const QString noteId = normalizedNoteId(m_editorBoundNoteId);
    if (noteId.isEmpty())
    {
        setPendingBodySave(false);
        return false;
    }

    setPendingBodySave(true);
    const QString bodyText = normalizeModifiedEditorText(rawBodyText);
    return enqueueEditorPersistence(noteId, bodyText, immediateFlush);
}

void ContentsEditorSessionController::releaseSyncGuard()
{
    QTimer::singleShot(0, this, [this]()
    {
        setSyncingEditorTextFromModel(false);
    });
}

void ContentsEditorSessionController::syncEditorTextFromSelection(
    const QString& noteId,
    const QString& text)
{
    const bool noteChanged = m_editorBoundNoteId != noteId;
    const bool textChanged = m_editorText != text;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorSession"),
        QStringLiteral("syncEditorTextFromSelection"),
        QStringLiteral("noteChanged=%1 textChanged=%2 nextNoteId=%3 %4")
            .arg(noteChanged)
            .arg(textChanged)
            .arg(noteId)
            .arg(WhatSon::Debug::summarizeText(text)));
    if (!noteChanged && !textChanged)
    {
        return;
    }

    if (noteChanged || textChanged)
    {
        setPendingBodySave(false);
    }

    setEditorBoundNoteId(noteId);
    if (noteChanged)
    {
        setLocalEditorAuthority(false);
        setLastLocalEditTimestampMs(0);
    }

    if (textChanged)
    {
        setSyncingEditorTextFromModel(true);
        setEditorText(text);
        releaseSyncGuard();
    }

    emit editorTextSynchronized();
}

double ContentsEditorSessionController::currentTimestampMs() const noexcept
{
    return static_cast<double>(QDateTime::currentMSecsSinceEpoch());
}
