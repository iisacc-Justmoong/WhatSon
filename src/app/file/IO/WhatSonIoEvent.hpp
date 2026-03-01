#pragma once

#include <QDateTime>
#include <QString>
#include <QVariantMap>

struct WhatSonIoEvent final
{
    QString name;
    QVariantMap payload;
    QDateTime createdAtUtc;
};
