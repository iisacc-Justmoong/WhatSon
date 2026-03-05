#pragma once

#include "DetailPanelState.hpp"

#include <QObject>
#include <QVariantList>

class DetailPanelViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int activeState READ activeState WRITE setActiveState NOTIFY activeStateChanged)
    Q_PROPERTY(QString activeStateName READ activeStateName NOTIFY activeStateChanged)
    Q_PROPERTY(QVariantList toolbarItems READ toolbarItems NOTIFY toolbarItemsChanged)

public:
    using DetailContentState = WhatSon::DetailPanel::ContentState;

    explicit DetailPanelViewModel(QObject* parent = nullptr);
    ~DetailPanelViewModel() override;

    int activeState() const noexcept;
    QString activeStateName() const;
    QVariantList toolbarItems() const;

    Q_INVOKABLE void setActiveState(int stateValue);
    Q_INVOKABLE void requestStateChange(int stateValue);

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
    WhatSon::DetailPanel::ContentState m_activeState = WhatSon::DetailPanel::ContentState::FileInfo;
    QVariantList m_toolbarItems;
};
