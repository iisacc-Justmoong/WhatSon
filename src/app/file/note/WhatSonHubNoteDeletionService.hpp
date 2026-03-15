#pragma once

#include "file/IO/WhatSonSystemIoGateway.hpp"
#include "file/hierarchy/library/LibraryNoteRecord.hpp"
#include "file/hub/WhatSonHubStat.hpp"
#include "file/validator/WhatSonHubStructureValidator.hpp"
#include "file/validator/WhatSonLibraryIndexIntegrityValidator.hpp"
#include "file/validator/WhatSonNoteStorageValidator.hpp"

#include <QString>
#include <QVector>

class WhatSonHubNoteDeletionService final
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
        QString noteId;
    };

    struct Result final
    {
        QString noteId;
        QString wshubPath;
        QString libraryPath;
        QString statPath;
        WhatSonHubStat hubStat;
        QVector<LibraryNoteRecord> remainingNotes;
    };

    WhatSonHubNoteDeletionService();
    ~WhatSonHubNoteDeletionService();

    bool deleteNote(Request request, Result* outResult = nullptr, QString* errorMessage = nullptr) const;

private:
    WhatSonSystemIoGateway m_ioGateway;
    WhatSonHubStructureValidator m_hubStructureValidator;
    WhatSonLibraryIndexIntegrityValidator m_libraryIndexIntegrityValidator;
    WhatSonNoteStorageValidator m_noteStorageValidator;
};
