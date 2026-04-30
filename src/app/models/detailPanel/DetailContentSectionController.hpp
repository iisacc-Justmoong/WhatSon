#pragma once

#include "app/models/detailPanel/DetailPanelState.hpp"

#include <QObject>
#include <QString>

class DetailContentSectionController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(int stateValue READ stateValue CONSTANT)
    Q_PROPERTY(QString stateName READ stateName CONSTANT)

public:
    using DetailContentState = WhatSon::DetailPanel::ContentState;

    explicit DetailContentSectionController(
        DetailContentState state,
        QObject* parent = nullptr);

    bool active() const noexcept;
    int stateValue() const noexcept;
    QString stateName() const;

    Q_INVOKABLE void setActive(bool active);

public
    slots  :




    void requestControllerHook()
    {
        emit controllerHookRequested();
    }

    signals  :


    void activeChanged();
    void controllerHookRequested();

private:
    DetailContentState m_state = DetailContentState::Properties;
    bool m_active = false;
};
