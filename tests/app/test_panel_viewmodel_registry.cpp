#include "viewmodel/panel/PanelViewModelRegistry.hpp"

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QtTest>

#include <algorithm>

namespace
{
    struct PanelBindingExpectation final
    {
        const char* relativeQmlPath = "";
        const char* panelKey = "";
    };

    QStringList expectedPanelKeys()
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
            QStringLiteral("MobileNormalLayout"),
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

    QList<PanelBindingExpectation> expectedPanelBindings()
    {
        return {
            {"view/panels/BodyLayout.qml", "BodyLayout"},
            {"view/panels/ContentViewLayout.qml", "ContentViewLayout"},
            {"view/panels/detail/DetailContents.qml", "detail.DetailContents"},
            {"view/panels/detail/DetailPanel.qml", "detail.DetailPanel"},
            {"view/panels/detail/DetailPanelHeaderToolbar.qml", "detail.DetailPanelHeaderToolbar"},
            {
                "view/panels/detail/DetailPanelHeaderToolbarButton.qml",
                "detail.DetailPanelHeaderToolbarButton"
            },
            {"view/panels/detail/RightPanel.qml", "detail.RightPanel"},
            {"view/panels/DetailPanelLayout.qml", "DetailPanelLayout"},
            {"view/panels/HierarchySidebarLayout.qml", "HierarchySidebarLayout"},
            {"view/panels/ListBarLayout.qml", "ListBarLayout"},
            {"view/panels/MobileNormalLayout.qml", "MobileNormalLayout"},
            {"view/panels/NavigationBarLayout.qml", "NavigationBarLayout"},
            {"view/panels/NoteListItem.qml", "NoteListItem"},
            {"view/panels/StatusBarLayout.qml", "StatusBarLayout"},
            {"view/panels/navigation/NavigationAddNewBar.qml", "navigation.NavigationAddNewBar"},
            {"view/panels/navigation/control/NavigationAppControlBar.qml", "navigation.NavigationAppControlBar"},
            {
                "view/panels/navigation/control/NavigationApplicationControlBar.qml",
                "navigation.NavigationApplicationControlBar"
            },
            {"view/panels/navigation/edit/NavigationApplicationEditBar.qml", "navigation.NavigationApplicationEditBar"},
            {"view/panels/navigation/view/NavigationApplicationViewBar.qml", "navigation.NavigationApplicationViewBar"},
            {"view/panels/navigation/NavigationCalendarBar.qml", "navigation.NavigationCalendarBar"},
            {"view/panels/navigation/NavigationEditorViewBar.qml", "navigation.NavigationEditorViewBar"},
            {"view/panels/navigation/control/NavigationExportBar.qml", "navigation.NavigationExportBar"},
            {"view/panels/navigation/NavigationInformationBar.qml", "navigation.NavigationInformationBar"},
            {"view/panels/navigation/NavigationModeBar.qml", "navigation.NavigationModeBar"},
            {"view/panels/navigation/NavigationPropertiesBar.qml", "navigation.NavigationPropertiesBar"},
            {"view/panels/navigation/NavigationPreferenceBar.qml", "navigation.NavigationPreferenceBar"},
            {"view/panels/sidebar/HierarchyViewBookmarks.qml", "sidebar.HierarchyViewBookmarks"},
            {"view/panels/sidebar/HierarchyViewEvent.qml", "sidebar.HierarchyViewEvent"},
            {"view/panels/sidebar/HierarchyViewLibrary.qml", "sidebar.HierarchyViewLibrary"},
            {"view/panels/sidebar/HierarchyViewPreset.qml", "sidebar.HierarchyViewPreset"},
            {"view/panels/sidebar/HierarchyViewProgress.qml", "sidebar.HierarchyViewProgress"},
            {"view/panels/sidebar/HierarchyViewProjects.qml", "sidebar.HierarchyViewProjects"},
            {"view/panels/sidebar/HierarchyViewResources.qml", "sidebar.HierarchyViewResources"},
            {"view/panels/sidebar/HierarchyViewTags.qml", "sidebar.HierarchyViewTags"},
            {"view/panels/sidebar/SidebarHierarchyView.qml", "sidebar.SidebarHierarchyView"}
        };
    }
} // namespace

class PanelViewModelRegistryTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void registry_mustExposeAllKnownPanels();
    void qmlPanels_mustBindPanelViewModelByPanelKey();
};

void PanelViewModelRegistryTest::registry_mustExposeAllKnownPanels()
{
    PanelViewModelRegistry registry;

    QStringList actual = registry.panelKeys();
    QStringList expected = expectedPanelKeys();

    actual.sort();
    expected.sort();

    QCOMPARE(actual, expected);
    QCOMPARE(registry.panelCount(), expected.size());

    for (const QString& key : expected)
    {
        QVERIFY2(registry.containsPanel(key), qPrintable(QStringLiteral("Missing panel key: %1").arg(key)));

        QObject* panelVm = registry.panelViewModel(key);
        QVERIFY2(panelVm != nullptr, qPrintable(QStringLiteral("panelViewModel returned null for key: %1").arg(key)));
        QCOMPARE(panelVm->property("panelKey").toString(), key);

        const int hookCountBefore = panelVm->property("hookRequestCount").toInt();
        QSignalSpy hookSignalSpy(panelVm, SIGNAL(viewModelHookRequested()));
        QSignalSpy hookCountSpy(panelVm, SIGNAL(hookRequestCountChanged()));
        QMetaObject::invokeMethod(panelVm, "requestViewModelHook", Qt::DirectConnection);
        QCOMPARE(hookSignalSpy.count(), 1);
        QCOMPARE(hookCountSpy.count(), 1);
        QCOMPARE(panelVm->property("hookRequestCount").toInt(), hookCountBefore + 1);
    }
}

void PanelViewModelRegistryTest::qmlPanels_mustBindPanelViewModelByPanelKey()
{
    const QDir testsDir(QStringLiteral(QT_TESTCASE_SOURCEDIR));
    const QString qmlRoot = testsDir.absoluteFilePath(QStringLiteral("../src/app/qml"));

    const QList<PanelBindingExpectation> bindings = expectedPanelBindings();
    for (const PanelBindingExpectation& binding : bindings)
    {
        const QString qmlPath = QDir(qmlRoot).absoluteFilePath(QString::fromUtf8(binding.relativeQmlPath));
        QFile qmlFile(qmlPath);
        QVERIFY2(qmlFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(qmlPath));

        const QString qmlText = QString::fromUtf8(qmlFile.readAll());
        const QString expectedBinding = QStringLiteral("panelViewModelRegistry.panelViewModel(\"%1\")")
            .arg(QString::fromUtf8(binding.panelKey));

        QVERIFY2(
            qmlText.contains(expectedBinding),
            qPrintable(QStringLiteral("Missing panel viewmodel binding. file=%1 key=%2")
                .arg(qmlPath, QString::fromUtf8(binding.panelKey))));
    }
}

QTEST_APPLESS_MAIN(PanelViewModelRegistryTest)

#include "test_panel_viewmodel_registry.moc"
