#include "NavigationModeSectionViewModel.hpp"

NavigationModeSectionViewModel::NavigationModeSectionViewModel(
    NavigationMode mode,
    QObject* parent)
    : QObject(parent)
      , m_mode(mode)
{
}

bool NavigationModeSectionViewModel::active() const noexcept
{
    return m_active;
}

int NavigationModeSectionViewModel::modeValue() const noexcept
{
    return WhatSon::NavigationBar::modeValue(m_mode);
}

QString NavigationModeSectionViewModel::modeName() const
{
    return WhatSon::NavigationBar::modeName(m_mode);
}

void NavigationModeSectionViewModel::setActive(bool active)
{
    if (m_active == active)
    {
        return;
    }

    m_active = active;
    emit activeChanged();
}
