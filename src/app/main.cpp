#include "viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/event/EventHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/library/LibraryHierarchyViewModel.hpp"
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
#include "viewmodel/panel/FocusedNoteDeletionBridge.hpp"
#include "viewmodel/panel/HierarchyDragDropBridge.hpp"
#include "viewmodel/panel/PanelViewModelRegistry.hpp"
#include "viewmodel/sidebar/HierarchySidebarDomain.hpp"
#include "viewmodel/sidebar/HierarchyViewModelProvider.hpp"
#include "viewmodel/sidebar/SidebarHierarchyViewModel.hpp"
#include "calendar/SystemCalendarStore.hpp"
#include "policy/ArchitecturePolicyLock.hpp"
#include "hub/WhatSonHubRuntimeStore.hpp"
#include "runtime/threading/WhatSonRuntimeParallelLoader.hpp"
#include "runtime/scheduler/WhatSonAsyncScheduler.hpp"
#include "file/hub/WhatSonHubCreator.hpp"
#include "file/WhatSonDebugTrace.hpp"
#include "permissions/ApplePermissionBridge.hpp"
#include "store/hub/SelectedHubStore.hpp"
#include "store/sidebar/SidebarSelectionStore.hpp"

#include <QByteArray>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QPermission>
#include <QPointer>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#include <QStringList>
#include <QTimer>
#include <QVariant>
#include <QVector>
#include <qqml.h>
#include <QtCore/qglobal.h>
#include <QtCore/qpermissions.h>

#include <cstdlib>
#include <functional>
#include <utility>

void qml_register_types_LVRS();

namespace
{
    constexpr auto kPermissionsGrantedSettingsKey = "permissions/granted";
    constexpr auto kPermissionsScopeKey = "permissions";

    struct LaunchOptions final
    {
        bool onboardingOnly = false;
    };

    void prependEnvPath(const char* variableName, const QByteArray& path)
    {
        if (path.isEmpty())
        {
            return;
        }

        const QByteArray currentValue = qgetenv(variableName);
        if (currentValue.isEmpty())
        {
            qputenv(variableName, path);
            return;
        }

        qputenv(variableName, path + ":" + currentValue);
    }

    QString resolveBlueprintHubPath()
    {
        const QStringList basePaths = {
            QDir::currentPath(),
            QCoreApplication::applicationDirPath()
        };

        WhatSon::Debug::trace(
            QStringLiteral("main.blueprint"),
            QStringLiteral("resolve.start"),
            QStringLiteral("basePathCount=%1").arg(basePaths.size()));

        for (const QString& basePath : basePaths)
        {
            WhatSon::Debug::trace(
                QStringLiteral("main.blueprint"),
                QStringLiteral("resolve.base"),
                QStringLiteral("basePath=%1").arg(basePath));
            QDir probe(basePath);
            for (int depth = 0; depth < 8; ++depth)
            {
                const QDir blueprintDir(probe.filePath(QStringLiteral("blueprint")));
                WhatSon::Debug::trace(
                    QStringLiteral("main.blueprint"),
                    QStringLiteral("resolve.probe"),
                    QStringLiteral("depth=%1 probe=%2").arg(depth).arg(blueprintDir.path()));
                if (blueprintDir.exists())
                {
                    WhatSon::Debug::trace(
                        QStringLiteral("main.blueprint"),
                        QStringLiteral("resolve.blueprintDirFound"),
                        QStringLiteral("path=%1").arg(blueprintDir.path()));
                    const QFileInfoList hubCandidates = blueprintDir.entryInfoList(
                        QStringList{QStringLiteral("*.wshub")},
                        QDir::Dirs | QDir::NoDotAndDotDot,
                        QDir::Name);
                    if (!hubCandidates.isEmpty())
                    {
                        const QString resolvedPath = QDir::cleanPath(hubCandidates.first().absoluteFilePath());
                        WhatSon::Debug::trace(
                            QStringLiteral("main.blueprint"),
                            QStringLiteral("resolve.found"),
                            QStringLiteral("wshub=%1").arg(resolvedPath));
                        return resolvedPath;
                    }
                }

                if (!probe.cdUp())
                {
                    break;
                }
            }
        }

        WhatSon::Debug::trace(
            QStringLiteral("main.blueprint"),
            QStringLiteral("resolve.notFound"));
        return QString();
    }

