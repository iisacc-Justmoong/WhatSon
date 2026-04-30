#pragma once

#include "app/runtime/threading/IWhatSonRuntimeParallelLoader.hpp"

#include <QString>

class LibraryHierarchyController;
class ProjectsHierarchyController;
class BookmarksHierarchyController;
class TagsHierarchyController;
class ResourcesHierarchyController;
class ProgressHierarchyController;
class EventHierarchyController;
class PresetHierarchyController;
class WhatSonHubRuntimeStore;

class WhatSonStartupRuntimeCoordinator final
{
public:
    using RuntimeTargets = IWhatSonRuntimeParallelLoader::Targets;

    WhatSonStartupRuntimeCoordinator();
    explicit WhatSonStartupRuntimeCoordinator(const RuntimeTargets& targets);

    void setTargets(const RuntimeTargets& targets);
    void setParallelLoader(const IWhatSonRuntimeParallelLoader* loader);
    bool loadHubIntoRuntime(const QString& hubPath, QString* errorMessage = nullptr);
    bool reloadResourcesDomainIntoRuntime(const QString& hubPath, QString* errorMessage = nullptr);

private:
    void applyHubRuntimeState(
        const QString& normalizedHubPath,
        const IWhatSonRuntimeParallelLoader::RequestedDomains& requestedDomains) const;
    bool loadHubIntoRuntimeWithRequestedDomains(
        const QString& hubPath,
        const IWhatSonRuntimeParallelLoader::RequestedDomains& requestedDomains,
        QString* errorMessage);

    RuntimeTargets m_targets;
    const IWhatSonRuntimeParallelLoader* m_parallelLoader = nullptr;
};
