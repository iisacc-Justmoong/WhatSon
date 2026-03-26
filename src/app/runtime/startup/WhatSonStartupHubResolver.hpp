#pragma once

#include <QByteArray>
#include <QString>

class SelectedHubStore;

namespace WhatSon::Runtime::Startup
{
    struct StartupHubSelection final
    {
        bool mounted = false;
        QString hubPath;
        QByteArray accessBookmark;
    };

    QString resolveStartupHubMountPath(
        const QString& hubPath,
        const QByteArray& hubAccessBookmark,
        QString* errorMessage = nullptr);

    StartupHubSelection resolveStartupHubSelection(
        SelectedHubStore& selectedHubStore,
        const QString& blueprintFallbackHubPath);
}
