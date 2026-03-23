#include "WhatSonRuntimeParallelLoader.hpp"

#include "WhatSonRuntimeDomainSnapshots.hpp"
#include "file/WhatSonDebugTrace.hpp"
#include "hub/WhatSonHubRuntimeStore.hpp"
#include "viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/event/EventHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/library/LibraryHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/preset/PresetHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/progress/ProgressHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/projects/ProjectsHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/resources/ResourcesHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/tags/TagsHierarchyViewModel.hpp"

#include <QElapsedTimer>
#include <QEventLoop>
#include <QThread>
#include <utility>

namespace
{
    template <typename Loader>
    QThread* spawnFunctionLoadThread(
        const QString& domain,
        const QString& wshubPath,
        WhatSonRuntimeParallelLoader::DomainLoadResult* result,
        Loader loader)
    {
        result->domain = domain;
        WhatSon::Debug::trace(
            QStringLiteral("runtime.parallel"),
            QStringLiteral("task.queued"),
            QStringLiteral("domain=%1 path=%2").arg(domain, wshubPath));
        QThread* thread = new QThread();
        QObject::connect(
            thread,
            &QThread::started,
            thread,
            [wshubPath, result, thread, domain, loader]()
            {
                WhatSon::Debug::trace(
                    QStringLiteral("runtime.parallel"),
                    QStringLiteral("task.begin"),
                    QStringLiteral("domain=%1 path=%2 threadPtr=%3")
                    .arg(domain, wshubPath)
                    .arg(reinterpret_cast<quintptr>(QThread::currentThread()), 0, 16));

                QElapsedTimer elapsedTimer;
                elapsedTimer.start();
                QString error;
                const bool succeeded = loader(wshubPath, &error);
                result->succeeded = succeeded;
                result->error = error.trimmed();

                WhatSon::Debug::trace(
                    QStringLiteral("runtime.parallel"),
                    QStringLiteral("load.completed"),
                    QStringLiteral("domain=%1 succeeded=%2 elapsedMs=%3 error=%4")
                    .arg(domain)
                    .arg(succeeded ? QStringLiteral("1") : QStringLiteral("0"))
                    .arg(elapsedTimer.elapsed())
                    .arg(result->error));
                thread->quit();
            },
            Qt::DirectConnection);
        return thread;
    }
} // namespace

