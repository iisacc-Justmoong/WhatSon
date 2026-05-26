#include "app/models/file/sync/WhatSonEditorRawPushController.hpp"

#include <QDir>

#include <utility>

namespace
{
    constexpr auto kIdleReason = "idle";
    constexpr auto kModifiedCountReason = "modified-count";
    constexpr auto kNoteDepartureReason = "note-departure";
}

WhatSonEditorRawPushController::WhatSonEditorRawPushController(QObject* parent)
    : QObject(parent)
{
    m_idleTimer.setSingleShot(true);
    m_idleTimer.setInterval(m_idleIntervalMs);
    connect(
        &m_idleTimer,
        &QTimer::timeout,
        this,
        &WhatSonEditorRawPushController::flushPendingPush);
}

int WhatSonEditorRawPushController::idleIntervalMs() const noexcept
{
    return m_idleIntervalMs;
}

void WhatSonEditorRawPushController::setIdleIntervalMs(const int idleIntervalMs)
{
    const int normalizedInterval = qMax(0, idleIntervalMs);
    if (m_idleIntervalMs == normalizedInterval)
    {
        return;
    }

    m_idleIntervalMs = normalizedInterval;
    m_idleTimer.setInterval(m_idleIntervalMs);
    emit idleIntervalMsChanged();
}

void WhatSonEditorRawPushController::setRawPushCallback(RawPushCallback callback)
{
    m_rawPushCallback = std::move(callback);
}

void WhatSonEditorRawPushController::requestIdlePush(
    const QString& editorFilePath,
    const QString& bodySourceText)
{
    const QString path = normalizedPath(editorFilePath);
    if (path.isEmpty())
    {
        return;
    }

    resetModifiedCountTrackingIfNeeded(path);
    if (m_hasPendingPush
        && m_pendingPush.reason == QString::fromLatin1(kModifiedCountReason))
    {
        return;
    }
    schedulePush({path, bodySourceText, QString::fromLatin1(kIdleReason), true});
}

void WhatSonEditorRawPushController::requestModifiedCountPush(
    const QString& editorFilePath,
    const int modifiedCount,
    const QString& bodySourceText)
{
    const QString path = normalizedPath(editorFilePath);
    if (path.isEmpty())
    {
        return;
    }

    resetModifiedCountTrackingIfNeeded(path);
    if (modifiedCount <= m_lastModifiedCount)
    {
        if (modifiedCount == m_lastModifiedCount
            && m_hasPendingPush
            && m_pendingPush.editorFilePath == path
            && m_pendingPush.reason == QString::fromLatin1(kModifiedCountReason))
        {
            m_pendingPush.bodySourceText = bodySourceText;
            m_pendingPush.hasBodySourceText = true;
        }
        return;
    }

    m_lastModifiedCount = modifiedCount;
    schedulePush({path, bodySourceText, QString::fromLatin1(kModifiedCountReason), true});
}

bool WhatSonEditorRawPushController::pushBeforeNoteDeparture(const QString& editorFilePath)
{
    const QString path = normalizedPath(editorFilePath);
    if (path.isEmpty())
    {
        return true;
    }

    m_idleTimer.stop();

    if (!m_hasPendingPush || m_pendingPush.editorFilePath != path)
    {
        return true;
    }
    PendingPush departurePush = m_pendingPush;
    departurePush.reason = QString::fromLatin1(kNoteDepartureReason);

    m_pendingPush = PendingPush();
    m_hasPendingPush = false;
    return executePush(departurePush);
}

bool WhatSonEditorRawPushController::flushPendingPush()
{
    if (!m_hasPendingPush)
    {
        return true;
    }

    m_idleTimer.stop();
    const PendingPush push = m_pendingPush;
    m_pendingPush = PendingPush();
    m_hasPendingPush = false;
    return executePush(push);
}

bool WhatSonEditorRawPushController::discardPendingPushForFile(const QString& editorFilePath)
{
    const QString path = normalizedPath(editorFilePath);
    if (path.isEmpty())
    {
        return false;
    }

    if (m_modifiedCountFilePath == path)
    {
        m_lastModifiedCount = -1;
    }

    if (!m_hasPendingPush || m_pendingPush.editorFilePath != path)
    {
        return false;
    }

    m_idleTimer.stop();
    m_pendingPush = PendingPush();
    m_hasPendingPush = false;
    return true;
}

bool WhatSonEditorRawPushController::hasPendingPushForFile(const QString& editorFilePath) const
{
    const QString path = normalizedPath(editorFilePath);
    return !path.isEmpty() && m_hasPendingPush && m_pendingPush.editorFilePath == path;
}

QString WhatSonEditorRawPushController::normalizedPath(const QString& path)
{
    const QString trimmedPath = path.trimmed();
    return trimmedPath.isEmpty() ? QString() : QDir::cleanPath(trimmedPath);
}

void WhatSonEditorRawPushController::schedulePush(PendingPush push)
{
    m_pendingPush = std::move(push);
    m_hasPendingPush = true;
    m_idleTimer.start(m_idleIntervalMs);
}

bool WhatSonEditorRawPushController::executePush(const PendingPush& push)
{
    if (push.editorFilePath.trimmed().isEmpty())
    {
        return true;
    }

    emit rawPushRequested(push.editorFilePath, push.reason);

    QString errorMessage;
    const bool success = m_rawPushCallback
        ? m_rawPushCallback(
              push.editorFilePath,
              push.bodySourceText,
              push.hasBodySourceText,
              push.reason,
              &errorMessage)
        : false;
    if (!success && errorMessage.trimmed().isEmpty())
    {
        errorMessage = QStringLiteral("Editor RAW push callback is not available.");
    }

    emit rawPushFinished(
        push.editorFilePath,
        push.reason,
        success,
        errorMessage);
    return success;
}

void WhatSonEditorRawPushController::resetModifiedCountTrackingIfNeeded(const QString& editorFilePath)
{
    if (m_modifiedCountFilePath == editorFilePath)
    {
        return;
    }

    m_modifiedCountFilePath = editorFilePath;
    m_lastModifiedCount = -1;
}
