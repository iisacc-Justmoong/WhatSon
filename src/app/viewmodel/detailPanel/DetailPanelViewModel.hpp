#pragma once

#include "DetailHierarchySelectionViewModel.hpp"
#include "DetailContentSectionViewModel.hpp"
#include "DetailPanelState.hpp"

#include <QObject>
#include <QVariantList>

class DetailPanelViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int activeState READ activeState WRITE setActiveState NOTIFY activeStateChanged)
    Q_PROPERTY(QObject* activeContentViewModel READ activeContentViewModel NOTIFY activeStateChanged)
    Q_PROPERTY(QString activeStateName READ activeStateName NOTIFY activeStateChanged)
    Q_PROPERTY(QObject* fileHistoryViewModel READ fileHistoryViewModel CONSTANT)
    Q_PROPERTY(QObject* fileStatViewModel READ fileStatViewModel CONSTANT)
    Q_PROPERTY(QObject* helpViewModel READ helpViewModel CONSTANT)
    Q_PROPERTY(QObject* insertViewModel READ insertViewModel CONSTANT)
    Q_PROPERTY(QObject* layerViewModel READ layerViewModel CONSTANT)
    Q_PROPERTY(QObject* projectSelectionViewModel READ projectSelectionViewModel CONSTANT)
    Q_PROPERTY(QObject* bookmarkSelectionViewModel READ bookmarkSelectionViewModel CONSTANT)
    Q_PROPERTY(QObject* progressSelectionViewModel READ progressSelectionViewModel CONSTANT)
    Q_PROPERTY(QObject* propertiesViewModel READ propertiesViewModel CONSTANT)
    Q_PROPERTY(QVariantList toolbarItems READ toolbarItems NOTIFY toolbarItemsChanged)

public:
    using DetailContentState = WhatSon::DetailPanel::ContentState;

    explicit DetailPanelViewModel(QObject* parent = nullptr);
    ~DetailPanelViewModel() override;

    int activeState() const noexcept;
    QObject* activeContentViewModel() const noexcept;
    QString activeStateName() const;
    Q_INVOKABLE QObject* contentViewModelForState(int stateValue) const noexcept;
    QObject* fileHistoryViewModel() const noexcept;
    QObject* fileStatViewModel() const noexcept;
    QObject* helpViewModel() const noexcept;
    QObject* insertViewModel() const noexcept;
    QObject* layerViewModel() const noexcept;
    QObject* projectSelectionViewModel() const noexcept;
    QObject* bookmarkSelectionViewModel() const noexcept;
    QObject* progressSelectionViewModel() const noexcept;
    QObject* propertiesViewModel() const noexcept;
    QVariantList toolbarItems() const;

    Q_INVOKABLE void setActiveState(int stateValue);
    Q_INVOKABLE void requestStateChange(int stateValue);
    void setProjectSelectionSourceViewModel(QObject* sourceViewModel);
    void setBookmarkSelectionSourceViewModel(QObject* sourceViewModel);
    void setProgressSelectionSourceViewModel(QObject* sourceViewModel);

public
    slots  :




    void requestViewModelHook()
    {
        emit viewModelHookRequested();
    }

    signals  :


    void activeStateChanged();
    void toolbarItemsChanged();
    void viewModelHookRequested();

private:
    void applyActiveContentViewModel(DetailContentState activeState);

    WhatSon::DetailPanel::ContentState m_activeState = WhatSon::DetailPanel::ContentState::Properties;
    DetailContentSectionViewModel m_propertiesViewModel;
    DetailContentSectionViewModel m_fileStatViewModel;
    DetailContentSectionViewModel m_insertViewModel;
    DetailContentSectionViewModel m_fileHistoryViewModel;
    DetailContentSectionViewModel m_layerViewModel;
    DetailContentSectionViewModel m_helpViewModel;
    DetailHierarchySelectionViewModel m_projectSelectionViewModel;
    DetailHierarchySelectionViewModel m_bookmarkSelectionViewModel;
    DetailHierarchySelectionViewModel m_progressSelectionViewModel;
    QObject* m_activeContentViewModel = nullptr;
    QVariantList m_toolbarItems;
};
