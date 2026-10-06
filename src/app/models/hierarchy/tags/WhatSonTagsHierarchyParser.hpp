#pragma once

#include <QString>

class WhatSonTagsHierarchyStore;

class WhatSonTagsHierarchyParser
{
public:
    WhatSonTagsHierarchyParser();
    ~WhatSonTagsHierarchyParser();

    bool parse(
        const QString& rawText,
        WhatSonTagsHierarchyStore* outStore,
        QString* errorMessage = nullptr) const;
};
