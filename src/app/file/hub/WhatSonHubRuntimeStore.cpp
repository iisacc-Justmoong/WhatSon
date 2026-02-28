#include "WhatSonHubRuntimeStore.hpp"

#include <QSet>
#include <utility>

WhatSonHubRuntimeStore::WhatSonHubRuntimeStore() = default;

WhatSonHubRuntimeStore::~WhatSonHubRuntimeStore() = default;

bool WhatSonHubRuntimeStore::loadFromWshub(
    const QString& wshubPath,
    QString* errorMessage)
{
    if (!m_placementStore.loadFromWshub(wshubPath, errorMessage))
    {
        return false;
    }
    if (!m_tagsStateStore.loadFromWshub(wshubPath, errorMessage))
    {
        return false;
    }
    return true;
}

bool WhatSonHubRuntimeStore::contains(const QString& wshubPath) const
{
    return m_placementStore.contains(wshubPath) || m_tagsStateStore.contains(wshubPath);
}

QStringList WhatSonHubRuntimeStore::hubPaths() const
{
    QSet<QString> merged = QSet<QString>(m_placementStore.hubPaths().begin(), m_placementStore.hubPaths().end());
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

void WhatSonHubRuntimeStore::setPlacement(WhatSonHubPlacement placement)
{
    m_placementStore.setPlacement(std::move(placement));
}

void WhatSonHubRuntimeStore::setTagDepthEntries(
    const QString& wshubPath,
    QVector<WhatSonTagDepthEntry> entries)
{
    m_tagsStateStore.setEntries(wshubPath, std::move(entries));
}

void WhatSonHubRuntimeStore::remove(const QString& wshubPath)
{
    m_placementStore.remove(wshubPath);
    m_tagsStateStore.remove(wshubPath);
}

void WhatSonHubRuntimeStore::clear()
{
    m_placementStore.clear();
    m_tagsStateStore.clear();
}
