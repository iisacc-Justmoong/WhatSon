#pragma once

#include "WhatSonHubNoteMutationSupport.hpp"
#include "WhatSonLocalNoteFileStore.hpp"
#include "file/hub/WhatSonHubStat.hpp"
#include "file/hierarchy/library/LibraryNoteRecord.hpp"
#include "file/validator/WhatSonHubStructureValidator.hpp"
#include "file/validator/WhatSonLibraryIndexIntegrityValidator.hpp"

#include <QString>
#include <QStringList>
#include <QVector>

class WhatSonHubNoteCreationService final
{
public:
    struct Request final
    {
        QString wshubPath;
        QString libraryPath;
        QString statPath;
        QString hubName;
        WhatSonHubStat hubStat;
        QVector<LibraryNoteRecord> notes;
        QString authorProfileName;
        QStringList assignedFolders;
    };

    struct Result final
    {
        QString noteId;
        QString wshubPath;
        QString libraryPath;
        QString statPath;
        WhatSonHubStat hubStat;
        QVector<LibraryNoteRecord> notes;
    };

    WhatSonHubNoteCreationService();
    ~WhatSonHubNoteCreationService();

    bool createNote(Request request, Result* outResult = nullptr, QString* errorMessage = nullptr) const;

private:
    WhatSonLocalNoteFileStore m_localNoteFileStore;
    WhatSonSystemIoGateway m_ioGateway;
    WhatSonHubStructureValidator m_hubStructureValidator;
    WhatSonLibraryIndexIntegrityValidator m_libraryIndexIntegrityValidator;
};
