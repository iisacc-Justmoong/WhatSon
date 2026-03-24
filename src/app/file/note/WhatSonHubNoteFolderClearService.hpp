#pragma once

#include "WhatSonHubNoteMutationSupport.hpp"
#include "WhatSonLocalNoteFileStore.hpp"
#include "file/hierarchy/library/LibraryNoteRecord.hpp"

#include <QString>
#include <QVector>

class WhatSonHubNoteFolderClearService final
{
public:
    struct Request final
    {
        QVector<LibraryNoteRecord> notes;
        QString noteId;
    };

    struct Result final
    {
        QString noteId;
        bool foldersCleared = false;
        QVector<LibraryNoteRecord> notes;
    };

    WhatSonHubNoteFolderClearService();
    ~WhatSonHubNoteFolderClearService();

    bool clearFolders(Request request, Result* outResult = nullptr, QString* errorMessage = nullptr) const;

private:
    WhatSonLocalNoteFileStore m_localNoteFileStore;
};
