#include "app/runtime/bootstrap/WhatSonQmlLaunchSupport.hpp"

#include <QDebug>
#include <QQmlApplicationEngine>

namespace WhatSon::Runtime::Bootstrap
{
    QObject* loadQmlRootObject(
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
            return nullptr;
        }

        return loadResult.rootObjects.constLast();
    }

    QObject* loadWhatSonAppRootObject(
        QQmlApplicationEngine& engine,
        const QString& rootObject,
        const QVariantMap& initialProperties,
        const lvrs::QmlWindowActivationPolicy activationPolicy)
    {
        return loadQmlRootObject(
            engine,
            QStringLiteral("WhatSon.App"),
            rootObject,
            initialProperties,
            activationPolicy);
    }

    QObject* loadMainWindow(
        QQmlApplicationEngine& engine,
        const QVariantMap& initialProperties,
        const lvrs::QmlWindowActivationPolicy activationPolicy)
    {
        return loadWhatSonAppRootObject(
            engine,
            QStringLiteral("Main"),
            initialProperties,
            activationPolicy);
    }
} // namespace WhatSon::Runtime::Bootstrap
