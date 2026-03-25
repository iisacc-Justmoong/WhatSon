#pragma once

#include <QDate>
#include <QString>

class WhatSonTrialInstallStore final
{
public:
    explicit WhatSonTrialInstallStore(QString installDateSettingsKey = defaultInstallDateSettingsKey());

    static QString defaultInstallDateSettingsKey();

    QString installDateSettingsKey() const;
    QDate loadInstallDate() const;
    QDate ensureInstallDate(const QDate& today = QDate::currentDate()) const;
    void storeInstallDate(const QDate& installDate) const;
    void clear() const;

private:
    QString m_installDateSettingsKey;
};
