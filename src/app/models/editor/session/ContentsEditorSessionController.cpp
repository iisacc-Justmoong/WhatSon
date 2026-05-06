#include "app/models/editor/session/ContentsEditorSessionController.hpp"

#include "app/models/editor/tags/ContentsAgendaBackend.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"

#include <QDateTime>
#include <QDir>
#include <QRegularExpression>
#include <QTimer>

#include <algorithm>
#include <cmath>

namespace
{
    QString describeTraceObject(const QObject* object)
    {
        if (object == nullptr)
        {
            return QStringLiteral("ptr=<null>");
        }

        const QString metaClass = object->metaObject() == nullptr
            ? QStringLiteral("unknown")
            : QString::fromUtf8(object->metaObject()->className());
        const QString objectName = object->objectName().isEmpty()
            ? QStringLiteral("<empty>")
            : object->objectName();
        return QStringLiteral("ptr=0x%1 class=%2 objectName=%3")
            .arg(QString::number(reinterpret_cast<quintptr>(object), 16))
            .arg(metaClass)
            .arg(objectName);
    }

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
        QStringLiteral("noteId=%1 noteDirectoryPath=%2 pendingBodySave=%3")
            .arg(m_editorBoundNoteId)
            .arg(m_editorBoundNoteDirectoryPath)
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

QString ContentsEditorSessionController::editorBoundNoteDirectoryPath() const
{
    return m_editorBoundNoteDirectoryPath;
}

void ContentsEditorSessionController::setEditorBoundNoteDirectoryPath(const QString& noteDirectoryPath)
{
    const QString normalizedPath = normalizedNoteDirectoryPath(noteDirectoryPath);
    if (m_editorBoundNoteDirectoryPath == normalizedPath)
    {
        return;
    }

    m_editorBoundNoteDirectoryPath = normalizedPath;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorSession"),
        QStringLiteral("editorBoundNoteDirectoryPathChanged"),
        QStringLiteral("noteDirectoryPath=%1").arg(m_editorBoundNoteDirectoryPath));
    emit editorBoundNoteDirectoryPathChanged();
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
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorSession"),
        QStringLiteral("setAgendaBackend"),
        QStringLiteral("agendaBackend={%1}").arg(describeTraceObject(m_agendaBackend)));
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
    const QString& bodyNoteId,
    const QString& noteDirectoryPath)
{
    const QString nextNoteId = normalizedNoteId(noteId);
    const QString nextBodyNoteId = normalizedNoteId(bodyNoteId);
    const QString nextNoteDirectoryPath = normalizedNoteDirectoryPath(noteDirectoryPath);
    const QString nextText = normalizedEditorText(text);
    const QString currentNoteId = normalizedNoteId(m_editorBoundNoteId);
    const QString currentNoteDirectoryPath = normalizedNoteDirectoryPath(m_editorBoundNoteDirectoryPath);
    const QString currentText = m_editorText;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorSession"),
        QStringLiteral("requestSyncEditorTextFromSelection"),
        QStringLiteral("nextNoteId=%1 bodyNoteId=%2 nextNoteDirectoryPath=%3 currentNoteId=%4 currentNoteDirectoryPath=%5 %6")
            .arg(nextNoteId)
            .arg(nextBodyNoteId)
            .arg(nextNoteDirectoryPath)
            .arg(currentNoteId)
            .arg(currentNoteDirectoryPath)
            .arg(WhatSon::Debug::summarizeText(nextText)));

    if (nextNoteId.isEmpty() || nextBodyNoteId != nextNoteId)
    {
        return false;
    }

    if (currentNoteId == nextNoteId
        && currentNoteDirectoryPath == nextNoteDirectoryPath
        && currentText == nextText)
    {
        return false;
    }

    if (currentNoteId == nextNoteId
        && currentNoteDirectoryPath == nextNoteDirectoryPath
        && !shouldAcceptModelBodyText(nextNoteId, nextNoteDirectoryPath, nextText))
    {
        WhatSon::Debug::traceEditorSelf(
            this,
            QStringLiteral("editorSession"),
            QStringLiteral("requestSyncEditorTextFromSelection.rejectedProtectedSameNote"),
            QStringLiteral("noteId=%1 noteDirectoryPath=%2 pendingBodySave=%3 typingActive=%4 current=%5 incoming=%6")
                .arg(nextNoteId)
                .arg(nextNoteDirectoryPath)
                .arg(m_pendingBodySave)
                .arg(isTypingSessionActive())
                .arg(WhatSon::Debug::summarizeText(currentText))
                .arg(WhatSon::Debug::summarizeText(nextText)));
        return false;
    }

    syncEditorTextFromSelection(nextNoteId, nextNoteDirectoryPath, nextText);
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

