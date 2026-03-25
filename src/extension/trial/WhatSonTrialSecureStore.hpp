#pragma once

#include <QString>

#include <memory>

enum class WhatSonTrialSecureStoreStatus
{
    Success,
    NotFound,
    Unavailable,
    Failed
};

struct WhatSonTrialSecureStoreReadResult final
{
    QString value;
    QString errorMessage;
    WhatSonTrialSecureStoreStatus status = WhatSonTrialSecureStoreStatus::Unavailable;

    [[nodiscard]] bool found() const;
    [[nodiscard]] bool backendAvailable() const;
};

struct WhatSonTrialSecureStoreWriteResult final
{
    QString errorMessage;
    WhatSonTrialSecureStoreStatus status = WhatSonTrialSecureStoreStatus::Unavailable;

    [[nodiscard]] bool succeeded() const;
    [[nodiscard]] bool backendAvailable() const;
};

class WhatSonTrialSecureStoreBackend
{
public:
    virtual ~WhatSonTrialSecureStoreBackend() = default;

    [[nodiscard]] virtual WhatSonTrialSecureStoreReadResult readSecret(
        const QString& serviceName,
        const QString& entryKey) const = 0;
    [[nodiscard]] virtual WhatSonTrialSecureStoreWriteResult writeSecret(
        const QString& serviceName,
        const QString& entryKey,
        const QString& value) const = 0;
    [[nodiscard]] virtual WhatSonTrialSecureStoreWriteResult removeSecret(
        const QString& serviceName,
        const QString& entryKey) const = 0;
};

class WhatSonTrialSecureStore final
{
public:
    explicit WhatSonTrialSecureStore(
        QString serviceName = defaultServiceName(),
        std::shared_ptr<WhatSonTrialSecureStoreBackend> backend = defaultBackend());

    static QString defaultServiceName();
    static std::shared_ptr<WhatSonTrialSecureStoreBackend> defaultBackend();

    [[nodiscard]] QString serviceName() const;

    [[nodiscard]] WhatSonTrialSecureStoreReadResult readEntry(const QString& entryKey) const;
    [[nodiscard]] WhatSonTrialSecureStoreWriteResult writeEntry(const QString& entryKey, const QString& value) const;
    [[nodiscard]] WhatSonTrialSecureStoreWriteResult removeEntry(const QString& entryKey) const;

private:
    QString m_serviceName;
    std::shared_ptr<WhatSonTrialSecureStoreBackend> m_backend;
};
