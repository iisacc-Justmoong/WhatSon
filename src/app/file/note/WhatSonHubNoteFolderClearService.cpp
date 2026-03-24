#include "WhatSonHubNoteFolderClearService.hpp"

#include <utility>

WhatSonHubNoteFolderClearService::WhatSonHubNoteFolderClearService() = default;

WhatSonHubNoteFolderClearService::~WhatSonHubNoteFolderClearService() = default;

bool WhatSonHubNoteFolderClearService::clearFolders(
    Request request,
    Result* outResult,
    QString* errorMessage) const
{
    const QString normalizedNoteId = request.noteId.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("noteId must not be empty.");
        }
        return false;
    }

    const int noteIndex = WhatSon::NoteMutationSupport::indexOfNoteRecordById(request.notes, normalizedNoteId);
    if (noteIndex < 0)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("noteId was not found in indexed notes: %1").arg(normalizedNoteId);
        }
        return false;
    }

    LibraryNoteRecord& note = request.notes[noteIndex];
    const QString headerPath = WhatSon::NoteMutationSupport::resolveNoteHeaderPath(note);
    if (headerPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to resolve note header path for: %1").arg(normalizedNoteId);
        }
        return false;
    }

    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = note.noteId;
    readRequest.noteDirectoryPath = note.noteDirectoryPath;
    readRequest.noteHeaderPath = headerPath;

    WhatSonLocalNoteDocument noteDocument;
    QString ioError;
    if (!m_localNoteFileStore.readNote(std::move(readRequest), &noteDocument, &ioError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = ioError;
        }
        return false;
    }

    bool foldersCleared = false;
    if (!noteDocument.headerStore.folders().isEmpty())
    {
        noteDocument.headerStore.setFolders({});
        noteDocument.headerStore.setLastModifiedAt(WhatSon::NoteMutationSupport::currentNoteTimestamp());

        WhatSonLocalNoteFileStore::UpdateRequest updateRequest;
        updateRequest.document = noteDocument;
        updateRequest.persistHeader = true;
        updateRequest.persistBody = false;

        QString writeError;
        if (!m_localNoteFileStore.updateNote(std::move(updateRequest), &noteDocument, &writeError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = writeError;
            }
            return false;
        }
        foldersCleared = true;
    }

    WhatSon::NoteMutationSupport::syncNoteRecordFromDocument(&note, noteDocument);

    if (outResult != nullptr)
    {
        outResult->noteId = normalizedNoteId;
        outResult->foldersCleared = foldersCleared;
        outResult->notes = std::move(request.notes);
    }

    return true;
}
