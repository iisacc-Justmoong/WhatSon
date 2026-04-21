#pragma once

#include <QString>

class WhatSonFoldersHierarchyStore;

class WhatSonFoldersHierarchyCreator
{
public:
    WhatSonFoldersHierarchyCreator();
    ~WhatSonFoldersHierarchyCreator();

    QString targetRelativePath() const;
    QString createText(const WhatSonFoldersHierarchyStore& store) const;
};
