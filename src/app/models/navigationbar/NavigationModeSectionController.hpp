#pragma once

#include "app/models/navigationbar/NavigationModeState.hpp"

#include <QObject>
#include <QString>

class NavigationModeSectionController final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(int modeValue READ modeValue CONSTANT)
    Q_PROPERTY(QString modeName READ modeName CONSTANT)

public:
    using NavigationMode = WhatSon::NavigationBar::Mode;

    explicit NavigationModeSectionController(
        NavigationMode mode,
        QObject* parent = nullptr);

    bool active() const noexcept;
    int modeValue() const noexcept;
    QString modeName() const;

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
    NavigationMode m_mode = NavigationMode::Control;
    bool m_active = false;
};
