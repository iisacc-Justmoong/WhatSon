#include "app/runtime/bootstrap/WhatSonQmlLaunchSupport.hpp"

#include <QDebug>
#include <QQmlApplicationEngine>

namespace WhatSon::Runtime::Bootstrap
{
    QObject* lastRootObject(const lvrs::QmlRootLoadResult& loadResult)
    {
        if (!loadResult.ok || loadResult.rootObjects.isEmpty())
        {
            return nullptr;
        }

        return loadResult.rootObjects.constLast();
    }

    lvrs::QmlRootLoadResult loadQmlRoot(
        QQmlApplicationEngine& engine,
        const QString& moduleUri,
        const QString& rootObject,
        const QVariantMap& initialProperties,
        const lvrs::QmlWindowActivationPolicy activationPolicy)
    {
        lvrs::QmlRootLoadSpec rootSpec;
        rootSpec.moduleUri = moduleUri;
        rootSpec.rootObject = rootObject;
        rootSpec.initialProperties = initialProperties;
        rootSpec.windowActivationPolicy = activationPolicy;

        lvrs::QmlRootLoadOptions loadOptions;
        loadOptions.logDiagnostics = true;
        loadOptions.defaultWindowActivationPolicy = lvrs::QmlWindowActivationPolicy::None;

        const lvrs::QmlRootLoadResult loadResult = lvrs::loadQmlRootObjects(
            engine,
            {rootSpec},
            loadOptions);
        if (!loadResult.ok || loadResult.rootObjects.isEmpty())
        {
            qWarning().noquote()
                << QStringLiteral("Failed to load QML root '%1.%2': %3")
                       .arg(moduleUri, rootObject, loadResult.errorMessage());
        }

        return loadResult;
    }

    QObject* loadQmlRootObject(
        QQmlApplicationEngine& engine,
        const QString& moduleUri,
        const QString& rootObject,
        const QVariantMap& initialProperties,
        const lvrs::QmlWindowActivationPolicy activationPolicy)
    {
        return lastRootObject(loadQmlRoot(
            engine,
            moduleUri,
            rootObject,
            initialProperties,
            activationPolicy));
    }

    lvrs::QmlRootLoadResult loadWhatSonAppRoot(
        QQmlApplicationEngine& engine,
        const QString& rootObject,
        const QVariantMap& initialProperties,
        const lvrs::QmlWindowActivationPolicy activationPolicy)
    {
        return loadQmlRoot(
            engine,
            QStringLiteral("WhatSon.App"),
            rootObject,
            initialProperties,
            activationPolicy);
    }

    QObject* loadWhatSonAppRootObject(
        QQmlApplicationEngine& engine,
        const QString& rootObject,
        const QVariantMap& initialProperties,
        const lvrs::QmlWindowActivationPolicy activationPolicy)
    {
        return lastRootObject(loadWhatSonAppRoot(
            engine,
            rootObject,
            initialProperties,
            activationPolicy));
    }

    lvrs::QmlRootLoadResult loadMainWindowRoot(
        QQmlApplicationEngine& engine,
        const QVariantMap& initialProperties,
        const lvrs::QmlWindowActivationPolicy activationPolicy)
    {
        return loadWhatSonAppRoot(
            engine,
            QStringLiteral("Main"),
            initialProperties,
            activationPolicy);
    }

    QObject* loadMainWindow(
        QQmlApplicationEngine& engine,
        const QVariantMap& initialProperties,
        const lvrs::QmlWindowActivationPolicy activationPolicy)
    {
        return lastRootObject(loadMainWindowRoot(engine, initialProperties, activationPolicy));
    }
} // namespace WhatSon::Runtime::Bootstrap
