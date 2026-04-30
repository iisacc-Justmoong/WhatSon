#pragma once

#include "app/models/navigationbar/NavigationModeSectionController.hpp"
#include "app/models/navigationbar/NavigationModeState.hpp"

#include <QObject>
#include <QString>

class NavigationModeController final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int activeMode READ activeMode WRITE setActiveMode NOTIFY activeModeChanged)
    Q_PROPERTY(QString activeModeName READ activeModeName NOTIFY activeModeChanged)
    Q_PROPERTY(QObject* activeModeController READ activeModeController NOTIFY activeModeChanged)
    Q_PROPERTY(QObject* viewModeController READ viewModeController CONSTANT)
    Q_PROPERTY(QObject* editModeController READ editModeController CONSTANT)
    Q_PROPERTY(QObject* controlModeController READ controlModeController CONSTANT)

public:
    using NavigationMode = WhatSon::NavigationBar::Mode;

    explicit NavigationModeController(QObject* parent = nullptr);
    ~NavigationModeController() override;

    int activeMode() const noexcept;
    QString activeModeName() const;
    QObject* activeModeController() const noexcept;
    QObject* viewModeController() const noexcept;
    QObject* editModeController() const noexcept;
    QObject* controlModeController() const noexcept;

    Q_INVOKABLE QObject* modeControllerForState(int modeValue) const noexcept;
    Q_INVOKABLE void setActiveMode(int modeValue);
    Q_INVOKABLE void requestModeChange(int modeValue);
    Q_INVOKABLE void requestNextMode();

public
    slots  :




    void requestControllerHook()
    {
        emit controllerHookRequested();
    }

    signals  :



    void activeModeChanged();
    void controllerHookRequested();

private:
    void applyActiveModeController(NavigationMode activeMode);

    NavigationMode m_activeMode = NavigationMode::View;
    NavigationModeSectionController m_viewModeController;
    NavigationModeSectionController m_editModeController;
    NavigationModeSectionController m_controlModeController;
    QObject* m_activeModeController = nullptr;
};
