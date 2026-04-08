#pragma once

#include "LibraryNoteRecord.hpp"

#include <QVector>

class LibraryDraft final
{
public:
    LibraryDraft();
    ~LibraryDraft();

    bool rebuild(const QVector<LibraryNoteRecord>& allNotes);
    static bool matches(const LibraryNoteRecord& note);
    bool upsertNote(const LibraryNoteRecord& note);
    bool removeNoteById(const QString& noteId);
    void setNotes(QVector<LibraryNoteRecord> notes);
    void clear();

    const QVector<LibraryNoteRecord>& notes() const noexcept;

private:
    QVector<LibraryNoteRecord> m_notes;
};
