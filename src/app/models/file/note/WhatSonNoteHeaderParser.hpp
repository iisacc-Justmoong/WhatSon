#pragma once

#include "WhatSonNoteHeaderStore.hpp"

#include <QString>

class WhatSonNoteHeaderParser
{
public:
    WhatSonNoteHeaderParser();
    ~WhatSonNoteHeaderParser();

    bool parse(
        const QString& wsnHeadText,
        WhatSonNoteHeaderStore* outStore,
        QString* errorMessage = nullptr) const;
};
