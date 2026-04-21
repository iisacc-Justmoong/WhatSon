#include "app/runtime/threading/WhatSonRuntimeDomainSnapshots.hpp"

#include "app/models/file/hierarchy/event/WhatSonEventHierarchyParser.hpp"
#include "app/models/file/hierarchy/event/WhatSonEventHierarchyStore.hpp"
#include "app/models/file/hierarchy/folders/WhatSonFoldersHierarchyParser.hpp"
#include "app/models/file/hierarchy/folders/WhatSonFoldersHierarchyStore.hpp"
#include "app/models/file/hierarchy/library/WhatSonLibraryIndexedState.hpp"
#include "app/models/file/hierarchy/preset/WhatSonPresetHierarchyParser.hpp"
#include "app/models/file/hierarchy/preset/WhatSonPresetHierarchyStore.hpp"
#include "app/models/file/hierarchy/progress/WhatSonProgressHierarchyParser.hpp"
#include "app/models/file/hierarchy/progress/WhatSonProgressHierarchyStore.hpp"
#include "app/models/file/hierarchy/projects/WhatSonProjectsHierarchyParser.hpp"
#include "app/models/file/hierarchy/projects/WhatSonProjectsHierarchyStore.hpp"
#include "app/models/file/hierarchy/resources/WhatSonResourcesHierarchyParser.hpp"
#include "app/models/file/hierarchy/resources/WhatSonResourcePackageSupport.hpp"
#include "app/models/file/hierarchy/resources/WhatSonResourcesHierarchyStore.hpp"
#include "app/models/file/hierarchy/tags/WhatSonTagsHierarchyParser.hpp"
#include "app/models/file/hierarchy/tags/WhatSonTagsHierarchyStore.hpp"
#include "app/models/file/hub/WhatSonHubPathUtils.hpp"
#include "app/models/file/hub/WhatSonHubRuntimeStore.hpp"
#include "app/viewmodel/hierarchy/event/EventHierarchyViewModelSupport.hpp"
#include "app/viewmodel/hierarchy/library/LibraryHierarchyViewModelSupport.hpp"
#include "app/viewmodel/hierarchy/preset/PresetHierarchyViewModelSupport.hpp"
#include "app/viewmodel/hierarchy/progress/ProgressHierarchyViewModelSupport.hpp"
#include "app/viewmodel/hierarchy/projects/ProjectsHierarchyViewModelSupport.hpp"
#include "app/viewmodel/hierarchy/resources/ResourcesHierarchyViewModelSupport.hpp"
#include "app/viewmodel/hierarchy/tags/TagsHierarchyViewModelSupport.hpp"

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

    template <typename Resolver>
    bool resolveSharedContentsDirectories(
        const QString& wshubPath,
        QStringList* contentsDirectories,
        QString* errorMessage,
        Resolver resolver)
    {
        if (contentsDirectories == nullptr)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Contents directory output is null.");
            }
            return false;
        }

        QStringList resolvedDirectories;
        QString resolveError;
        if (!resolver(wshubPath, &resolvedDirectories, &resolveError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = resolveError;
            }
            return false;
        }

        *contentsDirectories = resolvedDirectories;
        if (errorMessage != nullptr)
        {
            errorMessage->clear();
        }
        return true;
    }
} // namespace

WhatSonRuntimeDomainSnapshots::SharedContext WhatSonRuntimeDomainSnapshots::buildSharedContext(const QString& wshubPath)
{
    SharedContext context;
    context.normalizedHubPath = wshubPath.trimmed().isEmpty()
                                    ? QString()
                                    : WhatSon::HubPath::normalizeAbsolutePath(wshubPath);
    if (context.normalizedHubPath.isEmpty())
    {
        context.error = QStringLiteral("wshubPath is empty.");
        return context;
    }

    if (!resolveSharedContentsDirectories(
            context.normalizedHubPath,
            &context.contentsDirectories,
            &context.error,
            [](const QString& hubPath, QStringList* directories, QString* error)
            {
                return WhatSon::Hierarchy::LibrarySupport::resolveContentsDirectories(hubPath, directories, error);
            }))
    {
        return context;
    }

    context.succeeded = true;
    return context;
}

WhatSonRuntimeDomainSnapshots::LibrarySnapshot WhatSonRuntimeDomainSnapshots::loadLibrary(const QString& wshubPath)
{
    return loadLibrary(buildSharedContext(wshubPath));
}

