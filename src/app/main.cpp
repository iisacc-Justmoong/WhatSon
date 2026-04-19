#include "backend/runtime/appbootstrap.h"
#include "viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/event/EventHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/library/LibraryHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/library/LibraryNoteMutationViewModel.hpp"
#include "viewmodel/hierarchy/preset/PresetHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/progress/ProgressHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/projects/ProjectsHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/resources/ResourcesHierarchyViewModel.hpp"
#include "file/import/ResourcesImportViewModel.hpp"
#include "viewmodel/hierarchy/tags/TagsHierarchyViewModel.hpp"
#include "viewmodel/navigationbar/EditorViewModeViewModel.hpp"
#include "viewmodel/navigationbar/NavigationModeViewModel.hpp"
#include "viewmodel/detailPanel/DetailPanelCurrentHierarchyBinder.hpp"
#include "viewmodel/detailPanel/NoteDetailPanelViewModel.hpp"
#include "viewmodel/detailPanel/ResourceDetailPanelViewModel.hpp"
#include "viewmodel/calendar/DayCalendarViewModel.hpp"
#include "viewmodel/calendar/AgendaViewModel.hpp"
#include "viewmodel/calendar/MonthCalendarViewModel.hpp"
#include "viewmodel/calendar/WeekCalendarViewModel.hpp"
#include "viewmodel/calendar/YearCalendarViewModel.hpp"
#include "viewmodel/onboarding/OnboardingHubController.hpp"
#include "viewmodel/onboarding/OnboardingRouteBootstrapController.hpp"
#include "viewmodel/panel/PanelViewModelRegistry.hpp"
#include "viewmodel/sidebar/HierarchySidebarDomain.hpp"
#include "viewmodel/sidebar/HierarchyViewModelProvider.hpp"
#include "viewmodel/sidebar/SidebarHierarchyViewModel.hpp"
#include "calendar/CalendarBoardStore.hpp"
#include "calendar/SystemCalendarStore.hpp"
#include "policy/ArchitecturePolicyLock.hpp"
#include "hub/WhatSonHubRuntimeStore.hpp"
#include "runtime/threading/WhatSonRuntimeParallelLoader.hpp"
#include "runtime/startup/WhatSonStartupRuntimeCoordinator.hpp"
#include "runtime/startup/WhatSonStartupHubResolver.hpp"
#include "runtime/bootstrap/WhatSonAppLaunchSupport.hpp"
#include "runtime/bootstrap/WhatSonHubSyncWiring.hpp"
#include "runtime/bootstrap/WhatSonQmlContextBinder.hpp"
#include "runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.hpp"
#include "permissions/WhatSonPermissionBootstrapper.hpp"
#include "runtime/scheduler/WhatSonAsyncScheduler.hpp"
#include "file/hub/WhatSonHubCreator.hpp"
#include "file/hub/WhatSonHubPathUtils.hpp"
#include "file/WhatSonDebugTrace.hpp"
#include "platform/Android/WhatSonAndroidStorageBackend.hpp"
#include "platform/Apple/AppleSecurityScopedResourceAccess.hpp"
#include "permissions/ApplePermissionBridge.hpp"
#include "store/hub/SelectedHubStore.hpp"
#include "store/sidebar/SidebarSelectionStore.hpp"
#include "file/sync/WhatSonHubSyncController.hpp"
#if defined(WHATSON_IS_TRIAL_BUILD) && !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
#include "WhatSonTrialActivationPolicy.hpp"
#endif

#include <QByteArray>
#include <QCoreApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QIcon>
#include <QPointer>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSet>
#include <QTimer>
#include <QVariant>
#include <QVector>
#include <QWindow>
#include <QtCore/qglobal.h>

#include <cstdlib>
#include <functional>
#include <memory>
#include <utility>

void qml_register_types_LVRS();

