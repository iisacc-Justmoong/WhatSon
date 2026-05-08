#pragma once

#include "app/models/file/hierarchy/library/LibraryNoteRecord.hpp"

#include <QString>
#include <QVector>

namespace WhatSon::Hierarchy::NoteRecordSupport
{
    int indexOfNoteRecordById(const QVector<LibraryNoteRecord>& notes, const QString& noteId);
    bool applyPersistedBodyState(
        LibraryNoteRecord* note,
        const QString& normalizedBodyText,
        const QString& normalizedBodySourceText,
        const QString& lastModifiedAt);
    bool applyPersistedBodyState(
        QVector<LibraryNoteRecord>* notes,
        const QString& noteId,
        const QString& normalizedBodyText,
        const QString& normalizedBodySourceText,
        const QString& lastModifiedAt);
    QString directoryPathForNoteId(const QVector<LibraryNoteRecord>& notes, const QString& noteId);
    QString bodySourceTextForNoteId(const QVector<LibraryNoteRecord>& notes, const QString& noteId);
} // namespace WhatSon::Hierarchy::NoteRecordSupport
