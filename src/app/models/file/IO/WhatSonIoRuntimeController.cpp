#include "app/models/file/IO/WhatSonIoRuntimeController.hpp"

#include "app/models/file/WhatSonDebugTrace.hpp"

#include <QDateTime>

#include <utility>

namespace
{
    const QString kEnsureDirAction = QStringLiteral("io.ensuredir");
    const QString kWriteUtf8Action = QStringLiteral("io.writeutf8");
    const QString kAppendUtf8Action = QStringLiteral("io.appendutf8");
    const QString kReadUtf8Action = QStringLiteral("io.readutf8");
    const QString kRemoveFileAction = QStringLiteral("io.removefile");

    QString requiredPath(const QVariantMap& payload, QString* errorMessage)
    {
        const QString path = payload.value(QStringLiteral("path")).toString().trimmed();
        if (path.isEmpty() && errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("payload.path must not be empty.");
        }
        return path;
    }
} // namespace

WhatSonIoRuntimeController::WhatSonIoRuntimeController()
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.runtime.controller"),
                              QStringLiteral("ctor"));
}

WhatSonIoRuntimeController::~WhatSonIoRuntimeController() = default;

WhatSonIoEventListener* WhatSonIoRuntimeController::eventListener() noexcept
{
    return &m_eventListener;
}

const WhatSonIoEventListener* WhatSonIoRuntimeController::eventListener() const noexcept
{
    return &m_eventListener;
}

WhatSonSystemIoGateway* WhatSonIoRuntimeController::systemIo() noexcept
{
    return &m_systemIo;
}

const WhatSonSystemIoGateway* WhatSonIoRuntimeController::systemIo() const noexcept
{
    return &m_systemIo;
}

void WhatSonIoRuntimeController::enqueueLvrsEvent(QString eventName, QVariantMap payload)
{
    m_eventListener.pushLvrsEvent(std::move(eventName), std::move(payload));
}

bool WhatSonIoRuntimeController::processNext(QString* errorMessage)
{
    if (!m_eventListener.hasPendingEvents())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("No pending IO events.");
        }
        return false;
    }

    const WhatSonIoEvent next = m_eventListener.takeNextEvent();
    return processEvent(next, errorMessage);
}

int WhatSonIoRuntimeController::processAll(QString* errorMessage)
{
    int processedCount = 0;
    while (m_eventListener.hasPendingEvents())
    {
        const WhatSonIoEvent next = m_eventListener.takeNextEvent();
        QString processError;
        if (!processEvent(next, &processError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = processError;
            }
            return processedCount;
        }
        ++processedCount;
    }
    return processedCount;
}

QVariantMap WhatSonIoRuntimeController::lastResult() const
{
    return m_lastResult;
}

void WhatSonIoRuntimeController::clearLastResult()
{
    m_lastResult.clear();
}

bool WhatSonIoRuntimeController::processEvent(const WhatSonIoEvent& event, QString* errorMessage)
{
    const QString actionRaw = event.name.trimmed();
    const QString action = actionRaw.toCaseFolded();
    const QVariantMap& payload = event.payload;

    if (action == kEnsureDirAction)
    {
        QString commandError;
        const QString path = requiredPath(payload, &commandError);
        if (path.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = commandError;
            }
            m_lastResult = buildResult(false, actionRaw, commandError);
            return false;
        }

        if (!m_systemIo.ensureDirectory(path, &commandError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = commandError;
            }
            m_lastResult = buildResult(false, actionRaw, commandError);
            return false;
        }

        m_lastResult = buildResult(true, actionRaw, QStringLiteral("Directory ensured."), QVariantMap{
                                       {QStringLiteral("path"), path}
                                   });
        return true;
    }

    if (action == kWriteUtf8Action)
    {
        QString commandError;
        const QString path = requiredPath(payload, &commandError);
        if (path.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = commandError;
            }
            m_lastResult = buildResult(false, actionRaw, commandError);
            return false;
        }

        const QString text = payload.value(QStringLiteral("text")).toString();
        if (!m_systemIo.writeUtf8File(path, text, &commandError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = commandError;
            }
            m_lastResult = buildResult(false, actionRaw, commandError);
            return false;
        }

        m_lastResult = buildResult(true, actionRaw, QStringLiteral("UTF-8 file written."), QVariantMap{
                                       {QStringLiteral("path"), path},
                                       {QStringLiteral("bytes"), text.toUtf8().size()}
                                   });
        return true;
    }

    if (action == kAppendUtf8Action)
    {
        QString commandError;
        const QString path = requiredPath(payload, &commandError);
        if (path.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = commandError;
            }
            m_lastResult = buildResult(false, actionRaw, commandError);
            return false;
        }

        const QString text = payload.value(QStringLiteral("text")).toString();
        if (!m_systemIo.appendUtf8File(path, text, &commandError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = commandError;
            }
            m_lastResult = buildResult(false, actionRaw, commandError);
            return false;
        }

        m_lastResult = buildResult(true, actionRaw, QStringLiteral("UTF-8 file appended."), QVariantMap{
                                       {QStringLiteral("path"), path},
                                       {QStringLiteral("bytes"), text.toUtf8().size()}
                                   });
        return true;
    }

    if (action == kReadUtf8Action)
    {
        QString commandError;
        const QString path = requiredPath(payload, &commandError);
        if (path.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = commandError;
            }
            m_lastResult = buildResult(false, actionRaw, commandError);
            return false;
        }

        QString text;
        if (!m_systemIo.readUtf8File(path, &text, &commandError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = commandError;
            }
            m_lastResult = buildResult(false, actionRaw, commandError);
            return false;
        }

        m_lastResult = buildResult(true, actionRaw, QStringLiteral("UTF-8 file read."), QVariantMap{
                                       {QStringLiteral("path"), path},
                                       {QStringLiteral("text"), text},
                                       {QStringLiteral("bytes"), text.toUtf8().size()}
                                   });
        return true;
    }

    if (action == kRemoveFileAction)
    {
        QString commandError;
        const QString path = requiredPath(payload, &commandError);
        if (path.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = commandError;
            }
            m_lastResult = buildResult(false, actionRaw, commandError);
            return false;
        }

        if (!m_systemIo.removeFile(path, &commandError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = commandError;
            }
            m_lastResult = buildResult(false, actionRaw, commandError);
            return false;
        }

        m_lastResult = buildResult(true, actionRaw, QStringLiteral("File removed."), QVariantMap{
                                       {QStringLiteral("path"), path}
                                   });
        return true;
    }

    const QString unsupportedMessage = QStringLiteral("Unsupported IO action: %1").arg(actionRaw);
    if (errorMessage != nullptr)
    {
        *errorMessage = unsupportedMessage;
    }
    m_lastResult = buildResult(false, actionRaw, unsupportedMessage);
    return false;
}

QVariantMap WhatSonIoRuntimeController::buildResult(
    bool ok,
    const QString& action,
    const QString& message,
    const QVariantMap& extra) const
{
    QVariantMap result{
        {QStringLiteral("ok"), ok},
        {QStringLiteral("action"), action},
        {QStringLiteral("message"), message},
        {QStringLiteral("timestamp"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate)}
    };

    for (auto it = extra.constBegin(); it != extra.constEnd(); ++it)
    {
        result.insert(it.key(), it.value());
    }

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.runtime.controller"),
                              QStringLiteral("buildResult"),
                              QStringLiteral("ok=%1 action=%2").arg(
                                  ok ? QStringLiteral("true") : QStringLiteral("false"), action));
    return result;
}
