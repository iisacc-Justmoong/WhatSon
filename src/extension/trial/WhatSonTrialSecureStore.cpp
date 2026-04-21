#include "extension/trial/WhatSonTrialSecureStore.hpp"

#include <QByteArray>
#include <QProcess>
#include <QStandardPaths>

#if defined(Q_OS_MACOS)
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#endif

#include <memory>
#include <utility>

namespace
{
    QString trimTrailingNewline(QString value)
    {
        while (value.endsWith(QLatin1Char('\n')) || value.endsWith(QLatin1Char('\r')))
        {
            value.chop(1);
        }
        return value;
    }

#if defined(Q_OS_MACOS)
    class ScopedCfType final
    {
    public:
        explicit ScopedCfType(CFTypeRef value = nullptr)
            : m_value(value)
        {
        }

        ~ScopedCfType()
        {
            if (m_value != nullptr)
            {
                CFRelease(m_value);
            }
        }

        ScopedCfType(const ScopedCfType&) = delete;
        ScopedCfType& operator=(const ScopedCfType&) = delete;

        ScopedCfType(ScopedCfType&& other) noexcept
            : m_value(other.m_value)
        {
            other.m_value = nullptr;
        }

        ScopedCfType& operator=(ScopedCfType&& other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            if (m_value != nullptr)
            {
                CFRelease(m_value);
            }
            m_value = other.m_value;
            other.m_value = nullptr;
            return *this;
        }

        [[nodiscard]] CFTypeRef get() const
        {
            return m_value;
        }

        [[nodiscard]] CFMutableDictionaryRef mutableDictionary() const
        {
            return reinterpret_cast<CFMutableDictionaryRef>(const_cast<void*>(m_value));
        }

        [[nodiscard]] CFTypeRef release()
        {
            CFTypeRef value = m_value;
            m_value = nullptr;
            return value;
        }

    private:
        CFTypeRef m_value = nullptr;
    };

    ScopedCfType createCfString(const QString& value)
    {
        return ScopedCfType(
            CFStringCreateWithCString(kCFAllocatorDefault, value.toUtf8().constData(), kCFStringEncodingUTF8));
    }

    ScopedCfType createMacQuery(const QString& serviceName, const QString& entryKey)
    {
        ScopedCfType serviceRef = createCfString(serviceName);
        ScopedCfType entryRef = createCfString(entryKey);
        if (serviceRef.get() == nullptr || entryRef.get() == nullptr)
        {
            return ScopedCfType();
        }

        CFMutableDictionaryRef query = CFDictionaryCreateMutable(
            kCFAllocatorDefault,
            0,
            &kCFTypeDictionaryKeyCallBacks,
            &kCFTypeDictionaryValueCallBacks);
        if (query == nullptr)
        {
            return ScopedCfType();
        }

        CFDictionaryAddValue(query, kSecClass, kSecClassGenericPassword);
        CFDictionaryAddValue(query, kSecAttrService, serviceRef.get());
        CFDictionaryAddValue(query, kSecAttrAccount, entryRef.get());
        return ScopedCfType(query);
    }

    QString describeMacSecurityStatus(const OSStatus status)
    {
        ScopedCfType errorText(SecCopyErrorMessageString(status, nullptr));
        if (errorText.get() == nullptr)
        {
            return QStringLiteral("macOS security status %1").arg(status);
        }

        const auto cfString = static_cast<CFStringRef>(errorText.get());
        char buffer[512];
        if (CFStringGetCString(cfString, buffer, sizeof(buffer), kCFStringEncodingUTF8))
        {
            return QString::fromUtf8(buffer);
        }

        return QStringLiteral("macOS security status %1").arg(status);
    }

