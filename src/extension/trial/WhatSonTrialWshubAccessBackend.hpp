#pragma once

#include "WhatSonTrialActivationPolicy.hpp"
#include "WhatSonTrialInstallStore.hpp"

#include <QDate>
#include <QString>

struct WhatSonTrialWshubAccessDecision final
{
    QString targetPath;
    QString normalizedTargetPath;
    QString denialReason;
    WhatSonTrialActivationState trialState;
    bool wshubTarget = false;
    bool allowed = true;
    bool restrictedByExpiredTrial = false;

    bool operator==(const WhatSonTrialWshubAccessDecision& other) const = default;
};

class WhatSonTrialWshubAccessBackend final
{
public:
    explicit WhatSonTrialWshubAccessBackend(WhatSonTrialInstallStore installStore = WhatSonTrialInstallStore());

    WhatSonTrialWshubAccessDecision evaluateAccess(
        const QString& targetPath,
        const QDate& today = QDate::currentDate()) const;
    bool canAccess(
        const QString& targetPath,
        QString* denialReason = nullptr,
        const QDate& today = QDate::currentDate()) const;

private:
    static QString normalizeTargetPath(const QString& targetPath);
    static bool isWshubTargetPath(const QString& normalizedTargetPath);

    WhatSonTrialInstallStore m_installStore;
};
