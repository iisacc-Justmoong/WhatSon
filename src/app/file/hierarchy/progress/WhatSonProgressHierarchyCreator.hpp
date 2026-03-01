#pragma once

#include <QString>

class WhatSonProgressHierarchyStore;

class WhatSonProgressHierarchyCreator
{
public:
    WhatSonProgressHierarchyCreator();
    ~WhatSonProgressHierarchyCreator();

    QString targetRelativePath() const;
    QString createText(const WhatSonProgressHierarchyStore& store) const;
};
