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

    [[nodiscard]] inline bool startupWorkspaceReady(
        const bool startupHubMounted,
        const bool startupRuntimeLoaded) noexcept
    {
        return startupHubMounted && startupRuntimeLoaded;
    }

    void prependEnvPath(const char* variableName, const QByteArray& path);
    LaunchOptions parseLaunchOptions(QGuiApplication& app);
}
