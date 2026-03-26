#include "WhatSonPermissionBootstrapper.hpp"

#include "permissions/ApplePermissionBridge.hpp"

#include <QCoreApplication>
#include <QSettings>
#include <QTimer>
#include <QtCore/qpermissions.h>

namespace
{
    constexpr auto kPermissionsGrantedSettingsKey = "permissions/granted";
    constexpr auto kPermissionsScopeKey = "permissions";
}

WhatSonPermissionBootstrapper::WhatSonPermissionBootstrapper(QCoreApplication& app)
    : QObject(&app)
    , m_app(app)
{
    buildPermissionSteps();
}

void WhatSonPermissionBootstrapper::start()
{
    requestNextPermission();
}

QString WhatSonPermissionBootstrapper::permissionSettingKey(const QString& permissionId, const QString& suffix)
{
    return QStringLiteral("%1/%2/%3").arg(QString::fromLatin1(kPermissionsScopeKey), permissionId, suffix);
}

bool WhatSonPermissionBootstrapper::hasStoredDecision(const QString& permissionId) const
{
    QSettings settings;
    return settings.value(permissionSettingKey(permissionId, QStringLiteral("requested")), false).toBool();
}

bool WhatSonPermissionBootstrapper::isGranted(const QString& permissionId) const
{
    QSettings settings;
    return settings.value(permissionSettingKey(permissionId, QStringLiteral("granted")), false).toBool();
}

void WhatSonPermissionBootstrapper::storeDecision(const QString& permissionId, bool granted) const
{
    QSettings settings;
    settings.setValue(permissionSettingKey(permissionId, QStringLiteral("requested")), true);
    settings.setValue(permissionSettingKey(permissionId, QStringLiteral("granted")), granted);
    settings.sync();
}

void WhatSonPermissionBootstrapper::finalizePermissionBootstrap() const
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

void WhatSonPermissionBootstrapper::addStep(QString id, PermissionRequester request)
{
    m_steps.push_back(PermissionStep{std::move(id), std::move(request)});
}

void WhatSonPermissionBootstrapper::addApplePermissionStep(
    QString id,
    void (*requestApplePermission)(const WhatSon::Permissions::PermissionCallback& completion))
{
    addStep(std::move(id), [requestApplePermission](const PermissionCompletion& completion)
    {
        requestApplePermission(completion);
    });
}

void WhatSonPermissionBootstrapper::buildPermissionSteps()
{
#if defined(Q_OS_MACOS)
    addApplePermissionStep(QStringLiteral("full_disk_access"), WhatSon::Permissions::requestFullDiskAccessPermission);
#endif
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    addApplePermissionStep(QStringLiteral("photo_library"), WhatSon::Permissions::requestPhotoLibraryPermission);
#endif
#if QT_CONFIG(permissions)
    addQtPermissionStep(QStringLiteral("microphone"), []() -> QMicrophonePermission { return QMicrophonePermission{}; });
#endif
#if defined(Q_OS_MACOS)
    addApplePermissionStep(QStringLiteral("accessibility"), WhatSon::Permissions::requestAccessibilityPermission);
#endif
#if QT_CONFIG(permissions)
    addQtPermissionStep(QStringLiteral("calendar"), []() -> QCalendarPermission {
        QCalendarPermission permission;
        permission.setAccessMode(QCalendarPermission::ReadWrite);
        return permission;
    });
#endif
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    addApplePermissionStep(QStringLiteral("reminders"), WhatSon::Permissions::requestRemindersPermission);
    addApplePermissionStep(QStringLiteral("local_network"), WhatSon::Permissions::requestLocalNetworkPermission);
#endif
#if QT_CONFIG(permissions)
    addQtPermissionStep(QStringLiteral("location"), []() -> QLocationPermission {
        QLocationPermission permission;
        permission.setAvailability(QLocationPermission::WhenInUse);
        permission.setAccuracy(QLocationPermission::Precise);
        return permission;
    });
#endif
}

void WhatSonPermissionBootstrapper::requestNextPermission()
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
