#include "app/platform/Apple/WhatSonIosHubPickerBridge.hpp"

#if !defined(Q_OS_IOS)

class WhatSonIosHubPickerBridgePrivate
{
};

WhatSonIosHubPickerBridge::WhatSonIosHubPickerBridge(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<WhatSonIosHubPickerBridgePrivate>())
{
}

WhatSonIosHubPickerBridge::~WhatSonIosHubPickerBridge() = default;

bool WhatSonIosHubPickerBridge::busy() const noexcept
{
    return m_busy;
}

QString WhatSonIosHubPickerBridge::lastError() const
{
    return m_lastError;
}

bool WhatSonIosHubPickerBridge::open(const QUrl&)
{
    setLastError(QStringLiteral("The native iOS hub picker is unavailable on this platform."));
    return false;
}

void WhatSonIosHubPickerBridge::clearLastError()
{
    setLastError(QString());
}

void WhatSonIosHubPickerBridge::handlePickerAccepted(const QUrl& selectedUrl)
{
    Q_UNUSED(selectedUrl);
}

void WhatSonIosHubPickerBridge::handlePickerCanceled()
{
}

void WhatSonIosHubPickerBridge::handlePickerFailed(const QString& errorMessage)
{
    setLastError(errorMessage);
}

void WhatSonIosHubPickerBridge::setBusy(const bool busy)
{
    if (m_busy == busy)
    {
        return;
    }

    m_busy = busy;
    emit busyChanged();
}

void WhatSonIosHubPickerBridge::setLastError(const QString& errorMessage)
{
    const QString normalizedError = errorMessage.trimmed();
    if (m_lastError == normalizedError)
    {
        return;
    }

    m_lastError = normalizedError;
    emit lastErrorChanged();
}

#endif
