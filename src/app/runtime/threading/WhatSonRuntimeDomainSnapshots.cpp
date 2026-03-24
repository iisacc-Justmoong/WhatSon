#include "WhatSonRuntimeDomainSnapshots.hpp"

#include "file/hierarchy/event/WhatSonEventHierarchyParser.hpp"
#include "file/hierarchy/event/WhatSonEventHierarchyStore.hpp"
#include "file/hierarchy/folders/WhatSonFoldersHierarchyParser.hpp"
#include "file/hierarchy/folders/WhatSonFoldersHierarchyStore.hpp"
#include "file/hierarchy/library/LibraryAll.hpp"
#include "file/hierarchy/library/LibraryDraft.hpp"
#include "file/hierarchy/library/LibraryToday.hpp"
#include "file/hierarchy/preset/WhatSonPresetHierarchyParser.hpp"
#include "file/hierarchy/preset/WhatSonPresetHierarchyStore.hpp"
#include "file/hierarchy/progress/WhatSonProgressHierarchyParser.hpp"
#include "file/hierarchy/progress/WhatSonProgressHierarchyStore.hpp"
#include "file/hierarchy/projects/WhatSonProjectsHierarchyParser.hpp"
#include "file/hierarchy/projects/WhatSonProjectsHierarchyStore.hpp"
#include "file/hierarchy/resources/WhatSonResourcesHierarchyParser.hpp"
#include "file/hierarchy/resources/WhatSonResourcesHierarchyStore.hpp"
#include "file/hierarchy/tags/WhatSonTagsHierarchyParser.hpp"
#include "file/hierarchy/tags/WhatSonTagsHierarchyStore.hpp"
#include "hub/WhatSonHubRuntimeStore.hpp"
#include "viewmodel/hierarchy/event/EventHierarchyViewModelSupport.hpp"
#include "viewmodel/hierarchy/library/LibraryHierarchyViewModelSupport.hpp"
#include "viewmodel/hierarchy/preset/PresetHierarchyViewModelSupport.hpp"
#include "viewmodel/hierarchy/progress/ProgressHierarchyViewModelSupport.hpp"
#include "viewmodel/hierarchy/projects/ProjectsHierarchyViewModelSupport.hpp"
#include "viewmodel/hierarchy/resources/ResourcesHierarchyViewModelSupport.hpp"
#include "viewmodel/hierarchy/tags/TagsHierarchyViewModelSupport.hpp"

#include <QDir>
#include <QFileInfo>

namespace
{
    QString defaultSourceFilePath(const QStringList& contentsDirectories, const QString& fileName)
    {
        if (contentsDirectories.isEmpty())
        {
            return {};
        }
        return QDir(contentsDirectories.first()).filePath(fileName);
    }
} // namespace

