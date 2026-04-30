#include "app/models/editor/input/ContentsQmlBackedInputControllerBase.hpp"

#include "app/models/editor/resource/ContentsEditorDynamicObjectSupport.hpp"

#include <QMetaMethod>
#include <QMetaProperty>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QVariantMap>

ContentsQmlBackedInputControllerBase::ContentsQmlBackedInputControllerBase(QObject* parent)
    : QObject(parent)
{
}

void ContentsQmlBackedInputControllerBase::classBegin() {}

void ContentsQmlBackedInputControllerBase::componentComplete()
{
    m_componentComplete = true;
    createHelper();
}

QObject* ContentsQmlBackedInputControllerBase::helperObject() const noexcept
{
    return m_helperObject;
}

void ContentsQmlBackedInputControllerBase::syncHelperProperty(const char* propertyName, const QVariant& value)
{
    if (m_helperObject)
        m_helperObject->setProperty(propertyName, value);
}

bool ContentsQmlBackedInputControllerBase::connectHelperPropertyNotify(
    const char* helperPropertyName,
    const char* signalSignature)
{
    if (!m_helperObject)
        return false;

    const QMetaObject* helperMetaObject = m_helperObject->metaObject();
    const int helperPropertyIndex = helperMetaObject->indexOfProperty(helperPropertyName);
    if (helperPropertyIndex < 0)
        return false;

    const QMetaProperty helperProperty = helperMetaObject->property(helperPropertyIndex);
    if (!helperProperty.hasNotifySignal())
        return false;

    const int signalIndex = metaObject()->indexOfSignal(signalSignature);
    if (signalIndex < 0)
        return false;

    return QObject::connect(
        m_helperObject,
        helperProperty.notifySignal(),
        this,
        metaObject()->method(signalIndex),
        Qt::UniqueConnection);
}

QVariant ContentsQmlBackedInputControllerBase::helperProperty(const char* propertyName) const
{
    return WhatSon::Editor::DynamicObjectSupport::propertyValue(m_helperObject, propertyName);
}

QVariant ContentsQmlBackedInputControllerBase::invokeHelperVariant(
    const char* methodName,
    const QVariantList& arguments) const
{
    return WhatSon::Editor::DynamicObjectSupport::invokeVariant(m_helperObject, methodName, arguments);
}

QVariantMap ContentsQmlBackedInputControllerBase::invokeHelperVariantMap(
    const char* methodName,
    const QVariantList& arguments) const
{
    return invokeHelperVariant(methodName, arguments).toMap();
}

QVariantList ContentsQmlBackedInputControllerBase::invokeHelperVariantList(
    const char* methodName,
    const QVariantList& arguments) const
{
    return WhatSon::Editor::DynamicObjectSupport::normalizeSequentialVariant(
        invokeHelperVariant(methodName, arguments));
}

bool ContentsQmlBackedInputControllerBase::invokeHelperBool(
    const char* methodName,
    const QVariantList& arguments,
    const bool defaultValue) const
{
    return WhatSon::Editor::DynamicObjectSupport::invokeBool(m_helperObject, methodName, arguments, defaultValue);
}

int ContentsQmlBackedInputControllerBase::invokeHelperInt(
    const char* methodName,
    const QVariantList& arguments,
    const int defaultValue) const
{
    return WhatSon::Editor::DynamicObjectSupport::invokeInt(m_helperObject, methodName, arguments, defaultValue);
}

QString ContentsQmlBackedInputControllerBase::invokeHelperString(
    const char* methodName,
    const QVariantList& arguments,
    const QString& defaultValue) const
{
    return WhatSon::Editor::DynamicObjectSupport::invokeString(m_helperObject, methodName, arguments, defaultValue);
}

void ContentsQmlBackedInputControllerBase::invokeHelperVoid(
    const char* methodName,
    const QVariantList& arguments) const
{
    WhatSon::Editor::DynamicObjectSupport::invokeVariant(m_helperObject, methodName, arguments);
}

void ContentsQmlBackedInputControllerBase::createHelper()
{
    if (!m_componentComplete || m_helperObject)
        return;

    QQmlEngine* engine = qmlEngine(this);
    QQmlContext* context = QQmlEngine::contextForObject(this);
    if (!engine || !context)
        return;

    QQmlComponent component(engine, helperSourceUrl(), this);
    if (component.isError())
        return;

    QVariantMap initialProperties;
    const int propertyOffset = QObject::staticMetaObject.propertyCount();
    for (int index = propertyOffset; index < metaObject()->propertyCount(); ++index)
    {
        const QMetaProperty property = metaObject()->property(index);
        if (!property.isReadable() || !property.isWritable())
            continue;
        initialProperties.insert(QString::fromLatin1(property.name()), property.read(this));
    }

    QObject* instance = component.createWithInitialProperties(initialProperties, context);
    if (!instance)
        return;

    instance->setParent(this);
    m_helperObject = instance;
    emit helperReady();
}
