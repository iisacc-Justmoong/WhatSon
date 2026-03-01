#pragma once

#include <QString>

class WhatSonProgressHierarchyStore;

class WhatSonProgressHierarchyParser
{
public:
    WhatSonProgressHierarchyParser();
    ~WhatSonProgressHierarchyParser();

    bool parse(
        const QString& rawText,
        WhatSonProgressHierarchyStore* outStore,
        QString* errorMessage = nullptr) const;
};
