#pragma once

#include "WhatSonLocalNoteDocument.hpp"
#include "WhatSonLocalNoteFileStore.hpp"
#include "WhatSonNoteFolderBindingService.hpp"
#include "file/hierarchy/library/LibraryNoteRecord.hpp"

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
