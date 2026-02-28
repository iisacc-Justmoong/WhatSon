#include "WhatSonHubPlacementStore.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

#include <utility>

WhatSonHubPlacementStore::WhatSonHubPlacementStore() = default;

WhatSonHubPlacementStore::~WhatSonHubPlacementStore() = default;

bool WhatSonHubPlacementStore::loadFromWshub(
    const QString& wshubPath,
    QString* errorMessage)
{
    const QString normalized = normalizeHubPath(wshubPath);
    if (normalized.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("wshubPath must not be empty.");
        }
        return false;
    }

    QFileInfo hubInfo(normalized);
    if (!hubInfo.exists())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("wshubPath does not exist: %1").arg(normalized);
        }
        return false;
    }

    if (!hubInfo.fileName().endsWith(QStringLiteral(".wshub")))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Path is not a .wshub package: %1").arg(normalized);
        }
        return false;
    }

    double x = 0.0;
    double y = 0.0;
    if (!extractCoordinates(normalized, &x, &y, errorMessage))
    {
        return false;
    }

    m_store.insert(normalized, WhatSonHubPlacement(normalized, x, y));
    return true;
}

bool WhatSonHubPlacementStore::contains(const QString& wshubPath) const
{
    return m_store.contains(normalizeHubPath(wshubPath));
}

WhatSonHubPlacement WhatSonHubPlacementStore::placement(const QString& wshubPath) const
{
    const QString normalized = normalizeHubPath(wshubPath);
    if (!m_store.contains(normalized))
    {
        return {};
    }
    return m_store.value(normalized);
}

QStringList WhatSonHubPlacementStore::hubPaths() const
{
    return m_store.keys();
}

void WhatSonHubPlacementStore::setPlacement(WhatSonHubPlacement placement)
{
    const QString normalized = normalizeHubPath(placement.hubPath());
    placement.setHubPath(normalized);
    m_store.insert(normalized, std::move(placement));
}

void WhatSonHubPlacementStore::remove(const QString& wshubPath)
{
    m_store.remove(normalizeHubPath(wshubPath));
}

void WhatSonHubPlacementStore::clear()
{
    m_store.clear();
}

QString WhatSonHubPlacementStore::normalizeHubPath(const QString& hubPath)
{
    const QString trimmed = hubPath.trimmed();
    if (trimmed.isEmpty())
    {
        return {};
    }
    return QDir::cleanPath(trimmed);
}

bool WhatSonHubPlacementStore::extractCoordinates(
    const QString& wshubPath,
    double* outX,
    double* outY,
    QString* errorMessage)
{
    if (outX == nullptr || outY == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Output coordinate pointers must not be null.");
        }
        return false;
    }

    *outX = 0.0;
    *outY = 0.0;

    const QString manifestPath =
        QDir(wshubPath).filePath(QStringLiteral(".whatson/hub.json"));

    QFile manifestFile(manifestPath);
    if (!manifestFile.exists())
    {
        return true;
    }

    if (!manifestFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to open manifest: %1").arg(manifestPath);
        }
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document =
        QJsonDocument::fromJson(manifestFile.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage =
                QStringLiteral("Invalid hub manifest JSON: %1 (%2)")
                .arg(manifestPath, parseError.errorString());
        }
        return false;
    }

    const QJsonObject manifestObject = document.object();
    if (manifestObject.contains(QStringLiteral("coordinate"))
        && manifestObject.value(QStringLiteral("coordinate")).isObject())
    {
        const QJsonObject coordinateObject =
            manifestObject.value(QStringLiteral("coordinate")).toObject();
        *outX = coordinateObject.value(QStringLiteral("x")).toDouble(0.0);
        *outY = coordinateObject.value(QStringLiteral("y")).toDouble(0.0);
        return true;
    }

    *outX = manifestObject.value(QStringLiteral("x")).toDouble(0.0);
    *outY = manifestObject.value(QStringLiteral("y")).toDouble(0.0);
    return true;
}
