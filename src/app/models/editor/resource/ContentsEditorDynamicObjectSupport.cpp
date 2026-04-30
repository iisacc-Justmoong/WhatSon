#include "app/models/editor/resource/ContentsEditorDynamicObjectSupport.hpp"

#include <QMetaObject>
#include <QVariantMap>

namespace WhatSon::Editor::DynamicObjectSupport
{
namespace
{
    QVariant invokeVariantImpl(QObject* object, const char* methodName, const QVariantList& arguments)
    {
        if (!object || !methodName)
        {
            return {};
        }

        QVariant result;
        bool invoked = false;
        switch (arguments.size())
        {
        case 0:
            invoked = QMetaObject::invokeMethod(object, methodName, Q_RETURN_ARG(QVariant, result));
            break;
        case 1:
            invoked = QMetaObject::invokeMethod(
                object,
                methodName,
                Q_RETURN_ARG(QVariant, result),
                Q_ARG(QVariant, arguments.at(0)));
            break;
        case 2:
            invoked = QMetaObject::invokeMethod(
                object,
                methodName,
                Q_RETURN_ARG(QVariant, result),
                Q_ARG(QVariant, arguments.at(0)),
                Q_ARG(QVariant, arguments.at(1)));
            break;
        case 3:
            invoked = QMetaObject::invokeMethod(
                object,
                methodName,
                Q_RETURN_ARG(QVariant, result),
                Q_ARG(QVariant, arguments.at(0)),
                Q_ARG(QVariant, arguments.at(1)),
                Q_ARG(QVariant, arguments.at(2)));
            break;
        default:
            invoked = false;
            break;
        }

        return invoked ? result : QVariant{};
    }
}

QVariant propertyValue(const QObject* object, const char* propertyName)
{
    return object && propertyName ? object->property(propertyName) : QVariant{};
}

bool boolProperty(const QObject* object, const char* propertyName, const bool defaultValue)
{
    const QVariant value = propertyValue(object, propertyName);
    return value.isValid() ? value.toBool() : defaultValue;
}

int intProperty(const QObject* object, const char* propertyName, const int defaultValue)
{
    const QVariant value = propertyValue(object, propertyName);
    return value.isValid() ? value.toInt() : defaultValue;
}

qreal realProperty(const QObject* object, const char* propertyName, const qreal defaultValue)
{
    const QVariant value = propertyValue(object, propertyName);
    return value.isValid() ? value.toReal() : defaultValue;
}

QString stringProperty(const QObject* object, const char* propertyName, const QString& defaultValue)
{
    const QVariant value = propertyValue(object, propertyName);
    return value.isValid() ? value.toString() : defaultValue;
}

QVariant invokeVariant(QObject* object, const char* methodName, const QVariantList& arguments)
{
    return invokeVariantImpl(object, methodName, arguments);
}

bool invokeBool(QObject* object, const char* methodName, const QVariantList& arguments, const bool defaultValue)
{
    const QVariant result = invokeVariantImpl(object, methodName, arguments);
    return result.isValid() ? result.toBool() : defaultValue;
}

int invokeInt(QObject* object, const char* methodName, const QVariantList& arguments, const int defaultValue)
{
    const QVariant result = invokeVariantImpl(object, methodName, arguments);
    return result.isValid() ? result.toInt() : defaultValue;
}

QString invokeString(
    QObject* object,
    const char* methodName,
    const QVariantList& arguments,
    const QString& defaultValue)
{
    const QVariant result = invokeVariantImpl(object, methodName, arguments);
    return result.isValid() ? result.toString() : defaultValue;
}

QVariantList normalizeSequentialVariant(const QVariant& value)
{
    if (!value.isValid() || value.isNull())
    {
        return {};
    }

    if (value.metaType().id() == QMetaType::QVariantList)
    {
        return value.toList();
    }

    if (value.canConvert<QVariantList>())
    {
        return value.toList();
    }

    const QVariantMap map = value.toMap();
    if (!map.isEmpty())
    {
        bool ok = false;
        const int explicitLength = map.value(QStringLiteral("length")).toInt(&ok);
        if (ok && explicitLength >= 0)
        {
            QVariantList values;
            values.reserve(explicitLength);
            for (int index = 0; index < explicitLength; ++index)
            {
                values.append(map.value(QString::number(index)));
            }
            return values;
        }

        const int explicitCount = map.value(QStringLiteral("count")).toInt(&ok);
        if (ok && explicitCount >= 0)
        {
            QVariantList values;
            values.reserve(explicitCount);
            for (int index = 0; index < explicitCount; ++index)
            {
                values.append(map.value(QString::number(index)));
            }
            return values;
        }

        QList<int> numericKeys;
        numericKeys.reserve(map.size());
        for (auto it = map.cbegin(); it != map.cend(); ++it)
        {
            const int keyValue = it.key().toInt(&ok);
            if (ok)
            {
                numericKeys.append(keyValue);
            }
        }
        std::sort(numericKeys.begin(), numericKeys.end());
        numericKeys.erase(std::unique(numericKeys.begin(), numericKeys.end()), numericKeys.end());
        if (!numericKeys.isEmpty())
        {
            QVariantList values;
            values.reserve(numericKeys.size());
            for (const int keyValue : numericKeys)
            {
                values.append(map.value(QString::number(keyValue)));
            }
            return values;
        }
    }

    return { value };
}

} // namespace WhatSon::Editor::DynamicObjectSupport
