#pragma once

#include "app/models/hierarchy/library/LibraryNoteRecord.hpp"

#include <QString>
#include <QStringList>

namespace WhatSon::LibraryPreview
{
    inline QString truncatePreviewToMaxLines(const QString& value, int maxLines)
    {
        if (maxLines <= 0)
        {
            return {};
        }

        const QStringList lines = value.split(QLatin1Char('\n'));
        if (lines.size() <= maxLines)
        {
            return value;
        }

        QStringList truncated;
        truncated.reserve(maxLines);
        for (int index = 0; index < maxLines; ++index)
        {
            truncated.push_back(lines.at(index));
        }
        return truncated.join(QLatin1Char('\n'));
    }

    inline QString notePrimaryText(const LibraryNoteRecord& note, int maxLines = 5)
    {
        Q_UNUSED(maxLines)

        const QString noteId = note.noteId.trimmed();
        if (!noteId.isEmpty())
        {
            return noteId;
        }

        const QString project = note.project.trimmed();
        if (!project.isEmpty())
        {
            return project;
        }

        for (const QString& folder : note.folders)
        {
            const QString trimmedFolder = folder.trimmed();
            if (!trimmedFolder.isEmpty())
            {
                return trimmedFolder;
            }
        }

        return {};
    }

    inline QString notePrimaryHeadline(const LibraryNoteRecord& note, int maxLines = 5)
    {
        const QString primaryText = notePrimaryText(note, maxLines);
        const QStringList lines = primaryText.split(QLatin1Char('\n'));
        for (const QString& line : lines)
        {
            const QString trimmed = line.trimmed();
            if (!trimmed.isEmpty())
            {
                return trimmed;
            }
        }
        return {};
    }
} // namespace WhatSon::LibraryPreview
