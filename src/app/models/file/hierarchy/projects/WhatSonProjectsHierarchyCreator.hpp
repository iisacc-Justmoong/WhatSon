#pragma once

#include <QString>

class WhatSonProjectsHierarchyStore;

class WhatSonProjectsHierarchyCreator
{
public:
    WhatSonProjectsHierarchyCreator();
    ~WhatSonProjectsHierarchyCreator();

    QString targetRelativePath() const;
    QString createText(const WhatSonProjectsHierarchyStore& store) const;
};
