#include "backend/runtime/appbootstrap.h"
#include "backend/runtime/foregroundservices.h"
#include "app/viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.hpp"
#include "app/viewmodel/hierarchy/event/EventHierarchyViewModel.hpp"
#include "app/viewmodel/hierarchy/library/LibraryHierarchyViewModel.hpp"
#include "app/viewmodel/hierarchy/library/LibraryNoteMutationViewModel.hpp"
#include "app/viewmodel/hierarchy/preset/PresetHierarchyViewModel.hpp"
#include "app/viewmodel/hierarchy/progress/ProgressHierarchyViewModel.hpp"
#include "app/viewmodel/hierarchy/projects/ProjectsHierarchyViewModel.hpp"
#include "app/viewmodel/hierarchy/resources/ResourcesHierarchyViewModel.hpp"
#include "app/models/file/import/ResourcesImportViewModel.hpp"
#include "app/viewmodel/hierarchy/tags/TagsHierarchyViewModel.hpp"
#include "app/viewmodel/navigationbar/EditorViewModeViewModel.hpp"
#include "app/viewmodel/navigationbar/NavigationModeViewModel.hpp"
#include "app/viewmodel/detailPanel/DetailPanelCurrentHierarchyBinder.hpp"
#include "app/viewmodel/detailPanel/NoteDetailPanelViewModel.hpp"
#include "app/viewmodel/detailPanel/ResourceDetailPanelViewModel.hpp"
#include "app/viewmodel/calendar/DayCalendarViewModel.hpp"
#include "app/viewmodel/calendar/AgendaViewModel.hpp"
#include "app/viewmodel/calendar/MonthCalendarViewModel.hpp"
#include "app/viewmodel/calendar/WeekCalendarViewModel.hpp"
#include "app/viewmodel/calendar/YearCalendarViewModel.hpp"
#include "app/viewmodel/onboarding/OnboardingHubController.hpp"
#include "app/viewmodel/onboarding/OnboardingRouteBootstrapController.hpp"
#include "app/viewmodel/panel/PanelViewModelRegistry.hpp"
#include "app/viewmodel/sidebar/HierarchySidebarDomain.hpp"
#include "app/viewmodel/sidebar/HierarchyViewModelProvider.hpp"
#include "app/viewmodel/sidebar/SidebarHierarchyViewModel.hpp"
#include "app/models/calendar/CalendarBoardStore.hpp"
#include "app/models/calendar/SystemCalendarStore.hpp"
#include "app/policy/ArchitecturePolicyLock.hpp"
#include "app/models/file/hub/WhatSonHubRuntimeStore.hpp"
#include "app/runtime/threading/WhatSonRuntimeParallelLoader.hpp"
#include "app/runtime/startup/WhatSonStartupRuntimeCoordinator.hpp"
#include "app/runtime/startup/WhatSonStartupHubResolver.hpp"
#include "app/runtime/bootstrap/WhatSonAppLaunchSupport.hpp"
#include "app/runtime/bootstrap/WhatSonHubSyncWiring.hpp"
#include "app/runtime/bootstrap/WhatSonQmlContextBinder.hpp"
#include "app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.hpp"
#include "app/runtime/bootstrap/WhatSonQmlLaunchSupport.hpp"
#include "app/permissions/WhatSonPermissionBootstrapper.hpp"
#include "app/runtime/scheduler/WhatSonAsyncScheduler.hpp"
#include "app/models/file/hub/WhatSonHubCreator.hpp"
#include "app/models/file/hub/WhatSonHubMountValidator.hpp"
#include "app/models/file/hub/WhatSonHubPathUtils.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"
#include "app/platform/Android/WhatSonAndroidStorageBackend.hpp"
#include "app/platform/Apple/AppleSecurityScopedResourceAccess.hpp"
#include "app/permissions/ApplePermissionBridge.hpp"
#include "app/store/hub/SelectedHubStore.hpp"
#include "app/store/sidebar/SidebarSelectionStore.hpp"
#include "app/models/file/sync/WhatSonHubSyncController.hpp"
#if defined(WHATSON_IS_TRIAL_BUILD) && !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
#include "extension/trial/WhatSonTrialActivationPolicy.hpp"
#endif

#include <QByteArray>
#include <QCoreApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QIcon>
#include <QPointer>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QStringList>
#include <QTimer>
#include <QVariant>
#include <QVector>
#include <QtCore/qglobal.h>

#include <cstdlib>

void qml_register_types_LVRS();

