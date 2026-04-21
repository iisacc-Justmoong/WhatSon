#include "app/models/file/note/WhatSonHubNoteFolderClearService.hpp"

#include "app/models/file/note/WhatSonNoteFolderBindingService.hpp"

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

    WhatSonLocalNoteDocument noteDocument;
    QString ioError;
    if (!m_noteFolderBindingRepository.readDocument(
        note.noteId,
        note.noteDirectoryPath,
        headerPath,
        &noteDocument,
        &ioError))
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
        const WhatSonNoteFolderBindingService noteFolderBindingService;
        QString writeError;
        if (!m_noteFolderBindingRepository.writeFolderBindings(
            std::move(noteDocument),
            noteFolderBindingService.bindings({}, {}),
            &noteDocument,
            &writeError))
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
