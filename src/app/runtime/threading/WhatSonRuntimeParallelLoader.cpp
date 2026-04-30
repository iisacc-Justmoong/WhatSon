#include "app/runtime/threading/WhatSonRuntimeParallelLoader.hpp"

#include "backend/runtime/bootstrapparallel.h"

#include "app/runtime/threading/WhatSonRuntimeDomainSnapshots.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/models/file/hub/WhatSonHubRuntimeStore.hpp"
#include "app/models/file/hierarchy/bookmarks/BookmarksHierarchyController.hpp"
#include "app/models/file/hierarchy/event/EventHierarchyController.hpp"
#include "app/models/file/hierarchy/library/LibraryHierarchyController.hpp"
#include "app/models/file/hierarchy/preset/PresetHierarchyController.hpp"
#include "app/models/file/hierarchy/progress/ProgressHierarchyController.hpp"
#include "app/models/file/hierarchy/projects/ProjectsHierarchyController.hpp"
#include "app/models/file/hierarchy/resources/ResourcesHierarchyController.hpp"
#include "app/models/file/hierarchy/tags/TagsHierarchyController.hpp"

#include <QElapsedTimer>
#include <utility>

namespace
{
    constexpr int kDomainTaskPriorityStep = 10;

    WhatSonRuntimeParallelLoader::DomainLoadResult domainLoadResultFromBootstrapResult(
        const QString& domain,
        const lvrs::BootstrapParallelTaskResult& taskResult)
    {
        WhatSonRuntimeParallelLoader::DomainLoadResult result;
        result.domain = domain;
        result.succeeded = taskResult.loadOk;
        result.error = taskResult.errorMessage.trimmed();
        return result;
    }
} // namespace