namespace
{
lvrs::QmlAppLifecycleContext workspaceLifecycleContext(
    QGuiApplication& app,
    QQmlApplicationEngine& engine,
    QObject* workspaceRootObject,
    const lvrs::QmlAppLifecycleStage stage)
{
    lvrs::QmlAppLifecycleContext context;
    context.application = &app;
    context.engine = &engine;
    context.stage = stage;
    context.rootLoadResult.ok = workspaceRootObject != nullptr;

    if (workspaceRootObject != nullptr)
    {
        context.rootLoadResult.rootObjects.append(workspaceRootObject);
        if (QWindow* workspaceWindow = lvrs::qmlRootWindow(workspaceRootObject))
        {
            context.rootLoadResult.windows.append(workspaceWindow);
        }
    }

    return context;
}

QVector<int> deferredStartupHierarchyPrefetchOrder()
{
    return {
        static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Event),
        static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Preset)
    };
}

QString deferredStartupHierarchyPrefetchLabel(const int hierarchyIndex)
{
    switch (hierarchyIndex)
    {
    case static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Event):
        return QStringLiteral("event");
    case static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Preset):
        return QStringLiteral("preset");
    default:
        return QStringLiteral("unknown");
    }
}

bool scheduleDeferredStartupHierarchyPrefetch(
    QGuiApplication& app,
    QQmlApplicationEngine& engine,
    QObject* workspaceRootObject,
    WhatSonStartupRuntimeCoordinator& startupRuntimeCoordinator)
{
    if (!startupRuntimeCoordinator.startupDeferredBootstrapActive())
    {
        return true;
    }

    lvrs::QmlBootstrapTask prefetchTask;
    prefetchTask.name = QStringLiteral("whatson-deferred-startup-hierarchy-prefetch");
    prefetchTask.stage = lvrs::QmlAppLifecycleStage::AfterFirstIdle;
    prefetchTask.priority = 10;
    prefetchTask.fatal = false;
    prefetchTask.run = [&startupRuntimeCoordinator](const lvrs::QmlAppLifecycleContext&, QString* errorMessage)
    {
        QStringList failedHierarchyDomains;
        for (const int hierarchyIndex : deferredStartupHierarchyPrefetchOrder())
        {
            const bool loaded = startupRuntimeCoordinator.ensureDeferredStartupHierarchyLoaded(
                hierarchyIndex,
                QStringLiteral("startup-after-first-idle-prefetch"));
            if (!loaded)
            {
                failedHierarchyDomains.append(deferredStartupHierarchyPrefetchLabel(hierarchyIndex));
            }
        }

        if (failedHierarchyDomains.isEmpty())
        {
            return true;
        }

        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Deferred startup hierarchy prefetch failed for: %1")
                .arg(failedHierarchyDomains.join(QStringLiteral(", ")));
        }
        return false;
    };

    lvrs::QmlAppLifecycleHooks lifecycleHooks;
    lifecycleHooks.tasks.append(prefetchTask);
    return lvrs::scheduleQmlAppLifecycleStage(
        &app,
        workspaceLifecycleContext(app, engine, workspaceRootObject, lvrs::QmlAppLifecycleStage::AfterFirstIdle),
        lifecycleHooks,
        lvrs::QmlAppLifecycleStage::AfterFirstIdle,
        true);
}
} // namespace

