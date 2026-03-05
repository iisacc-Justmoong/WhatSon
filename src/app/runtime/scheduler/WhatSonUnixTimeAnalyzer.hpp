#pragma once

#include <QVariantMap>

namespace WhatSon::Runtime::Scheduler
{
    QVariantMap analyzeUnixSeconds(qint64 unixSeconds);
    qint64 unixNowSeconds();
}