    LaunchOptions parseLaunchOptions(QGuiApplication& app)
    {
        QCommandLineParser parser;
        parser.setApplicationDescription(QStringLiteral("WhatSon application launcher"));
        parser.addHelpOption();

        const QCommandLineOption onboardingOnlyOption(
            QStringList{QStringLiteral("onboarding-only")},
            QStringLiteral("Launch only the onboarding window and skip workspace initialization."));
        parser.addOption(onboardingOnlyOption);
        parser.process(app);

        LaunchOptions options;
        options.onboardingOnly = parser.isSet(onboardingOnlyOption);
        return options;
    }

    using PermissionCompletion = std::function<void(bool granted)>;
    using PermissionRequester = std::function<void(const PermissionCompletion& completion)>;

    struct PermissionStep final
    {
        QString id;
        PermissionRequester request;
    };

    class PermissionBootstrapper final : public QObject
    {
    public:
        explicit PermissionBootstrapper(QCoreApplication& app)
            : QObject(&app)
              , m_app(app)
        {
            buildPermissionSteps();
        }

        void start()
        {
            requestNextPermission();
        }

    private:
        static QString permissionSettingKey(const QString& permissionId, const QString& suffix)
        {
            return QStringLiteral("%1/%2/%3").arg(
                QString::fromLatin1(kPermissionsScopeKey),
                permissionId,
                suffix);
        }

        bool hasStoredDecision(const QString& permissionId) const
        {
            QSettings settings;
            return settings.value(permissionSettingKey(permissionId, QStringLiteral("requested")), false).toBool();
        }

        bool isGranted(const QString& permissionId) const
        {
            QSettings settings;
            return settings.value(permissionSettingKey(permissionId, QStringLiteral("granted")), false).toBool();
        }

        void storeDecision(const QString& permissionId, bool granted) const
        {
            QSettings settings;
            settings.setValue(permissionSettingKey(permissionId, QStringLiteral("requested")), true);
            settings.setValue(permissionSettingKey(permissionId, QStringLiteral("granted")), granted);
            settings.sync();
        }

        void finalizePermissionBootstrap() const
        {
            bool allGranted = true;
            for (const PermissionStep& step : m_steps)
            {
                if (!isGranted(step.id))
                {
                    allGranted = false;
                    break;
                }
            }

            QSettings settings;
            settings.setValue(QString::fromLatin1(kPermissionsGrantedSettingsKey), allGranted);
            settings.sync();
        }

        void addStep(QString id, PermissionRequester request)
        {
            m_steps.push_back(PermissionStep{std::move(id), std::move(request)});
        }

        template <typename PermissionFactory>
        void addQtPermissionStep(QString id, PermissionFactory permissionFactory)
        {
            addStep(std::move(id), [this, permissionFactory](const PermissionCompletion& completion)
            {
#if QT_CONFIG(permissions)
                const QPermission permission = permissionFactory();
                const Qt::PermissionStatus status = m_app.checkPermission(permission);
                if (status == Qt::PermissionStatus::Granted)
                {
                    completion(true);
                    return;
                }
                if (status == Qt::PermissionStatus::Denied)
                {
                    completion(false);
                    return;
                }

                m_app.requestPermission(permission, this, [this, completion](const QPermission& requestedPermission)
                {
                    const bool granted =
                        m_app.checkPermission(requestedPermission) == Qt::PermissionStatus::Granted;
                    completion(granted);
                });
#else
                completion(true);
#endif
            });
        }

        void addApplePermissionStep(
            QString id,
            void (*requestApplePermission)(const WhatSon::Permissions::PermissionCallback& completion))
        {
            addStep(std::move(id), [requestApplePermission](const PermissionCompletion& completion)
            {
                requestApplePermission([completion](bool granted)
                {
                    completion(granted);
                });
            });
        }

