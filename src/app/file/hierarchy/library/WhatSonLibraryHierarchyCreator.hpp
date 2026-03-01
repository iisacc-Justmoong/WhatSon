#pragma once

#include <QString>

class WhatSonLibraryHierarchyStore;

class WhatSonLibraryHierarchyCreator
{
public:
    WhatSonLibraryHierarchyCreator();
    ~WhatSonLibraryHierarchyCreator();

    QString targetRelativePath() const;
    QString createText(const WhatSonLibraryHierarchyStore& store) const;
};
