#pragma once

#include <QString>

class WhatSonHubTagsPathResolver
{
public:
    WhatSonHubTagsPathResolver();
    ~WhatSonHubTagsPathResolver();

    bool resolveTagsFilePath(
        const QString& wshubPath,
        QString* outTagsFilePath,
        QString* errorMessage = nullptr) const;
};
