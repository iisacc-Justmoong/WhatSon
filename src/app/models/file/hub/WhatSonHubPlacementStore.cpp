#include "WhatSonHubPlacementStore.hpp"

#include "WhatSonDebugTrace.hpp"
#include "WhatSonHubPathUtils.hpp"

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
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.placement"),
                              QStringLiteral("load.begin"),
                              QStringLiteral("path=%1 normalized=%2").arg(wshubPath, normalized));
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
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("hub.placement"),
                                  QStringLiteral("load.failed.extractCoordinates"),
                                  errorMessage != nullptr ? *errorMessage : QString());
        return false;
    }

    m_store.insert(normalized, WhatSonHubPlacement(normalized, x, y));
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.placement"),
                              QStringLiteral("load.success"),
                              QStringLiteral("path=%1 x=%2 y=%3").arg(normalized).arg(x).arg(y));
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
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.placement"),
                              QStringLiteral("setPlacement"),
                              QStringLiteral("path=%1 x=%2 y=%3").arg(normalized).arg(placement.x()).arg(
                                  placement.y()));
    placement.setHubPath(normalized);
    m_store.insert(normalized, std::move(placement));
}

void WhatSonHubPlacementStore::remove(const QString& wshubPath)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.placement"),
                              QStringLiteral("remove"),
                              QStringLiteral("path=%1").arg(wshubPath));
    m_store.remove(normalizeHubPath(wshubPath));
}

void WhatSonHubPlacementStore::clear()
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.placement"),
                              QStringLiteral("clear"),
                              QStringLiteral("previousCount=%1").arg(m_store.size()));
    m_store.clear();
}

QString WhatSonHubPlacementStore::normalizeHubPath(const QString& hubPath)
{
    return WhatSon::HubPath::normalizePath(hubPath);
}

bool WhatSonHubPlacementStore::extractCoordinates(
    const QString& wshubPath,
    double* outX,
    double* outY,
    QString* errorMessage)
{
    WhatSon::Debug::trace(
        QStringLiteral("hub.placement"),
        QStringLiteral("extractCoordinates.begin"),
        QStringLiteral("path=%1").arg(wshubPath));
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

    const QString manifestPath = WhatSon::HubPath::joinPath(wshubPath, QStringLiteral(".whatson/hub.json"));

    QFile manifestFile(manifestPath);
    if (!manifestFile.exists())
    {
        WhatSon::Debug::trace(
            QStringLiteral("hub.placement"),
            QStringLiteral("extractCoordinates.manifestMissing"),
            QStringLiteral("path=%1").arg(manifestPath));
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
        WhatSon::Debug::trace(
            QStringLiteral("hub.placement"),
            QStringLiteral("extractCoordinates.success.coordinate"),
            QStringLiteral("x=%1 y=%2").arg(*outX).arg(*outY));
        return true;
    }

    *outX = manifestObject.value(QStringLiteral("x")).toDouble(0.0);
    *outY = manifestObject.value(QStringLiteral("y")).toDouble(0.0);
    WhatSon::Debug::trace(
        QStringLiteral("hub.placement"),
        QStringLiteral("extractCoordinates.success.root"),
        QStringLiteral("x=%1 y=%2").arg(*outX).arg(*outY));
    return true;
}
