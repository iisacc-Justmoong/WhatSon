#pragma once

#include "WhatSonTagDepthEntry.hpp"

#include <QString>
#include <QVector>

class WhatSonTagsHierarchyStore
{
public:
    WhatSonTagsHierarchyStore();
    ~WhatSonTagsHierarchyStore();

    void clear();

    QString hubPath() const;
    void setHubPath(QString hubPath);

    QVector<WhatSonTagDepthEntry> tagEntries() const;
    void setTagEntries(QVector<WhatSonTagDepthEntry> tagEntries);

private:
    QString m_hubPath;
    QVector<WhatSonTagDepthEntry> m_tagEntries;
};
