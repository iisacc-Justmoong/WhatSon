#include "DetailContentSectionViewModel.hpp"

DetailContentSectionViewModel::DetailContentSectionViewModel(
    DetailContentState state,
    QObject* parent)
    : QObject(parent)
      , m_state(state)
{
    setObjectName(QStringLiteral("DetailContent.%1").arg(WhatSon::DetailPanel::stateName(m_state)));
}

bool DetailContentSectionViewModel::active() const noexcept
{
    return m_active;
}

int DetailContentSectionViewModel::stateValue() const noexcept
{
    return WhatSon::DetailPanel::stateValue(m_state);
}

QString DetailContentSectionViewModel::stateName() const
{
    return WhatSon::DetailPanel::stateName(m_state);
}

void DetailContentSectionViewModel::setActive(bool active)
{
    if (m_active == active)
    {
        return;
    }

    m_active = active;
    emit activeChanged();
}
