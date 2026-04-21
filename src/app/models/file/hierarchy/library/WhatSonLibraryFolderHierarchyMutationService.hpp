#pragma once

#include "models/file/hierarchy/WhatSonFolderDepthEntry.hpp"
#include "models/file/hierarchy/library/LibraryNoteRecord.hpp"
#include "models/file/note/WhatSonNoteFolderBindingRepository.hpp"

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
    WhatSonNoteFolderBindingRepository m_noteFolderBindingRepository;
};
