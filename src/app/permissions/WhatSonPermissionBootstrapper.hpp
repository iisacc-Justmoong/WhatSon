#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <functional>
#include <utility>

#include "permissions/ApplePermissionBridge.hpp"

#include <QCoreApplication>
#include <QtCore/qpermissions.h>

class WhatSonPermissionBootstrapper final : public QObject
{
    Q_OBJECT

public:
    explicit WhatSonPermissionBootstrapper(QCoreApplication& app);
    void start();

private:
    using PermissionCompletion = std::function<void(bool granted)>;
    using PermissionRequester = std::function<void(const PermissionCompletion& completion)>;

    struct PermissionStep final
    {
        QString id;
        PermissionRequester request;
    };

    static QString permissionSettingKey(const QString& permissionId, const QString& suffix);
    bool hasStoredDecision(const QString& permissionId) const;
    bool isGranted(const QString& permissionId) const;
    void storeDecision(const QString& permissionId, bool granted) const;
    void finalizePermissionBootstrap() const;
    void addStep(QString id, PermissionRequester request);

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
                const bool granted = m_app.checkPermission(requestedPermission) == Qt::PermissionStatus::Granted;
                completion(granted);
            });
#else
            completion(true);
#endif
        });
    }

    void addApplePermissionStep(
        QString id,
        void (*requestApplePermission)(const WhatSon::Permissions::PermissionCallback& completion));
    void buildPermissionSteps();
    void requestNextPermission();

    QCoreApplication& m_app;
    QVector<PermissionStep> m_steps;
    qsizetype m_nextIndex = 0;
};
