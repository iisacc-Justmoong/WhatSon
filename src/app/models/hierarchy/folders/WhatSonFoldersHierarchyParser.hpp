#pragma once

#include <QString>

class WhatSonFoldersHierarchyStore;

class WhatSonFoldersHierarchyParser
{
public:
    WhatSonFoldersHierarchyParser();
    ~WhatSonFoldersHierarchyParser();

    bool parse(
        const QString& rawText,
        WhatSonFoldersHierarchyStore* outStore,
        QString* errorMessage = nullptr,
        bool* outUuidMigrationRequired = nullptr) const;
};
