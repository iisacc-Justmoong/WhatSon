#pragma once

#include <QString>

class WhatSonBookmarksHierarchyStore;

class WhatSonBookmarksHierarchyCreator
{
public:
    WhatSonBookmarksHierarchyCreator();
    ~WhatSonBookmarksHierarchyCreator();

    QString targetRelativePath() const;
    QString createText(const WhatSonBookmarksHierarchyStore& store) const;
};
