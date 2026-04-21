#pragma once

#include "app/models/file/IO/WhatSonSystemIoGateway.hpp"
#include "app/models/file/validator/WhatSonNoteStorageValidator.hpp"
#include "app/models/file/hierarchy/library/LibraryNoteRecord.hpp"

#include <QString>
#include <QStringList>
#include <QVector>

class WhatSonLibraryIndexIntegrityValidator final
{
public:
    struct PruneResult final
    {
        QVector<LibraryNoteRecord> materializedRecords;
        QStringList prunedOrphanNoteIds;
    };

    WhatSonLibraryIndexIntegrityValidator();
    ~WhatSonLibraryIndexIntegrityValidator();

    PruneResult pruneOrphanRecords(const QVector<LibraryNoteRecord>& records) const;
    bool rewriteIndexesFromRecords(
        const QString& sourceWshubPath,
        const QStringList& libraryRoots,
        const QVector<LibraryNoteRecord>& records,
        QString* errorMessage = nullptr) const;

private:
    QString normalizePath(const QString& path) const;
    bool pathIsWithinRoot(const QString& path, const QString& rootPath) const;
    bool recordBelongsToLibraryRoot(const LibraryNoteRecord& record, const QString& libraryRoot) const;
    QStringList noteIdsForLibraryRoot(const QVector<LibraryNoteRecord>& records, const QString& libraryRoot) const;

    WhatSonSystemIoGateway m_ioGateway;
    WhatSonNoteStorageValidator m_noteStorageValidator;
};
