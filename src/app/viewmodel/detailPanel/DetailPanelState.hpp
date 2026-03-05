#pragma once

#include <QString>

namespace WhatSon::DetailPanel
{
    enum class ContentState
    {
        FileInfo = 0,
        FileStat,
        FileFormat,
        FileHistory,
        Appearance,
        Help
    };

    bool isValidStateValue(int stateValue) noexcept;
    ContentState stateFromValue(int stateValue) noexcept;
    int stateValue(ContentState state) noexcept;
    QString stateName(ContentState state);
    QString stateNameFromValue(int stateValue);
} // namespace WhatSon::DetailPanel
