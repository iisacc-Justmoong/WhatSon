#pragma once

#include <QString>
#include <QStringList>

struct LibraryNoteRecord
{
    QString noteId;
    QString title;
    QString bodySummary;
    QString createdAt;
    QString lastModifiedAt;
    QString author;
    QString modifiedBy;
    QString project;
    QStringList folders;
    QStringList tags;
    int progress = 0;
    bool bookmarked = false;
    bool preset = false;
    QString noteDirectoryPath;
    QString noteHeaderPath;
};
