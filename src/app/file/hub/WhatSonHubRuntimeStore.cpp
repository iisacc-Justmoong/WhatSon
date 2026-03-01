#include "WhatSonHubRuntimeStore.hpp"

#include "WhatSonDebugTrace.hpp"
#include "WhatSonHubParser.hpp"

#include <QDir>
#include <QSet>
#include <utility>

namespace
{
    QString normalizePath(const QString& path)
    {
        const QString trimmed = path.trimmed();
        if (trimmed.isEmpty())
        {
            return {};
        }
        return QDir::cleanPath(trimmed);
    }
} // namespace

WhatSonHubRuntimeStore::WhatSonHubRuntimeStore() = default;

WhatSonHubRuntimeStore::~WhatSonHubRuntimeStore() = default;

bool WhatSonHubRuntimeStore::loadFromWshub(
    const QString& wshubPath,
    QString* errorMessage)
{
    WhatSon::Debug::trace(
        QStringLiteral("hub.runtime"),
        QStringLiteral("load.begin"),
        QStringLiteral("path=%1").arg(wshubPath));

    WhatSonHubParser parser;
    WhatSonHubStore hubStore;
    if (!parser.parseFromWshub(wshubPath, &hubStore, errorMessage))
    {
        WhatSon::Debug::trace(
            QStringLiteral("hub.runtime"),
            QStringLiteral("load.failed.parser"),
            errorMessage != nullptr ? *errorMessage : QString());
        return false;
    }
    m_hubStore.insert(hubStore.hubPath(), hubStore);

    if (!m_placementStore.loadFromWshub(wshubPath, errorMessage))
    {
        WhatSon::Debug::trace(
            QStringLiteral("hub.runtime"),
            QStringLiteral("load.failed.placement"),
            errorMessage != nullptr ? *errorMessage : QString());
        return false;
    }
    if (!m_tagsStateStore.loadFromWshub(wshubPath, errorMessage))
    {
        WhatSon::Debug::trace(
            QStringLiteral("hub.runtime"),
            QStringLiteral("load.failed.tags"),
            errorMessage != nullptr ? *errorMessage : QString());
        return false;
    }

    const int tagCount = m_tagsStateStore.entries(wshubPath).size();
    const WhatSonHubStore loadedHub = m_hubStore.value(normalizePath(wshubPath));
    WhatSon::Debug::trace(
        QStringLiteral("hub.runtime"),
        QStringLiteral("load.success"),
        QStringLiteral("tagCount=%1 noteCount=%2 resourceCount=%3")
        .arg(tagCount)
        .arg(loadedHub.stat().noteCount())
        .arg(loadedHub.stat().resourceCount()));
    return true;
}

bool WhatSonHubRuntimeStore::contains(const QString& wshubPath) const
{
    const QString normalized = normalizePath(wshubPath);
    return m_hubStore.contains(normalized)
        || m_placementStore.contains(normalized)
        || m_tagsStateStore.contains(normalized);
}

QStringList WhatSonHubRuntimeStore::hubPaths() const
{
    QSet<QString> merged = QSet<QString>(m_hubStore.keys().begin(), m_hubStore.keys().end());
    merged.unite(QSet<QString>(m_placementStore.hubPaths().begin(), m_placementStore.hubPaths().end()));
    merged.unite(QSet<QString>(m_tagsStateStore.hubPaths().begin(), m_tagsStateStore.hubPaths().end()));

    QStringList paths = merged.values();
    paths.sort();
    return paths;
}

WhatSonHubPlacement WhatSonHubRuntimeStore::placement(const QString& wshubPath) const
{
    return m_placementStore.placement(wshubPath);
}

QVector<WhatSonTagDepthEntry> WhatSonHubRuntimeStore::tagDepthEntries(const QString& wshubPath) const
{
    return m_tagsStateStore.entries(wshubPath);
}

WhatSonHubStore WhatSonHubRuntimeStore::hub(const QString& wshubPath) const
{
    return m_hubStore.value(normalizePath(wshubPath));
}

WhatSonHubStat WhatSonHubRuntimeStore::hubStat(const QString& wshubPath) const
{
    return hub(wshubPath).stat();
}

void WhatSonHubRuntimeStore::setPlacement(WhatSonHubPlacement placement)
{
    WhatSon::Debug::trace(
        QStringLiteral("hub.runtime"),
        QStringLiteral("setPlacement"),
        QStringLiteral("path=%1").arg(placement.hubPath()));
    m_placementStore.setPlacement(std::move(placement));
}

void WhatSonHubRuntimeStore::setTagDepthEntries(
    const QString& wshubPath,
    QVector<WhatSonTagDepthEntry> entries)
{
    WhatSon::Debug::trace(
        QStringLiteral("hub.runtime"),
        QStringLiteral("setTagDepthEntries"),
        QStringLiteral("path=%1 count=%2").arg(wshubPath).arg(entries.size()));
    m_tagsStateStore.setEntries(wshubPath, std::move(entries));
}

void WhatSonHubRuntimeStore::setHub(WhatSonHubStore store)
{
    const QString normalized = normalizePath(store.hubPath());
    if (normalized.isEmpty())
    {
        return;
    }

    store.setHubPath(normalized);
    WhatSon::Debug::trace(
        QStringLiteral("hub.runtime"),
        QStringLiteral("setHub"),
        QStringLiteral("path=%1").arg(normalized));
    m_hubStore.insert(normalized, std::move(store));
}

void WhatSonHubRuntimeStore::setHubStat(const QString& wshubPath, WhatSonHubStat stat)
{
    const QString normalized = normalizePath(wshubPath);
    if (normalized.isEmpty())
    {
        return;
    }

    WhatSonHubStore store = m_hubStore.value(normalized);
    store.setHubPath(normalized);
    store.setStat(std::move(stat));
    m_hubStore.insert(normalized, std::move(store));
    WhatSon::Debug::trace(
        QStringLiteral("hub.runtime"),
        QStringLiteral("setHubStat"),
        QStringLiteral("path=%1").arg(normalized));
}

void WhatSonHubRuntimeStore::remove(const QString& wshubPath)
{
    WhatSon::Debug::trace(
        QStringLiteral("hub.runtime"),
        QStringLiteral("remove"),
        QStringLiteral("path=%1").arg(wshubPath));
    m_hubStore.remove(normalizePath(wshubPath));
    m_placementStore.remove(wshubPath);
    m_tagsStateStore.remove(wshubPath);
}

void WhatSonHubRuntimeStore::clear()
{
    WhatSon::Debug::trace(
        QStringLiteral("hub.runtime"),
        QStringLiteral("clear"));
    m_hubStore.clear();
    m_placementStore.clear();
    m_tagsStateStore.clear();
}
