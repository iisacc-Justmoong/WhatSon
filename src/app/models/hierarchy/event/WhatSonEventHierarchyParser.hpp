#pragma once

#include <QString>

class WhatSonEventHierarchyStore;

class WhatSonEventHierarchyParser
{
public:
    WhatSonEventHierarchyParser();
    ~WhatSonEventHierarchyParser();

    bool parse(
        const QString& rawText,
        WhatSonEventHierarchyStore* outStore,
        QString* errorMessage = nullptr) const;
};
