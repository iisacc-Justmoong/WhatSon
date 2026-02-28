#pragma once

#include "WhatSonHubPlacementStore.hpp"
#include "hierarchy/tags/WhatSonHubTagsStateStore.hpp"

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

    void setPlacement(WhatSonHubPlacement placement);
    void setTagDepthEntries(const QString& wshubPath, QVector<WhatSonTagDepthEntry> entries);

    void remove(const QString& wshubPath);
    void clear();

private:
    WhatSonHubPlacementStore m_placementStore;
    WhatSonHubTagsStateStore m_tagsStateStore;
};
