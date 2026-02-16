#include <QCoreApplication>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QPermission>
#include <QQmlApplicationEngine>
#include <QTimer>
#include <QUrl>
#include <QVector>
#include <QtCore/qpermissions.h>

#include <cstdlib>

namespace
{
    class PermissionBootstrapper final : public QObject
    {
    public:
        explicit PermissionBootstrapper(QCoreApplication& app)
            : QObject(&app)
              , m_app(app)
        {
#if QT_CONFIG(permissions)
            QCalendarPermission calendarPermission;
            calendarPermission.setAccessMode(QCalendarPermission::ReadWrite);
            m_permissions.push_back(calendarPermission);

            m_permissions.push_back(QMicrophonePermission{});

            QBluetoothPermission bluetoothPermission;
            bluetoothPermission.setCommunicationModes(QBluetoothPermission::Default);
            m_permissions.push_back(bluetoothPermission);
#endif
        }

        void start() { requestNextPermission(); }

    private:
        void requestNextPermission()
        {
#if QT_CONFIG(permissions)
            if (m_nextIndex < m_permissions.size())
            {
                const QPermission permission = m_permissions.at(m_nextIndex++);
                const Qt::PermissionStatus status = m_app.checkPermission(permission);
                if (status == Qt::PermissionStatus::Undetermined)
                {
                    m_app.requestPermission(permission, this, [this](const QPermission&)
                    {
                        QTimer::singleShot(0, this, [this]() { requestNextPermission(); });
                    });
                    return;
                }
            }
#endif
            openManualPrivacySettings();
        }

        static void openManualPrivacySettings()
        {
#if defined(Q_OS_MACOS)
            QDesktopServices::openUrl(
                QUrl(QStringLiteral("x-apple.systempreferences:com.apple.preference.security?Privacy")));
#elif defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
            QDesktopServices::openUrl(QUrl(QStringLiteral("app-settings:")));
#endif
        }

        QCoreApplication& m_app;
#if QT_CONFIG(permissions)
        QVector<QPermission> m_permissions;
        qsizetype m_nextIndex = 0;
#endif
    };
} // namespace

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("WhatSon"));
    QCoreApplication::setOrganizationName(QStringLiteral("WhatSon"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("whatson.local"));

    QQmlApplicationEngine engine;

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
