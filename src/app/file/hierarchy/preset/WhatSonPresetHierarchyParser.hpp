#pragma once

#include <QString>

class WhatSonPresetHierarchyStore;

class WhatSonPresetHierarchyParser
{
public:
    WhatSonPresetHierarchyParser();
    ~WhatSonPresetHierarchyParser();

    bool parse(
        const QString& rawText,
        WhatSonPresetHierarchyStore* outStore,
        QString* errorMessage = nullptr) const;
};
