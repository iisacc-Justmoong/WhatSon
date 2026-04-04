#pragma once

#include "WhatSonNoteHeaderStore.hpp"

#include <QString>
#include <QStringList>

namespace WhatSon::NoteFileStatSupport
{
    QStringList extractBacklinkTargets(const QString& bodySourceText, const QString& bodyDocumentText);

    bool applyTrackedStatistics(
        WhatSonNoteHeaderStore* headerStore,
        const QString& noteDirectoryPath,
        const QString& bodySourceText,
        const QString& bodyDocumentText,
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
