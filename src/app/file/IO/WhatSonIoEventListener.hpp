#pragma once

#include "WhatSonIoEvent.hpp"

#include <QString>
#include <QStringList>
#include <QVector>
#include <QVariantMap>

class WhatSonIoEventListener final
{
public:
    WhatSonIoEventListener();
    ~WhatSonIoEventListener();

    void clear();

    bool enabled() const noexcept;
    void setEnabled(bool enabled) noexcept;

    QStringList acceptedEventPrefixes() const;
    void setAcceptedEventPrefixes(QStringList prefixes);

    void pushLvrsEvent(QString eventName, QVariantMap payload = {});
    bool hasPendingEvents() const noexcept;
    int pendingCount() const noexcept;
    WhatSonIoEvent takeNextEvent();

private:
    bool acceptsEventName(const QString& eventName) const;

    bool m_enabled = true;
    QStringList m_acceptedEventPrefixes;
    QVector<WhatSonIoEvent> m_pendingEvents;
};
