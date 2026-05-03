#include "backend/runtime/appbootstrap.h"
#include "backend/runtime/foregroundservices.h"
#include "app/models/file/hierarchy/bookmarks/BookmarksHierarchyController.hpp"
#include "app/models/file/hierarchy/event/EventHierarchyController.hpp"
#include "app/models/file/hierarchy/library/LibraryHierarchyController.hpp"
#include "app/models/file/hierarchy/library/LibraryNoteMutationController.hpp"
#include "app/models/file/hierarchy/preset/PresetHierarchyController.hpp"
#include "app/models/file/hierarchy/progress/ProgressHierarchyController.hpp"
#include "app/models/file/hierarchy/projects/ProjectsHierarchyController.hpp"
#include "app/models/file/hierarchy/resources/ResourcesHierarchyController.hpp"
#include "app/models/file/import/ResourcesImportController.hpp"
#include "app/models/file/hierarchy/tags/TagsHierarchyController.hpp"
#include "app/models/navigationbar/EditorViewModeController.hpp"
#include "app/models/navigationbar/NavigationModeController.hpp"
#include "app/models/detailPanel/DetailPanelCurrentHierarchyBinder.hpp"
#include "app/models/detailPanel/NoteDetailPanelController.hpp"
#include "app/models/detailPanel/ResourceDetailPanelController.hpp"
#include "app/models/calendar/DayCalendarController.hpp"
#include "app/models/calendar/AgendaController.hpp"
#include "app/models/calendar/MonthCalendarController.hpp"
#include "app/models/calendar/WeekCalendarController.hpp"
#include "app/models/calendar/YearCalendarController.hpp"
#include "app/models/onboarding/OnboardingHubController.hpp"
#include "app/models/onboarding/OnboardingRouteBootstrapController.hpp"
#include "app/models/panel/PanelControllerRegistry.hpp"
#include "app/models/sidebar/HierarchySidebarDomain.hpp"
#include "app/models/sidebar/HierarchyControllerProvider.hpp"
#include "app/models/sidebar/SidebarHierarchyController.hpp"
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
#include <functional>

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

