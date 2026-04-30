#include "app/models/panel/PanelControllerRegistry.hpp"

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

PanelControllerRegistry::PanelControllerRegistry(QObject* parent)
    : QObject(parent)
      , m_panelKeys(panelKeyList())
{
    for (const QString& panelKey : m_panelKeys)
    {
        m_panelControllers.insert(panelKey, new PanelController(panelKey, this));
    }
}

QStringList PanelControllerRegistry::panelKeys() const
{
    return m_panelKeys;
}

int PanelControllerRegistry::panelCount() const noexcept
{
    return m_panelKeys.size();
}

QObject* PanelControllerRegistry::panelController(const QString& panelKey) const
{
    return m_panelControllers.value(panelKey.trimmed(), nullptr);
}

bool PanelControllerRegistry::containsPanel(const QString& panelKey) const
{
    return m_panelControllers.contains(panelKey.trimmed());
}
