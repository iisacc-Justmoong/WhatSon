#include "app/models/hierarchy/WhatSonHierarchyNoteRecordSupport.hpp"

#include "app/models/file/note/body/WhatSonNoteBodyPersistence.hpp"

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

    bool applyPersistedBodyState(
        LibraryNoteRecord* note,
        const QString& normalizedBodyText,
        const QString& normalizedBodySourceText,
        const QString& lastModifiedAt)
    {
        if (note == nullptr)
        {
            return false;
        }

        note->bodyPlainText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(normalizedBodyText);
        note->bodySourceText = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(normalizedBodySourceText);
        if (note->bodySourceText.isEmpty())
        {
            note->bodySourceText = note->bodyPlainText;
        }
        note->bodyFirstLine = WhatSon::NoteBodyPersistence::firstLineFromBodyPlainText(note->bodyPlainText);
        if (!lastModifiedAt.trimmed().isEmpty())
        {
            note->lastModifiedAt = lastModifiedAt.trimmed();
        }

        return true;
    }

    bool applyPersistedBodyState(
        QVector<LibraryNoteRecord>* notes,
        const QString& noteId,
        const QString& normalizedBodyText,
        const QString& normalizedBodySourceText,
        const QString& lastModifiedAt)
    {
        if (notes == nullptr)
        {
            return false;
        }

        const int noteIndex = indexOfNoteRecordById(*notes, noteId);
        if (noteIndex < 0 || noteIndex >= notes->size())
        {
            return false;
        }

        return applyPersistedBodyState(
            &(*notes)[noteIndex],
            normalizedBodyText,
            normalizedBodySourceText,
            lastModifiedAt);
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

    QString bodySourceTextForNoteId(const QVector<LibraryNoteRecord>& notes, const QString& noteId)
    {
        const int noteIndex = indexOfNoteRecordById(notes, noteId);
        if (noteIndex < 0 || noteIndex >= notes.size())
        {
            return {};
        }

        const LibraryNoteRecord& note = notes.at(noteIndex);
        if (!note.bodySourceText.isEmpty())
        {
            return note.bodySourceText;
        }

        return note.bodyPlainText;
    }
} // namespace WhatSon::Hierarchy::NoteRecordSupport
