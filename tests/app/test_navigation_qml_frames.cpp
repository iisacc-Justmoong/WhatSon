#include "viewmodel/panel/PanelViewModelRegistry.hpp"

#include <QDir>
#include <QFile>
#include <QtTest>

namespace
{
    QString readQml(const QString& relativePath)
    {
        const QDir testsDir(QStringLiteral(QT_TESTCASE_SOURCEDIR));
        const QString qmlRoot = testsDir.absoluteFilePath(QStringLiteral("../src/app/qml"));
        const QString qmlPath = QDir(qmlRoot).absoluteFilePath(relativePath);
        QFile file(qmlPath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return QString();
        }

        return QString::fromUtf8(file.readAll());
    }
} // namespace

class NavigationQmlFramesTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void mainQml_mustBindSidebarInitWidthToHierarchyToolbarWidth();
    void navigationPanels_mustExposeFrameScopedPanelKeys();
    void mainQml_mustBindTabShortcutForNavigationModeCycling();
    void navigationBar_mustComposePropertiesFrame();
    void navigationBar_mustSwitchApplicationBarsByNavigationMode();
    void navigationPropertiesFrame_mustComposeSeparatedChildFrames();
    void navigationChildFrames_mustBindPanelViewModelContracts();
    void navigationSelectionBars_mustUseContextMenuCombos();
    void hierarchySidebar_mustReceiveSharedHorizontalInset();
};

void NavigationQmlFramesTest::mainQml_mustBindSidebarInitWidthToHierarchyToolbarWidth()
{
    const QString mainQml = readQml(QStringLiteral("Main.qml"));
    const QString bodyLayout = readQml(QStringLiteral("view/panels/BodyLayout.qml"));
    const QString hierarchySidebarLayout = readQml(QStringLiteral("view/panels/HierarchySidebarLayout.qml"));
    const QString sidebarHierarchyView = readQml(QStringLiteral("view/panels/sidebar/SidebarHierarchyView.qml"));

    QVERIFY(!mainQml.isEmpty());
    QVERIFY(mainQml.contains(QStringLiteral("readonly property int baseSidebarWidth: hierarchyToolbarWidth")));
    QVERIFY(mainQml.contains(QStringLiteral("property int preferredSidebarWidth: baseSidebarWidth")));
    QVERIFY(mainQml.contains(QStringLiteral("sidebarHorizontalInset: applicationWindow.hierarchyHorizontalInset")));
    QVERIFY(bodyLayout.contains(QStringLiteral("property int sidebarHorizontalInset: 2")));
    QVERIFY(bodyLayout.contains(QStringLiteral("horizontalInset: hStack.sidebarHorizontalInset")));
    QVERIFY(hierarchySidebarLayout.contains(QStringLiteral("property int horizontalInset: 2")));
    QVERIFY(hierarchySidebarLayout.contains(QStringLiteral("horizontalInset: hierarchyView.horizontalInset")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("property int horizontalInset: 2")));
}

void NavigationQmlFramesTest::navigationPanels_mustExposeFrameScopedPanelKeys()
{
    PanelViewModelRegistry registry;

    QVERIFY(registry.containsPanel(QStringLiteral("navigation.NavigationPropertiesBar")));
    QVERIFY(registry.containsPanel(QStringLiteral("navigation.NavigationInformationBar")));
    QVERIFY(registry.containsPanel(QStringLiteral("navigation.NavigationModeBar")));
    QVERIFY(registry.containsPanel(QStringLiteral("navigation.NavigationEditorViewBar")));
    QVERIFY(registry.containsPanel(QStringLiteral("navigation.NavigationApplicationViewBar")));
    QVERIFY(registry.containsPanel(QStringLiteral("navigation.NavigationApplicationEditBar")));
    QVERIFY(registry.containsPanel(QStringLiteral("navigation.NavigationApplicationControlBar")));
}

void NavigationQmlFramesTest::mainQml_mustBindTabShortcutForNavigationModeCycling()
{
    const QString mainQml = readQml(QStringLiteral("Main.qml"));

    QVERIFY(!mainQml.isEmpty());
    QVERIFY(mainQml.contains(QStringLiteral("function hasFocusedTextInput()")));
    QVERIFY(mainQml.contains(QStringLiteral("current.text !== undefined")));
    QVERIFY(mainQml.contains(QStringLiteral("current.cursorPosition !== undefined")));
    QVERIFY(mainQml.contains(QStringLiteral("current.selectedText !== undefined")));
    QVERIFY(mainQml.contains(QStringLiteral("function cycleNavigationModeFromShortcut()")));
    QVERIFY(mainQml.contains(QStringLiteral("sequence: \"Tab\"")));
    QVERIFY(mainQml.contains(QStringLiteral("context: Qt.ApplicationShortcut")));
    QVERIFY(mainQml.contains(QStringLiteral("enabled: !applicationWindow.hasFocusedTextInput()")));
    QVERIFY(mainQml.contains(QStringLiteral("applicationWindow.navigationModeVm.requestNextMode();")));
}

void NavigationQmlFramesTest::navigationBar_mustComposePropertiesFrame()
{
    const QString navigationBarLayout = readQml(QStringLiteral("view/panels/NavigationBarLayout.qml"));

    QVERIFY(!navigationBarLayout.isEmpty());
    QVERIFY(navigationBarLayout.contains(QStringLiteral("property var editorViewModeViewModel: null")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral("property var navigationModeViewModel: null")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral("NavigationView.NavigationPropertiesBar {")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral(
        "editorViewModeViewModel: navigationBar.editorViewModeViewModel")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral(
        "navigationModeViewModel: navigationBar.navigationModeViewModel")));
}

