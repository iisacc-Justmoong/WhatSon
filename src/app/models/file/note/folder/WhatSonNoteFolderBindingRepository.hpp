#pragma once

#include "app/models/file/note/local/WhatSonLocalNoteDocument.hpp"
#include "app/models/file/note/local/WhatSonLocalNoteFileStore.hpp"
#include "app/models/file/note/folder/WhatSonNoteFolderBindingService.hpp"
#include "app/models/hierarchy/library/LibraryNoteRecord.hpp"

#include <QString>

class WhatSonNoteFolderBindingRepository final
{
public:
    WhatSonNoteFolderBindingRepository();
    ~WhatSonNoteFolderBindingRepository();

    bool readDocument(const LibraryNoteRecord& note, WhatSonLocalNoteDocument* outDocument, QString* errorMessage = nullptr) const;
    bool readDocument(
        const QString& noteId,
        const QString& noteDirectoryPath,
        const QString& noteHeaderPath,
        WhatSonLocalNoteDocument* outDocument,
        QString* errorMessage = nullptr) const;
    bool writeDocument(
        WhatSonLocalNoteDocument document,
        WhatSonLocalNoteDocument* outDocument = nullptr,
        QString* errorMessage = nullptr) const;
    bool writeFolderBindings(
        WhatSonLocalNoteDocument document,
        const WhatSonNoteFolderBindingService::Bindings& bindings,
        WhatSonLocalNoteDocument* outDocument = nullptr,
        QString* errorMessage = nullptr) const;

private:
    WhatSonLocalNoteFileStore m_localNoteFileStore;
};
