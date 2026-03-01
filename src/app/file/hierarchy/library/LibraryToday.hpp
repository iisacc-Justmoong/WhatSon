#pragma once

#include "LibraryNoteRecord.hpp"

#include <QDate>
#include <QVector>

class LibraryToday final
{
public:
    LibraryToday();
    ~LibraryToday();

    bool rebuild(const QVector<LibraryNoteRecord>& allNotes, const QDate& today = QDate::currentDate());
    void clear();

    const QVector<LibraryNoteRecord>& notes() const noexcept;

private:
    QVector<LibraryNoteRecord> m_notes;
};
