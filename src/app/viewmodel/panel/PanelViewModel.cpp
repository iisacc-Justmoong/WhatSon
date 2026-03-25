#include "PanelViewModel.hpp"

#include "file/WhatSonDebugTrace.hpp"

#include <utility>

PanelViewModel::PanelViewModel(QString panelKey, QObject* parent)
    : QObject(parent)
      , m_panelKey(std::move(panelKey))
{
}

QString PanelViewModel::panelKey() const
{
    return m_panelKey;
}

int PanelViewModel::hookRequestCount() const noexcept
{
    return m_hookRequestCount;
}

void PanelViewModel::requestViewModelHook(const QString& reason)
{
    ++m_hookRequestCount;
    const QString normalizedReason = reason.trimmed().isEmpty()
                                         ? QStringLiteral("manual")
                                         : reason.trimmed();
    const QString detail = QStringLiteral("panelKey=%1 hookRequestCount=%2 reason=%3")
                           .arg(m_panelKey)
                           .arg(m_hookRequestCount)
                           .arg(normalizedReason);
    WhatSon::Debug::traceSelf(this, QStringLiteral("panel.viewmodel"), QStringLiteral("hook.request"), detail);
    emit hookRequestCountChanged();
    emit viewModelHookRequested(normalizedReason);
}