WhatSonRuntimeDomainSnapshots::LibrarySnapshot WhatSonRuntimeDomainSnapshots::loadLibrary(const QString& wshubPath)
{
    LibrarySnapshot snapshot;

    LibraryAll libraryAll;
    QString indexError;
    if (!libraryAll.indexFromWshub(wshubPath, &indexError))
    {
        snapshot.succeeded = false;
        snapshot.error = indexError;
        return snapshot;
    }

    snapshot.allNotes = libraryAll.notes();

    LibraryDraft libraryDraft;
    libraryDraft.rebuild(snapshot.allNotes);
    snapshot.draftNotes = libraryDraft.notes();

    LibraryToday libraryToday;
    libraryToday.rebuild(snapshot.allNotes);
    snapshot.todayNotes = libraryToday.notes();

    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::LibrarySupport::resolveContentsDirectories(wshubPath, &contentsDirectories, &resolveError))
    {
        snapshot.succeeded = false;
        snapshot.error = resolveError;
        return snapshot;
    }

    WhatSonFoldersHierarchyParser foldersParser;

    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Folders.wsfolders"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }

        snapshot.foldersFileFound = true;
        if (snapshot.foldersFilePath.isEmpty())
        {
            snapshot.foldersFilePath = filePath;
        }

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::LibrarySupport::readUtf8File(filePath, &rawText, &readError))
        {
            snapshot.succeeded = false;
            snapshot.error = readError;
            return snapshot;
        }

        WhatSonFoldersHierarchyStore foldersStore;
        QString parseError;
        bool folderUuidMigrationRequired = false;
        if (!foldersParser.parse(rawText, &foldersStore, &parseError, &folderUuidMigrationRequired))
        {
            snapshot.succeeded = false;
            snapshot.error = parseError;
            return snapshot;
        }

        if (folderUuidMigrationRequired)
        {
            QString writeError;
            if (!foldersStore.writeToFile(filePath, &writeError))
            {
                snapshot.succeeded = false;
                snapshot.error = writeError;
                return snapshot;
            }
        }

        const QVector<WhatSonFolderDepthEntry> parsedEntries = foldersStore.folderEntries();
        for (const WhatSonFolderDepthEntry& entry : parsedEntries)
        {
            snapshot.folderEntries.push_back(entry);
        }
    }

    if (snapshot.foldersFilePath.isEmpty())
    {
        snapshot.foldersFilePath = defaultSourceFilePath(contentsDirectories, QStringLiteral("Folders.wsfolders"));
    }

    snapshot.succeeded = true;
    return snapshot;
}

WhatSonRuntimeDomainSnapshots::BookmarksSnapshot WhatSonRuntimeDomainSnapshots::loadBookmarks(const QString& wshubPath)
{
    BookmarksSnapshot snapshot;

    LibraryAll libraryAll;
    QString indexError;
    if (!libraryAll.indexFromWshub(wshubPath, &indexError))
    {
        snapshot.succeeded = false;
        snapshot.error = indexError;
        return snapshot;
    }

    const QVector<LibraryNoteRecord>& notes = libraryAll.notes();
    snapshot.bookmarkedNotes.reserve(notes.size());
    for (const LibraryNoteRecord& note : notes)
    {
        if (!note.bookmarked)
        {
            continue;
        }
        snapshot.bookmarkedNotes.push_back(note);
    }

    snapshot.succeeded = true;
    return snapshot;
}

WhatSonRuntimeDomainSnapshots::ProjectsSnapshot WhatSonRuntimeDomainSnapshots::loadProjects(const QString& wshubPath)
{
    ProjectsSnapshot snapshot;

    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::ProjectsSupport::resolveContentsDirectories(
        wshubPath, &contentsDirectories, &resolveError))
    {
        snapshot.succeeded = false;
        snapshot.error = resolveError;
        return snapshot;
    }

    WhatSonProjectsHierarchyParser parser;
    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("ProjectLists.wsproj"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }

        snapshot.fileFound = true;
        if (snapshot.projectsFilePath.isEmpty())
        {
            snapshot.projectsFilePath = filePath;
        }

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::ProjectsSupport::readUtf8File(filePath, &rawText, &readError))
        {
            snapshot.succeeded = false;
            snapshot.error = readError;
            return snapshot;
        }

        WhatSonProjectsHierarchyStore parsedStore;
        QString parseError;
        if (!parser.parse(rawText, &parsedStore, &parseError))
        {
            snapshot.succeeded = false;
            snapshot.error = parseError;
            return snapshot;
        }

        const QVector<WhatSonFolderDepthEntry> parsedEntries = parsedStore.folderEntries();
        for (const WhatSonFolderDepthEntry& entry : parsedEntries)
        {
            snapshot.projectEntries.push_back(entry);
        }
    }

    if (snapshot.projectsFilePath.isEmpty())
    {
        snapshot.projectsFilePath = defaultSourceFilePath(contentsDirectories, QStringLiteral("ProjectLists.wsproj"));
    }

    snapshot.succeeded = true;
    return snapshot;
}

