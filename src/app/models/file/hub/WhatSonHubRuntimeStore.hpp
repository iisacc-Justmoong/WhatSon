#pragma once

#include "app/models/file/hub/WhatSonHubStore.hpp"
#include "app/models/file/hub/WhatSonHubPlacementStore.hpp"
#include "app/models/file/hierarchy/tags/WhatSonHubTagsStateStore.hpp"

#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>

class WhatSonHubRuntimeStore
{
public:
    WhatSonHubRuntimeStore();
    ~WhatSonHubRuntimeStore();

    bool loadFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);

    bool contains(const QString& wshubPath) const;
    QStringList hubPaths() const;

    WhatSonHubPlacement placement(const QString& wshubPath) const;
    QVector<WhatSonTagDepthEntry> tagDepthEntries(const QString& wshubPath) const;
    WhatSonHubStore hub(const QString& wshubPath) const;
    WhatSonHubStat hubStat(const QString& wshubPath) const;

    void setPlacement(WhatSonHubPlacement placement);
    void setTagDepthEntries(const QString& wshubPath, QVector<WhatSonTagDepthEntry> entries);
    void setHub(WhatSonHubStore store);
    void setHubStat(const QString& wshubPath, WhatSonHubStat stat);

    void remove(const QString& wshubPath);
    void clear();

private:
    QHash<QString, WhatSonHubStore> m_hubStore;
    WhatSonHubPlacementStore m_placementStore;
    WhatSonHubTagsStateStore m_tagsStateStore;
};
