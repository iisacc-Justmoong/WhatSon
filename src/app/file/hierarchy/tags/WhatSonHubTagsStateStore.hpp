#pragma once

#include "WhatSonHubTagsDepthProvider.hpp"

#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>

class WhatSonHubTagsStateStore
{
public:
    WhatSonHubTagsStateStore();
    ~WhatSonHubTagsStateStore();

    bool loadFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);

    bool contains(const QString& wshubPath) const;
    QVector<WhatSonTagDepthEntry> entries(const QString& wshubPath) const;
    QStringList hubPaths() const;

    void setEntries(const QString& wshubPath, QVector<WhatSonTagDepthEntry> entries);
    void remove(const QString& wshubPath);
    void clear();

private:
    static QString normalizeHubPath(const QString& hubPath);

    WhatSonHubTagsDepthProvider m_provider;
    QHash<QString, QVector<WhatSonTagDepthEntry>> m_store;
};