    class PlatformTrialSecureStoreBackend final : public WhatSonTrialSecureStoreBackend
    {
    public:
        WhatSonTrialSecureStoreReadResult readSecret(
            const QString& serviceName,
            const QString& entryKey) const override
        {
            ScopedCfType query = createMacQuery(serviceName, entryKey);
            if (query.get() == nullptr)
            {
                return {
                    {},
                    QStringLiteral("Failed to create macOS keychain query."),
                    WhatSonTrialSecureStoreStatus::Failed
                };
            }

            CFDictionaryAddValue(query.mutableDictionary(), kSecMatchLimit, kSecMatchLimitOne);
            CFDictionaryAddValue(query.mutableDictionary(), kSecReturnData, kCFBooleanTrue);

            CFTypeRef item = nullptr;
            const OSStatus status = SecItemCopyMatching(
                static_cast<CFDictionaryRef>(query.get()),
                &item);
            ScopedCfType itemRef(item);
            if (status == errSecItemNotFound)
            {
                return {
                    {},
                    {},
                    WhatSonTrialSecureStoreStatus::NotFound
                };
            }
            if (status != errSecSuccess)
            {
                return {
                    {},
                    describeMacSecurityStatus(status),
                    WhatSonTrialSecureStoreStatus::Failed
                };
            }

            const auto data = static_cast<CFDataRef>(itemRef.get());
            if (data == nullptr)
            {
                return {
                    {},
                    QStringLiteral("macOS keychain returned an empty data item."),
                    WhatSonTrialSecureStoreStatus::Failed
                };
            }

            const QByteArray bytes(
                reinterpret_cast<const char*>(CFDataGetBytePtr(data)),
                static_cast<qsizetype>(CFDataGetLength(data)));
            return {
                QString::fromUtf8(bytes),
                {},
                WhatSonTrialSecureStoreStatus::Success
            };
        }

        WhatSonTrialSecureStoreWriteResult writeSecret(
            const QString& serviceName,
            const QString& entryKey,
            const QString& value) const override
        {
            removeSecret(serviceName, entryKey);

            ScopedCfType query = createMacQuery(serviceName, entryKey);
            if (query.get() == nullptr)
            {
                return {
                    QStringLiteral("Failed to create macOS keychain item."),
                    WhatSonTrialSecureStoreStatus::Failed
                };
            }

            const QByteArray utf8Value = value.toUtf8();
            ScopedCfType valueRef(
                CFDataCreate(
                    kCFAllocatorDefault,
                    reinterpret_cast<const UInt8*>(utf8Value.constData()),
                    static_cast<CFIndex>(utf8Value.size())));
            if (valueRef.get() == nullptr)
            {
                return {
                    QStringLiteral("Failed to encode macOS keychain value."),
                    WhatSonTrialSecureStoreStatus::Failed
                };
            }

            CFDictionaryAddValue(query.mutableDictionary(), kSecValueData, valueRef.get());
            const OSStatus status = SecItemAdd(static_cast<CFDictionaryRef>(query.get()), nullptr);
            if (status != errSecSuccess)
            {
                return {
                    describeMacSecurityStatus(status),
                    WhatSonTrialSecureStoreStatus::Failed
                };
            }

            return {
                {},
                WhatSonTrialSecureStoreStatus::Success
            };
        }

        WhatSonTrialSecureStoreWriteResult removeSecret(
            const QString& serviceName,
            const QString& entryKey) const override
        {
            ScopedCfType query = createMacQuery(serviceName, entryKey);
            if (query.get() == nullptr)
            {
                return {
                    QStringLiteral("Failed to create macOS keychain delete query."),
                    WhatSonTrialSecureStoreStatus::Failed
                };
            }

            const OSStatus status = SecItemDelete(static_cast<CFDictionaryRef>(query.get()));
            if (status == errSecItemNotFound)
            {
                return {
                    {},
                    WhatSonTrialSecureStoreStatus::NotFound
                };
            }
            if (status != errSecSuccess)
            {
                return {
                    describeMacSecurityStatus(status),
                    WhatSonTrialSecureStoreStatus::Failed
                };
            }

            return {
                {},
                WhatSonTrialSecureStoreStatus::Success
            };
        }
    };
#elif defined(Q_OS_LINUX)
    struct ProcessResult final
    {
        int exitCode = -1;
        QProcess::ExitStatus exitStatus = QProcess::CrashExit;
        QString stdoutText;
        QString stderrText;
        bool started = false;
    };