int main(int argc, char* argv[])
{
    lvrs::AppBootstrapOptions bootstrapOptions;
    bootstrapOptions.applicationName = QStringLiteral("WhatSon");
    bootstrapOptions.quickStyleName = QStringLiteral("Basic");

    const lvrs::AppBootstrapState bootstrapState = lvrs::preApplicationBootstrap(bootstrapOptions);
    if (!bootstrapState.ok)
    {
        qCritical().noquote() << bootstrapState.errorMessage;
        return EXIT_FAILURE;
    }

    QGuiApplication app(argc, argv);
    lvrs::postApplicationBootstrap(app, bootstrapOptions);

    if (qEnvironmentVariableIsEmpty("WHATSON_DEBUG_MODE"))
    {
        qputenv("WHATSON_DEBUG_MODE", QByteArrayLiteral("1"));
    }
#if defined(WHATSON_USE_LVRS_DYNAMIC_QML_IMPORT) && defined(WHATSON_LVRS_RUNTIME_IMPORT_ROOT)
    Q_CLEANUP_RESOURCE(qmake_LVRS);

    const QByteArray lvrsImportRoot = QByteArrayLiteral(WHATSON_LVRS_RUNTIME_IMPORT_ROOT);
    WhatSon::Runtime::Bootstrap::prependEnvPath("QML_IMPORT_PATH", lvrsImportRoot);
    WhatSon::Runtime::Bootstrap::prependEnvPath("QML2_IMPORT_PATH", lvrsImportRoot);
#endif

#if !defined(WHATSON_USE_LVRS_DYNAMIC_QML_IMPORT)
    qml_register_types_LVRS();
#endif
    const lvrs::QmlTypeRegistrationReport internalQmlTypeRegistrationReport =
        WhatSon::Runtime::Bootstrap::registerInternalQmlTypes();
    if (!internalQmlTypeRegistrationReport.ok)
    {
        qCritical().noquote() << QStringLiteral("Failed to register WhatSon internal QML types: %1")
            .arg(internalQmlTypeRegistrationReport.errorMessage());
        return EXIT_FAILURE;
    }
#if !defined(Q_OS_MACOS) && !defined(Q_OS_IOS)
    app.setWindowIcon(QIcon(QStringLiteral(":/whatson/AppIcon.png")));
#endif

    QCoreApplication::setApplicationName(QStringLiteral("WhatSon"));
    QCoreApplication::setOrganizationName(QStringLiteral("WhatSon"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("whatson.local"));
    const WhatSon::Runtime::Bootstrap::LaunchOptions launchOptions =
        WhatSon::Runtime::Bootstrap::parseLaunchOptions(app);

    WhatSon::Debug::trace(
        QStringLiteral("main"),
        QStringLiteral("startup"),
        QStringLiteral("debugMode=%1 cwd=%2 appDir=%3 launchMode=%4")
        .arg(WhatSon::Debug::isEnabled() ? QStringLiteral("on") : QStringLiteral("off"))
        .arg(QDir::currentPath())
        .arg(QCoreApplication::applicationDirPath())
        .arg(launchOptions.onboardingOnly ? QStringLiteral("onboardingOnly") : QStringLiteral("workspace")));

    QQmlApplicationEngine engine;
    CalendarBoardStore calendarBoardStore;
    SystemCalendarStore systemCalendarStore;
    LibraryHierarchyViewModel libraryHierarchyViewModel;
    LibraryNoteMutationViewModel libraryNoteMutationViewModel;
    ProjectsHierarchyViewModel projectsHierarchyViewModel;
    BookmarksHierarchyViewModel bookmarksHierarchyViewModel;
    TagsHierarchyViewModel tagsHierarchyViewModel;
    ResourcesHierarchyViewModel resourcesHierarchyViewModel;
    ResourcesImportViewModel resourcesImportViewModel;
    ProgressHierarchyViewModel progressHierarchyViewModel;
    EventHierarchyViewModel eventHierarchyViewModel;
    PresetHierarchyViewModel presetHierarchyViewModel;
    SidebarSelectionStore sidebarSelectionStore;
    SelectedHubStore selectedHubStore;
    HierarchyViewModelProvider hierarchyViewModelProvider;
    SidebarHierarchyViewModel sidebarHierarchyViewModel;
    DetailPanelCurrentHierarchyBinder detailPanelCurrentHierarchyBinder;
    NoteDetailPanelViewModel noteDetailPanelViewModel;
    ResourceDetailPanelViewModel resourceDetailPanelViewModel;
    EditorViewModeViewModel editorViewModeViewModel;
    NavigationModeViewModel navigationModeViewModel;
    WhatSonAsyncScheduler asyncScheduler;
    PanelViewModelRegistry panelViewModelRegistry;
    DayCalendarViewModel dayCalendarViewModel;
    AgendaViewModel agendaViewModel;
    MonthCalendarViewModel monthCalendarViewModel;
    WeekCalendarViewModel weekCalendarViewModel;
    YearCalendarViewModel yearCalendarViewModel;
    WhatSonHubRuntimeStore hubRuntimeStore;
    OnboardingHubController onboardingHubController;
    WhatSonHubCreator hubCreator(QDir::currentPath(), QStringLiteral("hubs"));
    WhatSonPermissionBootstrapper permissionBootstrapper(app);
#if defined(WHATSON_IS_TRIAL_BUILD) && !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
    WhatSonRegisterManager trialRegisterManager;
    WhatSonTrialActivationPolicy trialActivationPolicy;
    trialActivationPolicy.setRegisterManager(&trialRegisterManager);
    QObject::connect(
        &trialRegisterManager,
        &WhatSonRegisterManager::authenticatedChanged,
        &trialActivationPolicy,
        &WhatSonTrialActivationPolicy::refresh);
    trialActivationPolicy.refresh();
#endif
    libraryNoteMutationViewModel.setSourceViewModel(&libraryHierarchyViewModel);
    dayCalendarViewModel.setCalendarBoardStore(&calendarBoardStore);
    agendaViewModel.setCalendarBoardStore(&calendarBoardStore);
    monthCalendarViewModel.setCalendarBoardStore(&calendarBoardStore);
    weekCalendarViewModel.setCalendarBoardStore(&calendarBoardStore);
    yearCalendarViewModel.setCalendarBoardStore(&calendarBoardStore);
    calendarBoardStore.setProjectedNotesProvider(
        [&libraryHierarchyViewModel]()
        {
            return libraryHierarchyViewModel.indexedNotesSnapshot();
        });

    const auto requestNewLibraryNote = [&libraryNoteMutationViewModel, &sidebarHierarchyViewModel]()
    {
        sidebarHierarchyViewModel.setActiveHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library));
        if (!libraryNoteMutationViewModel.createEmptyNote())
        {
            qWarning().noquote() << QStringLiteral("Failed to create a new library note from navigation Add action.");
        }
    };
    const auto handlePanelCreateNoteRequest = [&requestNewLibraryNote](const QString& reason)
    {
        if (reason.trimmed() != QStringLiteral("create-note"))
        {
            return;
        }

        requestNewLibraryNote();
    };

    if (auto* addNewPanelViewModel = qobject_cast<PanelViewModel*>(
        panelViewModelRegistry.panelViewModel(QStringLiteral("navigation.NavigationAddNewBar"))))
    {
        QObject::connect(
            addNewPanelViewModel,
            &PanelViewModel::viewModelHookRequested,
            &app,
            handlePanelCreateNoteRequest);
    }
    if (auto* applicationControlPanelViewModel = qobject_cast<PanelViewModel*>(
        panelViewModelRegistry.panelViewModel(QStringLiteral("navigation.NavigationApplicationControlBar"))))
    {
        QObject::connect(
            applicationControlPanelViewModel,
            &PanelViewModel::viewModelHookRequested,
            &app,
            handlePanelCreateNoteRequest);
    }
    if (auto* applicationViewPanelViewModel = qobject_cast<PanelViewModel*>(
        panelViewModelRegistry.panelViewModel(QStringLiteral("navigation.NavigationApplicationViewBar"))))
    {
        QObject::connect(
            applicationViewPanelViewModel,
            &PanelViewModel::viewModelHookRequested,
            &app,
            handlePanelCreateNoteRequest);
    }
    if (auto* applicationEditPanelViewModel = qobject_cast<PanelViewModel*>(
        panelViewModelRegistry.panelViewModel(QStringLiteral("navigation.NavigationApplicationEditBar"))))
    {
        QObject::connect(
            applicationEditPanelViewModel,
            &PanelViewModel::viewModelHookRequested,
            &app,
            handlePanelCreateNoteRequest);
    }

    libraryHierarchyViewModel.setSystemCalendarStore(&systemCalendarStore);
    bookmarksHierarchyViewModel.setSystemCalendarStore(&systemCalendarStore);
    QObject::connect(
        &libraryHierarchyViewModel,
        &LibraryHierarchyViewModel::noteDeleted,
        &bookmarksHierarchyViewModel,
        [&bookmarksHierarchyViewModel](const QString& noteId)
        {
            bookmarksHierarchyViewModel.removeNoteById(noteId);
        });

    QObject::connect(
        &app,
        &QGuiApplication::applicationStateChanged,
        &systemCalendarStore,
        [&systemCalendarStore](Qt::ApplicationState state)
        {
            if (state == Qt::ApplicationActive)
            {
                systemCalendarStore.refreshFromSystem();
            }
        });

    WhatSonStartupRuntimeCoordinator::RuntimeTargets startupRuntimeTargets;
    startupRuntimeTargets.libraryViewModel = &libraryHierarchyViewModel;
    startupRuntimeTargets.projectsViewModel = &projectsHierarchyViewModel;
    startupRuntimeTargets.bookmarksViewModel = &bookmarksHierarchyViewModel;
    startupRuntimeTargets.tagsViewModel = &tagsHierarchyViewModel;
    startupRuntimeTargets.resourcesViewModel = &resourcesHierarchyViewModel;
    startupRuntimeTargets.progressViewModel = &progressHierarchyViewModel;
    startupRuntimeTargets.eventViewModel = &eventHierarchyViewModel;
    startupRuntimeTargets.presetViewModel = &presetHierarchyViewModel;
    startupRuntimeTargets.hubRuntimeStore = &hubRuntimeStore;
    WhatSonStartupRuntimeCoordinator startupRuntimeCoordinator(startupRuntimeTargets);
    WhatSonRuntimeParallelLoader runtimeParallelLoader;
    startupRuntimeCoordinator.setParallelLoader(&runtimeParallelLoader);

    WhatSonHubSyncController hubSyncController;
    const auto reloadCalendarProjectedNotesFromRuntime =
        [&calendarBoardStore, &hubSyncController, &libraryHierarchyViewModel]()
    {
        calendarBoardStore.setProjectedNotesHubPath(hubSyncController.currentHubPath());
        calendarBoardStore.reloadProjectedNotesFromSnapshot(
            libraryHierarchyViewModel.indexedNotesSnapshot());
    };
    const auto upsertCalendarProjectedNoteFromRuntime =
        [&calendarBoardStore, &libraryHierarchyViewModel](const QString& noteId)
    {
        LibraryNoteRecord note;
        if (!libraryHierarchyViewModel.indexedNoteRecordById(noteId, &note))
        {
            calendarBoardStore.removeProjectedNoteBySourceId(noteId);
            return;
        }

        calendarBoardStore.upsertProjectedNote(note);
    };
    const auto requestCalendarProjectedNotesReload = [&calendarBoardStore, &hubSyncController]()
    {
        calendarBoardStore.setProjectedNotesHubPath(hubSyncController.currentHubPath());
        calendarBoardStore.requestProjectedNotesReload();
    };
    hubSyncController.setReloadCallback(
        [&startupRuntimeCoordinator](const QString& hubPath, QString* errorMessage) -> bool
        {
            return startupRuntimeCoordinator.loadHubIntoRuntime(hubPath, errorMessage);
        });
    const WhatSon::Runtime::Bootstrap::HubSyncWiringResult hubSyncWiring =
        WhatSon::Runtime::Bootstrap::wireHubSyncController(
            &hubSyncController,
            &app,
            {
                &libraryHierarchyViewModel,
                &projectsHierarchyViewModel,
                &bookmarksHierarchyViewModel,
                &resourcesHierarchyViewModel,
                &progressHierarchyViewModel
            });
    QObject::connect(
        &hubSyncController,
        &WhatSonHubSyncController::syncReloaded,
        &app,
        [&calendarBoardStore, &libraryHierarchyViewModel](const QString& hubPath)
        {
            calendarBoardStore.setProjectedNotesHubPath(hubPath);
            calendarBoardStore.reloadProjectedNotesFromSnapshot(
                libraryHierarchyViewModel.indexedNotesSnapshot());
        });
    QObject::connect(
        &libraryHierarchyViewModel,
        &LibraryHierarchyViewModel::indexedNotesSnapshotChanged,
        &app,
        reloadCalendarProjectedNotesFromRuntime);
    QObject::connect(
        &libraryHierarchyViewModel,
        &LibraryHierarchyViewModel::indexedNoteUpserted,
        &app,
        upsertCalendarProjectedNoteFromRuntime);
    QObject::connect(
        &libraryHierarchyViewModel,
        &LibraryHierarchyViewModel::noteDeleted,
        &app,
        [&calendarBoardStore](const QString& noteId)
        {
            calendarBoardStore.removeProjectedNoteBySourceId(noteId);
        });
    QObject::connect(
        &bookmarksHierarchyViewModel,
        &BookmarksHierarchyViewModel::hubFilesystemMutated,
        &app,
        requestCalendarProjectedNotesReload);
    QObject::connect(
        &progressHierarchyViewModel,
        &ProgressHierarchyViewModel::hubFilesystemMutated,
        &app,
        requestCalendarProjectedNotesReload);
    Q_UNUSED(hubSyncWiring);
    resourcesImportViewModel.setReloadResourcesCallback(
        [&startupRuntimeCoordinator, &hubSyncController](const QString& hubPath, QString* errorMessage) -> bool
        {
            const bool reloaded = startupRuntimeCoordinator.reloadResourcesDomainIntoRuntime(hubPath, errorMessage);
            if (reloaded)
            {
                hubSyncController.acknowledgeLocalMutation();
            }
            return reloaded;
        });

    onboardingHubController.setCreateHubCallback(
        [&hubCreator](const QString& requestedHubPath, QString* outPackagePath, QString* errorMessage) -> bool
        {
            return hubCreator.createHubAtPath(requestedHubPath, outPackagePath, errorMessage);
        });
    onboardingHubController.setLoadHubCallback(
        [&startupRuntimeCoordinator](const QString& hubPath, QString* errorMessage) -> bool
        {
            return startupRuntimeCoordinator.loadHubIntoRuntime(hubPath, errorMessage);
        });
    OnboardingRouteBootstrapController onboardingRouteBootstrapController;
    onboardingRouteBootstrapController.setHubController(&onboardingHubController);
    QObject::connect(
        &onboardingHubController,
        &OnboardingHubController::hubLoaded,
        &app,
        [&onboardingHubController,
         &selectedHubStore,
         &hubSyncController,
         &resourcesImportViewModel,
         &calendarBoardStore,
         &libraryHierarchyViewModel](const QString& hubPath)
        {
            selectedHubStore.setSelectedHubSelection(
                hubPath,
                onboardingHubController.currentHubAccessBookmark());
            hubSyncController.setCurrentHubPath(hubPath);
            resourcesImportViewModel.setCurrentHubPath(hubPath);
            calendarBoardStore.setProjectedNotesHubPath(hubPath);
            calendarBoardStore.reloadProjectedNotesFromSnapshot(
                libraryHierarchyViewModel.indexedNotesSnapshot());
        });

    bool initialHubLoaded = false;
    const WhatSonHubMountValidator startupHubMountValidator;
    const WhatSon::Runtime::Startup::StartupHubSelection startupHubSelection =
        WhatSon::Runtime::Startup::resolveStartupHubSelection(
            selectedHubStore,
            startupHubMountValidator);
    if (!startupHubSelection.mounted && !startupHubSelection.failureMessage.trimmed().isEmpty())
    {
        onboardingHubController.failWorkspaceTransition(startupHubSelection.failureMessage);
    }

    if (startupHubSelection.mounted)
    {
        QString errorMessage;
        initialHubLoaded = startupRuntimeCoordinator.loadStartupHubIntoRuntime(
            startupHubSelection.hubPath,
            &errorMessage);
        if (initialHubLoaded)
        {
            onboardingHubController.syncCurrentHubSelection(startupHubSelection.hubPath);
            onboardingHubController.completeWorkspaceTransition();
            selectedHubStore.setSelectedHubSelection(
                startupHubSelection.hubPath,
                startupHubSelection.accessBookmark);
            hubSyncController.setCurrentHubPath(startupHubSelection.hubPath);
            resourcesImportViewModel.setCurrentHubPath(startupHubSelection.hubPath);
            calendarBoardStore.setProjectedNotesHubPath(startupHubSelection.hubPath);
            calendarBoardStore.reloadProjectedNotesFromSnapshot(
                libraryHierarchyViewModel.indexedNotesSnapshot());
        }
        if (!initialHubLoaded)
        {
            const QString startupLoadFailureMessage = errorMessage.trimmed().isEmpty()
                                                          ? QStringLiteral("Failed to load the startup WhatSon Hub.")
                                                          : errorMessage.trimmed();
            onboardingHubController.failWorkspaceTransition(startupLoadFailureMessage);
            qWarning().noquote()
                << QStringLiteral("Failed to load startup WhatSon Hub '%1': %2")
                       .arg(startupHubSelection.hubPath, startupLoadFailureMessage);
        }
    }

    hierarchyViewModelProvider.setMappings(QVector<HierarchyViewModelProvider::Mapping>{
        { static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &libraryHierarchyViewModel },
        { static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Projects), &projectsHierarchyViewModel },
        { static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Bookmarks), &bookmarksHierarchyViewModel },
        { static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags), &tagsHierarchyViewModel },
        { static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Resources), &resourcesHierarchyViewModel },
        { static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Progress), &progressHierarchyViewModel },
        { static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Event), &eventHierarchyViewModel },
        { static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Preset), &presetHierarchyViewModel },
    });
    noteDetailPanelViewModel.setProjectSelectionSourceViewModel(&projectsHierarchyViewModel);
    noteDetailPanelViewModel.setBookmarkSelectionSourceViewModel(&bookmarksHierarchyViewModel);
    noteDetailPanelViewModel.setProgressSelectionSourceViewModel(&progressHierarchyViewModel);
    noteDetailPanelViewModel.setTagsSourceViewModel(&tagsHierarchyViewModel);
    sidebarHierarchyViewModel.setSelectionStore(&sidebarSelectionStore);
    sidebarHierarchyViewModel.setViewModelProvider(&hierarchyViewModelProvider);
    detailPanelCurrentHierarchyBinder.setNoteDetailPanelViewModel(&noteDetailPanelViewModel);
    detailPanelCurrentHierarchyBinder.setResourceDetailPanelViewModel(&resourceDetailPanelViewModel);
    detailPanelCurrentHierarchyBinder.setHierarchyContextSource(&sidebarHierarchyViewModel);

    startupRuntimeCoordinator.bindSidebarActivation(&sidebarHierarchyViewModel);

    WhatSon::Policy::ArchitecturePolicyLock::lock();

    WhatSon::Runtime::Bootstrap::WorkspaceContextObjects workspaceContextObjects;
    workspaceContextObjects.libraryHierarchyViewModel = &libraryHierarchyViewModel;
    workspaceContextObjects.libraryNoteMutationViewModel = &libraryNoteMutationViewModel;
    workspaceContextObjects.projectsHierarchyViewModel = &projectsHierarchyViewModel;
    workspaceContextObjects.bookmarksHierarchyViewModel = &bookmarksHierarchyViewModel;
    workspaceContextObjects.tagsHierarchyViewModel = &tagsHierarchyViewModel;
    workspaceContextObjects.resourcesHierarchyViewModel = &resourcesHierarchyViewModel;
    workspaceContextObjects.resourcesImportViewModel = &resourcesImportViewModel;
    workspaceContextObjects.progressHierarchyViewModel = &progressHierarchyViewModel;
    workspaceContextObjects.eventHierarchyViewModel = &eventHierarchyViewModel;
    workspaceContextObjects.presetHierarchyViewModel = &presetHierarchyViewModel;
    workspaceContextObjects.detailPanelViewModel = &noteDetailPanelViewModel;
    workspaceContextObjects.noteDetailPanelViewModel = &noteDetailPanelViewModel;
    workspaceContextObjects.resourceDetailPanelViewModel = &resourceDetailPanelViewModel;
    workspaceContextObjects.editorViewModeViewModel = &editorViewModeViewModel;
    workspaceContextObjects.navigationModeViewModel = &navigationModeViewModel;
    workspaceContextObjects.sidebarHierarchyViewModel = &sidebarHierarchyViewModel;
    workspaceContextObjects.asyncScheduler = &asyncScheduler;
    workspaceContextObjects.calendarBoardStore = &calendarBoardStore;
    workspaceContextObjects.systemCalendarStore = &systemCalendarStore;
    workspaceContextObjects.dayCalendarViewModel = &dayCalendarViewModel;
    workspaceContextObjects.agendaViewModel = &agendaViewModel;
    workspaceContextObjects.monthCalendarViewModel = &monthCalendarViewModel;
    workspaceContextObjects.weekCalendarViewModel = &weekCalendarViewModel;
    workspaceContextObjects.yearCalendarViewModel = &yearCalendarViewModel;
    workspaceContextObjects.panelViewModelRegistry = &panelViewModelRegistry;
    const lvrs::QmlContextBindResult workspaceContextBindResult =
        WhatSon::Runtime::Bootstrap::bindWorkspaceContextObjects(engine, workspaceContextObjects);
    if (!workspaceContextBindResult.ok)
    {
        qCritical().noquote() << QStringLiteral("Failed to bind workspace QML context: %1")
            .arg(workspaceContextBindResult.errorMessage());
        return EXIT_FAILURE;
    }
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(EXIT_FAILURE); },
        Qt::QueuedConnection);

    lvrs::ForegroundServiceGate foregroundServiceGate(&app);
    const auto startForegroundServices =
        [&app, &engine, &asyncScheduler, &permissionBootstrapper, &foregroundServiceGate](
            QObject* workspaceRootObject) -> bool
    {
        lvrs::ForegroundServiceTask schedulerStart;
        schedulerStart.name = QStringLiteral("whatson-async-scheduler");
        schedulerStart.priority = 10;
        schedulerStart.fatal = true;
        schedulerStart.metadata.insert(QStringLiteral("service"), QStringLiteral("scheduler"));
        schedulerStart.start = [&asyncScheduler](const lvrs::ForegroundServiceStartContext&, QString* errorMessage)
        {
            if (asyncScheduler.start())
            {
                return true;
            }

            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("WhatSon async scheduler did not start.");
            }
            return false;
        };

        lvrs::ForegroundServiceTask permissionBootstrap;
        permissionBootstrap.name = QStringLiteral("whatson-permission-bootstrap");
        permissionBootstrap.priority = 20;
        permissionBootstrap.metadata.insert(QStringLiteral("service"), QStringLiteral("permissions"));
        permissionBootstrap.start =
            [&permissionBootstrapper](const lvrs::ForegroundServiceStartContext&, QString*) -> bool
        {
            QTimer::singleShot(0, &permissionBootstrapper, [&permissionBootstrapper]()
            {
                permissionBootstrapper.start();
            });
            return true;
        };

        lvrs::ForegroundServiceStartOptions options;
        options.requireVisibleWorkspace = true;
        options.logDiagnostics = true;
        options.metadata.insert(QStringLiteral("phase"), QStringLiteral("workspace-visible"));

        const lvrs::ForegroundServiceStartResult foregroundStartResult =
            foregroundServiceGate.startOnceWhenWorkspaceVisible(
                workspaceLifecycleContext(
                    app,
                    engine,
                    workspaceRootObject,
                    lvrs::QmlAppLifecycleStage::AfterWindowActivated),
                {schedulerStart, permissionBootstrap},
                options);

        if (!foregroundStartResult.ok)
        {
            qWarning().noquote() << QStringLiteral("Foreground services did not start after visible workspace: %1")
                .arg(foregroundStartResult.errorMessage());
            return false;
        }

        return true;
    };

