#pragma once

#include <QByteArray>
#include <QStringList>

struct WhatSonHubSyncObservation final
{
    QByteArray signature;
    QStringList directoryWatchPaths;
};
