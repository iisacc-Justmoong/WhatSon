#include "DetailPanelState.hpp"

namespace WhatSon::DetailPanel
{
    bool isValidStateValue(int stateValue) noexcept
    {
        return stateValue >= static_cast<int>(ContentState::FileInfo)
            && stateValue <= static_cast<int>(ContentState::Help);
    }

    ContentState stateFromValue(int stateValue) noexcept
    {
        if (!isValidStateValue(stateValue))
        {
            return ContentState::FileInfo;
        }

        return static_cast<ContentState>(stateValue);
    }

    int stateValue(ContentState state) noexcept
    {
        return static_cast<int>(state);
    }

    QString stateName(ContentState state)
    {
        switch (state)
        {
        case ContentState::FileInfo:
            return QStringLiteral("fileInfo");
        case ContentState::FileStat:
            return QStringLiteral("fileStat");
        case ContentState::FileFormat:
            return QStringLiteral("fileFormat");
        case ContentState::FileHistory:
            return QStringLiteral("fileHistory");
        case ContentState::Appearance:
            return QStringLiteral("appearance");
        case ContentState::Help:
            return QStringLiteral("help");
        }

        return QStringLiteral("fileInfo");
    }

    QString stateNameFromValue(int stateValueValue)
    {
        return stateName(stateFromValue(stateValueValue));
    }
} // namespace WhatSon::DetailPanel
