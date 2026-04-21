#pragma once

#include <QString>

class WhatSonResourcesHierarchyStore;

class WhatSonResourcesHierarchyCreator
{
public:
    WhatSonResourcesHierarchyCreator();
    ~WhatSonResourcesHierarchyCreator();

    QString targetRelativePath() const;
    QString createText(const WhatSonResourcesHierarchyStore& store) const;
};
