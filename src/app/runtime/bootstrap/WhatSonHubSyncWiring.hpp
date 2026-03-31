#pragma once

#include <QList>
#include <QMetaObject>

class QCoreApplication;
class QObject;
class WhatSonHubSyncController;

namespace WhatSon::Runtime::Bootstrap
{
    struct HubSyncWiringResult final
    {
        QMetaObject::Connection syncFailedLoggingConnection;
        QList<QMetaObject::Connection> localMutationConnections;
        bool localMutationWiringComplete = false;

        [[nodiscard]] bool allConnectionsValid() const noexcept;
    };

    HubSyncWiringResult wireHubSyncController(
        WhatSonHubSyncController* controller,
        QCoreApplication* app,
        const QList<QObject*>& localMutationSources);
} // namespace WhatSon::Runtime::Bootstrap
