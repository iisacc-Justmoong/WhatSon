#include "WhatSonProgressHierarchyStore.hpp"

#include "WhatSonDebugTrace.hpp"
#include "WhatSonProgressHierarchyCreator.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <algorithm>
#include <utility>

namespace
{
    QStringList defaultStates()
    {
        return {
            QStringLiteral("Ready"),
            QStringLiteral("Pending"),
            QStringLiteral("InProgress"),
            QStringLiteral("Done")
        };
    }

    QStringList sanitizeStates(QStringList states)
    {
        QStringList sanitized;
        sanitized.reserve(states.size());

        for (QString& state : states)
        {
            state = state.trimmed();
            if (!state.isEmpty())
            {
                sanitized.push_back(state);
            }
        }

        if (sanitized.isEmpty())
        {
            sanitized = defaultStates();
        }

        return sanitized;
    }
} // namespace

WhatSonProgressHierarchyStore::WhatSonProgressHierarchyStore()
    : m_progressStates(defaultStates())
{
}

WhatSonProgressHierarchyStore::~WhatSonProgressHierarchyStore() = default;

void WhatSonProgressHierarchyStore::clear()
{
    m_hubPath.clear();
    m_progressValue = 0;
    m_progressStates = defaultStates();
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.progress.store"),
        QStringLiteral("clear"));
}

QString WhatSonProgressHierarchyStore::hubPath() const
{
    return m_hubPath;
}

void WhatSonProgressHierarchyStore::setHubPath(QString hubPath)
{
    m_hubPath = hubPath.trimmed();
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.progress.store"),
        QStringLiteral("setHubPath"),
        QStringLiteral("value=%1").arg(m_hubPath));
}

int WhatSonProgressHierarchyStore::progressValue() const noexcept
{
    return m_progressValue;
}

void WhatSonProgressHierarchyStore::setProgressValue(int progressValue) noexcept
{
    m_progressValue = std::max(progressValue, 0);
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.progress.store"),
        QStringLiteral("setProgressValue"),
        QStringLiteral("value=%1").arg(m_progressValue));
}

QStringList WhatSonProgressHierarchyStore::progressStates() const
{
    return m_progressStates;
}

void WhatSonProgressHierarchyStore::setProgressStates(QStringList progressStates)
{
    const int rawCount = progressStates.size();
    m_progressStates = sanitizeStates(std::move(progressStates));
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.progress.store"),
        QStringLiteral("setProgressStates"),
        QStringLiteral("rawCount=%1 sanitizedCount=%2 values=[%3]")
        .arg(rawCount)
        .arg(m_progressStates.size())
        .arg(m_progressStates.join(QStringLiteral(", "))));
}

bool WhatSonProgressHierarchyStore::writeToFile(const QString& filePath, QString* errorMessage) const
{
    const QString normalizedPath = filePath.trimmed();
    if (normalizedPath.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Progress.wsprogress path is empty.");
        }
        return false;
    }

    WhatSonProgressHierarchyCreator creator;
    const QString text = creator.createText(*this);

    const QFileInfo info(normalizedPath);
    if (!QDir().mkpath(info.absolutePath()))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to create hierarchy directory: %1").arg(info.absolutePath());
        }
        return false;
    }

    QFile file(normalizedPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to open hierarchy file for write: %1").arg(normalizedPath);
        }
        return false;
    }

    file.write(text.toUtf8());
    file.close();
    WhatSon::Debug::trace(
        QStringLiteral("hierarchy.progress.store"),
        QStringLiteral("writeToFile"),
        QStringLiteral("path=%1 bytes=%2").arg(normalizedPath).arg(text.toUtf8().size()));
    return true;
}
