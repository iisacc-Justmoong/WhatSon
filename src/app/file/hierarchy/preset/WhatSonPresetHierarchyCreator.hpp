#pragma once

#include <QString>

class WhatSonPresetHierarchyStore;

class WhatSonPresetHierarchyCreator
{
public:
    WhatSonPresetHierarchyCreator();
    ~WhatSonPresetHierarchyCreator();

    QString targetRelativePath() const;
    QString createText(const WhatSonPresetHierarchyStore& store) const;
};
