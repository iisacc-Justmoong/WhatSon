#include "viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.hpp"
#include "viewmodel/bridge/WhatSonBackendBridge.hpp"
#include "viewmodel/hierarchy/event/EventHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/library/LibraryHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/preset/PresetHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/progress/ProgressHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/projects/ProjectsHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/resources/ResourcesHierarchyViewModel.hpp"
#include "viewmodel/hierarchy/tags/TagsHierarchyViewModel.hpp"
#include "hub/WhatSonHubRuntimeStore.hpp"
#include "file/WhatSonDebugTrace.hpp"
#include "permissions/ApplePermissionBridge.hpp"

#include <QByteArray>
#include <QAbstractItemModel>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QPermission>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#include <QTimer>
#include <QVector>
#include <QVariantMap>
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
#if defined(WHATSON_USE_LVRS_DYNAMIC_QML_IMPORT) && defined(WHATSON_LVRS_RUNTIME_IMPORT_ROOT)
    Q_CLEANUP_RESOURCE(qmake_LVRS);

    const QByteArray lvrsImportRoot = QByteArrayLiteral(WHATSON_LVRS_RUNTIME_IMPORT_ROOT);
    prependEnvPath("QML_IMPORT_PATH", lvrsImportRoot);
    prependEnvPath("QML2_IMPORT_PATH", lvrsImportRoot);
#endif

#if !defined(WHATSON_USE_LVRS_DYNAMIC_QML_IMPORT)
    qml_register_types_LVRS();
#endif
#if !defined(Q_OS_MACOS) && !defined(Q_OS_IOS)
    app.setWindowIcon(QIcon(QStringLiteral(":/whatson/AppIcon.png")));
