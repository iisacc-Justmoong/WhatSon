#pragma once

#include "app/register/WhatSonRegisterManager.hpp"
#include "extension/trial/WhatSonTrialInstallStore.hpp"

#include <QDate>
#include <QObject>
#include <QPointer>

struct WhatSonTrialActivationState final
{
    QDate installDate;
    QDate lastActiveDate;
    int trialLengthDays = 0;
    int elapsedDays = 0;
    int remainingDays = 0;
    bool active = false;
    bool bypassedByAuthentication = false;

    bool operator==(const WhatSonTrialActivationState& other) const = default;
};

class WhatSonTrialActivationPolicy final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ active NOTIFY stateChanged FINAL)
    Q_PROPERTY(QDate installDate READ installDate NOTIFY stateChanged FINAL)
    Q_PROPERTY(QDate lastActiveDate READ lastActiveDate NOTIFY stateChanged FINAL)
    Q_PROPERTY(int trialLengthDays READ trialLengthDays CONSTANT FINAL)
    Q_PROPERTY(int elapsedDays READ elapsedDays NOTIFY stateChanged FINAL)
    Q_PROPERTY(int remainingDays READ remainingDays NOTIFY stateChanged FINAL)
    Q_PROPERTY(bool bypassedByAuthentication READ bypassedByAuthentication NOTIFY stateChanged FINAL)

public:
    static constexpr int kDefaultTrialLengthDays = 90;

    explicit WhatSonTrialActivationPolicy(QObject* parent = nullptr);
    explicit WhatSonTrialActivationPolicy(WhatSonTrialInstallStore installStore, QObject* parent = nullptr);

    QDate installDate() const noexcept;
    QDate lastActiveDate() const noexcept;
    int trialLengthDays() const noexcept;
    int elapsedDays() const noexcept;
    int remainingDays() const noexcept;
    bool active() const noexcept;
    bool bypassedByAuthentication() const noexcept;

    void setRegisterManager(WhatSonRegisterManager* registerManager);

    WhatSonTrialActivationState currentState() const noexcept;
    WhatSonTrialActivationState refreshForDate(const QDate& today);

public slots:
    void refresh();

signals:
    void stateChanged();

private:
    static WhatSonTrialActivationState buildState(const QDate& installDate, const QDate& today);
    static WhatSonTrialActivationState buildAuthenticatedState(const QDate& installDate);
    void setState(const WhatSonTrialActivationState& state);

    WhatSonTrialInstallStore m_installStore;
    QPointer<WhatSonRegisterManager> m_registerManager;
    WhatSonTrialActivationState m_state;
};

Q_DECLARE_METATYPE(WhatSonTrialActivationState)
