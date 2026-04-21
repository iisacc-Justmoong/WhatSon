#pragma once

#include "WhatSonHubStat.hpp"

#include <QString>
#include <QVariantMap>

class WhatSonHubStore
{
public:
    WhatSonHubStore();
    ~WhatSonHubStore();

    void clear();

    QString hubPath() const;
    void setHubPath(QString value);

    QString hubName() const;
    void setHubName(QString value);

    QString contentsPath() const;
    void setContentsPath(QString value);

    QString libraryPath() const;
    void setLibraryPath(QString value);

    QString resourcesPath() const;
    void setResourcesPath(QString value);

    QString statPath() const;
    void setStatPath(QString value);

    WhatSonHubStat stat() const;
    void setStat(WhatSonHubStat value);

    QVariantMap domainValues() const;
    void setDomainValues(QVariantMap value);

private:
    QString m_hubPath;
    QString m_hubName;
    QString m_contentsPath;
    QString m_libraryPath;
    QString m_resourcesPath;
    QString m_statPath;
    WhatSonHubStat m_stat;
    QVariantMap m_domainValues;
};
