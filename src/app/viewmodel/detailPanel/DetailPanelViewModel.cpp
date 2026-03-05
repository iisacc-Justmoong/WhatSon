#include "DetailPanelViewModel.hpp"

#include "DetailPanelToolbarItemsFactory.hpp"

DetailPanelViewModel::DetailPanelViewModel(QObject* parent)
    : QObject(parent)
      , m_toolbarItems(WhatSon::DetailPanel::buildToolbarItems(m_activeState))
{
}

DetailPanelViewModel::~DetailPanelViewModel() = default;

int DetailPanelViewModel::activeState() const noexcept
{
    return WhatSon::DetailPanel::stateValue(m_activeState);
}

QString DetailPanelViewModel::activeStateName() const
{
    return WhatSon::DetailPanel::stateName(m_activeState);
}

QVariantList DetailPanelViewModel::toolbarItems() const
{
    return m_toolbarItems;
}

void DetailPanelViewModel::setActiveState(int stateValue)
{
    if (!WhatSon::DetailPanel::isValidStateValue(stateValue))
    {
        return;
    }

    const DetailContentState nextState = WhatSon::DetailPanel::stateFromValue(stateValue);
    if (nextState == m_activeState)
    {
        return;
    }

    m_activeState = nextState;
    m_toolbarItems = WhatSon::DetailPanel::buildToolbarItems(m_activeState);
    emit activeStateChanged();
    emit toolbarItemsChanged();
}

void DetailPanelViewModel::requestStateChange(int stateValue)
{
    setActiveState(stateValue);
}
