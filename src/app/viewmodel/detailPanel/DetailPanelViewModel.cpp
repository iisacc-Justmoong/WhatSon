#include "DetailPanelViewModel.hpp"

#include <QVariantMap>

#include <array>

namespace
{
    struct ToolbarSpec final
    {
        DetailPanelViewModel::DetailContentState state;
        const char* iconName;
    };

    constexpr std::array<ToolbarSpec, 6> kToolbarSpecs = {
        ToolbarSpec{DetailPanelViewModel::DetailContentState::FileInfo, "syncFilesModInfo"},
        ToolbarSpec{DetailPanelViewModel::DetailContentState::FileStat, "statisticsPanel"},
        ToolbarSpec{DetailPanelViewModel::DetailContentState::FileFormat, "fileFormat"},
        ToolbarSpec{DetailPanelViewModel::DetailContentState::FileHistory, "toolWindowClock"},
        ToolbarSpec{DetailPanelViewModel::DetailContentState::Appearance, "cwmPermissionView"},
        ToolbarSpec{DetailPanelViewModel::DetailContentState::Help, "featureAnswer"}
    };
} // namespace

DetailPanelViewModel::DetailPanelViewModel(QObject* parent)
    : QObject(parent)
{
    rebuildToolbarItems();
}

DetailPanelViewModel::~DetailPanelViewModel() = default;

int DetailPanelViewModel::activeState() const noexcept
{
    return static_cast<int>(m_activeState);
}

QString DetailPanelViewModel::activeStateName() const
{
    return stateName(m_activeState);
}

QVariantList DetailPanelViewModel::toolbarItems() const
{
    return m_toolbarItems;
}

void DetailPanelViewModel::setActiveState(int stateValue)
{
    if (!isValidStateValue(stateValue))
    {
        return;
    }

    const auto nextState = static_cast<DetailContentState>(stateValue);
    if (nextState == m_activeState)
    {
        return;
    }

    m_activeState = nextState;
    rebuildToolbarItems();
    emit activeStateChanged();
    emit toolbarItemsChanged();
}

void DetailPanelViewModel::requestStateChange(int stateValue)
{
    setActiveState(stateValue);
}

bool DetailPanelViewModel::isValidStateValue(int stateValue) noexcept
{
    return stateValue >= static_cast<int>(DetailContentState::FileInfo)
        && stateValue <= static_cast<int>(DetailContentState::Help);
}

QString DetailPanelViewModel::stateName(DetailContentState state)
{
    switch (state)
    {
    case DetailContentState::FileInfo:
        return QStringLiteral("fileInfo");
    case DetailContentState::FileStat:
        return QStringLiteral("fileStat");
    case DetailContentState::FileFormat:
        return QStringLiteral("fileFormat");
    case DetailContentState::FileHistory:
        return QStringLiteral("fileHistory");
    case DetailContentState::Appearance:
        return QStringLiteral("appearance");
    case DetailContentState::Help:
        return QStringLiteral("help");
    }

    return QStringLiteral("fileInfo");
}

void DetailPanelViewModel::rebuildToolbarItems()
{
    QVariantList items;
    items.reserve(static_cast<qsizetype>(kToolbarSpecs.size()));

    for (const ToolbarSpec& spec : kToolbarSpecs)
    {
        QVariantMap item;
        item.insert(QStringLiteral("iconName"), QString::fromUtf8(spec.iconName));
        item.insert(QStringLiteral("stateValue"), static_cast<int>(spec.state));
        item.insert(QStringLiteral("selected"), spec.state == m_activeState);
        items.push_back(item);
    }

    m_toolbarItems = std::move(items);
}
