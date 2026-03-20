#pragma once

#include "WhatSonHubPathUtils.hpp"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QMutexLocker>
#include <QSaveFile>
#include <QSysInfo>
#include <QUuid>

namespace WhatSon::HubWriteLease
{
    namespace Detail
    {
        struct LeaseRecord final
        {
            QString ownerId;
            QString ownerName;
            QString hostName;
            qint64 pid = 0;
            QString acquiredAtUtc;
            QString refreshedAtUtc;
        };

        inline QMutex& leaseMutex()
        {
            static QMutex mutex;
            return mutex;
        }

        inline QString sessionOwnerId()
        {
            static const QString ownerId = QUuid::createUuid().toString(QUuid::WithoutBraces);
            return ownerId;
        }

        inline QString sessionOwnerName()
        {
            const QString appName = QCoreApplication::applicationName().trimmed().isEmpty()
                                        ? QStringLiteral("WhatSon")
                                        : QCoreApplication::applicationName().trimmed();
            const QString hostName = QSysInfo::machineHostName().trimmed().isEmpty()
                                         ? QStringLiteral("unknown-host")
                                         : QSysInfo::machineHostName().trimmed();
            return QStringLiteral("%1@%2(pid=%3)")
                .arg(appName)
                .arg(hostName)
                .arg(QCoreApplication::applicationPid());
        }

        inline QString currentTimestampUtc()
        {
            return QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        }

        inline QDateTime parseUtc(const QString& value)
        {
            const QDateTime parsed = QDateTime::fromString(value.trimmed(), Qt::ISODate);
            return parsed.isValid() ? parsed.toUTC() : QDateTime();
        }

        inline int heartbeatIntervalMs()
        {
            return 15000;
        }

        inline int timeoutMs()
        {
            return heartbeatIntervalMs() * 3;
        }

        inline QString resolveExistingHubRootPath(const QString& path)
        {
            const QString normalizedPath = WhatSon::HubPath::normalizeAbsolutePath(path);
            if (normalizedPath.isEmpty() || WhatSon::HubPath::isNonLocalUrl(normalizedPath))
            {
                return {};
            }

            QString candidatePath = QFileInfo(normalizedPath).isDir()
                                        ? normalizedPath
                                        : QFileInfo(normalizedPath).absolutePath();
            while (!candidatePath.isEmpty())
            {
                const QFileInfo candidateInfo(candidatePath);
                if (candidateInfo.exists()
                    && candidateInfo.isDir()
                    && candidateInfo.fileName().endsWith(QStringLiteral(".wshub"), Qt::CaseInsensitive))
                {
                    return QDir::cleanPath(candidateInfo.absoluteFilePath());
                }

                const QString parentPath = candidateInfo.absolutePath();
                if (parentPath.isEmpty() || parentPath == candidatePath)
                {
                    break;
                }
                candidatePath = parentPath;
            }

            return {};
        }

        inline QString leaseFilePath(const QString& hubPath)
        {
            const QString normalizedHubPath = resolveExistingHubRootPath(hubPath);
            if (normalizedHubPath.isEmpty())
            {
                return {};
            }

            return QDir(normalizedHubPath).filePath(QStringLiteral(".whatson/write-lease.json"));
        }

        inline bool readLeaseRecord(const QString& hubPath, LeaseRecord* outRecord, QString* errorMessage)
        {
            if (outRecord != nullptr)
            {
                *outRecord = LeaseRecord{};
            }

            const QString path = leaseFilePath(hubPath);
            if (path.isEmpty())
            {
                return true;
            }

            QFile file(path);
            if (!file.exists())
            {
                return true;
            }

            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = QStringLiteral("Failed to open write lease file: %1 (%2)")
                        .arg(path, file.errorString());
                }
                return false;
            }

