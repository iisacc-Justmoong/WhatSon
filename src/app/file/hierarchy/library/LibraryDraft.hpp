#pragma once

#include "LibraryNoteRecord.hpp"

#include <QVector>

class LibraryDraft final
{
public:
    LibraryDraft();
    ~LibraryDraft();

    bool rebuild(const QVector<LibraryNoteRecord>& allNotes);
    void clear();

    const QVector<LibraryNoteRecord>& notes() const noexcept;

private:
    QVector<LibraryNoteRecord> m_notes;
};
