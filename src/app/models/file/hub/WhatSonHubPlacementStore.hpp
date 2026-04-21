#pragma once

#include "app/models/file/hub/WhatSonHubPlacement.hpp"

#include <QHash>
#include <QString>
#include <QStringList>

class WhatSonHubPlacementStore
{
public:
    WhatSonHubPlacementStore();
    ~WhatSonHubPlacementStore();

    bool loadFromWshub(const QString& wshubPath, QString* errorMessage = nullptr);

    bool contains(const QString& wshubPath) const;
    WhatSonHubPlacement placement(const QString& wshubPath) const;
    QStringList hubPaths() const;

    void setPlacement(WhatSonHubPlacement placement);
    void remove(const QString& wshubPath);
    void clear();

private:
    static QString normalizeHubPath(const QString& hubPath);
    static bool extractCoordinates(
        const QString& wshubPath,
        double* outX,
        double* outY,
        QString* errorMessage);

    QHash<QString, WhatSonHubPlacement> m_store;
};
