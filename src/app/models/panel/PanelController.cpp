#include "app/models/panel/PanelController.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"

#include <utility>

PanelController::PanelController(QString panelKey, QObject* parent)
    : QObject(parent)
      , m_panelKey(std::move(panelKey))
{
}

QString PanelController::panelKey() const
{
    return m_panelKey;
}

int PanelController::hookRequestCount() const noexcept
{
    return m_hookRequestCount;
}

void PanelController::requestControllerHook(const QString& reason)
{
    ++m_hookRequestCount;
    const QString normalizedReason = reason.trimmed().isEmpty()
                                         ? QStringLiteral("manual")
                                         : reason.trimmed();
    const QString detail = QStringLiteral("panelKey=%1 hookRequestCount=%2 reason=%3")
                           .arg(m_panelKey)
                           .arg(m_hookRequestCount)
                           .arg(normalizedReason);
    WhatSon::Debug::traceSelf(this, QStringLiteral("panel.controller"), QStringLiteral("hook.request"), detail);
    emit hookRequestCountChanged();
    emit controllerHookRequested(normalizedReason);
}
