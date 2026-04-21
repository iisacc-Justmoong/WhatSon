#pragma once

#include <QString>

class WhatSonResourcesHierarchyStore;

class WhatSonResourcesHierarchyParser
{
public:
    WhatSonResourcesHierarchyParser();
    ~WhatSonResourcesHierarchyParser();

    bool parse(
        const QString& rawText,
        WhatSonResourcesHierarchyStore* outStore,
        QString* errorMessage = nullptr) const;
};
