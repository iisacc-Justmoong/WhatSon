#include "WhatSonLibraryIndexedState.hpp"

WhatSonLibraryIndexedState::WhatSonLibraryIndexedState() = default;

WhatSonLibraryIndexedState::~WhatSonLibraryIndexedState() = default;

bool WhatSonLibraryIndexedState::indexFromWshub(const QString& wshubPath, QString* errorMessage)
{
    QString indexError;
    if (!m_libraryAll.indexFromWshub(wshubPath, &indexError))
    {
        m_libraryDraft.clear();
        m_libraryToday.clear();
        if (errorMessage != nullptr)
        {
            *errorMessage = indexError;
        }
        return false;
    }

    rebuildDerivedBuckets();
    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }
    return true;
}

void WhatSonLibraryIndexedState::applySnapshot(
    QString sourceWshubPath,
    QVector<LibraryNoteRecord> allNotes,
    QVector<LibraryNoteRecord> draftNotes,
    QVector<LibraryNoteRecord> todayNotes)
{
    m_libraryAll.setIndexedNotes(std::move(sourceWshubPath), std::move(allNotes));
    m_libraryDraft.setNotes(std::move(draftNotes));
    m_libraryToday.setNotes(std::move(todayNotes));
}

void WhatSonLibraryIndexedState::setIndexedNotes(QString sourceWshubPath, QVector<LibraryNoteRecord> notes)
{
    m_libraryAll.setIndexedNotes(std::move(sourceWshubPath), std::move(notes));
    rebuildDerivedBuckets();
}

void WhatSonLibraryIndexedState::setSourceWshubPath(QString sourceWshubPath)
{
    m_libraryAll.setSourceWshubPath(std::move(sourceWshubPath));
}

bool WhatSonLibraryIndexedState::upsertNote(const LibraryNoteRecord& note)
{
    const QString normalizedNoteId = note.noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        return false;
    }

    const bool allChanged = m_libraryAll.upsertNote(note);
    const bool draftChanged = m_libraryDraft.upsertNote(note);
    const bool todayChanged = m_libraryToday.upsertNote(note);
    return allChanged || draftChanged || todayChanged;
}

bool WhatSonLibraryIndexedState::removeNoteById(const QString& noteId)
{
    const bool allChanged = m_libraryAll.removeNoteById(noteId);
    const bool draftChanged = m_libraryDraft.removeNoteById(noteId);
    const bool todayChanged = m_libraryToday.removeNoteById(noteId);
    return allChanged || draftChanged || todayChanged;
}

bool WhatSonLibraryIndexedState::noteById(const QString& noteId, LibraryNoteRecord* outNote) const
{
    return m_libraryAll.noteById(noteId, outNote);
}

void WhatSonLibraryIndexedState::clear()
{
    m_libraryAll.clear();
    m_libraryDraft.clear();
    m_libraryToday.clear();
}

WhatSonLibraryIndexedState::Snapshot WhatSonLibraryIndexedState::snapshot() const
{
    Snapshot snapshot;
    snapshot.sourceWshubPath = sourceWshubPath();
    snapshot.allNotes = allNotes();
    snapshot.draftNotes = draftNotes();
    snapshot.todayNotes = todayNotes();
    return snapshot;
}

QString WhatSonLibraryIndexedState::sourceWshubPath() const
{
    return m_libraryAll.sourceWshubPath();
}

const QVector<LibraryNoteRecord>& WhatSonLibraryIndexedState::allNotes() const noexcept
{
    return m_libraryAll.notes();
}

const QVector<LibraryNoteRecord>& WhatSonLibraryIndexedState::draftNotes() const noexcept
{
    return m_libraryDraft.notes();
}

const QVector<LibraryNoteRecord>& WhatSonLibraryIndexedState::todayNotes() const noexcept
{
    return m_libraryToday.notes();
}

QVector<LibraryNoteRecord> WhatSonLibraryIndexedState::collectBookmarkedNotes(
    const QVector<LibraryNoteRecord>& allNotes)
{
    QVector<LibraryNoteRecord> bookmarkedNotes;
    bookmarkedNotes.reserve(allNotes.size());

    for (const LibraryNoteRecord& note : allNotes)
    {
        if (!note.bookmarked)
        {
            continue;
        }

        bookmarkedNotes.push_back(note);
    }

    return bookmarkedNotes;
}

void WhatSonLibraryIndexedState::rebuildDerivedBuckets()
{
    m_libraryDraft.rebuild(m_libraryAll.notes());
    m_libraryToday.rebuild(m_libraryAll.notes());
}