bool WhatSonRuntimeParallelLoader::loadFromWshub(
    const QString& wshubPath,
    const Targets& targets,
    const RequestedDomains& requestedDomains,
    QVector<DomainLoadResult>* outResults) const
{
    QElapsedTimer totalElapsedTimer;
    totalElapsedTimer.start();

    const QString normalizedPath = wshubPath.trimmed();
    QVector<DomainLoadResult> results;
    results.reserve(9);

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("runtime.parallel"),
                              QStringLiteral("loadFromWshub.begin"),
                              QStringLiteral("path=%1").arg(normalizedPath));

    if (normalizedPath.isEmpty())
    {
        DomainLoadResult result;
        result.domain = QStringLiteral("runtime");
        result.succeeded = false;
        result.error = QStringLiteral("wshubPath is empty.");
        results.push_back(result);
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("runtime.parallel"),
                                  QStringLiteral("loadFromWshub.failed"),
                                  QStringLiteral("reason=empty wshubPath"));
        if (outResults != nullptr)
        {
            *outResults = results;
        }
        return false;
    }

    WhatSonRuntimeDomainSnapshots::LibrarySnapshot librarySnapshot;
    WhatSonRuntimeDomainSnapshots::ProjectsSnapshot projectsSnapshot;
    WhatSonRuntimeDomainSnapshots::BookmarksSnapshot bookmarksSnapshot;
    WhatSonRuntimeDomainSnapshots::TagsSnapshot tagsSnapshot;
    WhatSonRuntimeDomainSnapshots::StringListSnapshot resourcesSnapshot;
    WhatSonRuntimeDomainSnapshots::ProgressSnapshot progressSnapshot;
    WhatSonRuntimeDomainSnapshots::StringListSnapshot eventSnapshot;
    WhatSonRuntimeDomainSnapshots::StringListSnapshot presetSnapshot;
    WhatSonRuntimeDomainSnapshots::HubRuntimeSnapshot hubRuntimeSnapshot;

    bool hasLibraryTask = false;
    bool hasProjectsTask = false;
    bool hasBookmarksTask = false;
    bool deriveBookmarksFromLibrary = false;
    bool hasTagsTask = false;
    bool hasResourcesTask = false;
    bool hasProgressTask = false;
    bool hasEventTask = false;
    bool hasPresetTask = false;

    const WhatSonRuntimeDomainSnapshots::SharedContext sharedContext =
        WhatSonRuntimeDomainSnapshots::buildSharedContext(normalizedPath);

    QList<lvrs::BootstrapParallelTask> bootstrapTasks;
    bootstrapTasks.reserve(9);
    QVector<int> bootstrapTaskResultIndexes;
    bootstrapTaskResultIndexes.reserve(9);
    int derivedBookmarksResultIndex = -1;

    auto addImmediateFailure = [&results](const QString& domain, const QString& error)
    {
        DomainLoadResult result;
        result.domain = domain;
        result.succeeded = false;
        result.error = error;
        results.push_back(std::move(result));
    };

    auto addSnapshotTask = [
        &results,
        &bootstrapTasks,
        &bootstrapTaskResultIndexes,
        &sharedContext,
        &normalizedPath](
        const QString& domain,
        auto* snapshot,
        auto loader)
    {
        results.push_back(DomainLoadResult{});
        results.back().domain = domain;
        const int resultIndex = results.size() - 1;

        lvrs::BootstrapParallelTask task;
        task.name = domain;
        task.priority = bootstrapTasks.size() * kDomainTaskPriorityStep;
        task.fatal = true;
        task.metadata.insert(QStringLiteral("path"), normalizedPath);
        task.metadata.insert(QStringLiteral("domain"), domain);
        task.load = [
            snapshot,
            loader,
            sharedContext,
            domain,
            normalizedPath](const lvrs::BootstrapParallelTaskContext&, QVariant*, QString* error) -> bool
        {
            WhatSon::Debug::trace(
                QStringLiteral("runtime.parallel"),
                QStringLiteral("task.begin"),
                QStringLiteral("domain=%1 path=%2").arg(domain, normalizedPath));

            *snapshot = loader(sharedContext);
            if (!snapshot->succeeded && error != nullptr)
            {
                *error = snapshot->error;
            }

            WhatSon::Debug::trace(
                QStringLiteral("runtime.parallel"),
                QStringLiteral("load.completed"),
                QStringLiteral("domain=%1 succeeded=%2 error=%3")
                .arg(domain)
                .arg(snapshot->succeeded ? QStringLiteral("1") : QStringLiteral("0"))
                .arg(snapshot->error.trimmed()));
            return snapshot->succeeded;
        };

        WhatSon::Debug::trace(
            QStringLiteral("runtime.parallel"),
            QStringLiteral("task.queued"),
            QStringLiteral("domain=%1 path=%2").arg(domain, normalizedPath));
        bootstrapTaskResultIndexes.push_back(resultIndex);
        bootstrapTasks.append(std::move(task));
    };

    if (requestedDomains.library)
    {
        if (targets.libraryController == nullptr)
        {
            addImmediateFailure(QStringLiteral("library"), QStringLiteral("Target controller is null."));
        }
        else
        {
            hasLibraryTask = true;
            addSnapshotTask(
                QStringLiteral("library"),
                &librarySnapshot,
                [](const WhatSonRuntimeDomainSnapshots::SharedContext& context)
                {
                    return WhatSonRuntimeDomainSnapshots::loadLibrary(context);
                });
        }
    }

    if (requestedDomains.projects)
    {
        if (targets.projectsController == nullptr)
        {
            addImmediateFailure(QStringLiteral("projects"), QStringLiteral("Target controller is null."));
        }
        else
        {
            hasProjectsTask = true;
            addSnapshotTask(
                QStringLiteral("projects"),
                &projectsSnapshot,
                [](const WhatSonRuntimeDomainSnapshots::SharedContext& context)
                {
                    return WhatSonRuntimeDomainSnapshots::loadProjects(context);
                });
        }
    }

    if (requestedDomains.bookmarks)
    {
        if (targets.bookmarksController == nullptr)
        {
            addImmediateFailure(QStringLiteral("bookmarks"), QStringLiteral("Target controller is null."));
        }
        else
        {
            hasBookmarksTask = true;
            if (hasLibraryTask)
            {
                deriveBookmarksFromLibrary = true;
                DomainLoadResult result;
                result.domain = QStringLiteral("bookmarks");
                results.push_back(result);
                derivedBookmarksResultIndex = results.size() - 1;
            }
            else
            {
                addSnapshotTask(
                    QStringLiteral("bookmarks"),
                    &bookmarksSnapshot,
                    [](const WhatSonRuntimeDomainSnapshots::SharedContext& context)
                    {
                        return WhatSonRuntimeDomainSnapshots::loadBookmarks(context);
                    });
            }
        }
    }

    if (requestedDomains.tags)
    {
        if (targets.tagsController == nullptr)
        {
            addImmediateFailure(QStringLiteral("tags"), QStringLiteral("Target controller is null."));
        }
        else
        {
            hasTagsTask = true;
            addSnapshotTask(
                QStringLiteral("tags"),
                &tagsSnapshot,
                [](const WhatSonRuntimeDomainSnapshots::SharedContext& context)
                {
                    return WhatSonRuntimeDomainSnapshots::loadTags(context);
                });
        }
    }

    if (requestedDomains.resources)
    {
        if (targets.resourcesController == nullptr)
        {
            addImmediateFailure(QStringLiteral("resources"), QStringLiteral("Target controller is null."));
        }
        else
        {
            hasResourcesTask = true;
            addSnapshotTask(
                QStringLiteral("resources"),
                &resourcesSnapshot,
                [](const WhatSonRuntimeDomainSnapshots::SharedContext& context)
                {
                    return WhatSonRuntimeDomainSnapshots::loadResources(context);
                });
        }
    }

    if (requestedDomains.progress)
    {
        if (targets.progressController == nullptr)
        {
            addImmediateFailure(QStringLiteral("progress"), QStringLiteral("Target controller is null."));
        }
        else
        {
            hasProgressTask = true;
            addSnapshotTask(
                QStringLiteral("progress"),
                &progressSnapshot,
                [](const WhatSonRuntimeDomainSnapshots::SharedContext& context)
                {
                    return WhatSonRuntimeDomainSnapshots::loadProgress(context);
                });
        }
    }

    if (requestedDomains.event)
    {
        if (targets.eventController == nullptr)
        {
            addImmediateFailure(QStringLiteral("event"), QStringLiteral("Target controller is null."));
        }
        else
        {
            hasEventTask = true;
            addSnapshotTask(
                QStringLiteral("event"),
                &eventSnapshot,
                [](const WhatSonRuntimeDomainSnapshots::SharedContext& context)
                {
                    return WhatSonRuntimeDomainSnapshots::loadEvent(context);
                });
        }
    }

    if (requestedDomains.preset)
    {
        if (targets.presetController == nullptr)
        {
            addImmediateFailure(QStringLiteral("preset"), QStringLiteral("Target controller is null."));
        }
        else
        {
            hasPresetTask = true;
            addSnapshotTask(
                QStringLiteral("preset"),
                &presetSnapshot,
                [](const WhatSonRuntimeDomainSnapshots::SharedContext& context)
                {
                    return WhatSonRuntimeDomainSnapshots::loadPreset(context);
                });
        }
    }

    if (requestedDomains.hubRuntimeStore)
    {
        if (targets.hubRuntimeStore == nullptr)
        {
            addImmediateFailure(QStringLiteral("hub.runtime"), QStringLiteral("Target runtime store is null."));
        }
        else
        {
            addSnapshotTask(
                QStringLiteral("hub.runtime"),
                &hubRuntimeSnapshot,
                [](const WhatSonRuntimeDomainSnapshots::SharedContext& context)
                {
                    return WhatSonRuntimeDomainSnapshots::loadHubRuntime(context);
                });
        }
    }

    lvrs::BootstrapParallelRunOptions bootstrapOptions;
    bootstrapOptions.skipApplyOnLoadFailure = true;
    bootstrapOptions.logDiagnostics = true;

    const lvrs::BootstrapParallelRunResult bootstrapResult =
        lvrs::runBootstrapParallelTasks(bootstrapTasks, bootstrapOptions);
    for (const lvrs::BootstrapParallelTaskResult& taskResult : bootstrapResult.taskResults)
    {
        if (taskResult.index < 0 || taskResult.index >= bootstrapTaskResultIndexes.size())
        {
            continue;
        }

        const int resultIndex = bootstrapTaskResultIndexes.at(taskResult.index);
        if (resultIndex < 0 || resultIndex >= results.size())
        {
            continue;
        }

        results[resultIndex] = domainLoadResultFromBootstrapResult(
            results.at(resultIndex).domain,
            taskResult);
    }

    if (deriveBookmarksFromLibrary)
    {
        if (librarySnapshot.succeeded)
        {
            bookmarksSnapshot = WhatSonRuntimeDomainSnapshots::buildBookmarks(librarySnapshot.allNotes);
        }
        else
        {
            bookmarksSnapshot.succeeded = false;
            bookmarksSnapshot.error = librarySnapshot.error.trimmed().isEmpty()
                                          ? QStringLiteral("Library snapshot failed while deriving bookmarks.")
                                          : librarySnapshot.error;
            bookmarksSnapshot.bookmarkedNotes.clear();
        }

        if (derivedBookmarksResultIndex >= 0 && derivedBookmarksResultIndex < results.size())
        {
            DomainLoadResult& bookmarksResult = results[derivedBookmarksResultIndex];
            bookmarksResult.succeeded = bookmarksSnapshot.succeeded;
            bookmarksResult.error = bookmarksSnapshot.error;
        }
    }

    bool allSucceeded = true;
    int failedCount = 0;
    for (const DomainLoadResult& result : results)
    {
        if (!result.succeeded)
        {
            allSucceeded = false;
            ++failedCount;
        }
    }

    if (outResults != nullptr)
    {
        *outResults = results;
    }

    if (!allSucceeded)
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("runtime.parallel"),
                                  QStringLiteral("loadFromWshub.failed"),
                                  QStringLiteral("path=%1 totalDomains=%2 failedDomains=%3 elapsedMs=%4 applySkipped=1")
                                      .arg(normalizedPath)
                                      .arg(results.size())
                                      .arg(failedCount)
                                      .arg(totalElapsedTimer.elapsed()));
        return false;
    }

    if (requestedDomains.hubRuntimeStore && targets.hubRuntimeStore != nullptr)
    {
        *targets.hubRuntimeStore = hubRuntimeSnapshot.store;
    }

    if (hasLibraryTask)
    {
        targets.libraryController->applyRuntimeSnapshot(
            normalizedPath,
            std::move(librarySnapshot.allNotes),
            std::move(librarySnapshot.draftNotes),
            std::move(librarySnapshot.todayNotes),
            std::move(librarySnapshot.folderEntries),
            std::move(librarySnapshot.foldersFilePath),
            librarySnapshot.succeeded,
            std::move(librarySnapshot.error));
    }

    if (hasProjectsTask)
    {
        targets.projectsController->applyRuntimeSnapshot(
            std::move(projectsSnapshot.projectEntries),
            std::move(projectsSnapshot.projectsFilePath),
            projectsSnapshot.succeeded,
            std::move(projectsSnapshot.error));
    }

    if (hasBookmarksTask)
    {
        targets.bookmarksController->applyRuntimeSnapshot(
            std::move(bookmarksSnapshot.bookmarkedNotes),
            bookmarksSnapshot.succeeded,
            std::move(bookmarksSnapshot.error));
    }

    if (hasTagsTask)
    {
        targets.tagsController->applyRuntimeSnapshot(
            std::move(tagsSnapshot.entries),
            std::move(tagsSnapshot.tagsFilePath),
            tagsSnapshot.succeeded,
            std::move(tagsSnapshot.error));
    }

    if (hasResourcesTask)
    {
        targets.resourcesController->applyRuntimeSnapshot(
            std::move(resourcesSnapshot.values),
            std::move(resourcesSnapshot.sourceFilePath),
            resourcesSnapshot.succeeded,
            std::move(resourcesSnapshot.error));
    }

    if (hasProgressTask)
    {
        targets.progressController->applyRuntimeSnapshot(
            progressSnapshot.progressValue,
            std::move(progressSnapshot.progressStates),
            std::move(progressSnapshot.sourceFilePath),
            progressSnapshot.succeeded,
            std::move(progressSnapshot.error));
    }

    if (hasEventTask)
    {
        targets.eventController->applyRuntimeSnapshot(
            std::move(eventSnapshot.values),
            std::move(eventSnapshot.sourceFilePath),
            eventSnapshot.succeeded,
            std::move(eventSnapshot.error));
    }

    if (hasPresetTask)
    {
        targets.presetController->applyRuntimeSnapshot(
            std::move(presetSnapshot.values),
            std::move(presetSnapshot.sourceFilePath),
            presetSnapshot.succeeded,
            std::move(presetSnapshot.error));
    }

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("runtime.parallel"),
                              QStringLiteral("loadFromWshub.success"),
                              QStringLiteral("path=%1 totalDomains=%2 failedDomains=%3 elapsedMs=%4")
                              .arg(normalizedPath)
                              .arg(results.size())
                              .arg(failedCount)
                              .arg(totalElapsedTimer.elapsed()));
    return true;
}
