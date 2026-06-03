#pragma once

#include "app/models/file/note/header/WhatSonNoteHeaderStore.hpp"

#include <QString>

namespace WhatSon::NoteFileStatSupport
{
    bool incrementOpenCountForNoteHeader(
        const QString& noteId,
        const QString& noteDirectoryPath,
        QString* errorMessage = nullptr);

    bool refreshTrackedStatisticsForNote(
        const QString& noteId,
        const QString& noteDirectoryPath,
        bool incrementOpenCount,
        QString* errorMessage = nullptr);

    bool refreshTrackedStatisticsForNoteId(
        const QString& noteId,
        const QString& referenceNoteDirectoryPath,
        QString* errorMessage = nullptr);
} // namespace WhatSon::NoteFileStatSupport
