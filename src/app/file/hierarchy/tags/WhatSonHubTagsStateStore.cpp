#include "WhatSonHubTagsStateStore.hpp"

#include "WhatSonDebugTrace.hpp"

#include <QDir>

WhatSonHubTagsStateStore::WhatSonHubTagsStateStore() = default;

WhatSonHubTagsStateStore::~WhatSonHubTagsStateStore() = default;

bool WhatSonHubTagsStateStore::loadFromWshub(
    const QString& wshubPath,
    QString* errorMessage)
{
    const QString normalized = normalizeHubPath(wshubPath);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.tags.state"),
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

    if (!m_provider.loadFromWshub(normalized, errorMessage))
    {
        WhatSon::Debug::traceSelf(this,
                                  QStringLiteral("hub.tags.state"),
                                  QStringLiteral("load.failed"),
                                  errorMessage != nullptr ? *errorMessage : QString());
        return false;
    }

    m_store.insert(normalized, m_provider.tagDepthEntries());
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.tags.state"),
                              QStringLiteral("load.success"),
                              QStringLiteral("path=%1 count=%2").arg(normalized).arg(m_store.value(normalized).size()));
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

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.tags.state"),
                              QStringLiteral("setEntries"),
                              QStringLiteral("path=%1 count=%2").arg(normalized).arg(entries.size()));
    m_store.insert(normalized, std::move(entries));
}

void WhatSonHubTagsStateStore::remove(const QString& wshubPath)
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.tags.state"),
                              QStringLiteral("remove"),
                              QStringLiteral("path=%1").arg(wshubPath));
    m_store.remove(normalizeHubPath(wshubPath));
}

void WhatSonHubTagsStateStore::clear()
{
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.tags.state"),
                              QStringLiteral("clear"),
                              QStringLiteral("previousCount=%1").arg(m_store.size()));
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
