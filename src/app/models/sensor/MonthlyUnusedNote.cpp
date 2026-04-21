#include "MonthlyUnusedNote.hpp"

#include "UnusedNoteSensorSupport.hpp"
#include "models/file/hub/WhatSonHubPathUtils.hpp"

#include <QDateTime>

#include <utility>

MonthlyUnusedNote::MonthlyUnusedNote(QObject* parent)
    : QObject(parent)
{
}

MonthlyUnusedNote::~MonthlyUnusedNote() = default;

QString MonthlyUnusedNote::hubPath() const
{
    return m_hubPath;
}

void MonthlyUnusedNote::setHubPath(QString hubPath)
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

QVariantList MonthlyUnusedNote::unusedNotes() const
{
    return m_unusedNotes;
}

QStringList MonthlyUnusedNote::unusedNoteIds() const
{
    return WhatSon::UnusedNoteSensorSupport::noteIdsFromEntries(m_unusedNotes);
}

int MonthlyUnusedNote::unusedNoteCount() const noexcept
{
    return m_unusedNotes.size();
}

QString MonthlyUnusedNote::lastError() const
{
    return m_lastError;
}

QVariantList MonthlyUnusedNote::scanUnusedNotes(const QString& hubPath)
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

QStringList MonthlyUnusedNote::collectUnusedNoteIds(const QString& hubPath)
{
    return scanUnusedNotes(hubPath).isEmpty() ? QStringList{} : unusedNoteIds();
}

void MonthlyUnusedNote::refresh()
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
        referenceUtc.addMonths(-1),
        &scanError);
    setLastError(scanError);
    setUnusedNotes(nextUnusedNotes);
    emit scanCompleted(m_unusedNotes);
}

void MonthlyUnusedNote::setLastError(QString errorMessage)
{
    errorMessage = errorMessage.trimmed();
    if (m_lastError == errorMessage)
    {
        return;
    }

    m_lastError = std::move(errorMessage);
    emit lastErrorChanged();
}

void MonthlyUnusedNote::setUnusedNotes(QVariantList unusedNotes)
{
    if (m_unusedNotes == unusedNotes)
    {
        return;
    }

    m_unusedNotes = std::move(unusedNotes);
    emit unusedNotesChanged();
}
