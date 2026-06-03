#pragma once

#include "app/models/hierarchy/library/LibraryNoteRecord.hpp"

#include <QString>
#include <QVector>

namespace WhatSon::Hierarchy::NoteRecordSupport
{
    int indexOfNoteRecordById(const QVector<LibraryNoteRecord>& notes, const QString& noteId);
    QString directoryPathForNoteId(const QVector<LibraryNoteRecord>& notes, const QString& noteId);
} // namespace WhatSon::Hierarchy::NoteRecordSupport
