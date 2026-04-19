#pragma once

#include <QObject>
#include <QString>
#include <QUrl>

#include <memory>

class WhatSonIosHubPickerBridgePrivate;

class WhatSonIosHubPickerBridge : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit WhatSonIosHubPickerBridge(QObject* parent = nullptr);
    ~WhatSonIosHubPickerBridge() override;

    [[nodiscard]] bool busy() const noexcept;
    [[nodiscard]] QString lastError() const;

    Q_INVOKABLE bool open(const QUrl& initialDirectoryUrl = QUrl());
    Q_INVOKABLE void clearLastError();

signals:
    void accepted(const QUrl& selectedUrl);
    void canceled();
    void busyChanged();
    void lastErrorChanged();

private slots:
    void handlePickerAccepted(const QUrl& selectedUrl);
    void handlePickerCanceled();
    void handlePickerFailed(const QString& errorMessage);

private:
    void setBusy(bool busy);
    void setLastError(const QString& errorMessage);

    std::unique_ptr<WhatSonIosHubPickerBridgePrivate> d;
    bool m_busy = false;
    QString m_lastError;
};