#endif

    QCoreApplication::setApplicationName(QStringLiteral("WhatSon"));
    QCoreApplication::setOrganizationName(QStringLiteral("WhatSon"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("whatson.local"));

    WhatSon::Debug::trace(
        QStringLiteral("main"),
        QStringLiteral("startup"),
        QStringLiteral("debugMode=%1 cwd=%2 appDir=%3")
        .arg(WhatSon::Debug::isEnabled() ? QStringLiteral("on") : QStringLiteral("off"))
        .arg(QDir::currentPath(), QCoreApplication::applicationDirPath()));

    QQmlApplicationEngine engine;
    LibraryHierarchyViewModel libraryHierarchyViewModel;
    ProjectsHierarchyViewModel projectsHierarchyViewModel;
    BookmarksHierarchyViewModel bookmarksHierarchyViewModel;
    TagsHierarchyViewModel tagsHierarchyViewModel;
    ResourcesHierarchyViewModel resourcesHierarchyViewModel;
    ProgressHierarchyViewModel progressHierarchyViewModel;
    EventHierarchyViewModel eventHierarchyViewModel;
    PresetHierarchyViewModel presetHierarchyViewModel;
    WhatSonBackendBridge backendBridge;
    WhatSonHubRuntimeStore hubRuntimeStore;
    const QString blueprintHubPath = resolveBlueprintHubPath();
    auto publishLoadEvent = [&backendBridge, &blueprintHubPath](
        const QString& domain,
        bool ok,
        const QString& errorMessage,
        int itemCount)
    {
        backendBridge.publish(
            domain,
            ok ? QStringLiteral("load.success") : QStringLiteral("load.failed"),
            QVariantMap{
                {QStringLiteral("ok"), ok},
                {QStringLiteral("path"), blueprintHubPath},
                {QStringLiteral("error"), errorMessage},
                {QStringLiteral("itemCount"), itemCount}
            });
    };
    if (!blueprintHubPath.isEmpty())
    {
        WhatSon::Debug::trace(
            QStringLiteral("main.runtime"),
            QStringLiteral("loadFromWshub.begin"),
            QStringLiteral("path=%1").arg(blueprintHubPath));
        QString libraryIndexError;
        if (!libraryHierarchyViewModel.loadFromWshub(blueprintHubPath, &libraryIndexError))
        {
            qWarning().noquote()
                << QStringLiteral("Failed to index library hierarchy from .wshub: %1")
                .arg(libraryIndexError);
            WhatSon::Debug::trace(
                QStringLiteral("main.runtime"),
                QStringLiteral("loadLibraryIndex.failed"),
                libraryIndexError);
            publishLoadEvent(QStringLiteral("library"), false, libraryIndexError, 0);
        }
        else
        {
            WhatSon::Debug::trace(
                QStringLiteral("main.runtime"),
                QStringLiteral("loadLibraryIndex.success"),
                QStringLiteral("path=%1").arg(blueprintHubPath));
            publishLoadEvent(
                QStringLiteral("library"),
                true,
                QString(),
                libraryHierarchyViewModel.itemModel()->rowCount());
        }
        QString hubLoadError;
        if (!hubRuntimeStore.loadFromWshub(blueprintHubPath, &hubLoadError))
        {
            qWarning().noquote()
                << QStringLiteral("Failed to load blueprint .wshub into runtime hub store: %1").arg(hubLoadError);
            WhatSon::Debug::trace(
                QStringLiteral("main.runtime"),
                QStringLiteral("loadFromWshub.failed"),
                hubLoadError);
            publishLoadEvent(QStringLiteral("runtime"), false, hubLoadError, 0);
        }
        else
        {
            const QVector<WhatSonTagDepthEntry> tags = hubRuntimeStore.tagDepthEntries(blueprintHubPath);
            WhatSon::Debug::trace(
                QStringLiteral("main.runtime"),
                QStringLiteral("loadFromWshub.success"),
                QStringLiteral("tagEntryCount=%1").arg(tags.size()));
            publishLoadEvent(QStringLiteral("runtime"), true, QString(), tags.size());
        }
        tagsHierarchyViewModel.setTagDepthEntries(hubRuntimeStore.tagDepthEntries(blueprintHubPath));
        backendBridge.publish(
            QStringLiteral("tags"),
            QStringLiteral("runtime.applyDepthEntries"),
            QVariantMap{
                {QStringLiteral("itemCount"), tagsHierarchyViewModel.itemModel()->rowCount()}
            });

        QString hierarchyLoadError;
        if (!projectsHierarchyViewModel.loadFromWshub(blueprintHubPath, &hierarchyLoadError))
        {
            WhatSon::Debug::trace(
                QStringLiteral("main.runtime"),
                QStringLiteral("loadProjects.failed"),
                hierarchyLoadError);
            publishLoadEvent(QStringLiteral("projects"), false, hierarchyLoadError, 0);
        }
        else
        {
            publishLoadEvent(
                QStringLiteral("projects"),
                true,
                QString(),
                projectsHierarchyViewModel.itemModel()->rowCount());
        }
        if (!bookmarksHierarchyViewModel.loadFromWshub(blueprintHubPath, &hierarchyLoadError))
        {
            WhatSon::Debug::trace(
                QStringLiteral("main.runtime"),
                QStringLiteral("loadBookmarks.failed"),
                hierarchyLoadError);
            publishLoadEvent(QStringLiteral("bookmarks"), false, hierarchyLoadError, 0);
        }
        else
        {
            publishLoadEvent(
                QStringLiteral("bookmarks"),
                true,
                QString(),
                bookmarksHierarchyViewModel.itemModel()->rowCount());
        }
        if (!resourcesHierarchyViewModel.loadFromWshub(blueprintHubPath, &hierarchyLoadError))
        {
            WhatSon::Debug::trace(
                QStringLiteral("main.runtime"),
                QStringLiteral("loadResources.failed"),
                hierarchyLoadError);
            publishLoadEvent(QStringLiteral("resources"), false, hierarchyLoadError, 0);
        }
        else
        {
            publishLoadEvent(
                QStringLiteral("resources"),
                true,
                QString(),
                resourcesHierarchyViewModel.itemModel()->rowCount());
        }
        if (!progressHierarchyViewModel.loadFromWshub(blueprintHubPath, &hierarchyLoadError))
        {
            WhatSon::Debug::trace(
                QStringLiteral("main.runtime"),
                QStringLiteral("loadProgress.failed"),
                hierarchyLoadError);
            publishLoadEvent(QStringLiteral("progress"), false, hierarchyLoadError, 0);
        }
        else
        {
            publishLoadEvent(
                QStringLiteral("progress"),
                true,
                QString(),
                progressHierarchyViewModel.itemModel()->rowCount());
        }
        if (!eventHierarchyViewModel.loadFromWshub(blueprintHubPath, &hierarchyLoadError))
        {
            WhatSon::Debug::trace(
                QStringLiteral("main.runtime"),
                QStringLiteral("loadEvent.failed"),
                hierarchyLoadError);
            publishLoadEvent(QStringLiteral("event"), false, hierarchyLoadError, 0);
        }
        else
        {
            publishLoadEvent(
                QStringLiteral("event"),
                true,
                QString(),
                eventHierarchyViewModel.itemModel()->rowCount());
        }
        if (!presetHierarchyViewModel.loadFromWshub(blueprintHubPath, &hierarchyLoadError))
        {
            WhatSon::Debug::trace(
                QStringLiteral("main.runtime"),
                QStringLiteral("loadPreset.failed"),
                hierarchyLoadError);
            publishLoadEvent(QStringLiteral("preset"), false, hierarchyLoadError, 0);
        }
        else
        {
            publishLoadEvent(
                QStringLiteral("preset"),
                true,
                QString(),
                presetHierarchyViewModel.itemModel()->rowCount());
        }
    }
    else
    {
        qWarning().noquote() << QStringLiteral("No .wshub package was found under blueprint directory.");
        WhatSon::Debug::trace(
            QStringLiteral("main.runtime"),
            QStringLiteral("loadFromWshub.skipped"),
            QStringLiteral("no blueprint .wshub detected"));
        backendBridge.publish(
            QStringLiteral("runtime"),
            QStringLiteral("load.skipped"),
            QVariantMap{
                {QStringLiteral("reason"), QStringLiteral("no blueprint .wshub detected")}
            });
    }

    QObject::connect(
        &libraryHierarchyViewModel,
        &LibraryHierarchyViewModel::selectedIndexChanged,
        &backendBridge,
        [&]()
        {
            backendBridge.publish(
                QStringLiteral("library"),
                QStringLiteral("selection.changed"),
                QVariantMap{
                    {QStringLiteral("selectedIndex"), libraryHierarchyViewModel.selectedIndex()},
                    {QStringLiteral("itemCount"), libraryHierarchyViewModel.itemModel()->rowCount()},
                    {QStringLiteral("noteCount"), libraryHierarchyViewModel.noteListModel()->rowCount()}
                });
        });
    QObject::connect(
        &projectsHierarchyViewModel,
        &ProjectsHierarchyViewModel::selectedIndexChanged,
        &backendBridge,
        [&]()
        {
            backendBridge.publish(
                QStringLiteral("projects"),
                QStringLiteral("selection.changed"),
                QVariantMap{
                    {QStringLiteral("selectedIndex"), projectsHierarchyViewModel.selectedIndex()},
                    {QStringLiteral("itemCount"), projectsHierarchyViewModel.itemModel()->rowCount()}
                });
        });
    QObject::connect(
        &bookmarksHierarchyViewModel,
        &BookmarksHierarchyViewModel::selectedIndexChanged,
        &backendBridge,
        [&]()
        {
            backendBridge.publish(
                QStringLiteral("bookmarks"),
                QStringLiteral("selection.changed"),
                QVariantMap{
                    {QStringLiteral("selectedIndex"), bookmarksHierarchyViewModel.selectedIndex()},
                    {QStringLiteral("itemCount"), bookmarksHierarchyViewModel.itemModel()->rowCount()},
                    {QStringLiteral("noteCount"), bookmarksHierarchyViewModel.noteListModel()->rowCount()}
                });
        });
    QObject::connect(
        &tagsHierarchyViewModel,
        &TagsHierarchyViewModel::selectedIndexChanged,
        &backendBridge,
        [&]()
        {
            backendBridge.publish(
                QStringLiteral("tags"),
                QStringLiteral("selection.changed"),
                QVariantMap{
                    {QStringLiteral("selectedIndex"), tagsHierarchyViewModel.selectedIndex()},
                    {QStringLiteral("itemCount"), tagsHierarchyViewModel.itemModel()->rowCount()}
                });
        });
    QObject::connect(
        &resourcesHierarchyViewModel,
        &ResourcesHierarchyViewModel::selectedIndexChanged,
        &backendBridge,
        [&]()
        {
            backendBridge.publish(
                QStringLiteral("resources"),
                QStringLiteral("selection.changed"),
                QVariantMap{
                    {QStringLiteral("selectedIndex"), resourcesHierarchyViewModel.selectedIndex()},
                    {QStringLiteral("itemCount"), resourcesHierarchyViewModel.itemModel()->rowCount()}
                });
        });
    QObject::connect(
        &progressHierarchyViewModel,
        &ProgressHierarchyViewModel::selectedIndexChanged,
        &backendBridge,
        [&]()
        {
            backendBridge.publish(
                QStringLiteral("progress"),
                QStringLiteral("selection.changed"),
                QVariantMap{
                    {QStringLiteral("selectedIndex"), progressHierarchyViewModel.selectedIndex()},
                    {QStringLiteral("itemCount"), progressHierarchyViewModel.itemModel()->rowCount()}
                });
        });
    QObject::connect(
        &eventHierarchyViewModel,
        &EventHierarchyViewModel::selectedIndexChanged,
        &backendBridge,
        [&]()
        {
            backendBridge.publish(
                QStringLiteral("event"),
                QStringLiteral("selection.changed"),
                QVariantMap{
                    {QStringLiteral("selectedIndex"), eventHierarchyViewModel.selectedIndex()},
                    {QStringLiteral("itemCount"), eventHierarchyViewModel.itemModel()->rowCount()}
                });
        });
    QObject::connect(
        &presetHierarchyViewModel,
        &PresetHierarchyViewModel::selectedIndexChanged,
        &backendBridge,
        [&]()
        {
            backendBridge.publish(
                QStringLiteral("preset"),
                QStringLiteral("selection.changed"),
                QVariantMap{
                    {QStringLiteral("selectedIndex"), presetHierarchyViewModel.selectedIndex()},
                    {QStringLiteral("itemCount"), presetHierarchyViewModel.itemModel()->rowCount()}
                });
        });

    QObject::connect(
        libraryHierarchyViewModel.noteListModel(),
        &QAbstractItemModel::modelReset,
        &backendBridge,
        [&]()
        {
            backendBridge.publish(
                QStringLiteral("library"),
                QStringLiteral("notelist.modelReset"),
                QVariantMap{
                    {QStringLiteral("rowCount"), libraryHierarchyViewModel.noteListModel()->rowCount()}
                });
        });
    QObject::connect(
        bookmarksHierarchyViewModel.noteListModel(),
        &QAbstractItemModel::modelReset,
        &backendBridge,
        [&]()
        {
            backendBridge.publish(
                QStringLiteral("bookmarks"),
                QStringLiteral("notelist.modelReset"),
                QVariantMap{
                    {QStringLiteral("rowCount"), bookmarksHierarchyViewModel.noteListModel()->rowCount()}
                });
        });

    QObject::connect(
        &backendBridge,
        &WhatSonBackendBridge::commandRequested,
        &backendBridge,
        [&](const QString& command, const QVariantMap& payload)
        {
            if (command.compare(QStringLiteral("bridge.ping"), Qt::CaseInsensitive) == 0)
            {
                backendBridge.publish(QStringLiteral("bridge"), QStringLiteral("pong"), payload);
                return;
            }

            if (command.compare(QStringLiteral("hierarchy.select"), Qt::CaseInsensitive) == 0)
            {
                const QString domain = payload.value(QStringLiteral("domain")).toString().trimmed().toCaseFolded();
                const int index = payload.value(QStringLiteral("index")).toInt();
                bool handled = true;

                if (domain == QStringLiteral("library"))
                {
                    libraryHierarchyViewModel.setSelectedIndex(index);
                }
                else if (domain == QStringLiteral("projects"))
                {
                    projectsHierarchyViewModel.setSelectedIndex(index);
                }
                else if (domain == QStringLiteral("bookmarks"))
                {
                    bookmarksHierarchyViewModel.setSelectedIndex(index);
                }
                else if (domain == QStringLiteral("tags"))
                {
                    tagsHierarchyViewModel.setSelectedIndex(index);
                }
                else if (domain == QStringLiteral("resources"))
                {
                    resourcesHierarchyViewModel.setSelectedIndex(index);
                }
                else if (domain == QStringLiteral("progress"))
                {
                    progressHierarchyViewModel.setSelectedIndex(index);
                }
                else if (domain == QStringLiteral("event"))
                {
                    eventHierarchyViewModel.setSelectedIndex(index);
                }
                else if (domain == QStringLiteral("preset"))
                {
                    presetHierarchyViewModel.setSelectedIndex(index);
                }
                else
                {
                    handled = false;
                }

                backendBridge.publish(
                    QStringLiteral("bridge"),
                    handled ? QStringLiteral("command.applied") : QStringLiteral("command.rejected"),
                    QVariantMap{
                        {QStringLiteral("command"), command},
                        {QStringLiteral("domain"), domain},
                        {QStringLiteral("index"), index},
                        {QStringLiteral("handled"), handled}
                    });
                return;
            }

            backendBridge.publish(
                QStringLiteral("bridge"),
                QStringLiteral("command.unsupported"),
                QVariantMap{
                    {QStringLiteral("command"), command}
                });
        });

    engine.rootContext()->setContextProperty(QStringLiteral("backendBridge"), &backendBridge);
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

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(EXIT_FAILURE); },
        Qt::QueuedConnection);

    engine.loadFromModule(QStringLiteral("WhatSon.App"), QStringLiteral("Main"));

    if (engine.rootObjects().isEmpty())
    {
        return EXIT_FAILURE;
    }

    PermissionBootstrapper permissionBootstrapper(app);
    QTimer::singleShot(0, &permissionBootstrapper, [&permissionBootstrapper]()
    {
        permissionBootstrapper.start();
    });

    return app.exec();
}
