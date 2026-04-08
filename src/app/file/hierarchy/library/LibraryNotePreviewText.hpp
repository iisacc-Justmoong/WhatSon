#pragma once

#include "LibraryNoteRecord.hpp"

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
        const QString firstLine = note.bodyFirstLine.trimmed();
        const QString bodyPlainText = truncatePreviewToMaxLines(note.bodyPlainText.trimmed(), maxLines);
        if (!firstLine.isEmpty())
        {
            if (bodyPlainText.isEmpty())
            {
                return firstLine;
            }

            if (!bodyPlainText.startsWith(firstLine))
            {
                return firstLine + QLatin1Char('\n') + bodyPlainText;
            }
        }

        if (!bodyPlainText.isEmpty())
        {
            return bodyPlainText;
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