void NavigationQmlFramesTest::navigationBar_mustSwitchApplicationBarsByNavigationMode()
{
    const QString navigationBarLayout = readQml(QStringLiteral("view/panels/NavigationBarLayout.qml"));
    const QString applicationViewBar = readQml(
        QStringLiteral("view/panels/navigation/NavigationApplicationViewBar.qml"));
    const QString applicationEditBar = readQml(
        QStringLiteral("view/panels/navigation/NavigationApplicationEditBar.qml"));

    QVERIFY(!navigationBarLayout.isEmpty());
    QVERIFY(!applicationViewBar.isEmpty());
    QVERIFY(!applicationEditBar.isEmpty());
    QVERIFY(navigationBarLayout.contains(QStringLiteral("readonly property string activeNavigationModeName")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral("NavigationView.NavigationApplicationViewBar {")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral("NavigationView.NavigationApplicationEditBar {")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral("NavigationView.NavigationApplicationControlBar {")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("NavigationPreferenceBar {")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("NavigationPreferenceBar {")));
}

void NavigationQmlFramesTest::navigationPropertiesFrame_mustComposeSeparatedChildFrames()
{
    const QString propertiesBar = readQml(QStringLiteral("view/panels/navigation/NavigationPropertiesBar.qml"));

    QVERIFY(!propertiesBar.isEmpty());
    QVERIFY(propertiesBar.contains(QStringLiteral("NavigationInformationBar {")));
    QVERIFY(propertiesBar.contains(QStringLiteral("NavigationModeBar {")));
    QVERIFY(propertiesBar.contains(QStringLiteral("NavigationEditorViewBar {")));
}

void NavigationQmlFramesTest::navigationChildFrames_mustBindPanelViewModelContracts()
{
    const QString propertiesBar = readQml(QStringLiteral("view/panels/navigation/NavigationPropertiesBar.qml"));
    const QString informationBar = readQml(QStringLiteral("view/panels/navigation/NavigationInformationBar.qml"));
    const QString modeBar = readQml(QStringLiteral("view/panels/navigation/NavigationModeBar.qml"));
    const QString editorViewBar = readQml(QStringLiteral("view/panels/navigation/NavigationEditorViewBar.qml"));
    const QString applicationViewBar = readQml(
        QStringLiteral("view/panels/navigation/NavigationApplicationViewBar.qml"));
    const QString applicationEditBar = readQml(
        QStringLiteral("view/panels/navigation/NavigationApplicationEditBar.qml"));
    const QString applicationControlBar = readQml(
        QStringLiteral("view/panels/navigation/NavigationApplicationControlBar.qml"));

    QVERIFY(propertiesBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationPropertiesBar\")")));
    QVERIFY(informationBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationInformationBar\")")));
    QVERIFY(modeBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationModeBar\")")));
    QVERIFY(editorViewBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationEditorViewBar\")")));
    QVERIFY(applicationViewBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationApplicationViewBar\")")));
    QVERIFY(applicationEditBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationApplicationEditBar\")")));
    QVERIFY(applicationControlBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationApplicationControlBar\")")));
}

void NavigationQmlFramesTest::navigationSelectionBars_mustUseContextMenuCombos()
{
    const QString modeBar = readQml(QStringLiteral("view/panels/navigation/NavigationModeBar.qml"));
    const QString editorViewBar = readQml(QStringLiteral("view/panels/navigation/NavigationEditorViewBar.qml"));

    QVERIFY(!modeBar.isEmpty());
    QVERIFY(!editorViewBar.isEmpty());

    QVERIFY(modeBar.contains(QStringLiteral("LV.ComboBox {")));
    QVERIFY(modeBar.contains(QStringLiteral("text: modeBar.activeModeText")));
    QVERIFY(modeBar.contains(QStringLiteral("LV.ContextMenu {")));
    QVERIFY(modeBar.contains(QStringLiteral("requestModeChange(index)")));
    QVERIFY(!modeBar.contains(QStringLiteral("requestNextMode();")));
    QVERIFY(!modeBar.contains(QStringLiteral("comboLabelRightInset")));
    QVERIFY(!modeBar.contains(QStringLiteral("resolvedBackgroundColor")));

    QVERIFY(editorViewBar.contains(QStringLiteral("LV.ComboBox {")));
    QVERIFY(editorViewBar.contains(QStringLiteral("text: editorViewBar.activeViewText")));
    QVERIFY(editorViewBar.contains(QStringLiteral("LV.ContextMenu {")));
    QVERIFY(editorViewBar.contains(QStringLiteral("requestViewModeChange(index)")));
    QVERIFY(!editorViewBar.contains(QStringLiteral("requestNextViewMode();")));
    QVERIFY(!editorViewBar.contains(QStringLiteral("comboLabelRightInset")));
    QVERIFY(!editorViewBar.contains(QStringLiteral("resolvedBackgroundColor")));
}

void NavigationQmlFramesTest::hierarchySidebar_mustReceiveSharedHorizontalInset()
{
    const QString hierarchySidebarLayout = readQml(QStringLiteral("view/panels/HierarchySidebarLayout.qml"));
    const QString sidebarHierarchyView = readQml(QStringLiteral("view/panels/sidebar/SidebarHierarchyView.qml"));

    QVERIFY(!hierarchySidebarLayout.isEmpty());
    QVERIFY(!sidebarHierarchyView.isEmpty());
    QVERIFY(hierarchySidebarLayout.contains(QStringLiteral("horizontalInset: hierarchyView.horizontalInset")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("anchors.leftMargin: sidebarHierarchyView.horizontalInset")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("anchors.rightMargin: sidebarHierarchyView.horizontalInset")));
}

QTEST_APPLESS_MAIN(NavigationQmlFramesTest)

#include "test_navigation_qml_frames.moc"
