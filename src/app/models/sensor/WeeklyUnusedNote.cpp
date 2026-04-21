#include "WeeklyUnusedNote.hpp"

#include "UnusedNoteSensorSupport.hpp"
#include "models/file/hub/WhatSonHubPathUtils.hpp"

#include <QDateTime>

#include <utility>

WeeklyUnusedNote::WeeklyUnusedNote(QObject* parent)
    : QObject(parent)
{
}

WeeklyUnusedNote::~WeeklyUnusedNote() = default;

QString WeeklyUnusedNote::hubPath() const
{
    return m_hubPath;
}

void WeeklyUnusedNote::setHubPath(QString hubPath)
{
    const QString normalizedHubPath = WhatSon::HubPath::normalizeAbsolutePath(hubPath);
    if (m_hubPath == normalizedHubPath)
    {
        return;
    }

    m_hubPath = std::move(normalizedHubPath);
    emit hubPathChanged();
    refresh();
}

QVariantList WeeklyUnusedNote::unusedNotes() const
{
    return m_unusedNotes;
}

QStringList WeeklyUnusedNote::unusedNoteIds() const
{
    return WhatSon::UnusedNoteSensorSupport::noteIdsFromEntries(m_unusedNotes);
}

int WeeklyUnusedNote::unusedNoteCount() const noexcept
{
    return m_unusedNotes.size();
}

QString WeeklyUnusedNote::lastError() const
{
    return m_lastError;
}

QVariantList WeeklyUnusedNote::scanUnusedNotes(const QString& hubPath)
{
    const QString trimmedHubPath = hubPath.trimmed();
    if (!trimmedHubPath.isEmpty())
    {
        const QString normalizedHubPath = WhatSon::HubPath::normalizeAbsolutePath(trimmedHubPath);
        if (m_hubPath != normalizedHubPath)
        {
            m_hubPath = normalizedHubPath;
            emit hubPathChanged();
        }
    }

    refresh();
    return m_unusedNotes;
}

QStringList WeeklyUnusedNote::collectUnusedNoteIds(const QString& hubPath)
{
    return scanUnusedNotes(hubPath).isEmpty() ? QStringList{} : unusedNoteIds();
}

void WeeklyUnusedNote::refresh()
{
    if (m_hubPath.trimmed().isEmpty())
    {
        setLastError(QString());
        setUnusedNotes({});
        emit scanCompleted(m_unusedNotes);
        return;
    }

    const QDateTime referenceUtc = QDateTime::currentDateTimeUtc();
    QString scanError;
    const QVariantList nextUnusedNotes = WhatSon::UnusedNoteSensorSupport::collectUnusedNoteEntries(
        m_hubPath,
        referenceUtc,
        referenceUtc.addDays(-7),
        &scanError);
    setLastError(scanError);
    setUnusedNotes(nextUnusedNotes);
    emit scanCompleted(m_unusedNotes);
}

void WeeklyUnusedNote::setLastError(QString errorMessage)
{
    errorMessage = errorMessage.trimmed();
    if (m_lastError == errorMessage)
    {
        return;
    }

    m_lastError = std::move(errorMessage);
    emit lastErrorChanged();
}

void WeeklyUnusedNote::setUnusedNotes(QVariantList unusedNotes)
{
    if (m_unusedNotes == unusedNotes)
    {
        return;
    }

    m_unusedNotes = std::move(unusedNotes);
    emit unusedNotesChanged();
}