#if defined(WHATSON_IS_TRIAL_BUILD) && !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
    const auto loadTrialStatusWindow =
        [&engine](QObject* hostWindow, QObject* trialPolicyObject) -> QObject*
    {
        const QVariantMap trialWindowInitialProperties{
            {QStringLiteral("hostWindow"), QVariant::fromValue(hostWindow)},
            {QStringLiteral("trialActivationPolicy"), QVariant::fromValue(trialPolicyObject)},
            {QStringLiteral("visible"), true}
        };
        return WhatSon::Runtime::Bootstrap::loadWhatSonAppRootObject(
            engine,
            QStringLiteral("TrialStatus"),
            trialWindowInitialProperties,
            lvrs::QmlWindowActivationPolicy::ShowRaiseAndActivate);
    };
#endif

    if (launchOptions.onboardingOnly)
    {
        const QVariantMap onboardingWindowInitialProperties{
            {QStringLiteral("hubSessionController"), QVariant::fromValue(static_cast<QObject*>(&onboardingHubController))},
            {QStringLiteral("standaloneMode"), true},
            {QStringLiteral("visible"), true}
        };
        QObject* onboardingWindow = WhatSon::Runtime::Bootstrap::loadWhatSonAppRootObject(
            engine,
            QStringLiteral("Onboarding"),
            onboardingWindowInitialProperties);
        if (onboardingWindow == nullptr)
        {
            qWarning().noquote() << QStringLiteral("Failed to resolve onboarding window root object.");
            return EXIT_FAILURE;
        }

#if defined(WHATSON_IS_TRIAL_BUILD) && !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
        QObject* trialWindow = loadTrialStatusWindow(
            onboardingWindow,
            static_cast<QObject*>(&trialActivationPolicy));
        if (trialWindow == nullptr)
        {
            qWarning().noquote() << QStringLiteral("Failed to resolve the trial status window root object.");
            return EXIT_FAILURE;
        }
#endif

        QPointer<QObject> onboardingWindowGuard(onboardingWindow);
        QObject* workspaceMainWindow = nullptr;

        const QMetaObject::Connection onboardingDismissedConnection = QObject::connect(
            onboardingWindow,
            SIGNAL(dismissed()),
            &app,
            SLOT(quit()));
        QObject::connect(
            &onboardingHubController,
            &OnboardingHubController::hubLoaded,
            &app,
            [&app,
                &engine,
                &startForegroundServices,
                &workspaceMainWindow,
                &onboardingHubController,
                &onboardingRouteBootstrapController,
                &onboardingDismissedConnection,
                onboardingWindowGuard](const QString&)
            {
                if (workspaceMainWindow != nullptr)
                {
                    return;
                }

                const QVariantMap mainWindowInitialProperties{
                    {
                        QStringLiteral("onboardingHubController"),
                        QVariant::fromValue(static_cast<QObject*>(&onboardingHubController))
                    },
                    {QStringLiteral("desktopOnboardingWindowVisible"), false},
                    {
                        QStringLiteral("onboardingRouteBootstrapController"),
                        QVariant::fromValue(static_cast<QObject*>(&onboardingRouteBootstrapController))
                    }
                };
                onboardingRouteBootstrapController.configure(false, true);
                workspaceMainWindow = WhatSon::Runtime::Bootstrap::loadMainWindow(
                    engine,
                    mainWindowInitialProperties,
                    lvrs::QmlWindowActivationPolicy::ShowRaiseAndActivate);
                if (workspaceMainWindow == nullptr)
                {
                    qWarning().noquote() <<
                        QStringLiteral("Failed to load the main workspace window after onboarding.");
                    QCoreApplication::exit(EXIT_FAILURE);
                    return;
                }

                QObject::disconnect(onboardingDismissedConnection);

                if (onboardingWindowGuard)
                {
                    QTimer::singleShot(0, &app, [onboardingWindowGuard]()
                    {
                        if (!onboardingWindowGuard)
                        {
                            return;
                        }

                        onboardingWindowGuard->setProperty("visible", false);
                    });
                }

                if (!startForegroundServices(workspaceMainWindow))
                {
                    QCoreApplication::exit(EXIT_FAILURE);
                    return;
                }
            });
        return app.exec();
    }

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    const bool useEmbeddedStartupOnboarding = true;
#else
    const bool useEmbeddedStartupOnboarding = false;