        void buildPermissionSteps()
        {
#if defined(Q_OS_MACOS)
            addApplePermissionStep(
                QStringLiteral("full_disk_access"),
                WhatSon::Permissions::requestFullDiskAccessPermission);
#endif
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
            addApplePermissionStep(QStringLiteral("photo_library"),
                                   WhatSon::Permissions::requestPhotoLibraryPermission);
#endif
#if QT_CONFIG(permissions)
            addQtPermissionStep(QStringLiteral("microphone"), []() -> QPermission
            {
                return QMicrophonePermission{};
            });
#endif
#if defined(Q_OS_MACOS)
            addApplePermissionStep(
                QStringLiteral("accessibility"),
                WhatSon::Permissions::requestAccessibilityPermission);
#endif
#if QT_CONFIG(permissions)
            addQtPermissionStep(QStringLiteral("calendar"), []() -> QPermission
            {
                QCalendarPermission permission;
                permission.setAccessMode(QCalendarPermission::ReadWrite);
                return permission;
            });
#endif
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
            addApplePermissionStep(QStringLiteral("reminders"), WhatSon::Permissions::requestRemindersPermission);
            addApplePermissionStep(
                QStringLiteral("local_network"),
                WhatSon::Permissions::requestLocalNetworkPermission);
#endif
#if QT_CONFIG(permissions)
            addQtPermissionStep(QStringLiteral("location"), []() -> QPermission
            {
                QLocationPermission permission;
                permission.setAvailability(QLocationPermission::WhenInUse);
                permission.setAccuracy(QLocationPermission::Precise);
                return permission;
            });
#endif
        }

        void requestNextPermission()
        {
            while (m_nextIndex < m_steps.size())
            {
                const PermissionStep step = m_steps.at(m_nextIndex++);
                if (hasStoredDecision(step.id))
                {
                    continue;
                }

                step.request([this, permissionId = step.id](bool granted)
                {
                    storeDecision(permissionId, granted);
                    QTimer::singleShot(0, this, [this]() { requestNextPermission(); });
                });
                return;
            }

            finalizePermissionBootstrap();
        }

        QCoreApplication& m_app;
        QVector<PermissionStep> m_steps;
        qsizetype m_nextIndex = 0;
    };
} // namespace

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    if (qEnvironmentVariableIsEmpty("WHATSON_DEBUG_MODE"))
    {
        qputenv("WHATSON_DEBUG_MODE", QByteArrayLiteral("1"));
    }
#if defined(WHATSON_USE_LVRS_DYNAMIC_QML_IMPORT) && defined(WHATSON_LVRS_RUNTIME_IMPORT_ROOT)
    Q_CLEANUP_RESOURCE(qmake_LVRS);

    const QByteArray lvrsImportRoot = QByteArrayLiteral(WHATSON_LVRS_RUNTIME_IMPORT_ROOT);
    prependEnvPath("QML_IMPORT_PATH", lvrsImportRoot);
    prependEnvPath("QML2_IMPORT_PATH", lvrsImportRoot);
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
#if !defined(Q_OS_MACOS) && !defined(Q_OS_IOS)
    app.setWindowIcon(QIcon(QStringLiteral(":/whatson/AppIcon.png")));
