#pragma once

#include "models/file/IO/WhatSonSystemIoGateway.hpp"
#include "models/file/hierarchy/library/LibraryNoteRecord.hpp"

#include <QString>

class WhatSonNoteStorageValidator final
{
public:
    WhatSonNoteStorageValidator();
    ~WhatSonNoteStorageValidator();

    QString resolveExistingNoteHeaderPath(const LibraryNoteRecord& record) const;
    QString resolveExistingNoteDirectoryPath(const LibraryNoteRecord& record) const;
    bool hasMaterializedStorage(const LibraryNoteRecord& record) const;
    bool normalizeWsnotePackage(const LibraryNoteRecord& record, QString* errorMessage = nullptr) const;

private:
    QString normalizePath(const QString& path) const;
    bool normalizeWsnotePackageByDirectoryPath(
        const QString& noteDirectoryPath,
        const QString& noteId,
        QString* errorMessage = nullptr) const;

    mutable WhatSonSystemIoGateway m_ioGateway;
};
