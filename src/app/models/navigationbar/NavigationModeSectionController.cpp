#include "app/models/navigationbar/NavigationModeSectionController.hpp"

NavigationModeSectionController::NavigationModeSectionController(
    NavigationMode mode,
    QObject* parent)
    : QObject(parent)
      , m_mode(mode)
{
}

bool NavigationModeSectionController::active() const noexcept
{
    return m_active;
}

int NavigationModeSectionController::modeValue() const noexcept
{
    return WhatSon::NavigationBar::modeValue(m_mode);
}

QString NavigationModeSectionController::modeName() const
{
    return WhatSon::NavigationBar::modeName(m_mode);
}

void NavigationModeSectionController::setActive(bool active)
{
    if (m_active == active)
    {
        return;
    }

    m_active = active;
    emit activeChanged();
}