WhatSonRuntimeDomainSnapshots::StringListSnapshot WhatSonRuntimeDomainSnapshots::loadResources(const QString& wshubPath)
{
    StringListSnapshot snapshot;

    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::ResourcesSupport::resolveContentsDirectories(
        wshubPath, &contentsDirectories, &resolveError))
    {
        snapshot.succeeded = false;
        snapshot.error = resolveError;
        return snapshot;
    }

    WhatSonResourcesHierarchyParser parser;
    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Resources.wsresources"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }

        snapshot.fileFound = true;
        if (snapshot.sourceFilePath.isEmpty())
        {
            snapshot.sourceFilePath = filePath;
        }

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::ResourcesSupport::readUtf8File(filePath, &rawText, &readError))
        {
            snapshot.succeeded = false;
            snapshot.error = readError;
            return snapshot;
        }

        WhatSonResourcesHierarchyStore store;
        QString parseError;
        if (!parser.parse(rawText, &store, &parseError))
        {
            snapshot.succeeded = false;
            snapshot.error = parseError;
            return snapshot;
        }

        for (const QString& value : store.resourcePaths())
        {
            snapshot.values.push_back(value);
        }
    }

    if (snapshot.sourceFilePath.isEmpty())
    {
        snapshot.sourceFilePath = defaultSourceFilePath(contentsDirectories, QStringLiteral("Resources.wsresources"));
    }

    snapshot.succeeded = true;
    return snapshot;
}

WhatSonRuntimeDomainSnapshots::ProgressSnapshot WhatSonRuntimeDomainSnapshots::loadProgress(const QString& wshubPath)
{
    ProgressSnapshot snapshot;

    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::ProgressSupport::resolveContentsDirectories(
        wshubPath, &contentsDirectories, &resolveError))
    {
        snapshot.succeeded = false;
        snapshot.error = resolveError;
        return snapshot;
    }

    WhatSonProgressHierarchyParser parser;
    WhatSonProgressHierarchyStore store;

    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Progress.wsprogress"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }

        snapshot.fileFound = true;
        snapshot.sourceFilePath = filePath;

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::ProgressSupport::readUtf8File(filePath, &rawText, &readError))
        {
            snapshot.succeeded = false;
            snapshot.error = readError;
            return snapshot;
        }

        QString parseError;
        if (!parser.parse(rawText, &store, &parseError))
        {
            snapshot.succeeded = false;
            snapshot.error = parseError;
            return snapshot;
        }
        break;
    }

    if (snapshot.sourceFilePath.isEmpty())
    {
        snapshot.sourceFilePath = defaultSourceFilePath(contentsDirectories, QStringLiteral("Progress.wsprogress"));
    }

    if (!snapshot.fileFound)
    {
        QString parseError;
        if (!parser.parse(QString(), &store, &parseError))
        {
            snapshot.succeeded = false;
            snapshot.error = parseError;
            return snapshot;
        }
    }

    snapshot.progressValue = store.progressValue();
    snapshot.progressStates = store.progressStates();
    snapshot.succeeded = true;
    return snapshot;
}

WhatSonRuntimeDomainSnapshots::StringListSnapshot WhatSonRuntimeDomainSnapshots::loadEvent(const QString& wshubPath)
{
    StringListSnapshot snapshot;

    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::EventSupport::resolveContentsDirectories(wshubPath, &contentsDirectories, &resolveError))
    {
        snapshot.succeeded = false;
        snapshot.error = resolveError;
        return snapshot;
    }

    WhatSonEventHierarchyParser parser;
    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Event.wsevent"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }

        snapshot.fileFound = true;
        if (snapshot.sourceFilePath.isEmpty())
        {
            snapshot.sourceFilePath = filePath;
        }

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::EventSupport::readUtf8File(filePath, &rawText, &readError))
        {
            snapshot.succeeded = false;
            snapshot.error = readError;
            return snapshot;
        }

        WhatSonEventHierarchyStore store;
        QString parseError;
        if (!parser.parse(rawText, &store, &parseError))
        {
            snapshot.succeeded = false;
            snapshot.error = parseError;
            return snapshot;
        }

        for (const QString& value : store.eventNames())
        {
            snapshot.values.push_back(value);
        }
    }

    if (snapshot.sourceFilePath.isEmpty())
    {
        snapshot.sourceFilePath = defaultSourceFilePath(contentsDirectories, QStringLiteral("Event.wsevent"));
    }

    snapshot.succeeded = true;
    return snapshot;
}

