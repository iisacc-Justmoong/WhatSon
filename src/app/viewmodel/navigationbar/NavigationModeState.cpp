#include "app/viewmodel/navigationbar/NavigationModeState.hpp"

namespace WhatSon::NavigationBar
{
    bool isValidModeValue(int value) noexcept
    {
        return value >= modeValue(Mode::View) && value <= modeValue(Mode::Control);
    }

    Mode modeFromValue(int value) noexcept
    {
        switch (value)
        {
        case 0:
            return Mode::View;
        case 1:
            return Mode::Edit;
        case 2:
            return Mode::Control;
        default:
            return Mode::View;
        }
    }

    int modeValue(Mode mode) noexcept
    {
        return static_cast<int>(mode);
    }

    QString modeName(Mode mode)
    {
        switch (mode)
        {
        case Mode::View:
            return QStringLiteral("View");
        case Mode::Edit:
            return QStringLiteral("Edit");
        case Mode::Control:
            return QStringLiteral("Control");
        }

        return QString();
    }

    QString modeNameFromValue(int value)
    {
        if (!isValidModeValue(value))
        {
            return QString();
        }

        return modeName(modeFromValue(value));
    }

    Mode nextMode(Mode mode) noexcept
    {
        switch (mode)
        {
        case Mode::View:
            return Mode::Edit;
        case Mode::Edit:
            return Mode::Control;
        case Mode::Control:
            return Mode::View;
        }

        return Mode::View;
    }
} // namespace WhatSon::NavigationBar