bool WhatSonRuntimeParallelLoader::loadFromWshub(
    const QString& wshubPath,
    const Targets& targets,
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

    bool hasLibraryTask = false;
    bool hasProjectsTask = false;
    bool hasBookmarksTask = false;
    bool hasTagsTask = false;
    bool hasResourcesTask = false;
    bool hasProgressTask = false;
    bool hasEventTask = false;
    bool hasPresetTask = false;

    QVector<QThread*> threads;
    threads.reserve(9);

    auto addImmediateFailure = [&results](const QString& domain, const QString& error)
    {
        DomainLoadResult result;
        result.domain = domain;
        result.succeeded = false;
        result.error = error;
        results.push_back(std::move(result));
    };

    auto addSnapshotTask = [&results, &threads, &normalizedPath](
        const QString& domain,
        auto* snapshot,
        auto loader)
    {
        results.push_back(DomainLoadResult{});
        DomainLoadResult* result = &results.back();

        QThread* thread = spawnFunctionLoadThread(
            domain,
            normalizedPath,
            result,
            [snapshot, loader](const QString& path, QString* error)
            {
                *snapshot = loader(path);
                if (!snapshot->succeeded && error != nullptr)
                {
                    *error = snapshot->error;
                }
                return snapshot->succeeded;
            });

        if (thread == nullptr)
        {
            result->domain = domain;
            result->succeeded = false;
            result->error = QStringLiteral("Failed to create worker thread.");
            return;
        }

        threads.push_back(thread);
    };

    if (targets.libraryViewModel == nullptr)
    {
        addImmediateFailure(QStringLiteral("library"), QStringLiteral("Target viewModel is null."));
    }
    else
    {
        hasLibraryTask = true;
        addSnapshotTask(
            QStringLiteral("library"),
            &librarySnapshot,
            [](const QString& path)
            {
                return WhatSonRuntimeDomainSnapshots::loadLibrary(path);
            });
    }

    if (targets.projectsViewModel == nullptr)
    {
        addImmediateFailure(QStringLiteral("projects"), QStringLiteral("Target viewModel is null."));
    }
    else
    {
        hasProjectsTask = true;
        addSnapshotTask(
            QStringLiteral("projects"),
            &projectsSnapshot,
            [](const QString& path)
            {
                return WhatSonRuntimeDomainSnapshots::loadProjects(path);
            });
    }

    if (targets.bookmarksViewModel == nullptr)
    {
        addImmediateFailure(QStringLiteral("bookmarks"), QStringLiteral("Target viewModel is null."));
    }
    else
    {
        hasBookmarksTask = true;
        addSnapshotTask(
            QStringLiteral("bookmarks"),
            &bookmarksSnapshot,
            [](const QString& path)
            {
                return WhatSonRuntimeDomainSnapshots::loadBookmarks(path);
            });
    }

    if (targets.tagsViewModel == nullptr)
    {
        addImmediateFailure(QStringLiteral("tags"), QStringLiteral("Target viewModel is null."));
    }
    else
    {
        hasTagsTask = true;
        addSnapshotTask(
            QStringLiteral("tags"),
            &tagsSnapshot,
            [](const QString& path)
            {
                return WhatSonRuntimeDomainSnapshots::loadTags(path);
            });
    }

    if (targets.resourcesViewModel == nullptr)
    {
        addImmediateFailure(QStringLiteral("resources"), QStringLiteral("Target viewModel is null."));
    }
    else
    {
        hasResourcesTask = true;
        addSnapshotTask(
            QStringLiteral("resources"),
            &resourcesSnapshot,
            [](const QString& path)
            {
                return WhatSonRuntimeDomainSnapshots::loadResources(path);
            });
    }

    if (targets.progressViewModel == nullptr)
    {
        addImmediateFailure(QStringLiteral("progress"), QStringLiteral("Target viewModel is null."));
    }
    else
    {
        hasProgressTask = true;
        addSnapshotTask(
            QStringLiteral("progress"),
            &progressSnapshot,
            [](const QString& path)
            {
                return WhatSonRuntimeDomainSnapshots::loadProgress(path);
            });
    }

    if (targets.eventViewModel == nullptr)
    {
        addImmediateFailure(QStringLiteral("event"), QStringLiteral("Target viewModel is null."));
    }
    else
    {
        hasEventTask = true;
        addSnapshotTask(
            QStringLiteral("event"),
            &eventSnapshot,
            [](const QString& path)
            {
                return WhatSonRuntimeDomainSnapshots::loadEvent(path);
            });
    }

    if (targets.presetViewModel == nullptr)
    {
        addImmediateFailure(QStringLiteral("preset"), QStringLiteral("Target viewModel is null."));
    }
    else
    {
        hasPresetTask = true;
        addSnapshotTask(
            QStringLiteral("preset"),
            &presetSnapshot,
            [](const QString& path)
            {
                return WhatSonRuntimeDomainSnapshots::loadPreset(path);
            });
    }

    if (targets.hubRuntimeStore == nullptr)
    {
        addImmediateFailure(QStringLiteral("hub.runtime"), QStringLiteral("Target runtime store is null."));
    }
    else
    {
        results.push_back(DomainLoadResult{});
        DomainLoadResult* hubResult = &results.back();
        QThread* hubThread = spawnFunctionLoadThread(
            QStringLiteral("hub.runtime"),
            normalizedPath,
            hubResult,
            [&targets](const QString& path, QString* error)
            {
                return WhatSonRuntimeDomainSnapshots::loadHubRuntime(path, targets.hubRuntimeStore, error);
            });

        if (hubThread == nullptr)
        {
            hubResult->domain = QStringLiteral("hub.runtime");
            hubResult->succeeded = false;
            hubResult->error = QStringLiteral("Failed to create worker thread.");
        }
        else
        {
            threads.push_back(hubThread);
        }
    }

    int pendingCount = 0;
    QEventLoop waitLoop;
    for (QThread* thread : threads)
    {
        ++pendingCount;
        QObject::connect(
            thread,
            &QThread::finished,
            &waitLoop,
            [&pendingCount, &waitLoop]()
            {
                --pendingCount;
                if (pendingCount <= 0)
                {
                    waitLoop.quit();
                }
            },
            Qt::QueuedConnection);
        thread->start();
    }

    if (pendingCount > 0)
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("runtime.parallel"),
                                  QStringLiteral("wait.begin"),
                                  QStringLiteral("pending=%1").arg(pendingCount));
        waitLoop.exec();
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("runtime.parallel"),
                                  QStringLiteral("wait.end"),
                                  QStringLiteral("pending=%1").arg(pendingCount));
    }

    for (QThread* thread : threads)
    {
        if (thread == nullptr)
        {
            continue;
        }
        thread->wait();
        delete thread;
    }

    if (hasLibraryTask)
    {
        targets.libraryViewModel->applyRuntimeSnapshot(
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
        targets.projectsViewModel->applyRuntimeSnapshot(
            std::move(projectsSnapshot.projectEntries),
            std::move(projectsSnapshot.projectsFilePath),
            projectsSnapshot.succeeded,
            std::move(projectsSnapshot.error));
    }

    if (hasBookmarksTask)
    {
        targets.bookmarksViewModel->applyRuntimeSnapshot(
            std::move(bookmarksSnapshot.bookmarkedNotes),
            bookmarksSnapshot.succeeded,
            std::move(bookmarksSnapshot.error));
    }

    if (hasTagsTask)
    {
        targets.tagsViewModel->applyRuntimeSnapshot(
            std::move(tagsSnapshot.entries),
            std::move(tagsSnapshot.tagsFilePath),
            tagsSnapshot.succeeded,
            std::move(tagsSnapshot.error));
    }

    if (hasResourcesTask)
    {
        targets.resourcesViewModel->applyRuntimeSnapshot(
            std::move(resourcesSnapshot.values),
            std::move(resourcesSnapshot.sourceFilePath),
            resourcesSnapshot.succeeded,
            std::move(resourcesSnapshot.error));
    }

    if (hasProgressTask)
    {
        targets.progressViewModel->applyRuntimeSnapshot(
            progressSnapshot.progressValue,
            std::move(progressSnapshot.progressStates),
            std::move(progressSnapshot.sourceFilePath),
            progressSnapshot.succeeded,
            std::move(progressSnapshot.error));
    }

    if (hasEventTask)
    {
        targets.eventViewModel->applyRuntimeSnapshot(
            std::move(eventSnapshot.values),
            std::move(eventSnapshot.sourceFilePath),
            eventSnapshot.succeeded,
            std::move(eventSnapshot.error));
    }

    if (hasPresetTask)
    {
        targets.presetViewModel->applyRuntimeSnapshot(
            std::move(presetSnapshot.values),
            std::move(presetSnapshot.sourceFilePath),
            presetSnapshot.succeeded,
            std::move(presetSnapshot.error));
    }

    if (outResults != nullptr)
    {
        *outResults = results;
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

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("runtime.parallel"),
                              allSucceeded
                                  ? QStringLiteral("loadFromWshub.success")
                                  : QStringLiteral("loadFromWshub.failed"),
                              QStringLiteral("path=%1 totalDomains=%2 failedDomains=%3 elapsedMs=%4")
                              .arg(normalizedPath)
                              .arg(results.size())
                              .arg(failedCount)
                              .arg(totalElapsedTimer.elapsed()));
    return allSucceeded;
}
