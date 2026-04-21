#include "WhatSonHubStore.hpp"

#include "models/file/WhatSonDebugTrace.hpp"
#include "WhatSonHubPathUtils.hpp"

#include <QDir>

#include <utility>

namespace
{
    QString normalizePath(const QString& value)
    {
        return WhatSon::HubPath::normalizePath(value);
    }
} // namespace

WhatSonHubStore::WhatSonHubStore() = default;

WhatSonHubStore::~WhatSonHubStore() = default;

void WhatSonHubStore::clear()
{
    m_hubPath.clear();
    m_hubName.clear();
    m_contentsPath.clear();
    m_libraryPath.clear();
    m_resourcesPath.clear();
    m_statPath.clear();
    m_stat.clear();
    m_domainValues.clear();

    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.store"),
                              QStringLiteral("clear"));
}

QString WhatSonHubStore::hubPath() const
{
    return m_hubPath;
}

void WhatSonHubStore::setHubPath(QString value)
{
    m_hubPath = normalizePath(value);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.store"),
                              QStringLiteral("setHubPath"),
                              QStringLiteral("value=%1").arg(m_hubPath));
}

QString WhatSonHubStore::hubName() const
{
    return m_hubName;
}

void WhatSonHubStore::setHubName(QString value)
{
    m_hubName = value.trimmed();
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.store"),
                              QStringLiteral("setHubName"),
                              QStringLiteral("value=%1").arg(m_hubName));
}

QString WhatSonHubStore::contentsPath() const
{
    return m_contentsPath;
}

void WhatSonHubStore::setContentsPath(QString value)
{
    m_contentsPath = normalizePath(value);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.store"),
                              QStringLiteral("setContentsPath"),
                              QStringLiteral("value=%1").arg(m_contentsPath));
}

QString WhatSonHubStore::libraryPath() const
{
    return m_libraryPath;
}

void WhatSonHubStore::setLibraryPath(QString value)
{
    m_libraryPath = normalizePath(value);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.store"),
                              QStringLiteral("setLibraryPath"),
                              QStringLiteral("value=%1").arg(m_libraryPath));
}

QString WhatSonHubStore::resourcesPath() const
{
    return m_resourcesPath;
}

void WhatSonHubStore::setResourcesPath(QString value)
{
    m_resourcesPath = normalizePath(value);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.store"),
                              QStringLiteral("setResourcesPath"),
                              QStringLiteral("value=%1").arg(m_resourcesPath));
}

QString WhatSonHubStore::statPath() const
{
    return m_statPath;
}

void WhatSonHubStore::setStatPath(QString value)
{
    m_statPath = normalizePath(value);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.store"),
                              QStringLiteral("setStatPath"),
                              QStringLiteral("value=%1").arg(m_statPath));
}

WhatSonHubStat WhatSonHubStore::stat() const
{
    return m_stat;
}

void WhatSonHubStore::setStat(WhatSonHubStat value)
{
    m_stat = std::move(value);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.store"),
                              QStringLiteral("setStat"),
                              QStringLiteral("noteCount=%1 resourceCount=%2 characterCount=%3")
                              .arg(m_stat.noteCount())
                              .arg(m_stat.resourceCount())
                              .arg(m_stat.characterCount()));
}

QVariantMap WhatSonHubStore::domainValues() const
{
    return m_domainValues;
}

void WhatSonHubStore::setDomainValues(QVariantMap value)
{
    m_domainValues = std::move(value);
    WhatSon::Debug::traceSelf(this,
                              QStringLiteral("hub.store"),
                              QStringLiteral("setDomainValues"),
                              QStringLiteral("count=%1").arg(m_domainValues.size()));
}
