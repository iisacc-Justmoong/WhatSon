#pragma once

#include "WhatSonTrialInstallStore.hpp"

#include <QDate>
#include <QObject>

struct WhatSonTrialActivationState final
{
    QDate installDate;
    QDate lastActiveDate;
    int trialLengthDays = 0;
    int elapsedDays = 0;
    int remainingDays = 0;
    bool active = false;

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

    WhatSonTrialActivationState currentState() const noexcept;
    WhatSonTrialActivationState refreshForDate(const QDate& today);

public slots:
    void refresh();

signals:
    void stateChanged();

private:
    static WhatSonTrialActivationState buildState(const QDate& installDate, const QDate& today);
    void setState(const WhatSonTrialActivationState& state);

    WhatSonTrialInstallStore m_installStore;
    WhatSonTrialActivationState m_state;
};

Q_DECLARE_METATYPE(WhatSonTrialActivationState)
