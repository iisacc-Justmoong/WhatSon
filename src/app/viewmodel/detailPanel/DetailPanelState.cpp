#include "app/viewmodel/detailPanel/DetailPanelState.hpp"

namespace WhatSon::DetailPanel
{
    bool isValidStateValue(int stateValue) noexcept
    {
        return stateValue >= static_cast<int>(ContentState::Properties)
            && stateValue <= static_cast<int>(ContentState::Help);
    }

    ContentState stateFromValue(int stateValue) noexcept
    {
        if (!isValidStateValue(stateValue))
        {
            return ContentState::Properties;
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
        case ContentState::Properties:
            return QStringLiteral("properties");
        case ContentState::FileStat:
            return QStringLiteral("fileStat");
        case ContentState::Insert:
            return QStringLiteral("insert");
        case ContentState::FileHistory:
            return QStringLiteral("fileHistory");
        case ContentState::Layer:
            return QStringLiteral("layer");
        case ContentState::Help:
            return QStringLiteral("help");
        }

        return QStringLiteral("properties");
    }

    QString stateNameFromValue(int stateValue)
    {
        return stateName(stateFromValue(stateValue));
    }
} // namespace WhatSon::DetailPanel