#endif

    QCoreApplication::setApplicationName(QStringLiteral("WhatSon"));
    QCoreApplication::setOrganizationName(QStringLiteral("WhatSon"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("whatson.local"));
    const LaunchOptions launchOptions = parseLaunchOptions(app);

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
    ProjectsHierarchyViewModel projectsHierarchyViewModel;
    BookmarksHierarchyViewModel bookmarksHierarchyViewModel;
    TagsHierarchyViewModel tagsHierarchyViewModel;
    ResourcesHierarchyViewModel resourcesHierarchyViewModel;
    ProgressHierarchyViewModel progressHierarchyViewModel;
    EventHierarchyViewModel eventHierarchyViewModel;
    PresetHierarchyViewModel presetHierarchyViewModel;
    SidebarSelectionStore sidebarSelectionStore;
    SelectedHubStore selectedHubStore;
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
    PermissionBootstrapper permissionBootstrapper(app);

    const auto requestNewLibraryNote = [&libraryHierarchyViewModel, &sidebarHierarchyViewModel]()
    {
        sidebarHierarchyViewModel.setActiveHierarchyIndex(static_cast<int>(WhatSon::Sidebar::HierarchyDomain::Library));
        if (!libraryHierarchyViewModel.createEmptyNote())
        {
            qWarning().noquote() << QStringLiteral("Failed to create a new library note from navigation Add action.");
        }
    };

    if (auto* addNewPanelViewModel = qobject_cast<PanelViewModel*>(
        panelViewModelRegistry.panelViewModel(QStringLiteral("navigation.NavigationAddNewBar"))))
    {
        QObject::connect(
            addNewPanelViewModel,
            &PanelViewModel::viewModelHookRequested,
            &app,
            requestNewLibraryNote);
    }
    if (auto* applicationControlPanelViewModel = qobject_cast<PanelViewModel*>(
        panelViewModelRegistry.panelViewModel(QStringLiteral("navigation.NavigationApplicationControlBar"))))
    {
        QObject::connect(
            applicationControlPanelViewModel,
            &PanelViewModel::viewModelHookRequested,
            &app,
            requestNewLibraryNote);
    }
    if (auto* mobilePanelViewModel = qobject_cast<PanelViewModel*>(
        panelViewModelRegistry.panelViewModel(QStringLiteral("MobileNormalLayout"))))
    {
        QObject::connect(
            mobilePanelViewModel,
            &PanelViewModel::viewModelHookRequested,
            &app,
            requestNewLibraryNote);
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

    const auto loadHubIntoRuntime =
        [&libraryHierarchyViewModel,
            &projectsHierarchyViewModel,
            &bookmarksHierarchyViewModel,
            &tagsHierarchyViewModel,
            &resourcesHierarchyViewModel,
            &progressHierarchyViewModel,
            &eventHierarchyViewModel,
            &presetHierarchyViewModel,
            &hubRuntimeStore](const QString& hubPath, QString* errorMessage) -> bool
    {
        const QString normalizedHubPath = hubPath.trimmed().isEmpty()
                                              ? QString()
                                              : QDir::cleanPath(QFileInfo(hubPath).absoluteFilePath());
        if (normalizedHubPath.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Hub path must not be empty.");
            }
            return false;
        }

        WhatSon::Debug::trace(
            QStringLiteral("main.runtime"),
            QStringLiteral("loadFromWshub.begin"),
            QStringLiteral("path=%1").arg(normalizedHubPath));

        WhatSonRuntimeParallelLoader parallelLoader;
        WhatSonRuntimeParallelLoader::Targets targets;
        targets.libraryViewModel = &libraryHierarchyViewModel;
        targets.projectsViewModel = &projectsHierarchyViewModel;
        targets.bookmarksViewModel = &bookmarksHierarchyViewModel;
        targets.tagsViewModel = &tagsHierarchyViewModel;
        targets.resourcesViewModel = &resourcesHierarchyViewModel;
        targets.progressViewModel = &progressHierarchyViewModel;
        targets.eventViewModel = &eventHierarchyViewModel;
        targets.presetViewModel = &presetHierarchyViewModel;
        targets.hubRuntimeStore = &hubRuntimeStore;

        QVector<WhatSonRuntimeParallelLoader::DomainLoadResult> loadResults;
        const bool loadSucceeded = parallelLoader.loadFromWshub(normalizedHubPath, targets, &loadResults);

        bool hubRuntimeLoadSucceeded = false;
        QStringList failedDomains;
        for (const WhatSonRuntimeParallelLoader::DomainLoadResult& result : loadResults)
        {
            if (result.domain == QStringLiteral("hub.runtime"))
            {
                hubRuntimeLoadSucceeded = result.succeeded;
            }

            if (result.succeeded)
            {
                WhatSon::Debug::trace(
                    QStringLiteral("main.runtime"),
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
                QStringLiteral("main.runtime"),
                QStringLiteral("load.failed"),
                QStringLiteral("domain=%1 reason=%2").arg(result.domain, domainErrorMessage));
        }

        if (hubRuntimeLoadSucceeded)
        {
            libraryHierarchyViewModel.setHubStore(hubRuntimeStore.hub(normalizedHubPath));
            tagsHierarchyViewModel.setTagDepthEntries(hubRuntimeStore.tagDepthEntries(normalizedHubPath));
            WhatSon::Debug::trace(
                QStringLiteral("main.runtime"),
                QStringLiteral("applyTagsDepthEntries.success"),
                QStringLiteral("itemCount=%1").arg(tagsHierarchyViewModel.itemModel()->rowCount()));
        }
        else
        {
            WhatSon::Debug::trace(
                QStringLiteral("main.runtime"),
                QStringLiteral("applyTagsDepthEntries.skipped"),
                QStringLiteral("reason=hub.runtime load failed"));
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

        if (!hubRuntimeLoadSucceeded)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to load the selected WhatSon Hub runtime state.");
            }
            return false;
        }

        if (errorMessage != nullptr)
        {
            errorMessage->clear();
        }
        return true;
    };

    onboardingHubController.setCreateHubCallback(
        [&hubCreator](const QString& requestedHubPath, QString* outPackagePath, QString* errorMessage) -> bool
        {
            return hubCreator.createHubAtPath(requestedHubPath, outPackagePath, errorMessage);
        });
    onboardingHubController.setLoadHubCallback(loadHubIntoRuntime);
    QObject::connect(
        &onboardingHubController,
        &OnboardingHubController::hubLoaded,
        &app,
        [&selectedHubStore](const QString& hubPath)
        {
            selectedHubStore.setSelectedHubPath(hubPath);
        });

    bool initialHubLoaded = false;
    const QString startupHubPath = selectedHubStore.startupHubPath(resolveBlueprintHubPath());
    if (!startupHubPath.isEmpty())
    {
        QString errorMessage;
        initialHubLoaded = loadHubIntoRuntime(startupHubPath, &errorMessage);
        if (initialHubLoaded)
        {
            onboardingHubController.syncCurrentHubSelection(startupHubPath);
            selectedHubStore.setSelectedHubPath(startupHubPath);
        }
        if (!initialHubLoaded && !errorMessage.trimmed().isEmpty())
        {
            qWarning().noquote()
                << QStringLiteral("Failed to load startup WhatSon Hub '%1': %2")
                .arg(startupHubPath, errorMessage.trimmed());
        }
    }
    else
    {
        qWarning().noquote() << QStringLiteral("No startup WhatSon Hub could be resolved.");
        WhatSon::Debug::trace(
            QStringLiteral("main.runtime"),
            QStringLiteral("loadFromWshub.skipped"),
            QStringLiteral("no startup .wshub detected"));
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
    sidebarHierarchyViewModel.setSelectionStore(&sidebarSelectionStore);
    sidebarHierarchyViewModel.setViewModelProvider(&hierarchyViewModelProvider);
    WhatSon::Policy::ArchitecturePolicyLock::lock();

    engine.rootContext()->setContextProperty(QStringLiteral("libraryHierarchyViewModel"), &libraryHierarchyViewModel);
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
    engine.rootContext()->setContextProperty(
        QStringLiteral("detailFileInfoViewModel"),
        detailPanelViewModel.fileInfoViewModel());
    engine.rootContext()->setContextProperty(
        QStringLiteral("detailFileStatViewModel"),
        detailPanelViewModel.fileStatViewModel());
    engine.rootContext()->setContextProperty(
        QStringLiteral("detailFileFormatViewModel"),
        detailPanelViewModel.fileFormatViewModel());
    engine.rootContext()->setContextProperty(
        QStringLiteral("detailFileHistoryViewModel"),
        detailPanelViewModel.fileHistoryViewModel());
    engine.rootContext()->setContextProperty(
        QStringLiteral("detailAppearanceViewModel"),
        detailPanelViewModel.appearanceViewModel());
    engine.rootContext()->setContextProperty(
        QStringLiteral("detailHelpViewModel"),
        detailPanelViewModel.helpViewModel());
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

    const auto loadMainWindow = [&engine]() -> QObject*
    {
        const qsizetype rootObjectCount = engine.rootObjects().size();
        engine.loadFromModule(QStringLiteral("WhatSon.App"), QStringLiteral("Main"));
        if (engine.rootObjects().size() <= rootObjectCount)
        {
            return nullptr;
        }

        return engine.rootObjects().constLast();
    };

    const bool mobileStandaloneOnboarding =
#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
        !initialHubLoaded;
#else
        false;
#endif

    if (launchOptions.onboardingOnly || mobileStandaloneOnboarding)
    {
        const qsizetype rootObjectCount = engine.rootObjects().size();
        engine.loadFromModule(QStringLiteral("WhatSon.App"), QStringLiteral("Onboarding"));
        if (engine.rootObjects().size() <= rootObjectCount)
        {
            return EXIT_FAILURE;
        }

        QObject* onboardingWindow = engine.rootObjects().constLast();
        if (onboardingWindow == nullptr)
        {
            qWarning().noquote() << QStringLiteral("Failed to resolve onboarding window root object.");
            return EXIT_FAILURE;
        }

        QPointer<QObject> onboardingWindowGuard(onboardingWindow);
        QObject* workspaceMainWindow = nullptr;
        onboardingWindow->setProperty(
            "hubSessionController",
            QVariant::fromValue(static_cast<QObject*>(&onboardingHubController)));

        QObject::connect(onboardingWindow, SIGNAL(dismissed()), &app, SLOT(quit()));
        QObject::connect(
            &onboardingHubController,
            &OnboardingHubController::hubLoaded,
            &app,
            [&engine,
                &startForegroundServices,
                &workspaceMainWindow,
                &onboardingHubController,
                onboardingWindowGuard](const QString&)
            {
                if (workspaceMainWindow != nullptr)
                {
                    return;
                }

                if (onboardingWindowGuard)
                {
                    onboardingWindowGuard->setProperty("visible", false);
                }

                const qsizetype rootObjectCountBeforeMainWindow = engine.rootObjects().size();
                engine.loadFromModule(QStringLiteral("WhatSon.App"), QStringLiteral("Main"));
                if (engine.rootObjects().size() <= rootObjectCountBeforeMainWindow)
                {
                    qWarning().noquote() <<
                        QStringLiteral("Failed to load the main workspace window after onboarding.");
                    QCoreApplication::exit(EXIT_FAILURE);
                    return;
                }

                workspaceMainWindow = engine.rootObjects().constLast();
                if (workspaceMainWindow != nullptr)
                {
                    workspaceMainWindow->setProperty(
                        "onboardingHubController",
                        QVariant::fromValue(static_cast<QObject*>(&onboardingHubController)));
                    workspaceMainWindow->setProperty("onboardingVisible", false);
                }

                if (onboardingWindowGuard)
                {
                    onboardingWindowGuard->deleteLater();
                }

                startForegroundServices();
            });

        onboardingWindow->setProperty("standaloneMode", true);
        onboardingWindow->setProperty("visible", true);
        return app.exec();
    }

    QObject* mainWindow = loadMainWindow();
    if (mainWindow == nullptr)
    {
        return EXIT_FAILURE;
    }

    mainWindow->setProperty(
        "onboardingHubController",
        QVariant::fromValue(static_cast<QObject*>(&onboardingHubController)));

    if (!initialHubLoaded)
    {
        mainWindow->setProperty("onboardingVisible", true);
    }

    startForegroundServices();
    return app.exec();
}