bool scheduleStartupRuntimeLoadAfterFirstIdle(
    QGuiApplication& app,
    QQmlApplicationEngine& engine,
    QObject* workspaceRootObject,
    const WhatSon::Runtime::Startup::StartupHubSelection& startupHubSelection,
    WhatSonStartupRuntimeCoordinator& startupRuntimeCoordinator,
    OnboardingHubController& onboardingHubController,
    const std::function<void(const QString&, const QByteArray&)>& publishLoadedHubConnection)
{
    if (!startupHubSelection.mounted)
    {
        return true;
    }

    lvrs::QmlBootstrapTask runtimeLoadTask;
    runtimeLoadTask.name = QStringLiteral("whatson-startup-runtime-load");
    runtimeLoadTask.stage = lvrs::QmlAppLifecycleStage::AfterFirstIdle;
    runtimeLoadTask.priority = 10;
    runtimeLoadTask.fatal = false;
    runtimeLoadTask.run =
        [&startupRuntimeCoordinator,
         &onboardingHubController,
         startupHubSelection,
         publishLoadedHubConnection](const lvrs::QmlAppLifecycleContext&, QString* errorMessage)
    {
        QString loadError;
        const bool loaded = startupRuntimeCoordinator.loadHubIntoRuntime(
            startupHubSelection.hubPath,
            &loadError);
        if (loaded)
        {
            if (publishLoadedHubConnection)
            {
                publishLoadedHubConnection(
                    startupHubSelection.hubPath,
                    startupHubSelection.accessBookmark);
            }
            onboardingHubController.completeWorkspaceTransition();
            return true;
        }

        const QString failureMessage = loadError.trimmed().isEmpty()
                                           ? QStringLiteral("Failed to load the startup WhatSon Hub.")
                                           : loadError.trimmed();
        onboardingHubController.failWorkspaceTransition(failureMessage);
        qWarning().noquote()
            << QStringLiteral("Failed to load startup WhatSon Hub '%1' after first idle: %2")
                   .arg(startupHubSelection.hubPath, failureMessage);

        if (errorMessage != nullptr)
        {
            *errorMessage = failureMessage;
        }
        return false;
    };

    lvrs::QmlAppLifecycleHooks lifecycleHooks;
    lifecycleHooks.tasks.append(runtimeLoadTask);
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
    WhatSon::Debug::installThirdPartyTraceMessageFilter();

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
    LibraryHierarchyController libraryHierarchyController;
    LibraryNoteMutationController libraryNoteMutationController;
    ProjectsHierarchyController projectsHierarchyController;
    BookmarksHierarchyController bookmarksHierarchyController;
    TagsHierarchyController tagsHierarchyController;
    ResourcesHierarchyController resourcesHierarchyController;
    ResourcesImportController resourcesImportController;
    ProgressHierarchyController progressHierarchyController;
    EventHierarchyController eventHierarchyController;
    PresetHierarchyController presetHierarchyController;
    SidebarSelectionStore sidebarSelectionStore;
    SelectedHubStore selectedHubStore;
    HierarchyControllerProvider hierarchyControllerProvider;
    SidebarHierarchyController sidebarHierarchyController;
    DetailPanelCurrentHierarchyBinder detailPanelCurrentHierarchyBinder;
    NoteDetailPanelController noteDetailPanelController;
    ResourceDetailPanelController resourceDetailPanelController;
    EditorViewModeController editorViewModeController;
    NavigationModeController navigationModeController;
    WhatSonAsyncScheduler asyncScheduler;
    PanelControllerRegistry panelControllerRegistry;
    DayCalendarController dayCalendarController;
    AgendaController agendaController;
    MonthCalendarController monthCalendarController;
    WeekCalendarController weekCalendarController;
    YearCalendarController yearCalendarController;
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
    libraryNoteMutationController.setSourceController(&libraryHierarchyController);
    dayCalendarController.setCalendarBoardStore(&calendarBoardStore);
    agendaController.setCalendarBoardStore(&calendarBoardStore);
    monthCalendarController.setCalendarBoardStore(&calendarBoardStore);
    weekCalendarController.setCalendarBoardStore(&calendarBoardStore);
    yearCalendarController.setCalendarBoardStore(&calendarBoardStore);
    calendarBoardStore.setProjectedNotesProvider(
        [&libraryHierarchyController]()
        {
            return libraryHierarchyController.indexedNotesSnapshot();
        });

    const auto requestNewLibraryNote = [&libraryNoteMutationController, &sidebarHierarchyController]()
    {
        sidebarHierarchyController.setActiveHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library));
        if (!libraryNoteMutationController.createEmptyNote())
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

    if (auto* addNewPanelController = qobject_cast<PanelController*>(
        panelControllerRegistry.panelController(QStringLiteral("navigation.NavigationAddNewBar"))))
    {
        QObject::connect(
            addNewPanelController,
            &PanelController::controllerHookRequested,
            &app,
            handlePanelCreateNoteRequest);
    }
    if (auto* applicationControlPanelController = qobject_cast<PanelController*>(
        panelControllerRegistry.panelController(QStringLiteral("navigation.NavigationApplicationControlBar"))))
    {
        QObject::connect(
            applicationControlPanelController,
            &PanelController::controllerHookRequested,
            &app,
            handlePanelCreateNoteRequest);
    }
    if (auto* applicationViewPanelController = qobject_cast<PanelController*>(
        panelControllerRegistry.panelController(QStringLiteral("navigation.NavigationApplicationViewBar"))))
    {
        QObject::connect(
            applicationViewPanelController,
            &PanelController::controllerHookRequested,
            &app,
            handlePanelCreateNoteRequest);
    }
    if (auto* applicationEditPanelController = qobject_cast<PanelController*>(
        panelControllerRegistry.panelController(QStringLiteral("navigation.NavigationApplicationEditBar"))))
    {
        QObject::connect(
            applicationEditPanelController,
            &PanelController::controllerHookRequested,
            &app,
            handlePanelCreateNoteRequest);
    }

    libraryHierarchyController.setSystemCalendarStore(&systemCalendarStore);
    bookmarksHierarchyController.setSystemCalendarStore(&systemCalendarStore);
    QObject::connect(
        &libraryHierarchyController,
        &LibraryHierarchyController::noteDeleted,
        &bookmarksHierarchyController,
        [&bookmarksHierarchyController](const QString& noteId)
        {
            bookmarksHierarchyController.removeNoteById(noteId);
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
    startupRuntimeTargets.libraryController = &libraryHierarchyController;
    startupRuntimeTargets.projectsController = &projectsHierarchyController;
    startupRuntimeTargets.bookmarksController = &bookmarksHierarchyController;
    startupRuntimeTargets.tagsController = &tagsHierarchyController;
    startupRuntimeTargets.resourcesController = &resourcesHierarchyController;
    startupRuntimeTargets.progressController = &progressHierarchyController;
    startupRuntimeTargets.eventController = &eventHierarchyController;
    startupRuntimeTargets.presetController = &presetHierarchyController;
    startupRuntimeTargets.hubRuntimeStore = &hubRuntimeStore;
    WhatSonStartupRuntimeCoordinator startupRuntimeCoordinator(startupRuntimeTargets);
    WhatSonRuntimeParallelLoader runtimeParallelLoader;
    startupRuntimeCoordinator.setParallelLoader(&runtimeParallelLoader);

    WhatSonHubSyncController hubSyncController;
    const auto reloadCalendarProjectedNotesFromRuntime =
        [&calendarBoardStore, &hubSyncController, &libraryHierarchyController]()
    {
        calendarBoardStore.setProjectedNotesHubPath(hubSyncController.currentHubPath());
        calendarBoardStore.reloadProjectedNotesFromSnapshot(
            libraryHierarchyController.indexedNotesSnapshot());
    };
    const auto upsertCalendarProjectedNoteFromRuntime =
        [&calendarBoardStore, &libraryHierarchyController](const QString& noteId)
    {
        LibraryNoteRecord note;
        if (!libraryHierarchyController.indexedNoteRecordById(noteId, &note))
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
                &libraryHierarchyController,
                &projectsHierarchyController,
                &bookmarksHierarchyController,
                &resourcesHierarchyController,
                &progressHierarchyController
            });
    QObject::connect(
        &hubSyncController,
        &WhatSonHubSyncController::syncReloaded,
        &app,
        [&calendarBoardStore, &libraryHierarchyController](const QString& hubPath)
        {
            calendarBoardStore.setProjectedNotesHubPath(hubPath);
            calendarBoardStore.reloadProjectedNotesFromSnapshot(
                libraryHierarchyController.indexedNotesSnapshot());
        });
    QObject::connect(
        &libraryHierarchyController,
        &LibraryHierarchyController::indexedNotesSnapshotChanged,
        &app,
        reloadCalendarProjectedNotesFromRuntime);
    QObject::connect(
        &libraryHierarchyController,
        &LibraryHierarchyController::indexedNoteUpserted,
        &app,
        upsertCalendarProjectedNoteFromRuntime);
    QObject::connect(
        &libraryHierarchyController,
        &LibraryHierarchyController::noteDeleted,
        &app,
        [&calendarBoardStore](const QString& noteId)
        {
            calendarBoardStore.removeProjectedNoteBySourceId(noteId);
        });
    QObject::connect(
        &bookmarksHierarchyController,
        &BookmarksHierarchyController::hubFilesystemMutated,
        &app,
        requestCalendarProjectedNotesReload);
    QObject::connect(
        &progressHierarchyController,
        &ProgressHierarchyController::hubFilesystemMutated,
        &app,
        requestCalendarProjectedNotesReload);
    Q_UNUSED(hubSyncWiring);
    resourcesImportController.setReloadResourcesCallback(
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
    const auto publishLoadedHubConnection =
        [&selectedHubStore,
         &hubSyncController,
         &resourcesImportController,
         &calendarBoardStore,
         &libraryHierarchyController](const QString& hubPath, const QByteArray& accessBookmark)
    {
        selectedHubStore.setSelectedHubSelection(hubPath, accessBookmark);
        hubSyncController.setCurrentHubPath(hubPath);
        resourcesImportController.setCurrentHubPath(hubPath);
        calendarBoardStore.setProjectedNotesHubPath(hubPath);
        calendarBoardStore.reloadProjectedNotesFromSnapshot(
            libraryHierarchyController.indexedNotesSnapshot());
    };
    QObject::connect(
        &onboardingHubController,
        &OnboardingHubController::hubLoaded,
        &app,
        [&onboardingHubController,
         &publishLoadedHubConnection](const QString& hubPath)
        {
            publishLoadedHubConnection(
                hubPath,
                onboardingHubController.currentHubAccessBookmark());
        });

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
        onboardingHubController.syncCurrentHubSelection(startupHubSelection.hubPath);
        onboardingHubController.completeWorkspaceTransition();
    }

    hierarchyControllerProvider.setMappings(QVector<HierarchyControllerProvider::Mapping>{
        { static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library), &libraryHierarchyController },
        { static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Projects), &projectsHierarchyController },
        { static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Bookmarks), &bookmarksHierarchyController },
        { static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Tags), &tagsHierarchyController },
        { static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Resources), &resourcesHierarchyController },
        { static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Progress), &progressHierarchyController },
        { static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Event), &eventHierarchyController },
        { static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Preset), &presetHierarchyController },
    });
    noteDetailPanelController.setProjectSelectionSourceController(&projectsHierarchyController);
    noteDetailPanelController.setBookmarkSelectionSourceController(&bookmarksHierarchyController);
    noteDetailPanelController.setProgressSelectionSourceController(&progressHierarchyController);
    noteDetailPanelController.setTagsSourceController(&tagsHierarchyController);
    sidebarHierarchyController.setSelectionStore(&sidebarSelectionStore);
    sidebarHierarchyController.setControllerProvider(&hierarchyControllerProvider);
    detailPanelCurrentHierarchyBinder.setNoteDetailPanelController(&noteDetailPanelController);
    detailPanelCurrentHierarchyBinder.setResourceDetailPanelController(&resourceDetailPanelController);
    detailPanelCurrentHierarchyBinder.setHierarchyContextSource(&sidebarHierarchyController);

    WhatSon::Policy::ArchitecturePolicyLock::lock();

    WhatSon::Runtime::Bootstrap::WorkspaceContextObjects workspaceContextObjects;
    workspaceContextObjects.libraryHierarchyController = &libraryHierarchyController;
    workspaceContextObjects.libraryNoteMutationController = &libraryNoteMutationController;
    workspaceContextObjects.projectsHierarchyController = &projectsHierarchyController;
    workspaceContextObjects.bookmarksHierarchyController = &bookmarksHierarchyController;
    workspaceContextObjects.tagsHierarchyController = &tagsHierarchyController;
    workspaceContextObjects.resourcesHierarchyController = &resourcesHierarchyController;
    workspaceContextObjects.resourcesImportController = &resourcesImportController;
    workspaceContextObjects.progressHierarchyController = &progressHierarchyController;
    workspaceContextObjects.eventHierarchyController = &eventHierarchyController;
    workspaceContextObjects.presetHierarchyController = &presetHierarchyController;
    workspaceContextObjects.detailPanelController = &noteDetailPanelController;
    workspaceContextObjects.noteDetailPanelController = &noteDetailPanelController;
    workspaceContextObjects.resourceDetailPanelController = &resourceDetailPanelController;
    workspaceContextObjects.editorViewModeController = &editorViewModeController;
    workspaceContextObjects.navigationModeController = &navigationModeController;
    workspaceContextObjects.sidebarHierarchyController = &sidebarHierarchyController;
    workspaceContextObjects.asyncScheduler = &asyncScheduler;
    workspaceContextObjects.calendarBoardStore = &calendarBoardStore;
    workspaceContextObjects.systemCalendarStore = &systemCalendarStore;
    workspaceContextObjects.dayCalendarController = &dayCalendarController;
    workspaceContextObjects.agendaController = &agendaController;
    workspaceContextObjects.monthCalendarController = &monthCalendarController;
    workspaceContextObjects.weekCalendarController = &weekCalendarController;
    workspaceContextObjects.yearCalendarController = &yearCalendarController;
    workspaceContextObjects.panelControllerRegistry = &panelControllerRegistry;
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
        startupHubSelection.mounted);
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

    if (!scheduleStartupRuntimeLoadAfterFirstIdle(
            app,
            engine,
            mainWindow,
            startupHubSelection,
            startupRuntimeCoordinator,
            onboardingHubController,
            publishLoadedHubConnection))
    {
        qWarning().noquote() << QStringLiteral("Failed to schedule startup runtime load lifecycle task.");
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
