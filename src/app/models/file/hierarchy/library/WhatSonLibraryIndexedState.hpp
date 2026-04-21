#pragma once

#include "app/models/file/hierarchy/library/LibraryAll.hpp"
#include "app/models/file/hierarchy/library/LibraryDraft.hpp"
#include "app/models/file/hierarchy/library/LibraryToday.hpp"

#include <QString>
#include <QVector>

class WhatSonLibraryIndexedState final
{
public:
    struct Snapshot
    {
        QString sourceWshubPath;
        QVector<LibraryNoteRecord> allNotes;
        QVector<LibraryNoteRecord> draftNotes;
        QVector<LibraryNoteRecord> todayNotes;
    };

    WhatSonLibraryIndexedState();
    ~WhatSonLibraryIndexedState();

    bool indexFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);
    void applySnapshot(
        QString sourceWshubPath,
        QVector<LibraryNoteRecord> allNotes,
        QVector<LibraryNoteRecord> draftNotes,
        QVector<LibraryNoteRecord> todayNotes);
    void setIndexedNotes(QString sourceWshubPath, QVector<LibraryNoteRecord> notes);
    void setSourceWshubPath(QString sourceWshubPath);
    bool upsertNote(const LibraryNoteRecord& note);
    bool removeNoteById(const QString& noteId);
    bool noteById(const QString& noteId, LibraryNoteRecord* outNote) const;
    void clear();

    [[nodiscard]] Snapshot snapshot() const;
    [[nodiscard]] QString sourceWshubPath() const;
    [[nodiscard]] const QVector<LibraryNoteRecord>& allNotes() const noexcept;
    [[nodiscard]] const QVector<LibraryNoteRecord>& draftNotes() const noexcept;
    [[nodiscard]] const QVector<LibraryNoteRecord>& todayNotes() const noexcept;

    static QVector<LibraryNoteRecord> collectBookmarkedNotes(const QVector<LibraryNoteRecord>& allNotes);

private:
    void rebuildDerivedBuckets();

    LibraryAll m_libraryAll;
    LibraryDraft m_libraryDraft;
    LibraryToday m_libraryToday;
};