WhatSonRuntimeDomainSnapshots::StringListSnapshot WhatSonRuntimeDomainSnapshots::loadPreset(const QString& wshubPath)
{
    StringListSnapshot snapshot;

    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::PresetSupport::resolveContentsDirectories(wshubPath, &contentsDirectories, &resolveError))
    {
        snapshot.succeeded = false;
        snapshot.error = resolveError;
        return snapshot;
    }

    WhatSonPresetHierarchyParser parser;
    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Preset.wspreset"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }

        snapshot.fileFound = true;
        if (snapshot.sourceFilePath.isEmpty())
        {
            snapshot.sourceFilePath = filePath;
        }

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::PresetSupport::readUtf8File(filePath, &rawText, &readError))
        {
            snapshot.succeeded = false;
            snapshot.error = readError;
            return snapshot;
        }

        WhatSonPresetHierarchyStore store;
        QString parseError;
        if (!parser.parse(rawText, &store, &parseError))
        {
            snapshot.succeeded = false;
            snapshot.error = parseError;
            return snapshot;
        }

        for (const QString& value : store.presetNames())
        {
            snapshot.values.push_back(value);
        }
    }

    if (snapshot.sourceFilePath.isEmpty())
    {
        snapshot.sourceFilePath = defaultSourceFilePath(contentsDirectories, QStringLiteral("Preset.wspreset"));
    }

    snapshot.succeeded = true;
    return snapshot;
}

WhatSonRuntimeDomainSnapshots::TagsSnapshot WhatSonRuntimeDomainSnapshots::loadTags(const QString& wshubPath)
{
    TagsSnapshot snapshot;

    QStringList contentsDirectories;
    QString resolveError;
    if (!WhatSon::Hierarchy::TagsSupport::resolveContentsDirectories(wshubPath, &contentsDirectories, &resolveError))
    {
        snapshot.succeeded = false;
        snapshot.error = resolveError;
        return snapshot;
    }

    WhatSonTagsHierarchyParser parser;
    for (const QString& contentsDirectory : contentsDirectories)
    {
        const QString filePath = QDir(contentsDirectory).filePath(QStringLiteral("Tags.wstags"));
        if (!QFileInfo(filePath).isFile())
        {
            continue;
        }

        snapshot.fileFound = true;
        if (snapshot.tagsFilePath.isEmpty())
        {
            snapshot.tagsFilePath = filePath;
        }

        QString rawText;
        QString readError;
        if (!WhatSon::Hierarchy::TagsSupport::readUtf8File(filePath, &rawText, &readError))
        {
            snapshot.succeeded = false;
            snapshot.error = readError;
            return snapshot;
        }

        WhatSonTagsHierarchyStore store;
        QString parseError;
        if (!parser.parse(rawText, &store, &parseError))
        {
            snapshot.succeeded = false;
            snapshot.error = parseError;
            return snapshot;
        }

        const QVector<WhatSonTagDepthEntry> parsedEntries = store.tagEntries();
        for (const WhatSonTagDepthEntry& entry : parsedEntries)
        {
            snapshot.entries.push_back(entry);
        }
    }

    if (snapshot.tagsFilePath.isEmpty())
    {
        snapshot.tagsFilePath = defaultSourceFilePath(contentsDirectories, QStringLiteral("Tags.wstags"));
    }

    snapshot.succeeded = true;
    return snapshot;
}

bool WhatSonRuntimeDomainSnapshots::loadHubRuntime(
    const QString& wshubPath,
    WhatSonHubRuntimeStore* store,
    QString* errorMessage)
{
    if (store == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Target runtime store is null.");
        }
        return false;
    }
    return store->loadFromWshub(wshubPath, errorMessage);
}
