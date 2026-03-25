#pragma once

#include <QObject>
#include <QString>

class WhatSonRegisterManager final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool authenticated READ authenticated WRITE setAuthenticated NOTIFY authenticatedChanged FINAL)

public:
    explicit WhatSonRegisterManager(
        QString authenticatedSettingsKey = defaultAuthenticatedSettingsKey(),
        QObject* parent = nullptr);

    static QString defaultAuthenticatedSettingsKey();

    QString authenticatedSettingsKey() const;
    bool authenticated() const noexcept;

    Q_INVOKABLE void reload();

public slots:
    void setAuthenticated(bool authenticated);
    void clearAuthentication();

signals:
    void authenticatedChanged();

private:
    void setAuthenticatedState(bool authenticated, bool emitSignal);

    QString m_authenticatedSettingsKey;
    bool m_authenticated = false;
};
