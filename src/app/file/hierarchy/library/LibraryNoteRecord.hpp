#pragma once

#include <QString>
#include <QStringList>

struct LibraryNoteRecord
{
    QString noteId;
    QString storageKind;
    QString bodyPlainText;
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
};