namespace
{
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
    WhatSon::Runtime::Bootstrap::registerInternalQmlTypes();
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
    OnboardingRouteBootstrapController onboardingRouteBootstrapController;
    onboardingRouteBootstrapController.setHubController(&onboardingHubController);
    const auto applyMountedHubSelection =
        [&selectedHubStore,
         &hubSyncController,
         &resourcesImportViewModel,
         &calendarBoardStore,
         &libraryHierarchyViewModel](const QString& hubPath,
                                     const QByteArray& accessBookmark,
                                     const QString& selectionUrl)
    {
        selectedHubStore.setSelectedHubSelection(
            hubPath,
            accessBookmark,
            selectionUrl);
        hubSyncController.setCurrentHubPath(hubPath);
        resourcesImportViewModel.setCurrentHubPath(hubPath);
        calendarBoardStore.setProjectedNotesHubPath(hubPath);
        calendarBoardStore.reloadProjectedNotesFromSnapshot(
            libraryHierarchyViewModel.indexedNotesSnapshot());
    };
    QObject::connect(
        &onboardingHubController,
        &OnboardingHubController::hubSelectionResolved,
        &app,
        [&onboardingHubController,
         &startupRuntimeCoordinator,
         &applyMountedHubSelection](const QString& hubPath)
        {
            onboardingHubController.beginHubLoad();

            QString errorMessage;
            const bool loaded = startupRuntimeCoordinator.loadHubIntoRuntime(
                hubPath,
                &errorMessage);
            if (!loaded)
            {
                onboardingHubController.failHubLoad(
                    errorMessage.trimmed().isEmpty()
                        ? QStringLiteral("Failed to load the selected WhatSon Hub.")
                        : errorMessage.trimmed());
                return;
            }

            applyMountedHubSelection(
                hubPath,
                onboardingHubController.currentHubAccessBookmark(),
                onboardingHubController.currentHubSelectionUrl());
            onboardingHubController.completeHubLoad(hubPath);
        });

    bool initialHubLoaded = false;
    const WhatSon::Runtime::Startup::StartupHubSelection startupHubSelection =
        WhatSon::Runtime::Startup::resolveStartupHubSelection(selectedHubStore);

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
            applyMountedHubSelection(
                startupHubSelection.hubPath,
                startupHubSelection.accessBookmark,
                startupHubSelection.selectionUrl);
        }
        if (!initialHubLoaded)
        {
            const QString startupLoadError = errorMessage.trimmed().isEmpty()
                ? QStringLiteral("Failed to load the selected WhatSon Hub.")
                : errorMessage.trimmed();
            onboardingHubController.failHubLoad(startupLoadError);
            qWarning().noquote()
                << QStringLiteral("Failed to load startup WhatSon Hub '%1': %2")
                       .arg(startupHubSelection.hubPath, startupLoadError);
        }
    }

    const bool startupHubConnected = startupHubSelection.mounted && initialHubLoaded;

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
    WhatSon::Runtime::Bootstrap::bindWorkspaceContextObjects(engine.rootContext(), workspaceContextObjects);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(EXIT_FAILURE); },
        Qt::QueuedConnection);

    bool foregroundServicesStarted = false;
    const auto startForegroundServices = [&asyncScheduler, &permissionBootstrapper, &foregroundServicesStarted]()
    {
        if (foregroundServicesStarted)
        {
            return;
        }

        foregroundServicesStarted = true;
        asyncScheduler.start();
        QTimer::singleShot(0, &permissionBootstrapper, [&permissionBootstrapper]()
        {
            permissionBootstrapper.start();
        });
    };

    const auto loadWindowFromModule =
        [&engine](const QString& moduleUri, const QString& typeName, const QVariantMap& initialProperties = {}) -> QObject*
    {
        const qsizetype rootObjectCount = engine.rootObjects().size();
        engine.setInitialProperties(initialProperties);
        engine.loadFromModule(moduleUri, typeName);
        engine.setInitialProperties({});
        if (engine.rootObjects().size() <= rootObjectCount)
        {
            return nullptr;
        }

        return engine.rootObjects().constLast();
    };

    const auto loadMainWindow =
        [&loadWindowFromModule](const QVariantMap& initialProperties = {}) -> QObject*
    {
        return loadWindowFromModule(
            QStringLiteral("WhatSon.App"),
            QStringLiteral("Main"),
            initialProperties);
    };
    const auto activateWindowObject = [](QObject* windowObject)
    {
        if (auto* window = qobject_cast<QWindow*>(windowObject))
        {
            window->show();
            window->raise();
            window->requestActivate();
        }
    };

