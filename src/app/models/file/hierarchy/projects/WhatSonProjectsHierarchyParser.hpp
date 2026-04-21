#pragma once

#include <QString>

class WhatSonProjectsHierarchyStore;

class WhatSonProjectsHierarchyParser
{
public:
    WhatSonProjectsHierarchyParser();
    ~WhatSonProjectsHierarchyParser();

    bool parse(
        const QString& rawText,
        WhatSonProjectsHierarchyStore* outStore,
        QString* errorMessage = nullptr) const;
};
