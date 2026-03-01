#pragma once

#include <QString>

class WhatSonTagsHierarchyStore;

class WhatSonTagsHierarchyCreator
{
public:
    WhatSonTagsHierarchyCreator();
    ~WhatSonTagsHierarchyCreator();

    QString targetRelativePath() const;
    QString createText(const WhatSonTagsHierarchyStore& store) const;
};
