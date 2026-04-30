#pragma once

#include <QString>

namespace WhatSon::NavigationBar
{
    enum class Mode
    {
        View = 0,
        Edit,
        Control
    };

    bool isValidModeValue(int value) noexcept;
    Mode modeFromValue(int value) noexcept;
    int modeValue(Mode mode) noexcept;
    QString modeName(Mode mode);
    QString modeNameFromValue(int value);
    Mode nextMode(Mode mode) noexcept;
} // namespace WhatSon::NavigationBar
