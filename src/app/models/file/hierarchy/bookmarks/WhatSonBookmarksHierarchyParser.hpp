#pragma once

#include <QString>

class WhatSonBookmarksHierarchyStore;

class WhatSonBookmarksHierarchyParser
{
public:
    WhatSonBookmarksHierarchyParser();
    ~WhatSonBookmarksHierarchyParser();

    bool parse(
        const QString& rawText,
        WhatSonBookmarksHierarchyStore* outStore,
        QString* errorMessage = nullptr) const;
};
