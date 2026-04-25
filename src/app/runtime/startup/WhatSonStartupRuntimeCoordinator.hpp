#pragma once

#include "app/runtime/threading/IWhatSonRuntimeParallelLoader.hpp"

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
class IActiveHierarchySource;

class WhatSonStartupRuntimeCoordinator final
{
public:
    using RuntimeTargets = IWhatSonRuntimeParallelLoader::Targets;

    WhatSonStartupRuntimeCoordinator();
    explicit WhatSonStartupRuntimeCoordinator(const RuntimeTargets& targets);

    void setTargets(const RuntimeTargets& targets);
    void setParallelLoader(const IWhatSonRuntimeParallelLoader* loader);
    bool loadHubIntoRuntime(const QString& hubPath, QString* errorMessage = nullptr);
    bool loadStartupHubIntoRuntime(const QString& hubPath, QString* errorMessage = nullptr);
    bool reloadResourcesDomainIntoRuntime(const QString& hubPath, QString* errorMessage = nullptr);
    bool ensureDeferredStartupHierarchyLoaded(int hierarchyIndex, const QString& reason);
    void bindSidebarActivation(IActiveHierarchySource* sidebarHierarchyViewModel);

    bool startupDeferredBootstrapActive() const noexcept;
    QSet<int> startupLoadedHierarchyIndices() const;
    QString currentLoadedHubPath() const;

private:
    void disableStartupDeferredBootstrap();
    void applyHubRuntimeState(
        const QString& normalizedHubPath,
        const IWhatSonRuntimeParallelLoader::RequestedDomains& requestedDomains) const;
    bool loadHubIntoRuntimeWithRequestedDomains(
        const QString& hubPath,
        const IWhatSonRuntimeParallelLoader::RequestedDomains& requestedDomains,
        QString* errorMessage);

    RuntimeTargets m_targets;
    const IWhatSonRuntimeParallelLoader* m_parallelLoader = nullptr;
    QString m_currentLoadedHubPath;
    bool m_startupDeferredBootstrapActive = false;
    QSet<int> m_startupLoadedHierarchyIndices;
};
