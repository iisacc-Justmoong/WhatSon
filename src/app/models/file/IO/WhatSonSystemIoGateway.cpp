#include "WhatSonSystemIoGateway.hpp"

#include "models/file/WhatSonDebugTrace.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QSaveFile>

namespace
{
    constexpr int kMaxEnsuredDirectoryCacheSize = 4096;
    QRecursiveMutex g_systemIoMutationMutex;

    QString previewEntries(const QStringList& values, const int maxCount = 8)
    {
        if (values.isEmpty())
        {
            return QStringLiteral("[]");
        }

        const int boundedCount = maxCount < 0 ? 0 : maxCount;
        if (values.size() <= boundedCount)
        {
            return QStringLiteral("[%1]").arg(values.join(QStringLiteral(", ")));
        }

        const QStringList head = values.mid(0, boundedCount);
        return QStringLiteral("[%1, ...] total=%2")
               .arg(head.join(QStringLiteral(", ")))
               .arg(values.size());
    }
} // namespace

WhatSonSystemIoGateway::WhatSonSystemIoGateway()
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.system.gateway"),
                              QStringLiteral("ctor"));
}

WhatSonSystemIoGateway::~WhatSonSystemIoGateway() = default;

bool WhatSonSystemIoGateway::ensureDirectory(const QString& directoryPath, QString* errorMessage) const
{
    const QString normalizedPath = normalizePath(directoryPath);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.system.gateway"),
                              QStringLiteral("ensureDirectory.begin"),
                              QStringLiteral("path=%1").arg(normalizedPath));
    if (normalizedPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("directoryPath must not be empty.");
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("ensureDirectory.failed"),
                                  QStringLiteral("reason=empty path"));
        return false;
    }

    QMutexLocker<QRecursiveMutex> mutationLock(&g_systemIoMutationMutex);

    if (m_ensuredDirectories.contains(normalizedPath))
    {
        return true;
    }

    const QDir existingDirectory(normalizedPath);
    if (existingDirectory.exists())
    {
        cacheEnsuredDirectory(normalizedPath);
        return true;
    }

    QDir directory;
    if (directory.mkpath(normalizedPath))
    {
        cacheEnsuredDirectory(normalizedPath);
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("ensureDirectory.success"),
                                  QStringLiteral("path=%1").arg(normalizedPath));
        return true;
    }

    if (errorMessage != nullptr)
    {
        *errorMessage = QStringLiteral("Failed to create directory: %1").arg(normalizedPath);
    }
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.system.gateway"),
                              QStringLiteral("ensureDirectory.failed"),
                              QStringLiteral("path=%1 reason=mkpath failed").arg(normalizedPath));
    return false;
}

bool WhatSonSystemIoGateway::writeUtf8File(const QString& filePath, const QString& text, QString* errorMessage) const
{
    const QString normalizedPath = normalizePath(filePath);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.system.gateway"),
                              QStringLiteral("writeUtf8File.begin"),
                              QStringLiteral("path=%1 bytes=%2").arg(normalizedPath).arg(text.toUtf8().size()));
    if (normalizedPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("filePath must not be empty.");
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("writeUtf8File.failed"),
                                  QStringLiteral("reason=empty path"));
        return false;
    }

    QMutexLocker<QRecursiveMutex> mutationLock(&g_systemIoMutationMutex);

    QString ensureError;
    const QString parentPath = QFileInfo(normalizedPath).absolutePath();
    if (!parentPath.isEmpty() && !ensureDirectory(parentPath, &ensureError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = ensureError;
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("writeUtf8File.failed"),
                                  QStringLiteral("path=%1 reason=%2").arg(normalizedPath, ensureError));
        return false;
    }

    QSaveFile file(normalizedPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to open file for writing: %1 (%2)")
                .arg(normalizedPath, file.errorString());
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("writeUtf8File.failed"),
                                  QStringLiteral("path=%1 reason=open failed error=%2")
                                  .arg(normalizedPath, file.errorString()));
        return false;
    }

    const QByteArray utf8Bytes = text.toUtf8();
    const qint64 bytesWritten = file.write(utf8Bytes);
    if (bytesWritten != utf8Bytes.size())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to write UTF-8 bytes: %1 (%2)")
                .arg(normalizedPath, file.errorString());
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("writeUtf8File.failed"),
                                  QStringLiteral("path=%1 reason=write failed error=%2")
                                  .arg(normalizedPath, file.errorString()));
        return false;
    }

    if (!file.commit())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to commit UTF-8 file: %1 (%2)")
                .arg(normalizedPath, file.errorString());
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("writeUtf8File.failed"),
                                  QStringLiteral("path=%1 reason=commit failed error=%2")
                                  .arg(normalizedPath, file.errorString()));
        return false;
    }

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.system.gateway"),
                              QStringLiteral("writeUtf8File"),
                              QStringLiteral("path=%1 bytes=%2").arg(normalizedPath).arg(bytesWritten));
    return true;
}

