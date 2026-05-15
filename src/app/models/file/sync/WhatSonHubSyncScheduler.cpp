#include "app/models/file/sync/WhatSonHubSyncScheduler.hpp"

#include <algorithm>

namespace
{
    constexpr int kDefaultPeriodicIntervalMs = 5000;
    constexpr int kDefaultDebounceIntervalMs = 350;
} // namespace

WhatSonHubSyncScheduler::WhatSonHubSyncScheduler(QObject* parent)
    : QObject(parent)
{
    m_periodicTimer.setInterval(kDefaultPeriodicIntervalMs);
    QObject::connect(&m_periodicTimer, &QTimer::timeout, this, &WhatSonHubSyncScheduler::requestSyncCheck);

    m_debounceTimer.setSingleShot(true);
    m_debounceTimer.setInterval(kDefaultDebounceIntervalMs);
    QObject::connect(&m_debounceTimer, &QTimer::timeout, this, &WhatSonHubSyncScheduler::emitSyncCheckDue);
}

void WhatSonHubSyncScheduler::setPeriodicIntervalMs(const int intervalMs)
{
    m_periodicTimer.setInterval(std::max(0, intervalMs));
}

void WhatSonHubSyncScheduler::setDebounceIntervalMs(const int intervalMs)
{
    m_debounceTimer.setInterval(std::max(0, intervalMs));
}

void WhatSonHubSyncScheduler::startPeriodic()
{
    if (!m_periodicTimer.isActive())
    {
        m_periodicTimer.start();
    }
}

void WhatSonHubSyncScheduler::stopPeriodic()
{
    m_periodicTimer.stop();
}

void WhatSonHubSyncScheduler::requestSyncCheck()
{
    if (m_debounceTimer.interval() <= 0)
    {
        QTimer::singleShot(0, this, &WhatSonHubSyncScheduler::emitSyncCheckDue);
        return;
    }

    m_debounceTimer.start();
}

void WhatSonHubSyncScheduler::emitSyncCheckDue()
{
    emit syncCheckDue();
}
