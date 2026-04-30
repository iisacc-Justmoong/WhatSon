#include "app/models/navigationbar/NavigationModeController.hpp"

NavigationModeController::NavigationModeController(QObject* parent)
    : QObject(parent)
      , m_viewModeController(NavigationMode::View, this)
      , m_editModeController(NavigationMode::Edit, this)
      , m_controlModeController(NavigationMode::Control, this)
{
    applyActiveModeController(m_activeMode);
}

NavigationModeController::~NavigationModeController() = default;

int NavigationModeController::activeMode() const noexcept
{
    return WhatSon::NavigationBar::modeValue(m_activeMode);
}

QString NavigationModeController::activeModeName() const
{
    return WhatSon::NavigationBar::modeName(m_activeMode);
}

QObject* NavigationModeController::activeModeController() const noexcept
{
    return m_activeModeController;
}

QObject* NavigationModeController::viewModeController() const noexcept
{
    return const_cast<NavigationModeSectionController*>(&m_viewModeController);
}

QObject* NavigationModeController::editModeController() const noexcept
{
    return const_cast<NavigationModeSectionController*>(&m_editModeController);
}

QObject* NavigationModeController::controlModeController() const noexcept
{
    return const_cast<NavigationModeSectionController*>(&m_controlModeController);
}

QObject* NavigationModeController::modeControllerForState(int modeValue) const noexcept
{
    if (!WhatSon::NavigationBar::isValidModeValue(modeValue))
    {
        return nullptr;
    }

    switch (WhatSon::NavigationBar::modeFromValue(modeValue))
    {
    case NavigationMode::View:
        return viewModeController();
    case NavigationMode::Edit:
        return editModeController();
    case NavigationMode::Control:
        return controlModeController();
    }

    return nullptr;
}

void NavigationModeController::setActiveMode(int modeValue)
{
    if (!WhatSon::NavigationBar::isValidModeValue(modeValue))
    {
        return;
    }

    const NavigationMode nextMode = WhatSon::NavigationBar::modeFromValue(modeValue);
    if (nextMode == m_activeMode)
    {
        return;
    }

    m_activeMode = nextMode;
    applyActiveModeController(m_activeMode);
    emit activeModeChanged();
}

void NavigationModeController::requestModeChange(int modeValue)
{
    setActiveMode(modeValue);
}

void NavigationModeController::requestNextMode()
{
    setActiveMode(WhatSon::NavigationBar::modeValue(WhatSon::NavigationBar::nextMode(m_activeMode)));
}

void NavigationModeController::applyActiveModeController(NavigationMode activeMode)
{
    m_viewModeController.setActive(activeMode == NavigationMode::View);
    m_editModeController.setActive(activeMode == NavigationMode::Edit);
    m_controlModeController.setActive(activeMode == NavigationMode::Control);
    m_activeModeController = modeControllerForState(WhatSon::NavigationBar::modeValue(activeMode));
}
