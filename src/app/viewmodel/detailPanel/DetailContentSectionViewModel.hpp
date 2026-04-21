#pragma once

#include "app/viewmodel/detailPanel/DetailPanelState.hpp"

#include <QObject>
#include <QString>

class DetailContentSectionViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(int stateValue READ stateValue CONSTANT)
    Q_PROPERTY(QString stateName READ stateName CONSTANT)

public:
    using DetailContentState = WhatSon::DetailPanel::ContentState;

    explicit DetailContentSectionViewModel(
        DetailContentState state,
        QObject* parent = nullptr);

    bool active() const noexcept;
    int stateValue() const noexcept;
    QString stateName() const;

    Q_INVOKABLE void setActive(bool active);

public
    slots  :




    void requestViewModelHook()
    {
        emit viewModelHookRequested();
    }

    signals  :


    void activeChanged();
    void viewModelHookRequested();

private:
    DetailContentState m_state = DetailContentState::Properties;
    bool m_active = false;
};
