#pragma once

#include <QByteArray>
#include <QGuiApplication>
#include <QString>

namespace WhatSon::Runtime::Bootstrap
{
    struct LaunchOptions final
    {
        bool onboardingOnly = false;
    };

    void prependEnvPath(const char* variableName, const QByteArray& path);
    QString resolveBlueprintHubPath();
    LaunchOptions parseLaunchOptions(QGuiApplication& app);
}