#endif
    const bool startupWorkspaceAvailable = WhatSon::Runtime::Bootstrap::startupWorkspaceReady(
        startupHubSelection.mounted,
        initialHubLoaded);
    const bool showDesktopStartupOnboarding = !startupWorkspaceAvailable && !useEmbeddedStartupOnboarding;
    onboardingRouteBootstrapController.configure(useEmbeddedStartupOnboarding, startupWorkspaceAvailable);

    const QVariantMap mainWindowInitialProperties{
        {
            QStringLiteral("onboardingHubController"),
            QVariant::fromValue(static_cast<QObject*>(&onboardingHubController))
        },
        {QStringLiteral("desktopOnboardingWindowVisible"), showDesktopStartupOnboarding},
        {
            QStringLiteral("onboardingRouteBootstrapController"),
            QVariant::fromValue(static_cast<QObject*>(&onboardingRouteBootstrapController))
        }
    };
    QObject* mainWindow = WhatSon::Runtime::Bootstrap::loadMainWindow(
        engine,
        mainWindowInitialProperties,
        lvrs::QmlWindowActivationPolicy::ShowRaiseAndActivate);
    if (mainWindow == nullptr)
    {
        return EXIT_FAILURE;
    }

    if (!scheduleDeferredStartupHierarchyPrefetch(app, engine, mainWindow, startupRuntimeCoordinator))
    {
        qWarning().noquote() << QStringLiteral("Failed to schedule deferred startup hierarchy prefetch lifecycle task.");
        return EXIT_FAILURE;
    }

#if defined(WHATSON_IS_TRIAL_BUILD) && !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
    QObject* trialWindow = loadTrialStatusWindow(
        mainWindow,
        static_cast<QObject*>(&trialActivationPolicy));
    if (trialWindow == nullptr)
    {
        qWarning().noquote() << QStringLiteral("Failed to resolve the trial status window root object.");
        return EXIT_FAILURE;
    }
#endif

    if (!startForegroundServices(mainWindow))
    {
        return EXIT_FAILURE;
    }
    return app.exec();
}
