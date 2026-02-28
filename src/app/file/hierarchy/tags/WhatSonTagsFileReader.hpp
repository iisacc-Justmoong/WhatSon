#pragma once

#include <QString>

class WhatSonTagsFileReader
{
public:
    WhatSonTagsFileReader();
    ~WhatSonTagsFileReader();

    bool readTextFile(
        const QString& filePath,
        QString* outText,
        QString* errorMessage = nullptr) const;
};