#if defined(WHATSON_IS_TRIAL_BUILD) && !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
    const auto loadTrialStatusWindow =
        [&loadWindowFromModule](QObject* hostWindow, QObject* trialPolicyObject) -> QObject*
    {
        const QVariantMap trialWindowInitialProperties{
            {QStringLiteral("hostWindow"), QVariant::fromValue(hostWindow)},
            {QStringLiteral("trialActivationPolicy"), QVariant::fromValue(trialPolicyObject)},
            {QStringLiteral("visible"), true}
        };
        return loadWindowFromModule(
            QStringLiteral("WhatSon.App"),
            QStringLiteral("TrialStatus"),
            trialWindowInitialProperties);
    };
#endif
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    const bool enableEmbeddedOnboardingPresentation = true;
#else
    const bool enableEmbeddedOnboardingPresentation = false;
#endif

    if (launchOptions.onboardingOnly)
    {
        const QVariantMap onboardingWindowInitialProperties{
            {QStringLiteral("hubSessionController"), QVariant::fromValue(static_cast<QObject*>(&onboardingHubController))},
            {QStringLiteral("standaloneMode"), true},
            {QStringLiteral("visible"), true}
        };
        QObject* onboardingWindow = loadWindowFromModule(
            QStringLiteral("WhatSon.App"),
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
        activateWindowObject(trialWindow);
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
                &loadMainWindow,
                &startForegroundServices,
                &workspaceMainWindow,
                &onboardingHubController,
                &onboardingRouteBootstrapController,
                &onboardingDismissedConnection,
                &activateWindowObject,
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
                onboardingRouteBootstrapController.configure(enableEmbeddedOnboardingPresentation, true);
                workspaceMainWindow = loadMainWindow(mainWindowInitialProperties);
                if (workspaceMainWindow == nullptr)
                {
                    qWarning().noquote() <<
                        QStringLiteral("Failed to load the main workspace window after onboarding.");
                    QCoreApplication::exit(EXIT_FAILURE);
                    return;
                }

                QObject::disconnect(onboardingDismissedConnection);
                activateWindowObject(workspaceMainWindow);

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

                startForegroundServices();
            });
        return app.exec();
    }

    const bool showDesktopStartupOnboarding = !startupHubConnected
                                              && !enableEmbeddedOnboardingPresentation;
    onboardingRouteBootstrapController.configure(
        enableEmbeddedOnboardingPresentation,
        startupHubConnected);

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
    QObject* mainWindow = loadMainWindow(mainWindowInitialProperties);
    if (mainWindow == nullptr)
    {
        return EXIT_FAILURE;
    }

    activateWindowObject(mainWindow);

    if (startupRuntimeCoordinator.startupDeferredBootstrapActive())
    {
        const QVector<int> deferredStartupHierarchyOrder{
            static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Event),
            static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Preset)
        };
        auto deferredStartupHierarchyCursor = std::make_shared<int>(0);
        auto scheduleDeferredStartupHierarchyLoad = std::make_shared<std::function<void()>>();
        *scheduleDeferredStartupHierarchyLoad =
            [&app,
                &startupRuntimeCoordinator,
                deferredStartupHierarchyOrder,
                deferredStartupHierarchyCursor,
                scheduleDeferredStartupHierarchyLoad]()
        {
            if (!startupRuntimeCoordinator.startupDeferredBootstrapActive())
            {
                return;
            }

            const QSet<int> startupLoadedHierarchyIndices =
                startupRuntimeCoordinator.startupLoadedHierarchyIndices();
            while (*deferredStartupHierarchyCursor < deferredStartupHierarchyOrder.size())
            {
                const int hierarchyIndex = deferredStartupHierarchyOrder.at(*deferredStartupHierarchyCursor);
                *deferredStartupHierarchyCursor += 1;
                if (startupLoadedHierarchyIndices.contains(hierarchyIndex))
                {
                    continue;
                }

                QTimer::singleShot(0, &app, [hierarchyIndex, &startupRuntimeCoordinator, scheduleDeferredStartupHierarchyLoad]()
                {
                    startupRuntimeCoordinator.ensureDeferredStartupHierarchyLoaded(
                        hierarchyIndex,
                        QStringLiteral("startup-idle-prefetch"));
                    (*scheduleDeferredStartupHierarchyLoad)();
                });
                return;
            }
        };
        QTimer::singleShot(0, &app, [scheduleDeferredStartupHierarchyLoad]()
        {
            (*scheduleDeferredStartupHierarchyLoad)();
        });
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
    activateWindowObject(trialWindow);
#endif

    startForegroundServices();
    return app.exec();
}
