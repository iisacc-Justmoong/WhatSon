#include "PanelViewModelRegistry.hpp"

#include <QStringList>

namespace
{
    QStringList panelKeyList()
    {
        return {
            QStringLiteral("BodyLayout"),
            QStringLiteral("ContentViewLayout"),
            QStringLiteral("detail.DetailContents"),
            QStringLiteral("detail.DetailPanel"),
            QStringLiteral("detail.DetailPanelHeaderToolbar"),
            QStringLiteral("detail.DetailPanelHeaderToolbarButton"),
            QStringLiteral("detail.RightPanel"),
            QStringLiteral("DetailPanelLayout"),
            QStringLiteral("HierarchySidebarLayout"),
            QStringLiteral("ListBarLayout"),
            QStringLiteral("mobile.MobileHierarchyPage"),
            QStringLiteral("NavigationBarLayout"),
            QStringLiteral("NoteListItem"),
            QStringLiteral("StatusBarLayout"),
            QStringLiteral("navigation.NavigationAddNewBar"),
            QStringLiteral("navigation.NavigationAppControlBar"),
            QStringLiteral("navigation.NavigationApplicationControlBar"),
            QStringLiteral("navigation.NavigationApplicationEditBar"),
            QStringLiteral("navigation.NavigationApplicationViewBar"),
            QStringLiteral("navigation.NavigationCalendarBar"),
            QStringLiteral("navigation.NavigationEditorViewBar"),
            QStringLiteral("navigation.NavigationExportBar"),
            QStringLiteral("navigation.NavigationInformationBar"),
            QStringLiteral("navigation.NavigationModeBar"),
            QStringLiteral("navigation.NavigationPropertiesBar"),
            QStringLiteral("navigation.NavigationPreferenceBar"),
            QStringLiteral("sidebar.HierarchyViewBookmarks"),
            QStringLiteral("sidebar.HierarchyViewEvent"),
            QStringLiteral("sidebar.HierarchyViewLibrary"),
            QStringLiteral("sidebar.HierarchyViewPreset"),
            QStringLiteral("sidebar.HierarchyViewProgress"),
            QStringLiteral("sidebar.HierarchyViewProjects"),
            QStringLiteral("sidebar.HierarchyViewResources"),
            QStringLiteral("sidebar.HierarchyViewTags"),
            QStringLiteral("sidebar.SidebarHierarchyView")
        };
    }
} // namespace

PanelViewModelRegistry::PanelViewModelRegistry(QObject* parent)
    : QObject(parent)
      , m_panelKeys(panelKeyList())
{
    for (const QString& panelKey : m_panelKeys)
    {
        m_panelViewModels.insert(panelKey, new PanelViewModel(panelKey, this));
    }
}

QStringList PanelViewModelRegistry::panelKeys() const
{
    return m_panelKeys;
}

int PanelViewModelRegistry::panelCount() const noexcept
{
    return m_panelKeys.size();
}

QObject* PanelViewModelRegistry::panelViewModel(const QString& panelKey) const
{
    return m_panelViewModels.value(panelKey.trimmed(), nullptr);
}

bool PanelViewModelRegistry::containsPanel(const QString& panelKey) const
{
    return m_panelViewModels.contains(panelKey.trimmed());
}
