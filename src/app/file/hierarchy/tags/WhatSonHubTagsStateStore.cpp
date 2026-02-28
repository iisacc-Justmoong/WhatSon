#include "WhatSonHubTagsStateStore.hpp"

#include <QDir>

WhatSonHubTagsStateStore::WhatSonHubTagsStateStore() = default;

WhatSonHubTagsStateStore::~WhatSonHubTagsStateStore() = default;

bool WhatSonHubTagsStateStore::loadFromWshub(
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

    if (!m_provider.loadFromWshub(normalized, errorMessage))
    {
        return false;
    }

    m_store.insert(normalized, m_provider.tagDepthEntries());
    return true;
}

bool WhatSonHubTagsStateStore::contains(const QString& wshubPath) const
{
    return m_store.contains(normalizeHubPath(wshubPath));
}

QVector<WhatSonTagDepthEntry> WhatSonHubTagsStateStore::entries(const QString& wshubPath) const
{
    return m_store.value(normalizeHubPath(wshubPath));
}

QStringList WhatSonHubTagsStateStore::hubPaths() const
{
    return m_store.keys();
}

void WhatSonHubTagsStateStore::setEntries(
    const QString& wshubPath,
    QVector<WhatSonTagDepthEntry> entries)
{
    const QString normalized = normalizeHubPath(wshubPath);
    if (normalized.isEmpty())
    {
        return;
    }

    m_store.insert(normalized, std::move(entries));
}

void WhatSonHubTagsStateStore::remove(const QString& wshubPath)
{
    m_store.remove(normalizeHubPath(wshubPath));
}

void WhatSonHubTagsStateStore::clear()
{
    m_store.clear();
}

QString WhatSonHubTagsStateStore::normalizeHubPath(const QString& hubPath)
{
    const QString trimmed = hubPath.trimmed();
    if (trimmed.isEmpty())
    {
        return {};
    }
    return QDir::cleanPath(trimmed);
}
