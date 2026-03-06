#pragma once

#include <QString>

class WhatSonTxtFileStore;

class WhatSonTxtFileParser
{
public:
    WhatSonTxtFileParser();
    ~WhatSonTxtFileParser();

    bool parse(
        const QString& rawText,
        WhatSonTxtFileStore* outStore,
        QString* errorMessage = nullptr) const;
};
