#pragma once

#include "file/hierarchy/library/LibraryNoteRecord.hpp"

#include <QString>

class WhatSonNoteStorageValidator final
{
public:
    WhatSonNoteStorageValidator();
    ~WhatSonNoteStorageValidator();

    QString resolveExistingNoteHeaderPath(const LibraryNoteRecord& record) const;
    QString resolveExistingNoteDirectoryPath(const LibraryNoteRecord& record) const;
    bool hasMaterializedStorage(const LibraryNoteRecord& record) const;

private:
    QString normalizePath(const QString& path) const;
};
