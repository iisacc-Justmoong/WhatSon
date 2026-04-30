#pragma once

#include <QString>

namespace WhatSon::DetailPanel
{
    enum class ContentState
    {
        Properties = 0,
        FileStat,
        Insert,
        FileHistory,
        Layer,
        Help
    };

    bool isValidStateValue(int stateValue) noexcept;
    ContentState stateFromValue(int stateValue) noexcept;
    int stateValue(ContentState state) noexcept;
    QString stateName(ContentState state);
    QString stateNameFromValue(int stateValue);
} // namespace WhatSon::DetailPanel
