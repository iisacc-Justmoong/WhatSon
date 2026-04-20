#pragma once

#include <QByteArray>
#include <QString>

class ISelectedHubStore;
class WhatSonHubMountValidator;

namespace WhatSon::Runtime::Startup
{
    enum class StartupHubSource
    {
        None,
        PersistedSelection
    };

    struct StartupHubSelection final
    {
        bool mounted = false;
        StartupHubSource source = StartupHubSource::None;
        QString hubPath;
        QByteArray accessBookmark;
        QString failureMessage;
    };

    StartupHubSelection resolveStartupHubSelection(
        ISelectedHubStore& selectedHubStore,
        const WhatSonHubMountValidator& hubMountValidator);
}
