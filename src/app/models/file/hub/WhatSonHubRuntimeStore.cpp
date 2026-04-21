#include "WhatSonHubRuntimeStore.hpp"

#include "models/file/WhatSonDebugTrace.hpp"
#include "WhatSonHubPathUtils.hpp"
#include "WhatSonHubParser.hpp"

#include <QDir>
#include <QSet>
#include <utility>

namespace
{
    QString normalizePath(const QString& path)
    {
        return WhatSon::HubPath::normalizePath(path);
    }
} // namespace

WhatSonHubRuntimeStore::WhatSonHubRuntimeStore() = default;

WhatSonHubRuntimeStore::~WhatSonHubRuntimeStore() = default;

bool WhatSonHubRuntimeStore::loadFromWshub(
    const QString& wshubPath,
    QString* errorMessage)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.runtime"),
                              QStringLiteral("load.begin"),
                              QStringLiteral("path=%1").arg(wshubPath));

    WhatSonHubParser parser;
    WhatSonHubStore hubStore;
    if (!parser.parseFromWshub(wshubPath, &hubStore, errorMessage))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("hub.runtime"),
                                  QStringLiteral("load.failed.parser"),
                                  errorMessage != nullptr ? *errorMessage : QString());
        return false;
    }

    // All-or-nothing policy: stage updates in local copies and commit only on full success.
    QHash<QString, WhatSonHubStore> stagedHubStore = m_hubStore;
    WhatSonHubPlacementStore stagedPlacementStore = m_placementStore;
    WhatSonHubTagsStateStore stagedTagsStateStore = m_tagsStateStore;

    stagedHubStore.insert(hubStore.hubPath(), hubStore);

    if (!stagedPlacementStore.loadFromWshub(wshubPath, errorMessage))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("hub.runtime"),
                                  QStringLiteral("load.failed.placement"),
                                  errorMessage != nullptr ? *errorMessage : QString());
        return false;
    }
    if (!stagedTagsStateStore.loadFromWshub(wshubPath, errorMessage))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("hub.runtime"),
                                  QStringLiteral("load.failed.tags"),
                                  errorMessage != nullptr ? *errorMessage : QString());
        return false;
    }

    m_hubStore = std::move(stagedHubStore);
    m_placementStore = std::move(stagedPlacementStore);
    m_tagsStateStore = std::move(stagedTagsStateStore);

    const QString normalizedPath = normalizePath(wshubPath);
    const int tagCount = m_tagsStateStore.entries(normalizedPath).size();
    const WhatSonHubStore loadedHub = m_hubStore.value(normalizedPath);
    WhatSon::Debug::traceSelf(this,
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
    const QStringList hubStorePaths = m_hubStore.keys();
    const QStringList placementPaths = m_placementStore.hubPaths();
    const QStringList tagStatePaths = m_tagsStateStore.hubPaths();

    QSet<QString> merged(hubStorePaths.begin(), hubStorePaths.end());
    merged.unite(QSet<QString>(placementPaths.begin(), placementPaths.end()));
    merged.unite(QSet<QString>(tagStatePaths.begin(), tagStatePaths.end()));

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
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.runtime"),
                              QStringLiteral("setPlacement"),
                              QStringLiteral("path=%1").arg(placement.hubPath()));
    m_placementStore.setPlacement(std::move(placement));
}

void WhatSonHubRuntimeStore::setTagDepthEntries(
    const QString& wshubPath,
    QVector<WhatSonTagDepthEntry> entries)
{
    WhatSon::Debug::traceSelf(this,
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
    WhatSon::Debug::traceSelf(this,
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
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.runtime"),
                              QStringLiteral("setHubStat"),
                              QStringLiteral("path=%1").arg(normalized));
}

void WhatSonHubRuntimeStore::remove(const QString& wshubPath)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.runtime"),
                              QStringLiteral("remove"),
                              QStringLiteral("path=%1").arg(wshubPath));
    m_hubStore.remove(normalizePath(wshubPath));
    m_placementStore.remove(wshubPath);
    m_tagsStateStore.remove(wshubPath);
}

void WhatSonHubRuntimeStore::clear()
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.runtime"),
                              QStringLiteral("clear"));
    m_hubStore.clear();
    m_placementStore.clear();
    m_tagsStateStore.clear();
}
