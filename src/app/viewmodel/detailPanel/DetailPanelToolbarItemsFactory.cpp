#include "DetailPanelToolbarItemsFactory.hpp"

#include <QVariantMap>

#include <array>

namespace WhatSon::DetailPanel
{
    namespace
    {
        struct ToolbarSpec final
        {
            ContentState state;
            const char* iconName;
        };

        constexpr std::array<ToolbarSpec, 6> kToolbarSpecs = {
            ToolbarSpec{ContentState::FileInfo, "generalprojectStructure"},
            ToolbarSpec{ContentState::FileStat, "chartBar"},
            ToolbarSpec{ContentState::FileFormat, "generaladd"},
            ToolbarSpec{ContentState::Appearance, "toolwindowdependencies"},
            ToolbarSpec{ContentState::FileHistory, "toolWindowClock"},
            ToolbarSpec{ContentState::Help, "featureAnswer"}
        };
    } // namespace

    QVariantList buildToolbarItems(ContentState activeState)
    {
        QVariantList items;
        items.reserve(static_cast<qsizetype>(kToolbarSpecs.size()));

        for (const ToolbarSpec& spec : kToolbarSpecs)
        {
            QVariantMap item;
            item.insert(QStringLiteral("iconName"), QString::fromUtf8(spec.iconName));
            item.insert(QStringLiteral("stateValue"), stateValue(spec.state));
            item.insert(QStringLiteral("selected"), spec.state == activeState);
            items.push_back(item);
        }

        return items;
    }
} // namespace WhatSon::DetailPanel