    ProcessResult runProcess(
        const QString& program,
        const QStringList& arguments,
        const QByteArray& stdinBytes = {})
    {
        QProcess process;
        process.start(program, arguments, QIODevice::ReadWrite);
        ProcessResult result;
        result.started = process.waitForStarted();
        if (!result.started)
        {
            result.stderrText = process.errorString();
            return result;
        }

        if (!stdinBytes.isEmpty())
        {
            process.write(stdinBytes);
        }
        process.closeWriteChannel();
        process.waitForFinished(-1);

        result.exitCode = process.exitCode();
        result.exitStatus = process.exitStatus();
        result.stdoutText = QString::fromUtf8(process.readAllStandardOutput());
        result.stderrText = QString::fromUtf8(process.readAllStandardError());
        return result;
    }

    class PlatformTrialSecureStoreBackend final : public WhatSonTrialSecureStoreBackend
    {
    public:
        WhatSonTrialSecureStoreReadResult readSecret(
            const QString& serviceName,
            const QString& entryKey) const override
        {
            const QString executable = QStandardPaths::findExecutable(QStringLiteral("secret-tool"));
            if (executable.isEmpty())
            {
                return {
                    {},
                    QStringLiteral("secret-tool is not available on PATH."),
                    WhatSonTrialSecureStoreStatus::Unavailable
                };
            }

            const ProcessResult result = runProcess(
                executable,
                {
                    QStringLiteral("lookup"),
                    QStringLiteral("service"),
                    serviceName,
                    QStringLiteral("account"),
                    entryKey
                });
            if (!result.started)
            {
                return {
                    {},
                    result.stderrText.trimmed(),
                    WhatSonTrialSecureStoreStatus::Failed
                };
            }
            if (result.exitStatus != QProcess::NormalExit)
            {
                return {
                    {},
                    QStringLiteral("secret-tool terminated unexpectedly."),
                    WhatSonTrialSecureStoreStatus::Failed
                };
            }
            if (result.exitCode != 0)
            {
                return {
                    {},
                    trimTrailingNewline(result.stderrText).trimmed(),
                    WhatSonTrialSecureStoreStatus::NotFound
                };
            }

            return {
                trimTrailingNewline(result.stdoutText),
                {},
                WhatSonTrialSecureStoreStatus::Success
            };
        }

        WhatSonTrialSecureStoreWriteResult writeSecret(
            const QString& serviceName,
            const QString& entryKey,
            const QString& value) const override
        {
            const QString executable = QStandardPaths::findExecutable(QStringLiteral("secret-tool"));
            if (executable.isEmpty())
            {
                return {
                    QStringLiteral("secret-tool is not available on PATH."),
                    WhatSonTrialSecureStoreStatus::Unavailable
                };
            }

            const ProcessResult result = runProcess(
                executable,
                {
                    QStringLiteral("store"),
                    QStringLiteral("--label=WhatSon Trial"),
                    QStringLiteral("service"),
                    serviceName,
                    QStringLiteral("account"),
                    entryKey
                },
                value.toUtf8());
            if (!result.started)
            {
                return {
                    result.stderrText.trimmed(),
                    WhatSonTrialSecureStoreStatus::Failed
                };
            }
            if (result.exitStatus != QProcess::NormalExit || result.exitCode != 0)
            {
                return {
                    trimTrailingNewline(result.stderrText).trimmed(),
                    WhatSonTrialSecureStoreStatus::Failed
                };
            }

            return {
                {},
                WhatSonTrialSecureStoreStatus::Success
            };
        }

        WhatSonTrialSecureStoreWriteResult removeSecret(
            const QString& serviceName,
            const QString& entryKey) const override
        {
            const QString executable = QStandardPaths::findExecutable(QStringLiteral("secret-tool"));
            if (executable.isEmpty())
            {
                return {
                    QStringLiteral("secret-tool is not available on PATH."),
                    WhatSonTrialSecureStoreStatus::Unavailable
                };
            }

            const ProcessResult result = runProcess(
                executable,
                {
                    QStringLiteral("clear"),
                    QStringLiteral("service"),
                    serviceName,
                    QStringLiteral("account"),
                    entryKey
                });
            if (!result.started)
            {
                return {
                    result.stderrText.trimmed(),
                    WhatSonTrialSecureStoreStatus::Failed
                };
            }
            if (result.exitStatus != QProcess::NormalExit)
            {
                return {
                    QStringLiteral("secret-tool terminated unexpectedly."),
                    WhatSonTrialSecureStoreStatus::Failed
                };
            }
            if (result.exitCode != 0)
            {
                return {
                    trimTrailingNewline(result.stderrText).trimmed(),
                    WhatSonTrialSecureStoreStatus::NotFound
                };
            }

            return {
                {},
                WhatSonTrialSecureStoreStatus::Success
            };
        }
    };
#else
    class PlatformTrialSecureStoreBackend final : public WhatSonTrialSecureStoreBackend
    {
    public:
        WhatSonTrialSecureStoreReadResult readSecret(
            const QString&,
            const QString&) const override
        {
            return {
                {},
                QStringLiteral("No OS secure-store backend is available on this platform."),
                WhatSonTrialSecureStoreStatus::Unavailable
            };
        }

