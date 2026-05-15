#include "app/models/file/sync/WhatSonEditorRawPullController.hpp"

#include <QDir>

#include <utility>

namespace
{
    constexpr auto kNoteEntryReason = "note-entry";
    constexpr auto kNoteOpenReason = "note-open";
}

WhatSonEditorRawPullController::WhatSonEditorRawPullController(QObject* parent)
    : QObject(parent)
{
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
