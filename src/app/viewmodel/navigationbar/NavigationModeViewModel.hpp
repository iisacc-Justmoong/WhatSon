#pragma once

#include "NavigationModeSectionViewModel.hpp"
#include "NavigationModeState.hpp"

#include <QObject>
#include <QString>

class NavigationModeViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int activeMode READ activeMode WRITE setActiveMode NOTIFY activeModeChanged)
    Q_PROPERTY(QString activeModeName READ activeModeName NOTIFY activeModeChanged)
    Q_PROPERTY(QObject* activeModeViewModel READ activeModeViewModel NOTIFY activeModeChanged)
    Q_PROPERTY(QObject* viewModeViewModel READ viewModeViewModel CONSTANT)
    Q_PROPERTY(QObject* editModeViewModel READ editModeViewModel CONSTANT)
    Q_PROPERTY(QObject* controlModeViewModel READ controlModeViewModel CONSTANT)
    Q_PROPERTY(QObject* presentationModeViewModel READ presentationModeViewModel CONSTANT)

public:
    using NavigationMode = WhatSon::NavigationBar::Mode;

    explicit NavigationModeViewModel(QObject* parent = nullptr);
    ~NavigationModeViewModel() override;

    int activeMode() const noexcept;
    QString activeModeName() const;
    QObject* activeModeViewModel() const noexcept;
    QObject* viewModeViewModel() const noexcept;
    QObject* editModeViewModel() const noexcept;
    QObject* controlModeViewModel() const noexcept;
    QObject* presentationModeViewModel() const noexcept;

    Q_INVOKABLE QObject* modeViewModelForState(int modeValue) const noexcept;
    Q_INVOKABLE void setActiveMode(int modeValue);
    Q_INVOKABLE void requestModeChange(int modeValue);
    Q_INVOKABLE void requestNextMode();

public
    slots  :




    void requestViewModelHook()
    {
        emit viewModelHookRequested();
    }

    signals  :



    void activeModeChanged();
    void viewModelHookRequested();

private:
    void applyActiveModeViewModel(NavigationMode activeMode);

    NavigationMode m_activeMode = NavigationMode::Control;
    NavigationModeSectionViewModel m_viewModeViewModel;
    NavigationModeSectionViewModel m_editModeViewModel;
    NavigationModeSectionViewModel m_controlModeViewModel;
    NavigationModeSectionViewModel m_presentationModeViewModel;
    QObject* m_activeModeViewModel = nullptr;
};
