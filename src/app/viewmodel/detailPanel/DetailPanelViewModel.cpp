#include "DetailPanelViewModel.hpp"

#include "DetailPanelToolbarItemsFactory.hpp"
#include "file/WhatSonDebugTrace.hpp"

DetailPanelViewModel::DetailPanelViewModel(QObject* parent)
    : QObject(parent)
      , m_fileInfoViewModel(DetailContentState::FileInfo, this)
      , m_fileStatViewModel(DetailContentState::FileStat, this)
      , m_fileFormatViewModel(DetailContentState::FileFormat, this)
      , m_fileHistoryViewModel(DetailContentState::FileHistory, this)
      , m_appearanceViewModel(DetailContentState::Appearance, this)
      , m_helpViewModel(DetailContentState::Help, this)
{
    applyActiveContentViewModel(m_activeState);
    m_toolbarItems = WhatSon::DetailPanel::buildToolbarItems(m_activeState);
    const QString detail = QStringLiteral("activeState=%1 toolbarItemCount=%2")
                           .arg(activeStateName())
                           .arg(m_toolbarItems.size());
    WhatSon::Debug::traceSelf(this, QStringLiteral("detail.panel.viewmodel"), QStringLiteral("ctor"), detail);
}

DetailPanelViewModel::~DetailPanelViewModel() = default;

int DetailPanelViewModel::activeState() const noexcept
{
    return WhatSon::DetailPanel::stateValue(m_activeState);
}

QObject* DetailPanelViewModel::activeContentViewModel() const noexcept
{
    return m_activeContentViewModel;
}

QString DetailPanelViewModel::activeStateName() const
{
    return WhatSon::DetailPanel::stateName(m_activeState);
}

QObject* DetailPanelViewModel::appearanceViewModel() const noexcept
{
    return const_cast<DetailContentSectionViewModel*>(&m_appearanceViewModel);
}

QObject* DetailPanelViewModel::contentViewModelForState(int stateValue) const noexcept
{
    if (!WhatSon::DetailPanel::isValidStateValue(stateValue))
    {
        return nullptr;
    }

    switch (WhatSon::DetailPanel::stateFromValue(stateValue))
    {
    case DetailContentState::FileInfo:
        return const_cast<DetailContentSectionViewModel*>(&m_fileInfoViewModel);
    case DetailContentState::FileStat:
        return const_cast<DetailContentSectionViewModel*>(&m_fileStatViewModel);
    case DetailContentState::FileFormat:
        return const_cast<DetailContentSectionViewModel*>(&m_fileFormatViewModel);
    case DetailContentState::FileHistory:
        return const_cast<DetailContentSectionViewModel*>(&m_fileHistoryViewModel);
    case DetailContentState::Appearance:
        return const_cast<DetailContentSectionViewModel*>(&m_appearanceViewModel);
    case DetailContentState::Help:
        return const_cast<DetailContentSectionViewModel*>(&m_helpViewModel);
    }

    return nullptr;
}

QObject* DetailPanelViewModel::fileFormatViewModel() const noexcept
{
    return const_cast<DetailContentSectionViewModel*>(&m_fileFormatViewModel);
}

QObject* DetailPanelViewModel::fileHistoryViewModel() const noexcept
{
    return const_cast<DetailContentSectionViewModel*>(&m_fileHistoryViewModel);
}

QObject* DetailPanelViewModel::fileInfoViewModel() const noexcept
{
    return const_cast<DetailContentSectionViewModel*>(&m_fileInfoViewModel);
}

QObject* DetailPanelViewModel::fileStatViewModel() const noexcept
{
    return const_cast<DetailContentSectionViewModel*>(&m_fileStatViewModel);
}

QObject* DetailPanelViewModel::helpViewModel() const noexcept
{
    return const_cast<DetailContentSectionViewModel*>(&m_helpViewModel);
}

QVariantList DetailPanelViewModel::toolbarItems() const
{
    return m_toolbarItems;
}

void DetailPanelViewModel::setActiveState(int stateValue)
{
    if (!WhatSon::DetailPanel::isValidStateValue(stateValue))
    {
        const QString detail = QStringLiteral("requestedStateValue=%1 result=ignored_invalid").arg(stateValue);
        WhatSon::Debug::traceSelf(
            this,
            QStringLiteral("detail.panel.viewmodel"),
            QStringLiteral("setActiveState.ignoredInvalid"),
            detail);
        return;
    }

    const QString previousStateName = activeStateName();
    const DetailContentState nextState = WhatSon::DetailPanel::stateFromValue(stateValue);
    if (nextState == m_activeState)
    {
        const QString detail = QStringLiteral("requestedStateValue=%1 requestedStateName=%2 result=ignored_same")
                               .arg(stateValue)
                               .arg(previousStateName);
        WhatSon::Debug::traceSelf(
            this,
            QStringLiteral("detail.panel.viewmodel"),
            QStringLiteral("setActiveState.ignoredSame"),
            detail);
        return;
    }

    m_activeState = nextState;
    applyActiveContentViewModel(m_activeState);
    m_toolbarItems = WhatSon::DetailPanel::buildToolbarItems(m_activeState);
    const QString detail = QStringLiteral("requestedStateValue=%1 previousState=%2 nextState=%3 toolbarItemCount=%4")
                           .arg(stateValue)
                           .arg(previousStateName)
                           .arg(activeStateName())
                           .arg(m_toolbarItems.size());
    WhatSon::Debug::traceSelf(
        this,
        QStringLiteral("detail.panel.viewmodel"),
        QStringLiteral("setActiveState.applied"),
        detail);
    emit activeStateChanged();
    emit toolbarItemsChanged();
}

void DetailPanelViewModel::requestStateChange(int stateValue)
{
    const QString detail = QStringLiteral("requestedStateValue=%1").arg(stateValue);
    WhatSon::Debug::traceSelf(
        this,
        QStringLiteral("detail.panel.viewmodel"),
        QStringLiteral("requestStateChange"),
        detail);
    setActiveState(stateValue);
}

void DetailPanelViewModel::applyActiveContentViewModel(DetailContentState activeState)
{
    m_fileInfoViewModel.setActive(activeState == DetailContentState::FileInfo);
    m_fileStatViewModel.setActive(activeState == DetailContentState::FileStat);
    m_fileFormatViewModel.setActive(activeState == DetailContentState::FileFormat);
    m_fileHistoryViewModel.setActive(activeState == DetailContentState::FileHistory);
    m_appearanceViewModel.setActive(activeState == DetailContentState::Appearance);
    m_helpViewModel.setActive(activeState == DetailContentState::Help);
    m_activeContentViewModel = contentViewModelForState(WhatSon::DetailPanel::stateValue(activeState));
}
