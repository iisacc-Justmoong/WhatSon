#pragma once

#include <QString>
#include <QVector>

class LibraryHierarchyController;
class ProjectsHierarchyController;
class BookmarksHierarchyController;
class TagsHierarchyController;
class ResourcesHierarchyController;
class ProgressHierarchyController;
class EventHierarchyController;
class PresetHierarchyController;
class WhatSonHubRuntimeStore;

class IWhatSonRuntimeParallelLoader
{
public:
    struct RequestedDomains
    {
        bool library = true;
        bool projects = true;
        bool bookmarks = true;
        bool tags = true;
        bool resources = true;
        bool progress = true;
        bool event = true;
        bool preset = true;
        bool hubRuntimeStore = true;
    };

    struct DomainLoadResult
    {
        QString domain;
        bool succeeded = false;
        QString error;
    };

    struct Targets
    {
        LibraryHierarchyController* libraryController = nullptr;
        ProjectsHierarchyController* projectsController = nullptr;
        BookmarksHierarchyController* bookmarksController = nullptr;
        TagsHierarchyController* tagsController = nullptr;
        ResourcesHierarchyController* resourcesController = nullptr;
        ProgressHierarchyController* progressController = nullptr;
        EventHierarchyController* eventController = nullptr;
        PresetHierarchyController* presetController = nullptr;
        WhatSonHubRuntimeStore* hubRuntimeStore = nullptr;
    };

    virtual ~IWhatSonRuntimeParallelLoader() = default;

    virtual bool loadFromWshub(
        const QString& wshubPath,
        const Targets& targets,
        const RequestedDomains& requestedDomains,
        QVector<DomainLoadResult>* outResults = nullptr) const = 0;
};
