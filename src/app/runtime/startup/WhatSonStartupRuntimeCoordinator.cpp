#include "app/runtime/startup/WhatSonStartupRuntimeCoordinator.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/models/file/hub/WhatSonHubPathUtils.hpp"
#include "app/models/file/hub/WhatSonHubRuntimeStore.hpp"
#include "app/models/file/hierarchy/bookmarks/BookmarksHierarchyController.hpp"
#include "app/models/file/hierarchy/event/EventHierarchyController.hpp"
#include "app/models/file/hierarchy/library/LibraryHierarchyController.hpp"
#include "app/models/file/hierarchy/preset/PresetHierarchyController.hpp"
#include "app/models/file/hierarchy/progress/ProgressHierarchyController.hpp"
#include "app/models/file/hierarchy/projects/ProjectsHierarchyController.hpp"
#include "app/models/file/hierarchy/resources/ResourcesHierarchyController.hpp"
#include "app/models/file/hierarchy/tags/TagsHierarchyController.hpp"

#include <QDebug>
#include <QStringList>

WhatSonStartupRuntimeCoordinator::WhatSonStartupRuntimeCoordinator() = default;

WhatSonStartupRuntimeCoordinator::WhatSonStartupRuntimeCoordinator(const RuntimeTargets& targets)
    : m_targets(targets)
{
}

void WhatSonStartupRuntimeCoordinator::setTargets(const RuntimeTargets& targets)
{
    m_targets = targets;
}

void WhatSonStartupRuntimeCoordinator::setParallelLoader(const IWhatSonRuntimeParallelLoader* loader)
{
    m_parallelLoader = loader;
}

void WhatSonStartupRuntimeCoordinator::applyHubRuntimeState(
    const QString& normalizedHubPath,
    const IWhatSonRuntimeParallelLoader::RequestedDomains& requestedDomains) const
{
    if (m_targets.hubRuntimeStore == nullptr || m_targets.libraryController == nullptr)
    {
        return;
    }

    m_targets.libraryController->setHubStore(m_targets.hubRuntimeStore->hub(normalizedHubPath));

    if (m_targets.tagsController != nullptr)
    {
        m_targets.tagsController->setTagDepthEntries(
            m_targets.hubRuntimeStore->tagDepthEntries(normalizedHubPath));
        WhatSon::Debug::trace(
            QStringLiteral("startup.runtime"),
            requestedDomains.tags
                ? QStringLiteral("applyTagsDepthEntries.success")
                : QStringLiteral("applyTagsDepthEntries.deferredReady"),
            QStringLiteral("itemCount=%1").arg(m_targets.tagsController->itemModel()->rowCount()));
    }
}

bool WhatSonStartupRuntimeCoordinator::loadHubIntoRuntimeWithRequestedDomains(
    const QString& hubPath,
    const IWhatSonRuntimeParallelLoader::RequestedDomains& requestedDomains,
    QString* errorMessage)
{
    const QString normalizedHubPath = hubPath.trimmed().isEmpty()
                                          ? QString()
                                          : WhatSon::HubPath::normalizeAbsolutePath(hubPath);
    if (normalizedHubPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Hub path must not be empty.");
        }
        return false;
    }

    WhatSon::Debug::trace(
        QStringLiteral("startup.runtime"),
        QStringLiteral("loadFromWshub.begin"),
        QStringLiteral("path=%1").arg(normalizedHubPath));

    if (m_parallelLoader == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Startup runtime loader interface is not configured.");
        }
        return false;
    }

    IWhatSonRuntimeParallelLoader::Targets targets;
    targets.libraryController = m_targets.libraryController;
    targets.projectsController = m_targets.projectsController;
    targets.bookmarksController = m_targets.bookmarksController;
    targets.tagsController = m_targets.tagsController;
    targets.resourcesController = m_targets.resourcesController;
    targets.progressController = m_targets.progressController;
    targets.eventController = m_targets.eventController;
    targets.presetController = m_targets.presetController;
    targets.hubRuntimeStore = m_targets.hubRuntimeStore;

    QVector<IWhatSonRuntimeParallelLoader::DomainLoadResult> loadResults;
    const bool loadSucceeded = m_parallelLoader->loadFromWshub(
        normalizedHubPath,
        targets,
        requestedDomains,
        &loadResults);

    const bool hubRuntimeRequested = requestedDomains.hubRuntimeStore;
    bool hubRuntimeLoadSucceeded = !hubRuntimeRequested;
    QStringList failedDomains;
    for (const IWhatSonRuntimeParallelLoader::DomainLoadResult& result : loadResults)
    {
        if (result.domain == QStringLiteral("hub.runtime"))
        {
            hubRuntimeLoadSucceeded = result.succeeded;
        }

        if (result.succeeded)
        {
            WhatSon::Debug::trace(
                QStringLiteral("startup.runtime"),
                QStringLiteral("load.success"),
                QStringLiteral("domain=%1").arg(result.domain));
            continue;
        }

        failedDomains.push_back(result.domain);
        const QString domainErrorMessage = result.error.trimmed().isEmpty()
                                               ? QStringLiteral("unknown load error")
                                               : result.error.trimmed();
        qWarning().noquote()
            << QStringLiteral("Failed to load domain '%1' from .wshub: %2")
                   .arg(result.domain, domainErrorMessage);
        WhatSon::Debug::trace(
            QStringLiteral("startup.runtime"),
            QStringLiteral("load.failed"),
            QStringLiteral("domain=%1 reason=%2").arg(result.domain, domainErrorMessage));
    }

    if (!loadSucceeded)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = failedDomains.isEmpty()
                                ? QStringLiteral("Failed to load the selected WhatSon Hub.")
                                : QStringLiteral("Failed to load .wshub domains: %1")
                                      .arg(failedDomains.join(QStringLiteral(", ")));
        }
        return false;
    }

    if (hubRuntimeRequested && !hubRuntimeLoadSucceeded)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to load the selected WhatSon Hub runtime state.");
        }
        return false;
    }

    if (hubRuntimeLoadSucceeded)
    {
        applyHubRuntimeState(normalizedHubPath, requestedDomains);
    }
    else if (hubRuntimeRequested)
    {
        WhatSon::Debug::trace(
            QStringLiteral("startup.runtime"),
            QStringLiteral("applyTagsDepthEntries.skipped"),
            QStringLiteral("reason=hub.runtime load failed"));
    }

    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }
    return true;
}

bool WhatSonStartupRuntimeCoordinator::loadHubIntoRuntime(const QString& hubPath, QString* errorMessage)
{
    return loadHubIntoRuntimeWithRequestedDomains(
        hubPath,
        IWhatSonRuntimeParallelLoader::RequestedDomains{},
        errorMessage);
}

bool WhatSonStartupRuntimeCoordinator::reloadResourcesDomainIntoRuntime(const QString& hubPath, QString* errorMessage)
{
    IWhatSonRuntimeParallelLoader::RequestedDomains requestedDomains;
    requestedDomains.library = false;
    requestedDomains.projects = false;
    requestedDomains.bookmarks = false;
    requestedDomains.tags = false;
    requestedDomains.resources = true;
    requestedDomains.progress = false;
    requestedDomains.event = false;
    requestedDomains.preset = false;
    requestedDomains.hubRuntimeStore = true;
    return loadHubIntoRuntimeWithRequestedDomains(hubPath, requestedDomains, errorMessage);
}
