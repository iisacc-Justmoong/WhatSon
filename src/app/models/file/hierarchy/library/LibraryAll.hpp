#pragma once

#include "LibraryNoteRecord.hpp"
#include "validator/WhatSonHubStructureValidator.hpp"
#include "validator/WhatSonLibraryIndexIntegrityValidator.hpp"

#include <QString>
#include <QVector>

class LibraryAll final
{
public:
    LibraryAll();
    ~LibraryAll();

    bool indexFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);
    void setIndexedNotes(QString sourceWshubPath, QVector<LibraryNoteRecord> notes);
    void setSourceWshubPath(QString sourceWshubPath);
    bool upsertNote(const LibraryNoteRecord& note);
    bool removeNoteById(const QString& noteId);
    bool noteById(const QString& noteId, LibraryNoteRecord* outNote) const;
    void clear();

    QString sourceWshubPath() const;
    const QVector<LibraryNoteRecord>& notes() const noexcept;

private:
    QString m_sourceWshubPath;
    QVector<LibraryNoteRecord> m_notes;
    WhatSonHubStructureValidator m_hubStructureValidator;
    WhatSonLibraryIndexIntegrityValidator m_libraryIndexIntegrityValidator;
};
