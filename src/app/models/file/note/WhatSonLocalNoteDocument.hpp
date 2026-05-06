#pragma once

#include "app/models/file/note/WhatSonNoteBodyPersistence.hpp"
#include "app/models/file/note/WhatSonNoteHeaderStore.hpp"
#include "app/models/file/hierarchy/library/LibraryNoteRecord.hpp"

#include <QString>

struct WhatSonLocalNoteDocument
{
    QString noteDirectoryPath;
    QString noteHeaderPath;
    QString noteBodyPath;
    QString noteVersionPath;
    QString notePaintPath;
    WhatSonNoteHeaderStore headerStore;
    QString bodyPlainText;
    QString bodySourceText;
    QString bodyFirstLine;
    bool bodyHasResource = false;
    QString bodyFirstResourceThumbnailUrl;

    QString effectiveBodyText() const
    {
        const QString normalizedSource = WhatSon::NoteBodyPersistence::normalizeBodyPlainText(bodySourceText);
        if (!normalizedSource.isEmpty())
        {
            return normalizedSource;
        }
        return WhatSon::NoteBodyPersistence::normalizeBodyPlainText(bodyPlainText);
    }

    void normalizeBodyFields()
    {
        const QString normalizedBodyText = effectiveBodyText();
        bodyPlainText = normalizedBodyText;
        bodySourceText = normalizedBodyText;
        bodyFirstLine = WhatSon::NoteBodyPersistence::firstLineFromBodyPlainText(normalizedBodyText);
    }

    LibraryNoteRecord toLibraryNoteRecord() const
    {
        LibraryNoteRecord record;
        record.noteId = headerStore.noteId();
        record.storageKind = QStringLiteral("wsnote");
        record.bodyPlainText = bodyPlainText;
        record.bodySourceText = bodySourceText;
        record.bodyFirstLine = bodyFirstLine;
        record.normalizeBodyFields();
        record.bodyHasResource = bodyHasResource;
        record.bodyFirstResourceThumbnailUrl = bodyFirstResourceThumbnailUrl.trimmed();
        record.createdAt = headerStore.createdAt();
        record.lastModifiedAt = headerStore.lastModifiedAt();
        record.author = headerStore.author();
        record.modifiedBy = headerStore.modifiedBy();
        record.project = headerStore.project();
        record.folders = headerStore.folders();
        record.folderUuids = headerStore.folderUuids();
        record.bookmarkColors = headerStore.bookmarkColors();
        record.tags = headerStore.tags();
        record.progress = headerStore.progress();
        record.bookmarked = headerStore.isBookmarked();
        record.preset = headerStore.isPreset();
        record.noteDirectoryPath = noteDirectoryPath.trimmed();
        record.noteHeaderPath = noteHeaderPath.trimmed();
        return record;
    }
};
