#include "app/models/file/sync/WhatSonEditorRawPullController.hpp"

#include <QDir>

#include <algorithm>
#include <utility>

namespace
{
    constexpr auto kNoteEntryReason = "note-entry";
    constexpr auto kNoteOpenReason = "note-open";
    constexpr auto kIdleReason = "idle";
}

WhatSonEditorRawPullController::WhatSonEditorRawPullController(QObject* parent)
    : QObject(parent)
{
    m_idlePullTimer.setSingleShot(true);
    m_idlePullTimer.setInterval(m_idlePullIntervalMs);
    connect(
        &m_idlePullTimer,
        &QTimer::timeout,
        this,
        &WhatSonEditorRawPullController::requestActiveIdlePull);
}

int WhatSonEditorRawPullController::idlePullIntervalMs() const noexcept
{
    return m_idlePullIntervalMs;
}

void WhatSonEditorRawPullController::setIdlePullIntervalMs(const int idlePullIntervalMs)
{
    const int normalizedInterval = std::max(0, idlePullIntervalMs);
    if (m_idlePullIntervalMs == normalizedInterval)
    {
        return;
    }

    m_idlePullIntervalMs = normalizedInterval;
    m_idlePullTimer.setInterval(m_idlePullIntervalMs);
    if (!m_activeIdlePullNoteId.isEmpty()
        && !m_activeIdlePullNoteDirectoryPath.isEmpty()
        && m_idlePullTimer.isActive())
    {
        m_idlePullTimer.start(m_idlePullIntervalMs);
    }
    emit idlePullIntervalMsChanged();
}

void WhatSonEditorRawPullController::setRawPullCallback(RawPullCallback callback)
{
    m_rawPullCallback = std::move(callback);
}

quint64 WhatSonEditorRawPullController::requestNoteEntryPull(
    const QString& noteId,
    const QString& noteDirectoryPath)
{
    return executePull(
        normalizedNoteId(noteId),
        normalizedPath(noteDirectoryPath),
        QString::fromLatin1(kNoteEntryReason));
}

quint64 WhatSonEditorRawPullController::requestNoteOpenPull(
    const QString& noteId,
    const QString& noteDirectoryPath)
{
    return executePull(
        normalizedNoteId(noteId),
        normalizedPath(noteDirectoryPath),
        QString::fromLatin1(kNoteOpenReason));
}

void WhatSonEditorRawPullController::setActiveNoteForIdlePull(
    const QString& noteId,
    const QString& noteDirectoryPath)
{
    const QString normalizedId = normalizedNoteId(noteId);
    const QString normalizedDirectoryPath = normalizedPath(noteDirectoryPath);
    if (normalizedId.isEmpty() || normalizedDirectoryPath.isEmpty())
    {
        clearActiveNoteForIdlePull();
        return;
    }

    const bool sameActiveNote =
        m_activeIdlePullNoteId == normalizedId
        && m_activeIdlePullNoteDirectoryPath == normalizedDirectoryPath;
    m_activeIdlePullNoteId = normalizedId;
    m_activeIdlePullNoteDirectoryPath = normalizedDirectoryPath;
    if (!sameActiveNote || !m_idlePullTimer.isActive())
    {
        scheduleIdlePull();
    }
}

void WhatSonEditorRawPullController::clearActiveNoteForIdlePull()
{
    m_idlePullTimer.stop();
    m_activeIdlePullNoteId.clear();
    m_activeIdlePullNoteDirectoryPath.clear();
}

void WhatSonEditorRawPullController::recordUserActivity()
{
    scheduleIdlePull();
}

quint64 WhatSonEditorRawPullController::requestActiveIdlePull()
{
    const QString noteId = m_activeIdlePullNoteId;
    const QString noteDirectoryPath = m_activeIdlePullNoteDirectoryPath;
    if (noteId.isEmpty() || noteDirectoryPath.isEmpty())
    {
        return 0;
    }

    const quint64 sequence = executePull(
        noteId,
        noteDirectoryPath,
        QString::fromLatin1(kIdleReason));
    if (m_activeIdlePullNoteId == noteId
        && m_activeIdlePullNoteDirectoryPath == noteDirectoryPath)
    {
        scheduleIdlePull();
    }
    return sequence;
}

QString WhatSonEditorRawPullController::normalizedNoteId(const QString& noteId)
{
    return noteId.trimmed();
}

QString WhatSonEditorRawPullController::normalizedPath(const QString& path)
{
    const QString trimmedPath = path.trimmed();
    if (trimmedPath.isEmpty())
    {
        return {};
    }
    const QString cleanPath = QDir::cleanPath(trimmedPath);
    return cleanPath == QStringLiteral(".") ? QString() : cleanPath;
}

quint64 WhatSonEditorRawPullController::executePull(
    const QString& noteId,
    const QString& noteDirectoryPath,
    const QString& reason)
{
    if (noteId.isEmpty() || noteDirectoryPath.isEmpty())
    {
        return 0;
    }

    emit rawPullRequested(noteId, noteDirectoryPath, reason);

    QString errorMessage;
    const quint64 sequence = m_rawPullCallback
        ? m_rawPullCallback(noteId, noteDirectoryPath, reason, &errorMessage)
        : 0;
    const bool success = sequence != 0;
    if (!success && errorMessage.trimmed().isEmpty())
    {
        errorMessage = QStringLiteral("Editor RAW pull callback is not available.");
    }

    emit rawPullFinished(
        noteId,
        noteDirectoryPath,
        reason,
        sequence,
        success,
        errorMessage);
    return sequence;
}

void WhatSonEditorRawPullController::scheduleIdlePull()
{
    if (m_activeIdlePullNoteId.isEmpty()
        || m_activeIdlePullNoteDirectoryPath.isEmpty())
    {
        return;
    }

    m_idlePullTimer.start(m_idlePullIntervalMs);
}
