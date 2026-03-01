#pragma once

#include <QString>

class WhatSonLibraryHierarchyStore;

class WhatSonLibraryHierarchyParser
{
public:
    WhatSonLibraryHierarchyParser();
    ~WhatSonLibraryHierarchyParser();

    bool parse(
        const QString& rawText,
        WhatSonLibraryHierarchyStore* outStore,
        QString* errorMessage = nullptr) const;
};
