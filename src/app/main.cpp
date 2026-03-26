#include "backend/runtime/appbootstrap.h"
#include "viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/event/EventHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/library/LibraryHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/library/LibraryNoteMutationViewModel.hpp"
#include "viewmodel/hierarchy/preset/PresetHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/progress/ProgressHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/projects/ProjectsHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/resources/ResourcesHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/tags/TagsHierarchyViewModel.hpp"
#include "viewmodel/navigationbar/EditorViewModeViewModel.hpp"
#include "viewmodel/navigationbar/NavigationModeViewModel.hpp"
#include "viewmodel/detailPanel/DetailPanelViewModel.hpp"
#include "viewmodel/content/ContentsEditorSelectionBridge.hpp"
#include "viewmodel/content/ContentsGutterMarkerBridge.hpp"
#include "viewmodel/content/ContentsLogicalTextBridge.hpp"
#include "viewmodel/onboarding/OnboardingHubController.hpp"
#include "viewmodel/onboarding/OnboardingRouteBootstrapController.hpp"
#include "viewmodel/panel/FocusedNoteDeletionBridge.hpp"
#include "viewmodel/panel/HierarchyDragDropBridge.hpp"
#include "viewmodel/panel/HierarchyInteractionBridge.hpp"
#include "viewmodel/panel/PanelViewModelRegistry.hpp"
#include "viewmodel/sidebar/HierarchySidebarDomain.hpp"
#include "viewmodel/sidebar/HierarchyViewModelProvider.hpp"
#include "viewmodel/sidebar/SidebarHierarchyViewModel.hpp"
#include "calendar/SystemCalendarStore.hpp"
#include "policy/ArchitecturePolicyLock.hpp"
#include "hub/WhatSonHubRuntimeStore.hpp"
#include "runtime/threading/WhatSonRuntimeParallelLoader.hpp"
#include "runtime/startup/WhatSonStartupRuntimeCoordinator.hpp"
#include "runtime/startup/WhatSonStartupHubResolver.hpp"
#include "runtime/bootstrap/WhatSonAppLaunchSupport.hpp"
#include "runtime/permissions/WhatSonPermissionBootstrapper.hpp"
#include "runtime/scheduler/WhatSonAsyncScheduler.hpp"
#include "file/hub/WhatSonHubCreator.hpp"
#include "file/hub/WhatSonHubPathUtils.hpp"
#include "file/hub/WhatSonHubWriteLease.hpp"
#include "file/WhatSonDebugTrace.hpp"
#include "platform/Android/WhatSonAndroidStorageBackend.hpp"
#include "platform/Apple/AppleSecurityScopedResourceAccess.hpp"
#include "permissions/ApplePermissionBridge.hpp"
#include "store/hub/SelectedHubStore.hpp"
#include "store/sidebar/SidebarSelectionStore.hpp"
#include "sync/WhatSonHubSyncController.hpp"
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
#include <qqml.h>
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
    qmlRegisterType<ContentsEditorSelectionBridge>(
        "WhatSon.App.Internal", 1, 0, "ContentsEditorSelectionBridge");
    qmlRegisterType<ContentsLogicalTextBridge>(
        "WhatSon.App.Internal", 1, 0, "ContentsLogicalTextBridge");
    qmlRegisterType<ContentsGutterMarkerBridge>(
        "WhatSon.App.Internal", 1, 0, "ContentsGutterMarkerBridge");
    qmlRegisterType<FocusedNoteDeletionBridge>(
        "WhatSon.App.Internal", 1, 0, "FocusedNoteDeletionBridge");
    qmlRegisterType<HierarchyDragDropBridge>(
        "WhatSon.App.Internal", 1, 0, "HierarchyDragDropBridge");
    qmlRegisterType<HierarchyInteractionBridge>(
        "WhatSon.App.Internal", 1, 0, "HierarchyInteractionBridge");
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
    SystemCalendarStore systemCalendarStore;
    LibraryHierarchyViewModel libraryHierarchyViewModel;
    LibraryNoteMutationViewModel libraryNoteMutationViewModel;
    ProjectsHierarchyViewModel projectsHierarchyViewModel;
    BookmarksHierarchyViewModel bookmarksHierarchyViewModel;
    TagsHierarchyViewModel tagsHierarchyViewModel;
    ResourcesHierarchyViewModel resourcesHierarchyViewModel;
    ProgressHierarchyViewModel progressHierarchyViewModel;
    EventHierarchyViewModel eventHierarchyViewModel;
    PresetHierarchyViewModel presetHierarchyViewModel;
    SidebarSelectionStore sidebarSelectionStore;
    SelectedHubStore selectedHubStore;
    QString currentWriteLeaseHubPath;
    QTimer hubWriteLeaseTimer;
    hubWriteLeaseTimer.setInterval(WhatSon::HubWriteLease::heartbeatIntervalMs());
    const auto updateWriteLeaseOwnership = [&currentWriteLeaseHubPath, &hubWriteLeaseTimer](const QString& hubPath)
    {
        const QString normalizedHubPath = hubPath.trimmed().isEmpty()
                                              ? QString()
                                              : WhatSon::HubPath::normalizeAbsolutePath(hubPath);
        if (normalizedHubPath.isEmpty())
        {
            if (!currentWriteLeaseHubPath.isEmpty())
            {
                WhatSon::HubWriteLease::releaseWriteLeaseForHub(currentWriteLeaseHubPath, nullptr);
                currentWriteLeaseHubPath.clear();
            }
            hubWriteLeaseTimer.stop();
            return;
        }

        if (currentWriteLeaseHubPath != normalizedHubPath && !currentWriteLeaseHubPath.isEmpty())
        {
            WhatSon::HubWriteLease::releaseWriteLeaseForHub(currentWriteLeaseHubPath, nullptr);
        }

        currentWriteLeaseHubPath = normalizedHubPath;
        hubWriteLeaseTimer.start();
    };
    QObject::connect(&hubWriteLeaseTimer, &QTimer::timeout, &app, [&currentWriteLeaseHubPath]()
    {
        if (currentWriteLeaseHubPath.isEmpty())
        {
            return;
        }

        QString leaseError;
        if (!WhatSon::HubWriteLease::refreshWriteLeaseForHub(currentWriteLeaseHubPath, &leaseError))
        {
            qWarning().noquote()
                << QStringLiteral("Failed to refresh WhatSon Hub write lease '%1': %2")
                .arg(currentWriteLeaseHubPath, leaseError.trimmed());
        }
    });
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &app, [&currentWriteLeaseHubPath]()
    {
        if (currentWriteLeaseHubPath.isEmpty())
        {
            return;
        }

        WhatSon::HubWriteLease::releaseWriteLeaseForHub(currentWriteLeaseHubPath, nullptr);
    });
    HierarchyViewModelProvider hierarchyViewModelProvider;
    SidebarHierarchyViewModel sidebarHierarchyViewModel;
    DetailPanelViewModel detailPanelViewModel;
    EditorViewModeViewModel editorViewModeViewModel;
    NavigationModeViewModel navigationModeViewModel;
    WhatSonAsyncScheduler asyncScheduler;
    PanelViewModelRegistry panelViewModelRegistry;
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

    WhatSonHubSyncController hubSyncController;
    hubSyncController.setReloadCallback(
        [&startupRuntimeCoordinator](const QString& hubPath, QString* errorMessage) -> bool
        {
            return startupRuntimeCoordinator.loadHubIntoRuntime(hubPath, errorMessage);
        });
    QObject::connect(
        &hubSyncController,
        &WhatSonHubSyncController::syncFailed,
        &app,
        [](const QString& errorMessage)
        {
            if (!errorMessage.trimmed().isEmpty())
            {
                qWarning().noquote() << QStringLiteral("Hub sync failed: %1").arg(errorMessage.trimmed());
            }
        });
    QObject::connect(
        &libraryHierarchyViewModel,
        &LibraryHierarchyViewModel::hubFilesystemMutated,
        &hubSyncController,
        &WhatSonHubSyncController::acknowledgeLocalMutation);
    QObject::connect(
        &bookmarksHierarchyViewModel,
        &BookmarksHierarchyViewModel::hubFilesystemMutated,
        &hubSyncController,
        &WhatSonHubSyncController::acknowledgeLocalMutation);

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
        [&onboardingHubController, &selectedHubStore, &updateWriteLeaseOwnership, &hubSyncController](const QString& hubPath)
        {
            selectedHubStore.setSelectedHubSelection(
                hubPath,
                onboardingHubController.currentHubAccessBookmark());
            updateWriteLeaseOwnership(hubPath);
            hubSyncController.setCurrentHubPath(hubPath);
        });

    bool initialHubLoaded = false;
    const QString blueprintFallbackHubPath = WhatSon::Runtime::Bootstrap::resolveBlueprintHubPath();
    const WhatSon::Runtime::Startup::StartupHubSelection startupHubSelection =
        WhatSon::Runtime::Startup::resolveStartupHubSelection(selectedHubStore, blueprintFallbackHubPath);

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
            updateWriteLeaseOwnership(startupHubSelection.hubPath);
            hubSyncController.setCurrentHubPath(startupHubSelection.hubPath);
        }
        if (!initialHubLoaded && !errorMessage.trimmed().isEmpty())
        {
            qWarning().noquote()
                << QStringLiteral("Failed to load startup WhatSon Hub '%1': %2")
                       .arg(startupHubSelection.hubPath, errorMessage.trimmed());
        }
    }

    HierarchyViewModelProvider::Targets hierarchyViewModelTargets;
    hierarchyViewModelTargets.libraryViewModel = &libraryHierarchyViewModel;
    hierarchyViewModelTargets.projectsViewModel = &projectsHierarchyViewModel;
    hierarchyViewModelTargets.bookmarksViewModel = &bookmarksHierarchyViewModel;
    hierarchyViewModelTargets.tagsViewModel = &tagsHierarchyViewModel;
    hierarchyViewModelTargets.resourcesViewModel = &resourcesHierarchyViewModel;
    hierarchyViewModelTargets.progressViewModel = &progressHierarchyViewModel;
    hierarchyViewModelTargets.eventViewModel = &eventHierarchyViewModel;
    hierarchyViewModelTargets.presetViewModel = &presetHierarchyViewModel;
    hierarchyViewModelProvider.setTargets(hierarchyViewModelTargets);
    detailPanelViewModel.setProjectSelectionSourceViewModel(&projectsHierarchyViewModel);
    detailPanelViewModel.setBookmarkSelectionSourceViewModel(&bookmarksHierarchyViewModel);
    detailPanelViewModel.setProgressSelectionSourceViewModel(&progressHierarchyViewModel);
    detailPanelViewModel.setCurrentNoteListModel(libraryHierarchyViewModel.noteListModel());
    detailPanelViewModel.setCurrentNoteDirectorySourceViewModel(&libraryHierarchyViewModel);
    sidebarHierarchyViewModel.setSelectionStore(&sidebarSelectionStore);
    sidebarHierarchyViewModel.setViewModelProvider(&hierarchyViewModelProvider);

    startupRuntimeCoordinator.bindSidebarActivation(&sidebarHierarchyViewModel);

    WhatSon::Policy::ArchitecturePolicyLock::lock();

    engine.rootContext()->setContextProperty(QStringLiteral("libraryHierarchyViewModel"), &libraryHierarchyViewModel);
    engine.rootContext()->setContextProperty(
        QStringLiteral("libraryNoteMutationViewModel"),
        &libraryNoteMutationViewModel);
    engine.rootContext()->setContextProperty(QStringLiteral("projectsHierarchyViewModel"), &projectsHierarchyViewModel);
    engine.rootContext()->setContextProperty(
        QStringLiteral("bookmarksHierarchyViewModel"),
        &bookmarksHierarchyViewModel);
    engine.rootContext()->setContextProperty(QStringLiteral("tagsHierarchyViewModel"), &tagsHierarchyViewModel);
    engine.rootContext()->setContextProperty(
        QStringLiteral("resourcesHierarchyViewModel"),
        &resourcesHierarchyViewModel);
    engine.rootContext()->setContextProperty(
        QStringLiteral("progressHierarchyViewModel"),
        &progressHierarchyViewModel);
    engine.rootContext()->setContextProperty(QStringLiteral("eventHierarchyViewModel"), &eventHierarchyViewModel);
    engine.rootContext()->setContextProperty(QStringLiteral("presetHierarchyViewModel"), &presetHierarchyViewModel);
    engine.rootContext()->setContextProperty(QStringLiteral("detailPanelViewModel"), &detailPanelViewModel);
    engine.rootContext()->setContextProperty(QStringLiteral("editorViewModeViewModel"), &editorViewModeViewModel);
    engine.rootContext()->setContextProperty(QStringLiteral("navigationModeViewModel"), &navigationModeViewModel);
    engine.rootContext()->setContextProperty(QStringLiteral("sidebarHierarchyViewModel"), &sidebarHierarchyViewModel);
    engine.rootContext()->setContextProperty(QStringLiteral("asyncScheduler"), &asyncScheduler);
    engine.rootContext()->setContextProperty(QStringLiteral("systemCalendarStore"), &systemCalendarStore);
    engine.rootContext()->setContextProperty(QStringLiteral("panelViewModelRegistry"), &panelViewModelRegistry);
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
                onboardingRouteBootstrapController.configure(false, true);
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

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    const bool useEmbeddedStartupOnboarding = true;
#else
    const bool useEmbeddedStartupOnboarding = false;
#endif
    const bool showDesktopStartupOnboarding = !startupHubSelection.mounted && !useEmbeddedStartupOnboarding;
    onboardingRouteBootstrapController.configure(useEmbeddedStartupOnboarding, startupHubSelection.mounted);

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
