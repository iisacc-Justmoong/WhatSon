#include "WhatSonRuntimeParallelLoader.hpp"

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

#include <QCoreApplication>
#include <QEventLoop>
#include <QThread>

namespace
{
    template <typename TObject, typename Loader>
    QThread* spawnObjectLoadThread(
        const QString& domain,
        TObject* object,
        const QString& wshubPath,
        QThread* mainThread,
        WhatSonRuntimeParallelLoader::DomainLoadResult* result,
        Loader loader)
    {
        result->domain = domain;
        if (object == nullptr)
        {
            result->succeeded = false;
            result->error = QStringLiteral("Target object is null.");
            return nullptr;
        }

        QThread* thread = new QThread();
        object->moveToThread(thread);

        QObject::connect(
            thread,
            &QThread::started,
            thread,
            [object, wshubPath, mainThread, result, thread, domain, loader]()
            {
                QString error;
                const bool succeeded = loader(object, wshubPath, &error);
                result->succeeded = succeeded;
                result->error = error.trimmed();

                WhatSon::Debug::trace(
                    QStringLiteral("runtime.parallel"),
                    QStringLiteral("load.completed"),
                    QStringLiteral("domain=%1 succeeded=%2 error=%3")
                    .arg(domain)
                    .arg(succeeded ? QStringLiteral("1") : QStringLiteral("0"))
                    .arg(result->error));

                object->moveToThread(mainThread);
                thread->quit();
            },
            Qt::DirectConnection);

        return thread;
    }

    template <typename Loader>
    QThread* spawnFunctionLoadThread(
        const QString& domain,
        const QString& wshubPath,
        WhatSonRuntimeParallelLoader::DomainLoadResult* result,
        Loader loader)
    {
        result->domain = domain;
        QThread* thread = new QThread();
        QObject::connect(
            thread,
            &QThread::started,
            thread,
            [wshubPath, result, thread, domain, loader]()
            {
                QString error;
                const bool succeeded = loader(wshubPath, &error);
                result->succeeded = succeeded;
                result->error = error.trimmed();

                WhatSon::Debug::trace(
                    QStringLiteral("runtime.parallel"),
                    QStringLiteral("load.completed"),
                    QStringLiteral("domain=%1 succeeded=%2 error=%3")
                    .arg(domain)
                    .arg(succeeded ? QStringLiteral("1") : QStringLiteral("0"))
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
    const QString normalizedPath = wshubPath.trimmed();
    QVector<DomainLoadResult> results;
    results.reserve(9);

    if (normalizedPath.isEmpty())
    {
        DomainLoadResult result;
        result.domain = QStringLiteral("runtime");
        result.succeeded = false;
        result.error = QStringLiteral("wshubPath is empty.");
        results.push_back(result);
        if (outResults != nullptr)
        {
            *outResults = results;
        }
        return false;
    }

    QThread* mainThread = QThread::currentThread();
    if (QCoreApplication::instance() != nullptr && QCoreApplication::instance()->thread() != nullptr)
    {
        mainThread = QCoreApplication::instance()->thread();
    }

    QVector<QThread*> threads;
    threads.reserve(9);

    auto addObjectTask = [&results, &threads, &normalizedPath, mainThread](
        const QString& domain,
        auto* object,
        auto loader)
    {
        results.push_back(DomainLoadResult{});
        DomainLoadResult* result = &results.back();
        QThread* thread = spawnObjectLoadThread(domain, object, normalizedPath, mainThread, result, loader);
        if (thread != nullptr)
        {
            threads.push_back(thread);
        }
    };

    auto addFunctionTask = [&results, &threads, &normalizedPath](
        const QString& domain,
        auto loader)
    {
        results.push_back(DomainLoadResult{});
        DomainLoadResult* result = &results.back();
        QThread* thread = spawnFunctionLoadThread(domain, normalizedPath, result, loader);
        if (thread != nullptr)
        {
            threads.push_back(thread);
        }
    };

    addObjectTask(
        QStringLiteral("library"),
        targets.libraryViewModel,
        [](LibraryHierarchyViewModel* viewModel, const QString& path, QString* error)
        {
            return viewModel->loadFromWshub(path, error);
        });
    addObjectTask(
        QStringLiteral("projects"),
        targets.projectsViewModel,
        [](ProjectsHierarchyViewModel* viewModel, const QString& path, QString* error)
        {
            return viewModel->loadFromWshub(path, error);
        });
    addObjectTask(
        QStringLiteral("bookmarks"),
        targets.bookmarksViewModel,
        [](BookmarksHierarchyViewModel* viewModel, const QString& path, QString* error)
        {
            return viewModel->loadFromWshub(path, error);
        });
    addObjectTask(
        QStringLiteral("tags"),
        targets.tagsViewModel,
        [](TagsHierarchyViewModel* viewModel, const QString& path, QString* error)
        {
            return viewModel->loadFromWshub(path, error);
        });
    addObjectTask(
        QStringLiteral("resources"),
        targets.resourcesViewModel,
        [](ResourcesHierarchyViewModel* viewModel, const QString& path, QString* error)
        {
            return viewModel->loadFromWshub(path, error);
        });
    addObjectTask(
        QStringLiteral("progress"),
        targets.progressViewModel,
        [](ProgressHierarchyViewModel* viewModel, const QString& path, QString* error)
        {
            return viewModel->loadFromWshub(path, error);
        });
    addObjectTask(
        QStringLiteral("event"),
        targets.eventViewModel,
        [](EventHierarchyViewModel* viewModel, const QString& path, QString* error)
        {
            return viewModel->loadFromWshub(path, error);
        });
    addObjectTask(
        QStringLiteral("preset"),
        targets.presetViewModel,
        [](PresetHierarchyViewModel* viewModel, const QString& path, QString* error)
        {
            return viewModel->loadFromWshub(path, error);
        });
    addFunctionTask(
        QStringLiteral("hub.runtime"),
        [&targets](const QString& path, QString* error)
        {
            if (targets.hubRuntimeStore == nullptr)
            {
                if (error != nullptr)
                {
                    *error = QStringLiteral("Target runtime store is null.");
                }
                return false;
            }
            return targets.hubRuntimeStore->loadFromWshub(path, error);
        });

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
        waitLoop.exec();
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

    if (outResults != nullptr)
    {
        *outResults = results;
    }

    bool allSucceeded = true;
    for (const DomainLoadResult& result : results)
    {
        if (!result.succeeded)
        {
            allSucceeded = false;
            break;
        }
    }
    return allSucceeded;
}
