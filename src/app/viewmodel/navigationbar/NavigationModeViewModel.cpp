#include "NavigationModeViewModel.hpp"

NavigationModeViewModel::NavigationModeViewModel(QObject* parent)
    : QObject(parent)
      , m_viewModeViewModel(NavigationMode::View, this)
      , m_editModeViewModel(NavigationMode::Edit, this)
      , m_controlModeViewModel(NavigationMode::Control, this)
      , m_presentationModeViewModel(NavigationMode::Presentation, this)
{
    applyActiveModeViewModel(m_activeMode);
}

NavigationModeViewModel::~NavigationModeViewModel() = default;

int NavigationModeViewModel::activeMode() const noexcept
{
    return WhatSon::NavigationBar::modeValue(m_activeMode);
}

QString NavigationModeViewModel::activeModeName() const
{
    return WhatSon::NavigationBar::modeName(m_activeMode);
}

QObject* NavigationModeViewModel::activeModeViewModel() const noexcept
{
    return m_activeModeViewModel;
}

QObject* NavigationModeViewModel::viewModeViewModel() const noexcept
{
    return const_cast<NavigationModeSectionViewModel*>(&m_viewModeViewModel);
}

QObject* NavigationModeViewModel::editModeViewModel() const noexcept
{
    return const_cast<NavigationModeSectionViewModel*>(&m_editModeViewModel);
}

QObject* NavigationModeViewModel::controlModeViewModel() const noexcept
{
    return const_cast<NavigationModeSectionViewModel*>(&m_controlModeViewModel);
}

QObject* NavigationModeViewModel::presentationModeViewModel() const noexcept
{
    return const_cast<NavigationModeSectionViewModel*>(&m_presentationModeViewModel);
}

QObject* NavigationModeViewModel::modeViewModelForState(int modeValue) const noexcept
{
    if (!WhatSon::NavigationBar::isValidModeValue(modeValue))
    {
        return nullptr;
    }

    switch (WhatSon::NavigationBar::modeFromValue(modeValue))
    {
    case NavigationMode::View:
        return viewModeViewModel();
    case NavigationMode::Edit:
        return editModeViewModel();
    case NavigationMode::Control:
        return controlModeViewModel();
    case NavigationMode::Presentation:
        return presentationModeViewModel();
    }

    return nullptr;
}

void NavigationModeViewModel::setActiveMode(int modeValue)
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
    applyActiveModeViewModel(m_activeMode);
    emit activeModeChanged();
}

void NavigationModeViewModel::requestModeChange(int modeValue)
{
    setActiveMode(modeValue);
}

void NavigationModeViewModel::requestNextMode()
{
    setActiveMode(WhatSon::NavigationBar::modeValue(WhatSon::NavigationBar::nextMode(m_activeMode)));
}

void NavigationModeViewModel::applyActiveModeViewModel(NavigationMode activeMode)
{
    m_viewModeViewModel.setActive(activeMode == NavigationMode::View);
    m_editModeViewModel.setActive(activeMode == NavigationMode::Edit);
    m_controlModeViewModel.setActive(activeMode == NavigationMode::Control);
    m_presentationModeViewModel.setActive(activeMode == NavigationMode::Presentation);
    m_activeModeViewModel = modeViewModelForState(WhatSon::NavigationBar::modeValue(activeMode));
}
