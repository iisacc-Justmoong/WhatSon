#pragma once

#include <QString>

class WhatSonEventHierarchyStore;

class WhatSonEventHierarchyCreator
{
public:
    WhatSonEventHierarchyCreator();
    ~WhatSonEventHierarchyCreator();

    QString targetRelativePath() const;
    QString createText(const WhatSonEventHierarchyStore& store) const;
};
