#pragma once

#include <QByteArray>
#include <QString>

class ISelectedHubStore;

namespace WhatSon::Runtime::Startup
{
    struct StartupHubSelection final
    {
        bool mounted = false;
        QString hubPath;
        QString selectionUrl;
        QByteArray accessBookmark;
    };

    QString resolveStartupHubMountPath(
        const QString& hubPath,
        const QString& hubSelectionUrl,
        const QByteArray& hubAccessBookmark,
        QString* errorMessage = nullptr);

    StartupHubSelection resolveStartupHubSelection(ISelectedHubStore& selectedHubStore);
}