bool WhatSonSystemIoGateway::appendUtf8File(const QString& filePath, const QString& text, QString* errorMessage) const
{
    const QString normalizedPath = normalizePath(filePath);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.system.gateway"),
                              QStringLiteral("appendUtf8File.begin"),
                              QStringLiteral("path=%1 bytes=%2").arg(normalizedPath).arg(text.toUtf8().size()));
    if (normalizedPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("filePath must not be empty.");
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("appendUtf8File.failed"),
                                  QStringLiteral("reason=empty path"));
        return false;
    }

    QMutexLocker<QRecursiveMutex> mutationLock(&g_systemIoMutationMutex);

    QString ensureError;
    const QString parentPath = QFileInfo(normalizedPath).absolutePath();
    if (!parentPath.isEmpty() && !ensureDirectory(parentPath, &ensureError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = ensureError;
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("appendUtf8File.failed"),
                                  QStringLiteral("path=%1 reason=%2").arg(normalizedPath, ensureError));
        return false;
    }

    QFile file(normalizedPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to open file for appending: %1 (%2)")
                .arg(normalizedPath, file.errorString());
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("appendUtf8File.failed"),
                                  QStringLiteral("path=%1 reason=open failed error=%2")
                                  .arg(normalizedPath, file.errorString()));
        return false;
    }

    const qint64 bytesWritten = file.write(text.toUtf8());
    if (bytesWritten < 0)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to append UTF-8 bytes: %1 (%2)")
                .arg(normalizedPath, file.errorString());
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("appendUtf8File.failed"),
                                  QStringLiteral("path=%1 reason=append failed error=%2")
                                  .arg(normalizedPath, file.errorString()));
        return false;
    }

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.system.gateway"),
                              QStringLiteral("appendUtf8File"),
                              QStringLiteral("path=%1 bytes=%2").arg(normalizedPath).arg(bytesWritten));
    return true;
}

bool WhatSonSystemIoGateway::readUtf8File(const QString& filePath, QString* outText, QString* errorMessage) const
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.system.gateway"),
                              QStringLiteral("readUtf8File.begin"),
                              QStringLiteral("path=%1").arg(filePath));
    if (outText == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("outText must not be null.");
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("readUtf8File.failed"),
                                  QStringLiteral("path=%1 reason=outText is null").arg(filePath));
        return false;
    }

    const QString normalizedPath = normalizePath(filePath);
    if (normalizedPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("filePath must not be empty.");
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("readUtf8File.failed"),
                                  QStringLiteral("reason=empty path"));
        return false;
    }

    QFile file(normalizedPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to open file for reading: %1 (%2)")
                .arg(normalizedPath, file.errorString());
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("readUtf8File.failed"),
                                  QStringLiteral("path=%1 reason=open failed error=%2")
                                  .arg(normalizedPath, file.errorString()));
        return false;
    }

    const QByteArray bytes = file.readAll();
    *outText = QString::fromUtf8(bytes);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.system.gateway"),
                              QStringLiteral("readUtf8File.success"),
                              QStringLiteral("path=%1 bytes=%2").arg(normalizedPath).arg(bytes.size()));
    return true;
}

bool WhatSonSystemIoGateway::removeFile(const QString& filePath, QString* errorMessage) const
{
    const QString normalizedPath = normalizePath(filePath);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.system.gateway"),
                              QStringLiteral("removeFile.begin"),
                              QStringLiteral("path=%1").arg(normalizedPath));
    if (normalizedPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("filePath must not be empty.");
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("removeFile.failed"),
                                  QStringLiteral("reason=empty path"));
        return false;
    }

    QMutexLocker<QRecursiveMutex> mutationLock(&g_systemIoMutationMutex);

    QFile file(normalizedPath);
    if (!file.exists())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("removeFile.skipped"),
                                  QStringLiteral("path=%1 reason=not exists").arg(normalizedPath));
        return true;
    }

    if (file.remove())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("removeFile.success"),
                                  QStringLiteral("path=%1").arg(normalizedPath));
        return true;
    }

    if (errorMessage != nullptr)
    {
        *errorMessage = QStringLiteral("Failed to remove file: %1 (%2)")
            .arg(normalizedPath, file.errorString());
    }
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.system.gateway"),
                              QStringLiteral("removeFile.failed"),
                              QStringLiteral("path=%1 reason=remove failed error=%2")
                              .arg(normalizedPath, file.errorString()));
    return false;
}

