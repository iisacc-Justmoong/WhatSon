#include "app/models/hierarchy/WhatSonHierarchyNoteRecordSupport.hpp"

namespace WhatSon::Hierarchy::NoteRecordSupport
{
    int indexOfNoteRecordById(const QVector<LibraryNoteRecord>& notes, const QString& noteId)
    {
        const QString normalizedNoteId = noteId.trimmed();
        if (normalizedNoteId.isEmpty())
        {
            return -1;
        }

        for (int index = 0; index < notes.size(); ++index)
        {
            if (notes.at(index).noteId.trimmed() == normalizedNoteId)
            {
                return index;
            }
        }

        return -1;
    }

    QString directoryPathForNoteId(const QVector<LibraryNoteRecord>& notes, const QString& noteId)
    {
        const int noteIndex = indexOfNoteRecordById(notes, noteId);
        if (noteIndex < 0 || noteIndex >= notes.size())
        {
            return {};
        }

        return notes.at(noteIndex).noteDirectoryPath.trimmed();
    }
} // namespace WhatSon::Hierarchy::NoteRecordSupport