            QJsonParseError parseError;
            const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
            if (parseError.error != QJsonParseError::NoError || !document.isObject())
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = QStringLiteral("Invalid write lease file JSON: %1 (%2)")
                        .arg(path, parseError.errorString());
                }
                return false;
            }

            if (outRecord == nullptr)
            {
                return true;
            }

            const QJsonObject root = document.object();
            outRecord->ownerId = root.value(QStringLiteral("ownerId")).toString().trimmed();
            outRecord->ownerName = root.value(QStringLiteral("ownerName")).toString().trimmed();
            outRecord->hostName = root.value(QStringLiteral("hostName")).toString().trimmed();
            outRecord->pid = root.value(QStringLiteral("pid")).toVariant().toLongLong();
            outRecord->acquiredAtUtc = root.value(QStringLiteral("acquiredAtUtc")).toString().trimmed();
            outRecord->refreshedAtUtc = root.value(QStringLiteral("refreshedAtUtc")).toString().trimmed();
            return true;
        }

        inline bool leaseIsOwnedByCurrentSession(const LeaseRecord& leaseRecord)
        {
            return !leaseRecord.ownerId.isEmpty() && leaseRecord.ownerId == sessionOwnerId();
        }

        inline bool leaseIsActive(const LeaseRecord& leaseRecord, const QDateTime& nowUtc)
        {
            const QDateTime refreshedAtUtc = parseUtc(leaseRecord.refreshedAtUtc);
            if (!refreshedAtUtc.isValid())
            {
                return false;
            }

            return refreshedAtUtc.msecsTo(nowUtc) <= timeoutMs();
        }

        inline bool writeLeaseRecord(
            const QString& hubPath,
            const LeaseRecord& previousLeaseRecord,
            QString* errorMessage)
        {
            const QString normalizedHubPath = resolveExistingHubRootPath(hubPath);
            if (normalizedHubPath.isEmpty())
            {
                return true;
            }

            const QString leasePath = leaseFilePath(normalizedHubPath);
            if (leasePath.isEmpty())
            {
                return true;
            }

            const QString leaseDirectoryPath = QFileInfo(leasePath).absolutePath();
            if (!leaseDirectoryPath.isEmpty() && !QDir().mkpath(leaseDirectoryPath))
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = QStringLiteral("Failed to create write lease directory: %1").arg(leaseDirectoryPath);
                }
                return false;
            }

            const QString nowUtc = currentTimestampUtc();
            const QString acquiredAtUtc = leaseIsOwnedByCurrentSession(previousLeaseRecord)
                                              && !previousLeaseRecord.acquiredAtUtc.trimmed().isEmpty()
                                              ? previousLeaseRecord.acquiredAtUtc.trimmed()
                                              : nowUtc;

            QJsonObject root;
            root.insert(QStringLiteral("version"), 1);
            root.insert(QStringLiteral("schema"), QStringLiteral("whatson.hub.writeLease"));
            root.insert(QStringLiteral("hubPath"), normalizedHubPath);
            root.insert(QStringLiteral("ownerId"), sessionOwnerId());
            root.insert(QStringLiteral("ownerName"), sessionOwnerName());
            root.insert(QStringLiteral("hostName"), QSysInfo::machineHostName().trimmed());
            root.insert(QStringLiteral("pid"), static_cast<qint64>(QCoreApplication::applicationPid()));
            root.insert(QStringLiteral("acquiredAtUtc"), acquiredAtUtc);
            root.insert(QStringLiteral("refreshedAtUtc"), nowUtc);

            QSaveFile file(leasePath);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = QStringLiteral("Failed to open write lease file: %1 (%2)")
                        .arg(leasePath, file.errorString());
                }
                return false;
            }

            if (file.write(QJsonDocument(root).toJson(QJsonDocument::Indented)) < 0)
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = QStringLiteral("Failed to write write lease file: %1 (%2)")
                        .arg(leasePath, file.errorString());
                }
                file.cancelWriting();
                return false;
            }

            if (!file.commit())
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = QStringLiteral("Failed to commit write lease file: %1 (%2)")
                        .arg(leasePath, file.errorString());
                }
                return false;
            }

            return true;
        }

        inline QString activeWriterError(const LeaseRecord& leaseRecord)
        {
            const QString ownerName = leaseRecord.ownerName.trimmed().isEmpty()
                                          ? QStringLiteral("another WhatSon session")
                                          : leaseRecord.ownerName.trimmed();
            const QString refreshedAtUtc = leaseRecord.refreshedAtUtc.trimmed();
            if (refreshedAtUtc.isEmpty())
            {
                return QStringLiteral("The selected WhatSon Hub is currently locked for writing by %1.")
                    .arg(ownerName);
            }

            return QStringLiteral(
                       "The selected WhatSon Hub is currently locked for writing by %1. Last heartbeat: %2 UTC.")
                .arg(ownerName, refreshedAtUtc);
        }
    } // namespace Detail

    inline QString currentSessionOwnerId()
    {
        return Detail::sessionOwnerId();
    }

    inline int heartbeatIntervalMs()
    {
        return Detail::heartbeatIntervalMs();
    }

    inline bool refreshWriteLeaseForHub(const QString& hubPath, QString* errorMessage = nullptr)
    {
        const QString normalizedHubPath = Detail::resolveExistingHubRootPath(hubPath);
        if (normalizedHubPath.isEmpty())
        {
            return true;
        }

        QMutexLocker locker(&Detail::leaseMutex());
        Detail::LeaseRecord leaseRecord;
        QString readError;
        if (!Detail::readLeaseRecord(normalizedHubPath, &leaseRecord, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            return false;
        }

        const QDateTime nowUtc = QDateTime::currentDateTimeUtc();
        if (!leaseRecord.ownerId.isEmpty()
            && !Detail::leaseIsOwnedByCurrentSession(leaseRecord)
            && Detail::leaseIsActive(leaseRecord, nowUtc))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = Detail::activeWriterError(leaseRecord);
            }
            return false;
        }

        return Detail::writeLeaseRecord(normalizedHubPath, leaseRecord, errorMessage);
    }

    inline bool ensureWriteLeaseForPath(const QString& path, QString* errorMessage = nullptr)
    {
        return refreshWriteLeaseForHub(path, errorMessage);
    }

    inline bool releaseWriteLeaseForHub(const QString& hubPath, QString* errorMessage = nullptr)
    {
        const QString normalizedHubPath = Detail::resolveExistingHubRootPath(hubPath);
        if (normalizedHubPath.isEmpty())
        {
            return true;
        }

        QMutexLocker locker(&Detail::leaseMutex());
        Detail::LeaseRecord leaseRecord;
        QString readError;
        if (!Detail::readLeaseRecord(normalizedHubPath, &leaseRecord, &readError))
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = readError;
            }
            return false;
        }

        if (!Detail::leaseIsOwnedByCurrentSession(leaseRecord))
        {
            return true;
        }

        const QString leasePath = Detail::leaseFilePath(normalizedHubPath);
        if (leasePath.isEmpty())
        {
            return true;
        }

        QFile file(leasePath);
        if (!file.exists())
        {
            return true;
        }

        if (!file.remove())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Failed to remove write lease file: %1 (%2)")
                    .arg(leasePath, file.errorString());
            }
            return false;
        }

        return true;
    }
} // namespace WhatSon::HubWriteLease
