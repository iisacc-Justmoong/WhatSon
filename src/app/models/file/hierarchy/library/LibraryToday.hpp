#pragma once

#include "app/models/file/hierarchy/library/LibraryNoteRecord.hpp"

#include <QDate>
#include <QVector>

class LibraryToday final
{
public:
    LibraryToday();
    ~LibraryToday();

    bool rebuild(const QVector<LibraryNoteRecord>& allNotes, const QDate& today = QDate::currentDate());
    static bool matches(const LibraryNoteRecord& note, const QDate& today = QDate::currentDate());
    bool upsertNote(const LibraryNoteRecord& note, const QDate& today = QDate::currentDate());
    bool removeNoteById(const QString& noteId);
    void setNotes(QVector<LibraryNoteRecord> notes);
    void clear();

    const QVector<LibraryNoteRecord>& notes() const noexcept;

private:
    QVector<LibraryNoteRecord> m_notes;
};