WhatSonRuntimeDomainSnapshots::LibrarySnapshot WhatSonRuntimeDomainSnapshots::loadLibrary(const SharedContext& context)
{
    LibrarySnapshot snapshot;
    if (!context.succeeded)
    {
        snapshot.succeeded = false;
        snapshot.error = context.error;
        return snapshot;
    }

    WhatSonLibraryIndexedState indexedState;
    QString indexError;
    if (!indexedState.indexFromWshub(context.normalizedHubPath, &indexError))
    {
        snapshot.succeeded = false;
        snapshot.error = indexError;
        return snapshot;
    }

    const WhatSonLibraryIndexedState::Snapshot indexedSnapshot = indexedState.snapshot();
    snapshot.allNotes = indexedSnapshot.allNotes;
    snapshot.draftNotes = indexedSnapshot.draftNotes;
    snapshot.todayNotes = indexedSnapshot.todayNotes;

    WhatSonFoldersHierarchyParser foldersParser;
    for (const QString& contentsDirectory : context.contentsDirectories)
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
        snapshot.foldersFilePath = defaultSourceFilePath(context.contentsDirectories, QStringLiteral("Folders.wsfolders"));
    }

    snapshot.succeeded = true;
    return snapshot;
}

WhatSonRuntimeDomainSnapshots::BookmarksSnapshot WhatSonRuntimeDomainSnapshots::buildBookmarks(
    const QVector<LibraryNoteRecord>& allNotes)
{
    BookmarksSnapshot snapshot;
    snapshot.bookmarkedNotes = WhatSonLibraryIndexedState::collectBookmarkedNotes(allNotes);
    snapshot.succeeded = true;
    return snapshot;
}

WhatSonRuntimeDomainSnapshots::BookmarksSnapshot WhatSonRuntimeDomainSnapshots::loadBookmarks(const QString& wshubPath)
{
    return loadBookmarks(buildSharedContext(wshubPath));
}

WhatSonRuntimeDomainSnapshots::BookmarksSnapshot WhatSonRuntimeDomainSnapshots::loadBookmarks(const SharedContext& context)
{
    if (!context.succeeded)
    {
        BookmarksSnapshot snapshot;
        snapshot.succeeded = false;
        snapshot.error = context.error;
        return snapshot;
    }

    WhatSonLibraryIndexedState indexedState;
    QString indexError;
    if (!indexedState.indexFromWshub(context.normalizedHubPath, &indexError))
    {
        BookmarksSnapshot snapshot;
        snapshot.succeeded = false;
        snapshot.error = indexError;
        return snapshot;
    }

    return buildBookmarks(indexedState.allNotes());
}

WhatSonRuntimeDomainSnapshots::ProjectsSnapshot WhatSonRuntimeDomainSnapshots::loadProjects(const QString& wshubPath)
{
    return loadProjects(buildSharedContext(wshubPath));
}

WhatSonRuntimeDomainSnapshots::ProjectsSnapshot WhatSonRuntimeDomainSnapshots::loadProjects(const SharedContext& context)
{
    ProjectsSnapshot snapshot;
    if (!context.succeeded)
    {
        snapshot.succeeded = false;
        snapshot.error = context.error;
        return snapshot;
    }

    WhatSonProjectsHierarchyParser parser;
    for (const QString& contentsDirectory : context.contentsDirectories)
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
        snapshot.projectsFilePath = defaultSourceFilePath(context.contentsDirectories, QStringLiteral("ProjectLists.wsproj"));
    }

    snapshot.succeeded = true;
    return snapshot;
}

WhatSonRuntimeDomainSnapshots::StringListSnapshot WhatSonRuntimeDomainSnapshots::loadResources(const QString& wshubPath)
{
    return loadResources(buildSharedContext(wshubPath));
}

WhatSonRuntimeDomainSnapshots::StringListSnapshot WhatSonRuntimeDomainSnapshots::loadResources(const SharedContext& context)
{
    StringListSnapshot snapshot;
    if (!context.succeeded)
    {
        snapshot.succeeded = false;
        snapshot.error = context.error;
        return snapshot;
    }

    WhatSonResourcesHierarchyParser parser;
    for (const QString& contentsDirectory : context.contentsDirectories)
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

    if (snapshot.values.isEmpty())
    {
        snapshot.values = WhatSon::Resources::listRelativeResourcePackagePathsForHub(context.normalizedHubPath);
    }

    if (snapshot.sourceFilePath.isEmpty())
    {
        snapshot.sourceFilePath = defaultSourceFilePath(context.contentsDirectories, QStringLiteral("Resources.wsresources"));
    }

    snapshot.succeeded = true;
    return snapshot;
}

