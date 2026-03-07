#include "NavigationModeState.hpp"

namespace WhatSon::NavigationBar
{
    bool isValidModeValue(int value) noexcept
    {
        return value >= modeValue(Mode::View) && value <= modeValue(Mode::Presentation);
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
        case 3:
            return Mode::Presentation;
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
        case Mode::Presentation:
            return QStringLiteral("Presentation");
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
            return Mode::Presentation;
        case Mode::Presentation:
            return Mode::View;
        }

        return Mode::View;
    }
} // namespace WhatSon::NavigationBar
