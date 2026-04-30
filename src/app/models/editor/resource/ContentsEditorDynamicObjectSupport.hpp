#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantList>

namespace WhatSon::Editor::DynamicObjectSupport
{
QVariant propertyValue(const QObject* object, const char* propertyName);
bool boolProperty(const QObject* object, const char* propertyName, bool defaultValue = false);
int intProperty(const QObject* object, const char* propertyName, int defaultValue = 0);
qreal realProperty(const QObject* object, const char* propertyName, qreal defaultValue = 0.0);
QString stringProperty(const QObject* object, const char* propertyName, const QString& defaultValue = {});

QVariant invokeVariant(QObject* object, const char* methodName, const QVariantList& arguments = {});
bool invokeBool(QObject* object, const char* methodName, const QVariantList& arguments = {}, bool defaultValue = false);
int invokeInt(QObject* object, const char* methodName, const QVariantList& arguments = {}, int defaultValue = 0);
QString invokeString(
    QObject* object,
    const char* methodName,
    const QVariantList& arguments = {},
    const QString& defaultValue = {});

QVariantList normalizeSequentialVariant(const QVariant& value);

} // namespace WhatSon::Editor::DynamicObjectSupport
