#pragma once

#include "backend/runtime/qmltyperegistrar.h"

#include <QList>

namespace WhatSon::Runtime::Bootstrap
{
    QList<lvrs::QmlTypeRegistration> internalQmlTypeRegistrationManifest();
    lvrs::QmlTypeRegistrationReport registerInternalQmlTypes();
}
