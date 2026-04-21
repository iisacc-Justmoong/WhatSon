#pragma once

#include "WhatSonIoEventListener.hpp"
#include "WhatSonSystemIoGateway.hpp"

#include <QString>
#include <QVariantMap>

class WhatSonIoRuntimeController final
{
public:
    WhatSonIoRuntimeController();
    ~WhatSonIoRuntimeController();

    WhatSonIoEventListener* eventListener() noexcept;
    const WhatSonIoEventListener* eventListener() const noexcept;

    WhatSonSystemIoGateway* systemIo() noexcept;
    const WhatSonSystemIoGateway* systemIo() const noexcept;

    void enqueueLvrsEvent(QString eventName, QVariantMap payload = {});
    bool processNext(QString* errorMessage = nullptr);
    int processAll(QString* errorMessage = nullptr);

    QVariantMap lastResult() const;
    void clearLastResult();

private:
    bool processEvent(const WhatSonIoEvent& event, QString* errorMessage = nullptr);
    QVariantMap buildResult(
        bool ok,
        const QString& action,
        const QString& message,
        const QVariantMap& extra = {}) const;

    WhatSonIoEventListener m_eventListener;
    WhatSonSystemIoGateway m_systemIo;
    QVariantMap m_lastResult;
};
