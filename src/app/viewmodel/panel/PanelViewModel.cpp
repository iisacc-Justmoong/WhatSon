#include "PanelViewModel.hpp"

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

void PanelViewModel::requestViewModelHook()
{
    ++m_hookRequestCount;
    emit hookRequestCountChanged();
    emit viewModelHookRequested();
}