bool WhatSonSystemIoGateway::removeDirectoryRecursively(const QString& directoryPath, QString* errorMessage) const
{
    const QString normalizedPath = normalizePath(directoryPath);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.system.gateway"),
                              QStringLiteral("removeDirectoryRecursively.begin"),
                              QStringLiteral("path=%1").arg(normalizedPath));
    if (normalizedPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("directoryPath must not be empty.");
        }
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("removeDirectoryRecursively.failed"),
                                  QStringLiteral("reason=empty path"));
        return false;
    }

    QMutexLocker<QRecursiveMutex> mutationLock(&g_systemIoMutationMutex);

    QDir directory(normalizedPath);
    if (!directory.exists())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("removeDirectoryRecursively.skipped"),
                                  QStringLiteral("path=%1 reason=not exists").arg(normalizedPath));
        return true;
    }

    if (directory.removeRecursively())
    {
        invalidateDirectoryCachePrefix(normalizedPath);
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("removeDirectoryRecursively.success"),
                                  QStringLiteral("path=%1").arg(normalizedPath));
        return true;
    }

    if (errorMessage != nullptr)
    {
        *errorMessage = QStringLiteral("Failed to remove directory recursively: %1").arg(normalizedPath);
    }
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.system.gateway"),
                              QStringLiteral("removeDirectoryRecursively.failed"),
                              QStringLiteral("path=%1 reason=removeRecursively failed").arg(normalizedPath));
    return false;
}

QStringList WhatSonSystemIoGateway::listFileNames(const QString& directoryPath) const
{
    const QString normalizedPath = normalizePath(directoryPath);
    if (normalizedPath.isEmpty())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("listFileNames.skipped"),
                                  QStringLiteral("reason=empty path"));
        return {};
    }

    const QDir directory(normalizedPath);
    const QStringList values = directory.entryList(
        QStringList{},
        QDir::Files | QDir::NoDotAndDotDot,
        QDir::Name);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.system.gateway"),
                              QStringLiteral("listFileNames.success"),
                              QStringLiteral("path=%1 count=%2 values=%3")
                              .arg(normalizedPath)
                              .arg(values.size())
                              .arg(previewEntries(values)));
    return values;
}

QStringList WhatSonSystemIoGateway::listDirectoryNames(const QString& directoryPath) const
{
    const QString normalizedPath = normalizePath(directoryPath);
    if (normalizedPath.isEmpty())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("listDirectoryNames.skipped"),
                                  QStringLiteral("reason=empty path"));
        return {};
    }

    const QDir directory(normalizedPath);
    const QStringList values = directory.entryList(
        QStringList{},
        QDir::Dirs | QDir::NoDotAndDotDot,
        QDir::Name);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.system.gateway"),
                              QStringLiteral("listDirectoryNames.success"),
                              QStringLiteral("path=%1 count=%2 values=%3")
                              .arg(normalizedPath)
                              .arg(values.size())
                              .arg(previewEntries(values)));
    return values;
}

bool WhatSonSystemIoGateway::exists(const QString& path) const
{
    const QString normalizedPath = normalizePath(path);
    if (normalizedPath.isEmpty())
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("io.system.gateway"),
                                  QStringLiteral("exists.skipped"),
                                  QStringLiteral("reason=empty path"));
        return false;
    }

    const bool pathExists = QFileInfo::exists(normalizedPath);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("io.system.gateway"),
                              QStringLiteral("exists.result"),
                              QStringLiteral("path=%1 exists=%2").arg(normalizedPath).arg(
                                  pathExists ? QStringLiteral("true") : QStringLiteral("false")));
    return pathExists;
}

QString WhatSonSystemIoGateway::normalizePath(const QString& path) const
{
    const QString trimmed = path.trimmed();
    if (trimmed.isEmpty())
    {
        return {};
    }

    return QDir::cleanPath(trimmed);
}

void WhatSonSystemIoGateway::cacheEnsuredDirectory(const QString& normalizedDirectoryPath) const
{
    if (normalizedDirectoryPath.isEmpty())
    {
        return;
    }

    if (m_ensuredDirectories.size() >= kMaxEnsuredDirectoryCacheSize
        && !m_ensuredDirectories.contains(normalizedDirectoryPath))
    {
        m_ensuredDirectories.clear();
    }

    m_ensuredDirectories.insert(normalizedDirectoryPath);
}

void WhatSonSystemIoGateway::invalidateDirectoryCachePrefix(const QString& normalizedDirectoryPath) const
{
    if (normalizedDirectoryPath.isEmpty() || m_ensuredDirectories.isEmpty())
    {
        return;
    }

    const QString prefix = normalizedDirectoryPath + QLatin1Char('/');
    for (auto it = m_ensuredDirectories.begin(); it != m_ensuredDirectories.end();)
    {
        if (*it == normalizedDirectoryPath || it->startsWith(prefix))
        {
            it = m_ensuredDirectories.erase(it);
            continue;
        }
        ++it;
    }
}
