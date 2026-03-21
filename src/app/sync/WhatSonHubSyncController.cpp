#include "WhatSonHubSyncController.hpp"

#include "file/hub/WhatSonHubPathUtils.hpp"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDirIterator>
#include <QEvent>
#include <QFileInfo>
#include <QStringList>

#include <algorithm>

namespace
{
    constexpr int kDefaultPeriodicIntervalMs = 5000;
    constexpr int kDefaultDebounceIntervalMs = 350;

    QString signatureRecordForInfo(const QString& relativePath, const QFileInfo& info)
    {
        return QStringLiteral("%1|%2|%3|%4")
            .arg(relativePath,
                 info.isDir() ? QStringLiteral("dir") : QStringLiteral("file"),
                 QString::number(info.size()),
                 QString::number(info.lastModified().toMSecsSinceEpoch()));
    }
}

WhatSonHubSyncController::WhatSonHubSyncController(QObject* parent)
    : QObject(parent)
{
    m_periodicTimer.setInterval(kDefaultPeriodicIntervalMs);
    QObject::connect(&m_periodicTimer, &QTimer::timeout, this, &WhatSonHubSyncController::onPeriodicTimeout);

    m_debounceTimer.setSingleShot(true);
    m_debounceTimer.setInterval(kDefaultDebounceIntervalMs);
    QObject::connect(&m_debounceTimer, &QTimer::timeout, this, &WhatSonHubSyncController::onDebounceTimeout);

    QObject::connect(
        &m_fileSystemWatcher,
        &QFileSystemWatcher::directoryChanged,
        this,
        &WhatSonHubSyncController::onWatchedPathChanged);
    QObject::connect(
        &m_fileSystemWatcher,
        &QFileSystemWatcher::fileChanged,
        this,
        &WhatSonHubSyncController::onWatchedPathChanged);
}

void WhatSonHubSyncController::attachToApplication(QCoreApplication* application)
{
    if (m_application == application)
    {
        return;
    }

    if (m_application != nullptr)
    {
        m_application->removeEventFilter(this);
    }

    m_application = application;
    if (m_application == nullptr)
    {
        return;
    }

    m_application->installEventFilter(this);
    QObject::connect(
        m_application,
        SIGNAL(applicationStateChanged(Qt::ApplicationState)),
        this,
        SLOT(handleApplicationStateChanged(Qt::ApplicationState)));
}

void WhatSonHubSyncController::setReloadCallback(std::function<bool(const QString&, QString*)> callback)
{
    m_reloadCallback = std::move(callback);
}

void WhatSonHubSyncController::setCurrentHubPath(const QString& hubPath)
{
    const QString normalizedHubPath = hubPath.trimmed().isEmpty()
                                          ? QString()
                                          : WhatSon::HubPath::normalizeAbsolutePath(hubPath);
    if (m_currentHubPath == normalizedHubPath)
    {
        refreshBaseline(true);
        return;
    }

    m_currentHubPath = normalizedHubPath;
    m_localMutationPending = false;
    if (m_currentHubPath.isEmpty())
    {
        m_periodicTimer.stop();
        m_lastKnownHubSignature.clear();
        clearWatcher();
        return;
    }

    refreshBaseline(true);
    if (!m_periodicTimer.isActive())
    {
        m_periodicTimer.start();
    }
}

QString WhatSonHubSyncController::currentHubPath() const
{
    return m_currentHubPath;
}

void WhatSonHubSyncController::setPeriodicIntervalMs(const int intervalMs)
{
    m_periodicTimer.setInterval(std::max(0, intervalMs));
}

void WhatSonHubSyncController::setDebounceIntervalMs(const int intervalMs)
{
    m_debounceTimer.setInterval(std::max(0, intervalMs));
}

void WhatSonHubSyncController::requestSyncHint()
{
    if (m_currentHubPath.isEmpty())
    {
        return;
    }

    scheduleSyncCheck();
}

void WhatSonHubSyncController::acknowledgeLocalMutation()
{
    if (m_currentHubPath.isEmpty())
    {
        return;
    }

    m_localMutationPending = true;
    scheduleSyncCheck();
}

bool WhatSonHubSyncController::eventFilter(QObject* watched, QEvent* event)
{
    Q_UNUSED(watched);
    if (event != nullptr && isInteractiveSyncEvent(event->type()))
    {
        requestSyncHint();
    }

    return QObject::eventFilter(watched, event);
}

void WhatSonHubSyncController::onWatchedPathChanged(const QString& path)
{
    Q_UNUSED(path);
    scheduleSyncCheck();
}

void WhatSonHubSyncController::handleApplicationStateChanged(const Qt::ApplicationState state)
{
    if (state == Qt::ApplicationActive)
    {
        requestSyncHint();
    }
}

void WhatSonHubSyncController::onPeriodicTimeout()
{
    requestSyncHint();
}

