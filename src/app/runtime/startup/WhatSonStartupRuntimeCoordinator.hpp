#pragma once

#include "runtime/threading/WhatSonRuntimeParallelLoader.hpp"

#include <QSet>
#include <QString>

class LibraryHierarchyViewModel;
class ProjectsHierarchyViewModel;
class BookmarksHierarchyViewModel;
class TagsHierarchyViewModel;
class ResourcesHierarchyViewModel;
class ProgressHierarchyViewModel;
class EventHierarchyViewModel;
class PresetHierarchyViewModel;
class WhatSonHubRuntimeStore;
class SidebarHierarchyViewModel;

class WhatSonStartupRuntimeCoordinator final
{
public:
    struct RuntimeTargets
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

    WhatSonStartupRuntimeCoordinator();
    explicit WhatSonStartupRuntimeCoordinator(const RuntimeTargets& targets);

    void setTargets(const RuntimeTargets& targets);
    bool loadHubIntoRuntime(const QString& hubPath, QString* errorMessage = nullptr);
    bool loadStartupHubIntoRuntime(const QString& hubPath, QString* errorMessage = nullptr);
    bool reloadResourcesDomainIntoRuntime(const QString& hubPath, QString* errorMessage = nullptr);
    void ensureDeferredStartupHierarchyLoaded(int hierarchyIndex, const QString& reason);
    void bindSidebarActivation(SidebarHierarchyViewModel* sidebarHierarchyViewModel);

    bool startupDeferredBootstrapActive() const noexcept;
    QSet<int> startupLoadedHierarchyIndices() const;
    QString currentLoadedHubPath() const;

private:
    void disableStartupDeferredBootstrap();
    bool loadHubIntoRuntimeWithRequestedDomains(
        const QString& hubPath,
        const WhatSonRuntimeParallelLoader::RequestedDomains& requestedDomains,
        QString* errorMessage);

    RuntimeTargets m_targets;
    QString m_currentLoadedHubPath;
    bool m_startupDeferredBootstrapActive = false;
    QSet<int> m_startupLoadedHierarchyIndices;
};
