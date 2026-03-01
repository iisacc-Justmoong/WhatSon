#include "WhatSonHubCreator.hpp"

#include "WhatSonDebugTrace.hpp"

#include <QDir>
#include <QRegularExpression>
#include <QSaveFile>

#include <utility>

WhatSonHubCreator::WhatSonHubCreator(QString workspaceRootPath)
    : m_workspaceRootPath(std::move(workspaceRootPath))
{
}

WhatSonHubCreator::~WhatSonHubCreator() = default;

void WhatSonHubCreator::setWorkspaceRootPath(QString workspaceRootPath)
{
    WhatSon::Debug::trace(
        QStringLiteral("hub.creator.base"),
        QStringLiteral("setWorkspaceRootPath"),
        QStringLiteral("path=%1").arg(workspaceRootPath));
    m_workspaceRootPath = std::move(workspaceRootPath);
}

const QString& WhatSonHubCreator::workspaceRootPath() const noexcept
{
    return m_workspaceRootPath;
}

QString WhatSonHubCreator::sanitizeHubName(const QString& hubName) const
{
    QString normalized = hubName.trimmed().toLower();
    normalized.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral("-"));
    normalized.replace(QRegularExpression(QStringLiteral("[^a-z0-9._-]")), QStringLiteral(""));

    if (normalized.isEmpty())
    {
        WhatSon::Debug::trace(
            QStringLiteral("hub.creator.base"),
            QStringLiteral("sanitizeHubName.fallback"),
            QStringLiteral("input=%1").arg(hubName));
        return QStringLiteral("untitled-hub");
    }

    WhatSon::Debug::trace(
        QStringLiteral("hub.creator.base"),
        QStringLiteral("sanitizeHubName.success"),
        QStringLiteral("input=%1 output=%2").arg(hubName, normalized));
    return normalized;
}

QString WhatSonHubCreator::joinPath(const QString& left, const QString& right) const
{
    if (left.isEmpty())
    {
        return QDir::cleanPath(right);
    }
    if (right.isEmpty())
    {
        return QDir::cleanPath(left);
    }

    return QDir::cleanPath(left + QLatin1Char('/') + right);
}

bool WhatSonHubCreator::ensureDirectory(const QString& absolutePath, QString* errorMessage) const
{
    WhatSon::Debug::trace(
        QStringLiteral("hub.creator.base"),
        QStringLiteral("ensureDirectory.begin"),
        QStringLiteral("path=%1").arg(absolutePath));
    QDir dir;
    if (dir.exists(absolutePath))
    {
        return true;
    }

    if (dir.mkpath(absolutePath))
    {
        WhatSon::Debug::trace(
            QStringLiteral("hub.creator.base"),
            QStringLiteral("ensureDirectory.created"),
            QStringLiteral("path=%1").arg(absolutePath));
        return true;
    }

    if (errorMessage != nullptr)
    {
        *errorMessage = QStringLiteral("Failed to create directory: %1").arg(absolutePath);
    }
    return false;
}

bool WhatSonHubCreator::writeTextFile(
    const QString& absolutePath,
    const QString& content,
    QString* errorMessage) const
{
    WhatSon::Debug::trace(
        QStringLiteral("hub.creator.base"),
        QStringLiteral("writeTextFile.begin"),
        QStringLiteral("path=%1 bytes=%2").arg(absolutePath).arg(content.toUtf8().size()));
    QSaveFile file(absolutePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to open file for writing: %1").arg(absolutePath);
        }
        return false;
    }

    const QByteArray bytes = content.toUtf8();
    if (file.write(bytes) != bytes.size())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to write file: %1").arg(absolutePath);
        }
        return false;
    }

    if (!file.commit())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to commit file: %1").arg(absolutePath);
        }
        return false;
    }

    WhatSon::Debug::trace(
        QStringLiteral("hub.creator.base"),
        QStringLiteral("writeTextFile.success"),
        QStringLiteral("path=%1").arg(absolutePath));
    return true;
}
