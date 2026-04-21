#pragma once

#include "app/models/file/hub/WhatSonHubRuntimeStore.hpp"
#include "app/models/file/hierarchy/WhatSonFolderDepthEntry.hpp"
#include "app/models/file/hierarchy/library/LibraryNoteRecord.hpp"
#include "app/models/file/hierarchy/tags/WhatSonTagDepthEntry.hpp"

#include <QString>
#include <QStringList>
#include <QVector>

class WhatSonRuntimeDomainSnapshots final
{
public:
    struct SharedContext
    {
        bool succeeded = false;
        QString error;
        QString normalizedHubPath;
        QStringList contentsDirectories;
    };

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

    struct HubRuntimeSnapshot
    {
        bool succeeded = false;
        QString error;
        WhatSonHubRuntimeStore store;
    };

    static SharedContext buildSharedContext(const QString& wshubPath);
    static LibrarySnapshot loadLibrary(const QString& wshubPath);
    static LibrarySnapshot loadLibrary(const SharedContext& context);
    static BookmarksSnapshot buildBookmarks(const QVector<LibraryNoteRecord>& allNotes);
    static BookmarksSnapshot loadBookmarks(const QString& wshubPath);
    static BookmarksSnapshot loadBookmarks(const SharedContext& context);
    static ProjectsSnapshot loadProjects(const QString& wshubPath);
    static ProjectsSnapshot loadProjects(const SharedContext& context);
    static StringListSnapshot loadResources(const QString& wshubPath);
    static StringListSnapshot loadResources(const SharedContext& context);
    static ProgressSnapshot loadProgress(const QString& wshubPath);
    static ProgressSnapshot loadProgress(const SharedContext& context);
    static StringListSnapshot loadEvent(const QString& wshubPath);
    static StringListSnapshot loadEvent(const SharedContext& context);
    static StringListSnapshot loadPreset(const QString& wshubPath);
    static StringListSnapshot loadPreset(const SharedContext& context);
    static TagsSnapshot loadTags(const QString& wshubPath);
    static TagsSnapshot loadTags(const SharedContext& context);
    static HubRuntimeSnapshot loadHubRuntime(const QString& wshubPath);
    static HubRuntimeSnapshot loadHubRuntime(const SharedContext& context);
};
