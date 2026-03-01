#include "WhatSonSystemIoGateway.hpp"

#include "WhatSonDebugTrace.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>

WhatSonSystemIoGateway::WhatSonSystemIoGateway()
{
    WhatSon::Debug::trace(
        QStringLiteral("io.system.gateway"),
        QStringLiteral("ctor"));
}

WhatSonSystemIoGateway::~WhatSonSystemIoGateway() = default;

bool WhatSonSystemIoGateway::ensureDirectory(const QString& directoryPath, QString* errorMessage) const
{
    const QString normalizedPath = normalizePath(directoryPath);
    if (normalizedPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("directoryPath must not be empty.");
        }
        return false;
    }

    QDir directory;
    if (directory.mkpath(normalizedPath))
    {
        return true;
    }

    if (errorMessage != nullptr)
    {
        *errorMessage = QStringLiteral("Failed to create directory: %1").arg(normalizedPath);
    }
    return false;
}

bool WhatSonSystemIoGateway::writeUtf8File(const QString& filePath, const QString& text, QString* errorMessage) const
{
    const QString normalizedPath = normalizePath(filePath);
    if (normalizedPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("filePath must not be empty.");
        }
        return false;
    }

    QString ensureError;
    const QString parentPath = QFileInfo(normalizedPath).absolutePath();
    if (!parentPath.isEmpty() && !ensureDirectory(parentPath, &ensureError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = ensureError;
        }
        return false;
    }

    QFile file(normalizedPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to open file for writing: %1").arg(normalizedPath);
        }
        return false;
    }

    const qint64 bytesWritten = file.write(text.toUtf8());
    if (bytesWritten < 0)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to write UTF-8 bytes: %1").arg(normalizedPath);
        }
        return false;
    }

    WhatSon::Debug::trace(
        QStringLiteral("io.system.gateway"),
        QStringLiteral("writeUtf8File"),
        QStringLiteral("path=%1 bytes=%2").arg(normalizedPath).arg(bytesWritten));
    return true;
}

bool WhatSonSystemIoGateway::appendUtf8File(const QString& filePath, const QString& text, QString* errorMessage) const
{
    const QString normalizedPath = normalizePath(filePath);
    if (normalizedPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("filePath must not be empty.");
        }
        return false;
    }

    QString ensureError;
    const QString parentPath = QFileInfo(normalizedPath).absolutePath();
    if (!parentPath.isEmpty() && !ensureDirectory(parentPath, &ensureError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = ensureError;
        }
        return false;
    }

    QFile file(normalizedPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to open file for appending: %1").arg(normalizedPath);
        }
        return false;
    }

    const qint64 bytesWritten = file.write(text.toUtf8());
    if (bytesWritten < 0)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to append UTF-8 bytes: %1").arg(normalizedPath);
        }
        return false;
    }

    WhatSon::Debug::trace(
        QStringLiteral("io.system.gateway"),
        QStringLiteral("appendUtf8File"),
        QStringLiteral("path=%1 bytes=%2").arg(normalizedPath).arg(bytesWritten));
    return true;
}

bool WhatSonSystemIoGateway::readUtf8File(const QString& filePath, QString* outText, QString* errorMessage) const
{
    if (outText == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("outText must not be null.");
        }
        return false;
    }

    const QString normalizedPath = normalizePath(filePath);
    if (normalizedPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("filePath must not be empty.");
        }
        return false;
    }

    QFile file(normalizedPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to open file for reading: %1").arg(normalizedPath);
        }
        return false;
    }

    *outText = QString::fromUtf8(file.readAll());
    return true;
}

bool WhatSonSystemIoGateway::removeFile(const QString& filePath, QString* errorMessage) const
{
    const QString normalizedPath = normalizePath(filePath);
    if (normalizedPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("filePath must not be empty.");
        }
        return false;
    }

    QFile file(normalizedPath);
    if (!file.exists())
    {
        return true;
    }

    if (file.remove())
    {
        return true;
    }

    if (errorMessage != nullptr)
    {
        *errorMessage = QStringLiteral("Failed to remove file: %1").arg(normalizedPath);
    }
    return false;
}

bool WhatSonSystemIoGateway::removeDirectoryRecursively(const QString& directoryPath, QString* errorMessage) const
{
    const QString normalizedPath = normalizePath(directoryPath);
    if (normalizedPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("directoryPath must not be empty.");
        }
        return false;
    }

    QDir directory(normalizedPath);
    if (!directory.exists())
    {
        return true;
    }

    if (directory.removeRecursively())
    {
        return true;
    }

    if (errorMessage != nullptr)
    {
        *errorMessage = QStringLiteral("Failed to remove directory recursively: %1").arg(normalizedPath);
    }
    return false;
}

QStringList WhatSonSystemIoGateway::listFileNames(const QString& directoryPath) const
{
    const QString normalizedPath = normalizePath(directoryPath);
    if (normalizedPath.isEmpty())
    {
        return {};
    }

    const QDir directory(normalizedPath);
    return directory.entryList(
        QStringList{},
        QDir::Files | QDir::NoDotAndDotDot,
        QDir::Name);
}

QStringList WhatSonSystemIoGateway::listDirectoryNames(const QString& directoryPath) const
{
    const QString normalizedPath = normalizePath(directoryPath);
    if (normalizedPath.isEmpty())
    {
        return {};
    }

    const QDir directory(normalizedPath);
    return directory.entryList(
        QStringList{},
        QDir::Dirs | QDir::NoDotAndDotDot,
        QDir::Name);
}

bool WhatSonSystemIoGateway::exists(const QString& path) const
{
    const QString normalizedPath = normalizePath(path);
    if (normalizedPath.isEmpty())
    {
        return false;
    }

    return QFileInfo::exists(normalizedPath);
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