void WhatSonHubSyncController::onDebounceTimeout()
{
    if (m_reloadInProgress || m_currentHubPath.isEmpty())
    {
        return;
    }

    const QByteArray currentSignature = computeHubSignature(m_currentHubPath);
    if (currentSignature == m_lastKnownHubSignature)
    {
        m_localMutationPending = false;
        rebuildWatcher();
        return;
    }

    if (m_localMutationPending)
    {
        m_localMutationPending = false;
        m_lastKnownHubSignature = currentSignature;
        rebuildWatcher();
        return;
    }

    if (!m_reloadCallback)
    {
        m_lastKnownHubSignature = currentSignature;
        rebuildWatcher();
        return;
    }

    QString reloadError;
    m_reloadInProgress = true;
    const bool reloadSucceeded = m_reloadCallback(m_currentHubPath, &reloadError);
    m_reloadInProgress = false;

    if (!reloadSucceeded)
    {
        emit syncFailed(reloadError.trimmed().isEmpty()
                            ? QStringLiteral("Failed to synchronize the mounted WhatSon Hub.")
                            : reloadError.trimmed());
        return;
    }

    refreshBaseline(true);
    emit syncReloaded(m_currentHubPath);
}

QByteArray WhatSonHubSyncController::computeHubSignature(const QString& hubPath) const
{
    const QFileInfo rootInfo(hubPath);
    if (!rootInfo.exists() || !rootInfo.isDir())
    {
        return {};
    }

    QStringList signatureRecords;
    signatureRecords.reserve(64);
    signatureRecords.push_back(signatureRecordForInfo(QStringLiteral("."), rootInfo));

    QDirIterator iterator(
        hubPath,
        QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System,
        QDirIterator::Subdirectories);
    const QDir rootDirectory(hubPath);
    while (iterator.hasNext())
    {
        iterator.next();
        const QFileInfo info = iterator.fileInfo();
        if (!info.exists())
        {
            continue;
        }

        signatureRecords.push_back(signatureRecordForInfo(rootDirectory.relativeFilePath(info.absoluteFilePath()), info));
    }

    std::sort(signatureRecords.begin(), signatureRecords.end());
    QCryptographicHash hash(QCryptographicHash::Sha256);
    for (const QString& record : signatureRecords)
    {
        hash.addData(record.toUtf8());
        hash.addData(QByteArrayView("\n", 1));
    }
    return hash.result();
}

QStringList WhatSonHubSyncController::collectWatchPaths(const QString& hubPath) const
{
    QStringList paths;
    const QFileInfo rootInfo(hubPath);
    if (!rootInfo.exists() || !rootInfo.isDir())
    {
        return paths;
    }

    paths.push_back(rootInfo.absoluteFilePath());
    QDirIterator iterator(
        hubPath,
        QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System,
        QDirIterator::Subdirectories);
    while (iterator.hasNext())
    {
        iterator.next();
        const QFileInfo info = iterator.fileInfo();
        if (!info.exists())
        {
            continue;
        }
        paths.push_back(info.absoluteFilePath());
    }

    paths.removeDuplicates();
    return paths;
}

bool WhatSonHubSyncController::isInteractiveSyncEvent(const QEvent::Type type) const
{
    switch (type)
    {
    case QEvent::MouseButtonPress:
    case QEvent::TouchBegin:
    case QEvent::TabletPress:
        return true;
    default:
        return false;
    }
}

void WhatSonHubSyncController::scheduleSyncCheck()
{
    if (m_debounceTimer.interval() <= 0)
    {
        QTimer::singleShot(0, this, &WhatSonHubSyncController::onDebounceTimeout);
        return;
    }

    m_debounceTimer.start();
}

void WhatSonHubSyncController::refreshBaseline(const bool rebuildWatcherRequested)
{
    if (m_currentHubPath.isEmpty())
    {
        m_lastKnownHubSignature.clear();
        clearWatcher();
        return;
    }

    m_lastKnownHubSignature = computeHubSignature(m_currentHubPath);
    if (rebuildWatcherRequested)
    {
        rebuildWatcher();
    }
}

void WhatSonHubSyncController::rebuildWatcher()
{
    clearWatcher();

    const QStringList watchPaths = collectWatchPaths(m_currentHubPath);
    if (!watchPaths.isEmpty())
    {
        m_fileSystemWatcher.addPaths(watchPaths);
    }
}

void WhatSonHubSyncController::clearWatcher()
{
    const QStringList filePaths = m_fileSystemWatcher.files();
    if (!filePaths.isEmpty())
    {
        m_fileSystemWatcher.removePaths(filePaths);
    }

    const QStringList directoryPaths = m_fileSystemWatcher.directories();
    if (!directoryPaths.isEmpty())
    {
        m_fileSystemWatcher.removePaths(directoryPaths);
    }
}
