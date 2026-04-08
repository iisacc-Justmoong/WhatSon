#pragma once

#include <QString>
#include <QStringList>

struct LibraryNoteRecord
{
    QString noteId;
    QString storageKind;
    QString bodyPlainText;
    QString bodySourceText;
    QString bodyFirstLine;
    bool bodyHasResource = false;
    QString bodyFirstResourceThumbnailUrl;
    QString createdAt;
    QString lastModifiedAt;
    QString author;
    QString modifiedBy;
    QString project;
    QStringList folders;
    QStringList folderUuids;
    QStringList bookmarkColors;
    QStringList tags;
    int progress = -1;
    bool bookmarked = false;
    bool preset = false;
    QString noteDirectoryPath;
    QString noteHeaderPath;

    bool operator==(const LibraryNoteRecord& other) const
    {
        return noteId == other.noteId
            && storageKind == other.storageKind
            && bodyPlainText == other.bodyPlainText
            && bodySourceText == other.bodySourceText
            && bodyFirstLine == other.bodyFirstLine
            && bodyHasResource == other.bodyHasResource
            && bodyFirstResourceThumbnailUrl == other.bodyFirstResourceThumbnailUrl
            && createdAt == other.createdAt
            && lastModifiedAt == other.lastModifiedAt
            && author == other.author
            && modifiedBy == other.modifiedBy
            && project == other.project
            && folders == other.folders
            && folderUuids == other.folderUuids
            && bookmarkColors == other.bookmarkColors
            && tags == other.tags
            && progress == other.progress
            && bookmarked == other.bookmarked
            && preset == other.preset
            && noteDirectoryPath == other.noteDirectoryPath
            && noteHeaderPath == other.noteHeaderPath;
    }

    bool operator!=(const LibraryNoteRecord& other) const
    {
        return !(*this == other);
    }
};
