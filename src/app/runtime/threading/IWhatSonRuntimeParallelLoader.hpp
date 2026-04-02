#pragma once

#include <QString>
#include <QVector>

class LibraryHierarchyViewModel;
class ProjectsHierarchyViewModel;
class BookmarksHierarchyViewModel;
class TagsHierarchyViewModel;
class ResourcesHierarchyViewModel;
class ProgressHierarchyViewModel;
class EventHierarchyViewModel;
class PresetHierarchyViewModel;
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
        LibraryHierarchyViewModel* libraryViewModel = nullptr;
        ProjectsHierarchyViewModel* projectsViewModel = nullptr;
        BookmarksHierarchyViewModel* bookmarksViewModel = nullptr;
        TagsHierarchyViewModel* tagsViewModel = nullptr;
        ResourcesHierarchyViewModel* resourcesViewModel = nullptr;
        ProgressHierarchyViewModel* progressViewModel = nullptr;
        EventHierarchyViewModel* eventViewModel = nullptr;
        PresetHierarchyViewModel* presetViewModel = nullptr;
        WhatSonHubRuntimeStore* hubRuntimeStore = nullptr;
    };

    virtual ~IWhatSonRuntimeParallelLoader() = default;

    virtual bool loadFromWshub(
        const QString& wshubPath,
        const Targets& targets,
        const RequestedDomains& requestedDomains,
        QVector<DomainLoadResult>* outResults = nullptr) const = 0;
};
