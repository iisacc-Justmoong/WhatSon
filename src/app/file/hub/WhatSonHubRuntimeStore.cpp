#include "WhatSonHubRuntimeStore.hpp"

#include "WhatSonDebugTrace.hpp"

#include <QSet>
#include <utility>

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
    WhatSon::Debug::trace(
        QStringLiteral("hub.runtime"),
        QStringLiteral("load.success"),
        QStringLiteral("tagCount=%1").arg(tagCount));
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

void WhatSonHubRuntimeStore::remove(const QString& wshubPath)
{
    WhatSon::Debug::trace(
        QStringLiteral("hub.runtime"),
        QStringLiteral("remove"),
        QStringLiteral("path=%1").arg(wshubPath));
    m_placementStore.remove(wshubPath);
    m_tagsStateStore.remove(wshubPath);
}

void WhatSonHubRuntimeStore::clear()
{
    WhatSon::Debug::trace(
        QStringLiteral("hub.runtime"),
        QStringLiteral("clear"));
    m_placementStore.clear();
    m_tagsStateStore.clear();
}
