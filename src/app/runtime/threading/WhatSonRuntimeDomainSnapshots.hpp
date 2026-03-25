#pragma once

#include "file/hierarchy/WhatSonFolderDepthEntry.hpp"
#include "file/hierarchy/library/LibraryNoteRecord.hpp"
#include "file/hierarchy/tags/WhatSonTagDepthEntry.hpp"

#include <QString>
#include <QStringList>
#include <QVector>

class WhatSonHubRuntimeStore;

class WhatSonRuntimeDomainSnapshots final
{
public:
    struct LibrarySnapshot
    {
        bool succeeded = false;
        QString error;
        QString foldersFilePath;
        bool foldersFileFound = false;
        QVector<LibraryNoteRecord> allNotes;
        QVector<LibraryNoteRecord> draftNotes;
        QVector<LibraryNoteRecord> todayNotes;
        QVector<WhatSonFolderDepthEntry> folderEntries;
    };

    struct BookmarksSnapshot
    {
        bool succeeded = false;
        QString error;
        QVector<LibraryNoteRecord> bookmarkedNotes;
    };

    struct ProjectsSnapshot
    {
        bool succeeded = false;
        QString error;
        QString projectsFilePath;
        bool fileFound = false;
        QVector<WhatSonFolderDepthEntry> projectEntries;
    };

    struct StringListSnapshot
    {
        bool succeeded = false;
        QString error;
        QString sourceFilePath;
        bool fileFound = false;
        QStringList values;
    };

    struct ProgressSnapshot
    {
        bool succeeded = false;
        QString error;
        QString sourceFilePath;
        bool fileFound = false;
        int progressValue = 0;
        QStringList progressStates;
    };

    struct TagsSnapshot
    {
        bool succeeded = false;
        QString error;
        QString tagsFilePath;
        bool fileFound = false;
        QVector<WhatSonTagDepthEntry> entries;
    };

    static LibrarySnapshot loadLibrary(const QString& wshubPath);
    static BookmarksSnapshot buildBookmarks(const QVector<LibraryNoteRecord>& allNotes);
    static BookmarksSnapshot loadBookmarks(const QString& wshubPath);
    static ProjectsSnapshot loadProjects(const QString& wshubPath);
    static StringListSnapshot loadResources(const QString& wshubPath);
    static ProgressSnapshot loadProgress(const QString& wshubPath);
    static StringListSnapshot loadEvent(const QString& wshubPath);
    static StringListSnapshot loadPreset(const QString& wshubPath);
    static TagsSnapshot loadTags(const QString& wshubPath);

    static bool loadHubRuntime(const QString& wshubPath, WhatSonHubRuntimeStore* store, QString* errorMessage);
};
