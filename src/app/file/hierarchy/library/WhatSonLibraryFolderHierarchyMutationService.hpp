#pragma once

#include "file/hierarchy/WhatSonFolderDepthEntry.hpp"
#include "file/hierarchy/library/LibraryNoteRecord.hpp"
#include "file/note/WhatSonLocalNoteFileStore.hpp"

#include <QHash>
#include <QString>
#include <QVector>

class WhatSonLibraryFolderHierarchyMutationService final
{
public:
    struct Request final
    {
        QString foldersFilePath;
        bool runtimeIndexLoaded = false;
        QVector<WhatSonFolderDepthEntry> currentFolderEntries;
        QVector<WhatSonFolderDepthEntry> stagedFolderEntries;
        QVector<LibraryNoteRecord> notes;
        QHash<QString, QString> movedFolderPathMap;
    };

    struct Result final
    {
        QVector<LibraryNoteRecord> notes;
    };

    WhatSonLibraryFolderHierarchyMutationService();
    ~WhatSonLibraryFolderHierarchyMutationService();

    bool commitMutation(Request request, Result* outResult = nullptr, QString* errorMessage = nullptr) const;

private:
    WhatSonLocalNoteFileStore m_localNoteFileStore;
};