        WhatSonTrialSecureStoreWriteResult writeSecret(
            const QString&,
            const QString&,
            const QString&) const override
        {
            return {
                QStringLiteral("No OS secure-store backend is available on this platform."),
                WhatSonTrialSecureStoreStatus::Unavailable
            };
        }

        WhatSonTrialSecureStoreWriteResult removeSecret(
            const QString&,
            const QString&) const override
        {
            return {
                QStringLiteral("No OS secure-store backend is available on this platform."),
                WhatSonTrialSecureStoreStatus::Unavailable
            };
        }
    };
#endif
}

bool WhatSonTrialSecureStoreReadResult::found() const
{
    return status == WhatSonTrialSecureStoreStatus::Success;
}

bool WhatSonTrialSecureStoreReadResult::backendAvailable() const
{
    return status != WhatSonTrialSecureStoreStatus::Unavailable;
}

bool WhatSonTrialSecureStoreWriteResult::succeeded() const
{
    return status == WhatSonTrialSecureStoreStatus::Success || status == WhatSonTrialSecureStoreStatus::NotFound;
}

bool WhatSonTrialSecureStoreWriteResult::backendAvailable() const
{
    return status != WhatSonTrialSecureStoreStatus::Unavailable;
}

WhatSonTrialSecureStore::WhatSonTrialSecureStore(
    QString serviceName,
    std::shared_ptr<WhatSonTrialSecureStoreBackend> backend)
    : m_serviceName(std::move(serviceName))
    , m_backend(std::move(backend))
{
}

QString WhatSonTrialSecureStore::defaultServiceName()
{
    return QStringLiteral("com.iisacc.app.whatson.trial");
}

std::shared_ptr<WhatSonTrialSecureStoreBackend> WhatSonTrialSecureStore::defaultBackend()
{
    static const std::shared_ptr<WhatSonTrialSecureStoreBackend> backend =
        std::make_shared<PlatformTrialSecureStoreBackend>();
    return backend;
}

QString WhatSonTrialSecureStore::serviceName() const
{
    return m_serviceName;
}

WhatSonTrialSecureStoreReadResult WhatSonTrialSecureStore::readEntry(const QString& entryKey) const
{
    if (m_backend == nullptr || entryKey.trimmed().isEmpty())
    {
        return {
            {},
            QStringLiteral("Trial secure-store backend is not configured."),
            WhatSonTrialSecureStoreStatus::Unavailable
        };
    }

    return m_backend->readSecret(m_serviceName, entryKey.trimmed());
}

WhatSonTrialSecureStoreWriteResult WhatSonTrialSecureStore::writeEntry(
    const QString& entryKey,
    const QString& value) const
{
    if (m_backend == nullptr || entryKey.trimmed().isEmpty())
    {
        return {
            QStringLiteral("Trial secure-store backend is not configured."),
            WhatSonTrialSecureStoreStatus::Unavailable
        };
    }

    return m_backend->writeSecret(m_serviceName, entryKey.trimmed(), value);
}

WhatSonTrialSecureStoreWriteResult WhatSonTrialSecureStore::removeEntry(const QString& entryKey) const
{
    if (m_backend == nullptr || entryKey.trimmed().isEmpty())
    {
        return {
            QStringLiteral("Trial secure-store backend is not configured."),
            WhatSonTrialSecureStoreStatus::Unavailable
        };
    }

    return m_backend->removeSecret(m_serviceName, entryKey.trimmed());
}
