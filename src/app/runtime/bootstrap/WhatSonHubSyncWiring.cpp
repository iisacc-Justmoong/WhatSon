#include "app/runtime/bootstrap/WhatSonHubSyncWiring.hpp"

#include "app/models/file/sync/WhatSonHubSyncController.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QObject>
#include <QString>

namespace WhatSon::Runtime::Bootstrap
{
    bool HubSyncWiringResult::allConnectionsValid() const noexcept
    {
        if (!syncFailedLoggingConnection)
        {
            return false;
        }

        for (const QMetaObject::Connection& connection : localMutationConnections)
        {
            if (!connection)
            {
                return false;
            }
        }

        return localMutationWiringComplete;
    }

    HubSyncWiringResult wireHubSyncController(
        WhatSonHubSyncController* controller,
        QCoreApplication* app,
        const QList<QObject*>& localMutationSources)
    {
        HubSyncWiringResult result;
        if (controller == nullptr || app == nullptr)
        {
            return result;
        }

        result.syncFailedLoggingConnection = QObject::connect(
            controller,
            &WhatSonHubSyncController::syncFailed,
            app,
            [](const QString& errorMessage)
            {
                if (!errorMessage.trimmed().isEmpty())
                {
                    qWarning().noquote() << QStringLiteral("Hub sync failed: %1").arg(errorMessage.trimmed());
                }
            });

        bool allSourcesConnected = true;
        for (QObject* source : localMutationSources)
        {
            if (source == nullptr)
            {
                allSourcesConnected = false;
                continue;
            }

            const QMetaObject::Connection connection = QObject::connect(
                source,
                SIGNAL(hubFilesystemMutated()),
                controller,
                SLOT(acknowledgeLocalMutation()));
            if (!connection)
            {
                allSourcesConnected = false;
            }
            result.localMutationConnections.push_back(connection);
        }

        result.localMutationWiringComplete =
            allSourcesConnected && result.localMutationConnections.size() == localMutationSources.size();
        return result;
    }
} // namespace WhatSon::Runtime::Bootstrap
