#include "WhatSonNoteFolderBindingRepository.hpp"

#include "WhatSonHubNoteMutationSupport.hpp"

#include <utility>

WhatSonNoteFolderBindingRepository::WhatSonNoteFolderBindingRepository() = default;

WhatSonNoteFolderBindingRepository::~WhatSonNoteFolderBindingRepository() = default;

bool WhatSonNoteFolderBindingRepository::readDocument(
    const LibraryNoteRecord& note,
    WhatSonLocalNoteDocument* outDocument,
    QString* errorMessage) const
{
    return readDocument(
        note.noteId,
        note.noteDirectoryPath,
        WhatSon::NoteMutationSupport::resolveNoteHeaderPath(note),
        outDocument,
        errorMessage);
}

bool WhatSonNoteFolderBindingRepository::readDocument(
    const QString& noteId,
    const QString& noteDirectoryPath,
    const QString& noteHeaderPath,
    WhatSonLocalNoteDocument* outDocument,
    QString* errorMessage) const
{
    if (outDocument == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("outDocument must not be null.");
        }
        return false;
    }

    const QString normalizedNoteId = noteId.trimmed();
    const QString normalizedHeaderPath = noteHeaderPath.trimmed();
    if (normalizedNoteId.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("noteId must not be empty.");
        }
        return false;
    }
    if (normalizedHeaderPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("noteHeaderPath must not be empty.");
        }
        return false;
    }

    WhatSonLocalNoteFileStore::ReadRequest readRequest;
    readRequest.noteId = normalizedNoteId;
    readRequest.noteDirectoryPath = noteDirectoryPath;
    readRequest.noteHeaderPath = normalizedHeaderPath;
    return m_localNoteFileStore.readNote(std::move(readRequest), outDocument, errorMessage);
}

bool WhatSonNoteFolderBindingRepository::writeDocument(
    WhatSonLocalNoteDocument document,
    WhatSonLocalNoteDocument* outDocument,
    QString* errorMessage) const
{
    WhatSonLocalNoteFileStore::UpdateRequest updateRequest;
    updateRequest.document = std::move(document);
    updateRequest.persistHeader = true;
    updateRequest.persistBody = false;
    return m_localNoteFileStore.updateNote(std::move(updateRequest), outDocument, errorMessage);
}

bool WhatSonNoteFolderBindingRepository::writeFolderBindings(
    WhatSonLocalNoteDocument document,
    const WhatSonNoteFolderBindingService::Bindings& bindings,
    WhatSonLocalNoteDocument* outDocument,
    QString* errorMessage) const
{
    document.headerStore.setFolderBindings(bindings.folders, bindings.folderUuids);
    return writeDocument(std::move(document), outDocument, errorMessage);
}
