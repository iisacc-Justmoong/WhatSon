#include "WhatSonAppLaunchSupport.hpp"

#include "file/WhatSonDebugTrace.hpp"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStringList>

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

    QString resolveBlueprintHubPath()
    {
        const QStringList basePaths = {
            QDir::currentPath(),
            QCoreApplication::applicationDirPath()
        };

        WhatSon::Debug::trace(
            QStringLiteral("main.blueprint"),
            QStringLiteral("resolve.start"),
            QStringLiteral("basePathCount=%1").arg(basePaths.size()));

        for (const QString& basePath : basePaths)
        {
            WhatSon::Debug::trace(
                QStringLiteral("main.blueprint"),
                QStringLiteral("resolve.base"),
                QStringLiteral("basePath=%1").arg(basePath));
            QDir probe(basePath);
            for (int depth = 0; depth < 8; ++depth)
            {
                const QDir blueprintDir(probe.filePath(QStringLiteral("blueprint")));
                WhatSon::Debug::trace(
                    QStringLiteral("main.blueprint"),
                    QStringLiteral("resolve.probe"),
                    QStringLiteral("depth=%1 probe=%2").arg(depth).arg(blueprintDir.path()));
                if (blueprintDir.exists())
                {
                    WhatSon::Debug::trace(
                        QStringLiteral("main.blueprint"),
                        QStringLiteral("resolve.blueprintDirFound"),
                        QStringLiteral("path=%1").arg(blueprintDir.path()));
                    const QFileInfoList hubCandidates = blueprintDir.entryInfoList(
                        QStringList{QStringLiteral("*.wshub")},
                        QDir::Dirs | QDir::NoDotAndDotDot,
                        QDir::Name);
                    if (!hubCandidates.isEmpty())
                    {
                        const QString resolvedPath = QDir::cleanPath(hubCandidates.first().absoluteFilePath());
                        WhatSon::Debug::trace(
                            QStringLiteral("main.blueprint"),
                            QStringLiteral("resolve.found"),
                            QStringLiteral("wshub=%1").arg(resolvedPath));
                        return resolvedPath;
                    }
                }

                if (!probe.cdUp())
                {
                    break;
                }
            }
        }

        WhatSon::Debug::trace(
            QStringLiteral("main.blueprint"),
            QStringLiteral("resolve.notFound"));
        return QString();
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