WhatSonRuntimeDomainSnapshots::ProgressSnapshot WhatSonRuntimeDomainSnapshots::loadProgress(const QString& wshubPath)
{
    return loadProgress(buildSharedContext(wshubPath));
}

WhatSonRuntimeDomainSnapshots::ProgressSnapshot WhatSonRuntimeDomainSnapshots::loadProgress(const SharedContext& context)
{
    ProgressSnapshot snapshot;
    if (!context.succeeded)
    {
        snapshot.succeeded = false;
        snapshot.error = context.error;
        return snapshot;
    }

    WhatSonProgressHierarchyParser parser;
    WhatSonProgressHierarchyStore store;

    for (const QString& contentsDirectory : context.contentsDirectories)
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
        snapshot.sourceFilePath = defaultSourceFilePath(context.contentsDirectories, QStringLiteral("Progress.wsprogress"));
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
    return loadEvent(buildSharedContext(wshubPath));
}

WhatSonRuntimeDomainSnapshots::StringListSnapshot WhatSonRuntimeDomainSnapshots::loadEvent(const SharedContext& context)
{
    StringListSnapshot snapshot;
    if (!context.succeeded)
    {
        snapshot.succeeded = false;
        snapshot.error = context.error;
        return snapshot;
    }

    WhatSonEventHierarchyParser parser;
    for (const QString& contentsDirectory : context.contentsDirectories)
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
        snapshot.sourceFilePath = defaultSourceFilePath(context.contentsDirectories, QStringLiteral("Event.wsevent"));
    }

    snapshot.succeeded = true;
    return snapshot;
}

WhatSonRuntimeDomainSnapshots::StringListSnapshot WhatSonRuntimeDomainSnapshots::loadPreset(const QString& wshubPath)
{
    return loadPreset(buildSharedContext(wshubPath));
}

WhatSonRuntimeDomainSnapshots::StringListSnapshot WhatSonRuntimeDomainSnapshots::loadPreset(const SharedContext& context)
{
    StringListSnapshot snapshot;
    if (!context.succeeded)
    {
        snapshot.succeeded = false;
        snapshot.error = context.error;
        return snapshot;
    }

    WhatSonPresetHierarchyParser parser;
    for (const QString& contentsDirectory : context.contentsDirectories)
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
        snapshot.sourceFilePath = defaultSourceFilePath(context.contentsDirectories, QStringLiteral("Preset.wspreset"));
    }

    snapshot.succeeded = true;
    return snapshot;
}

WhatSonRuntimeDomainSnapshots::TagsSnapshot WhatSonRuntimeDomainSnapshots::loadTags(const QString& wshubPath)
{
    return loadTags(buildSharedContext(wshubPath));
}

WhatSonRuntimeDomainSnapshots::TagsSnapshot WhatSonRuntimeDomainSnapshots::loadTags(const SharedContext& context)
{
    TagsSnapshot snapshot;
    if (!context.succeeded)
    {
        snapshot.succeeded = false;
        snapshot.error = context.error;
        return snapshot;
    }

    WhatSonTagsHierarchyParser parser;
    for (const QString& contentsDirectory : context.contentsDirectories)
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
        snapshot.tagsFilePath = defaultSourceFilePath(context.contentsDirectories, QStringLiteral("Tags.wstags"));
    }

    snapshot.succeeded = true;
    return snapshot;
}

WhatSonRuntimeDomainSnapshots::HubRuntimeSnapshot WhatSonRuntimeDomainSnapshots::loadHubRuntime(
    const QString& wshubPath)
{
    return loadHubRuntime(buildSharedContext(wshubPath));
}

WhatSonRuntimeDomainSnapshots::HubRuntimeSnapshot WhatSonRuntimeDomainSnapshots::loadHubRuntime(
    const SharedContext& context)
{
    HubRuntimeSnapshot snapshot;
    if (!context.succeeded)
    {
        snapshot.succeeded = false;
        snapshot.error = context.error;
        return snapshot;
    }

    snapshot.succeeded = snapshot.store.loadFromWshub(context.normalizedHubPath, &snapshot.error);
    return snapshot;
}
