#pragma once

#include "backend/runtime/appentry.h"

#include <QString>
#include <QVariantMap>

class QObject;
class QQmlApplicationEngine;

namespace WhatSon::Runtime::Bootstrap
{
    [[nodiscard]] QObject* loadQmlRootObject(
        QQmlApplicationEngine& engine,
        const QString& moduleUri,
        const QString& rootObject,
        const QVariantMap& initialProperties = {},
        lvrs::QmlWindowActivationPolicy activationPolicy = lvrs::QmlWindowActivationPolicy::None);

    [[nodiscard]] QObject* loadWhatSonAppRootObject(
        QQmlApplicationEngine& engine,
        const QString& rootObject,
        const QVariantMap& initialProperties = {},
        lvrs::QmlWindowActivationPolicy activationPolicy = lvrs::QmlWindowActivationPolicy::None);

    [[nodiscard]] QObject* loadMainWindow(
        QQmlApplicationEngine& engine,
        const QVariantMap& initialProperties = {},
        lvrs::QmlWindowActivationPolicy activationPolicy = lvrs::QmlWindowActivationPolicy::None);
} // namespace WhatSon::Runtime::Bootstrap