bool ContentsEditorSessionController::commitRawEditorTextMutation(const QString& text)
{
    const QString normalizedText = normalizedEditorText(text);
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorSession"),
        QStringLiteral("commitRawEditorTextMutation"),
        QStringLiteral("noteId=%1 %2")
            .arg(normalizedNoteId(m_editorBoundNoteId))
            .arg(WhatSon::Debug::summarizeText(normalizedText)));
    if (m_editorText == normalizedText)
    {
        return false;
    }

    setEditorText(normalizedText);
    markLocalEditorAuthority();
    setPendingBodySave(true);
    return true;
}

QString ContentsEditorSessionController::normalizedNoteId(const QString& noteId)
{
    return noteId.trimmed();
}

QString ContentsEditorSessionController::normalizedNoteDirectoryPath(const QString& noteDirectoryPath)
{
    const QString normalizedPath = QDir::cleanPath(noteDirectoryPath.trimmed());
    return normalizedPath == QStringLiteral(".") ? QString() : normalizedPath;
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

bool ContentsEditorSessionController::shouldAcceptModelBodyText(
    const QString& noteId,
    const QString& noteDirectoryPath,
    const QString& text) const
{
    const QString nextNoteId = normalizedNoteId(noteId);
    const QString nextNoteDirectoryPath = normalizedNoteDirectoryPath(noteDirectoryPath);
    const QString currentNoteId = normalizedNoteId(m_editorBoundNoteId);
    const QString currentNoteDirectoryPath = normalizedNoteDirectoryPath(m_editorBoundNoteDirectoryPath);
    if (nextNoteId != currentNoteId
        || nextNoteDirectoryPath != currentNoteDirectoryPath)
    {
        return true;
    }

    if (m_editorText == text)
    {
        return true;
    }

    if (m_localEditorAuthority)
    {
        return false;
    }

    if (isTypingSessionActive())
    {
        return false;
    }

    return !m_pendingBodySave;
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
    const QString& noteDirectoryPath,
    const QString& text)
{
    const bool noteIdentityChanged =
        m_editorBoundNoteId != noteId
        || normalizedNoteDirectoryPath(m_editorBoundNoteDirectoryPath) != normalizedNoteDirectoryPath(noteDirectoryPath);
    const bool textChanged = m_editorText != text;
    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorSession"),
        QStringLiteral("syncEditorTextFromSelection"),
        QStringLiteral("noteIdentityChanged=%1 textChanged=%2 nextNoteId=%3 nextNoteDirectoryPath=%4 %5")
            .arg(noteIdentityChanged)
            .arg(textChanged)
            .arg(noteId)
            .arg(normalizedNoteDirectoryPath(noteDirectoryPath))
            .arg(WhatSon::Debug::summarizeText(text)));
    if (!noteIdentityChanged && !textChanged)
    {
        return;
    }

    if (noteIdentityChanged || textChanged)
    {
        setPendingBodySave(false);
    }

    setEditorBoundNoteId(noteId);
    setEditorBoundNoteDirectoryPath(noteDirectoryPath);
    if (noteIdentityChanged)
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
