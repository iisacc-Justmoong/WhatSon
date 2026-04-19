#include "WhatSonAppLaunchSupport.hpp"

#include <QCommandLineOption>
#include <QCommandLineParser>

namespace WhatSon::Runtime::Bootstrap
{
    void prependEnvPath(const char* variableName, const QByteArray& path)
    {
        if (path.isEmpty())
        {
            return;
        }

        const QByteArray currentValue = qgetenv(variableName);
        if (currentValue.isEmpty())
        {
            qputenv(variableName, path);
            return;
        }

        qputenv(variableName, path + ":" + currentValue);
    }

    LaunchOptions parseLaunchOptions(QGuiApplication& app)
    {
        QCommandLineParser parser;
        parser.setApplicationDescription(QStringLiteral("WhatSon application launcher"));
        parser.addHelpOption();

        const QCommandLineOption onboardingOnlyOption(
            QStringList{QStringLiteral("onboarding-only")},
            QStringLiteral("Launch only the onboarding window and skip workspace initialization."));
        parser.addOption(onboardingOnlyOption);
        parser.process(app);

        LaunchOptions options;
        options.onboardingOnly = parser.isSet(onboardingOnlyOption);
        return options;
    }
}
