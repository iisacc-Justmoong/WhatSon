#include "permissions/ApplePermissionBridge.hpp"
#include "sidebar/SidebarHierarchyStore.hpp"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QPermission>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#include <QTimer>
#include <QVector>
#include <QtCore/qpermissions.h>

#include <cstdlib>
#include <functional>
#include <utility>

void qml_register_types_LVRS();

namespace
{
    constexpr auto kPermissionsGrantedSettingsKey = "permissions/granted";
    constexpr auto kPermissionsScopeKey = "permissions";

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
    qml_register_types_LVRS();

    QCoreApplication::setApplicationName(QStringLiteral("WhatSon"));
    QCoreApplication::setOrganizationName(QStringLiteral("WhatSon"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("whatson.local"));

    QQmlApplicationEngine engine;
    SidebarHierarchyStore sidebarHierarchyStore;
    engine.rootContext()->setContextProperty(QStringLiteral("sidebarHierarchyStore"), &sidebarHierarchyStore);

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
