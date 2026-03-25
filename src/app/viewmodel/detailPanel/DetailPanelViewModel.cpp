#include "DetailPanelViewModel.hpp"

#include "DetailPanelToolbarItemsFactory.hpp"
#include "file/WhatSonDebugTrace.hpp"

DetailPanelViewModel::DetailPanelViewModel(QObject* parent)
    : QObject(parent)
      , m_propertiesViewModel(DetailContentState::Properties, this)
      , m_fileStatViewModel(DetailContentState::FileStat, this)
      , m_insertViewModel(DetailContentState::Insert, this)
      , m_fileHistoryViewModel(DetailContentState::FileHistory, this)
      , m_layerViewModel(DetailContentState::Layer, this)
      , m_helpViewModel(DetailContentState::Help, this)
      , m_projectSelectionViewModel(QStringLiteral("DetailContent.ProjectSelection"), this)
      , m_bookmarkSelectionViewModel(QStringLiteral("DetailContent.BookmarkSelection"), this)
      , m_progressSelectionViewModel(QStringLiteral("DetailContent.ProgressSelection"), this)
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

QObject* DetailPanelViewModel::contentViewModelForState(int stateValue) const noexcept
{
    if (!WhatSon::DetailPanel::isValidStateValue(stateValue))
    {
        return nullptr;
    }

    switch (WhatSon::DetailPanel::stateFromValue(stateValue))
    {
    case DetailContentState::Properties:
        return const_cast<DetailContentSectionViewModel*>(&m_propertiesViewModel);
    case DetailContentState::FileStat:
        return const_cast<DetailContentSectionViewModel*>(&m_fileStatViewModel);
    case DetailContentState::Insert:
        return const_cast<DetailContentSectionViewModel*>(&m_insertViewModel);
    case DetailContentState::FileHistory:
        return const_cast<DetailContentSectionViewModel*>(&m_fileHistoryViewModel);
    case DetailContentState::Layer:
        return const_cast<DetailContentSectionViewModel*>(&m_layerViewModel);
    case DetailContentState::Help:
        return const_cast<DetailContentSectionViewModel*>(&m_helpViewModel);
    }

    return nullptr;
}

QObject* DetailPanelViewModel::insertViewModel() const noexcept
{
    return const_cast<DetailContentSectionViewModel*>(&m_insertViewModel);
}

QObject* DetailPanelViewModel::fileHistoryViewModel() const noexcept
{
    return const_cast<DetailContentSectionViewModel*>(&m_fileHistoryViewModel);
}

QObject* DetailPanelViewModel::layerViewModel() const noexcept
{
    return const_cast<DetailContentSectionViewModel*>(&m_layerViewModel);
}

QObject* DetailPanelViewModel::projectSelectionViewModel() const noexcept
{
    return const_cast<DetailHierarchySelectionViewModel*>(&m_projectSelectionViewModel);
}

QObject* DetailPanelViewModel::bookmarkSelectionViewModel() const noexcept
{
    return const_cast<DetailHierarchySelectionViewModel*>(&m_bookmarkSelectionViewModel);
}

QObject* DetailPanelViewModel::progressSelectionViewModel() const noexcept
{
    return const_cast<DetailHierarchySelectionViewModel*>(&m_progressSelectionViewModel);
}

QObject* DetailPanelViewModel::fileStatViewModel() const noexcept
{
    return const_cast<DetailContentSectionViewModel*>(&m_fileStatViewModel);
}

QObject* DetailPanelViewModel::helpViewModel() const noexcept
{
    return const_cast<DetailContentSectionViewModel*>(&m_helpViewModel);
}

QObject* DetailPanelViewModel::propertiesViewModel() const noexcept
{
    return const_cast<DetailContentSectionViewModel*>(&m_propertiesViewModel);
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

void DetailPanelViewModel::setProjectSelectionSourceViewModel(QObject* sourceViewModel)
{
    m_projectSelectionViewModel.setSourceViewModel(sourceViewModel);
}

void DetailPanelViewModel::setBookmarkSelectionSourceViewModel(QObject* sourceViewModel)
{
    m_bookmarkSelectionViewModel.setSourceViewModel(sourceViewModel);
}

void DetailPanelViewModel::setProgressSelectionSourceViewModel(QObject* sourceViewModel)
{
    m_progressSelectionViewModel.setSourceViewModel(sourceViewModel);
}

void DetailPanelViewModel::applyActiveContentViewModel(DetailContentState activeState)
{
    m_propertiesViewModel.setActive(activeState == DetailContentState::Properties);
    m_fileStatViewModel.setActive(activeState == DetailContentState::FileStat);
    m_insertViewModel.setActive(activeState == DetailContentState::Insert);
    m_fileHistoryViewModel.setActive(activeState == DetailContentState::FileHistory);
    m_layerViewModel.setActive(activeState == DetailContentState::Layer);
    m_helpViewModel.setActive(activeState == DetailContentState::Help);
    m_activeContentViewModel = contentViewModelForState(WhatSon::DetailPanel::stateValue(activeState));
}
