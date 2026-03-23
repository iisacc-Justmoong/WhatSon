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
    void mainQml_mustBindNewShortcutForNoteCreation();
    void navigationBar_mustComposePropertiesFrame();
    void navigationBar_mustSwitchApplicationBarsByNavigationMode();
    void navigationPropertiesFrame_mustComposeSeparatedChildFrames();
    void navigationChildFrames_mustBindPanelViewModelContracts();
    void navigationSelectionBars_mustUseContextMenuCombos();
    void navigationApplicationControlBar_mustMatchFigmaChildOrder();
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
    QVERIFY(mainQml.contains(QStringLiteral(
        "readonly property real hierarchyToolbarSpacing: hierarchyToolbarCount > 1 ? 40 / (hierarchyToolbarCount - 1) : 0")));
    QVERIFY(mainQml.contains(QStringLiteral("sidebarHorizontalInset: applicationWindow.hierarchyHorizontalInset")));
    QVERIFY(bodyLayout.contains(QStringLiteral("property int sidebarHorizontalInset: 2")));
    QVERIFY(bodyLayout.contains(QStringLiteral("horizontalInset: hStack.sidebarHorizontalInset")));
    QVERIFY(hierarchySidebarLayout.contains(QStringLiteral("property int horizontalInset: LV.Theme.gap2")));
    QVERIFY(hierarchySidebarLayout.contains(QStringLiteral("horizontalInset: hierarchyView.horizontalInset")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("property int horizontalInset: LV.Theme.gap2")));
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
    const QString interactionController = readQml(QStringLiteral("MainWindowInteractionController.qml"));

    QVERIFY(!mainQml.isEmpty());
    QVERIFY(!interactionController.isEmpty());
    QVERIFY(interactionController.contains(QStringLiteral("function hasFocusedTextInput()")));
    QVERIFY(interactionController.contains(QStringLiteral("current.text !== undefined")));
    QVERIFY(interactionController.contains(QStringLiteral("current.cursorPosition !== undefined")));
    QVERIFY(interactionController.contains(QStringLiteral("current.selectedText !== undefined")));
    QVERIFY(interactionController.contains(QStringLiteral("function cycleNavigationModeFromShortcut()")));
    QVERIFY(mainQml.contains(QStringLiteral("sequence: \"Tab\"")));
    QVERIFY(mainQml.contains(QStringLiteral("context: Qt.ApplicationShortcut")));
    QVERIFY(mainQml.contains(QStringLiteral("enabled: !windowInteractions.hasFocusedTextInput()")));
    QVERIFY(interactionController.contains(
        QStringLiteral("interactionController.navigationModeViewModel.requestNextMode();")));
}

void NavigationQmlFramesTest::mainQml_mustBindNewShortcutForNoteCreation()
{
    const QString mainQml = readQml(QStringLiteral("Main.qml"));
    const QString interactionController = readQml(QStringLiteral("MainWindowInteractionController.qml"));

    QVERIFY(!mainQml.isEmpty());
    QVERIFY(!interactionController.isEmpty());
    QVERIFY(mainQml.contains(QStringLiteral("readonly property int libraryHierarchyIndex: 0")));
    QVERIFY(interactionController.contains(QStringLiteral("property int libraryHierarchyIndex: 0")));
    QVERIFY(interactionController.contains(QStringLiteral("property var libraryHierarchyViewModel: null")));
    QVERIFY(interactionController.contains(QStringLiteral("property var sidebarHierarchyViewModel: null")));
    QVERIFY(interactionController.contains(QStringLiteral("function createNoteFromShortcut()")));
    QVERIFY(interactionController.contains(QStringLiteral(
        "interactionController.sidebarHierarchyViewModel.setActiveHierarchyIndex(interactionController.libraryHierarchyIndex);")));
    QVERIFY(interactionController.contains(QStringLiteral(
        "interactionController.libraryHierarchyViewModel.createEmptyNote()")));
    QVERIFY(mainQml.contains(QStringLiteral("sequence: StandardKey.New")));
    QVERIFY(mainQml.contains(QStringLiteral("context: Qt.ApplicationShortcut")));
    QVERIFY(mainQml.contains(QStringLiteral("onActivated: windowInteractions.createNoteFromShortcut()")));
    QVERIFY(mainQml.contains(QStringLiteral("libraryHierarchyIndex: applicationWindow.libraryHierarchyIndex")));
    QVERIFY(mainQml.contains(QStringLiteral("libraryHierarchyViewModel: applicationWindow.libraryHierarchyVm")));
    QVERIFY(mainQml.contains(QStringLiteral("sidebarHierarchyViewModel: applicationWindow.sidebarHierarchyVm")));
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
        QStringLiteral("view/panels/navigation/view/NavigationApplicationViewBar.qml"));
    const QString applicationEditBar = readQml(
        QStringLiteral("view/panels/navigation/edit/NavigationApplicationEditBar.qml"));

    QVERIFY(!navigationBarLayout.isEmpty());
    QVERIFY(!applicationViewBar.isEmpty());
    QVERIFY(!applicationEditBar.isEmpty());
    QVERIFY(navigationBarLayout.contains(QStringLiteral("readonly property string activeNavigationModeName")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral("NavigationViewMode.NavigationApplicationViewBar {")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral("NavigationEditMode.NavigationApplicationEditBar {")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral("NavigationControlMode.NavigationApplicationControlBar {")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("NavigationShared.NavigationPreferenceBar {")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("NavigationShared.NavigationPreferenceBar {")));
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
    const QString preferenceBar = readQml(QStringLiteral("view/panels/navigation/NavigationPreferenceBar.qml"));
    const QString applicationViewBar = readQml(
        QStringLiteral("view/panels/navigation/view/NavigationApplicationViewBar.qml"));
    const QString applicationEditBar = readQml(
        QStringLiteral("view/panels/navigation/edit/NavigationApplicationEditBar.qml"));
    const QString applicationControlBar = readQml(
        QStringLiteral("view/panels/navigation/control/NavigationApplicationControlBar.qml"));

    QVERIFY(propertiesBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationPropertiesBar\")")));
    QVERIFY(informationBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationInformationBar\")")));
    QVERIFY(modeBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationModeBar\")")));
    QVERIFY(editorViewBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationEditorViewBar\")")));
    QVERIFY(preferenceBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationPreferenceBar\")")));
    QVERIFY(applicationViewBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationApplicationViewBar\")")));
    QVERIFY(applicationEditBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationApplicationEditBar\")")));
    QVERIFY(applicationControlBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationApplicationControlBar\")")));
    QVERIFY(preferenceBar.contains(QStringLiteral("iconName: \"columnIndex\"")));
    QVERIFY(preferenceBar.contains(QStringLiteral("rotation: 180")));
    QVERIFY(preferenceBar.contains(QStringLiteral("transformOrigin: Item.Center")));
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
    QVERIFY(modeBar.contains(QStringLiteral("keyVisible: false")));
    QVERIFY(!modeBar.contains(QStringLiteral("requestNextMode();")));
    QVERIFY(!modeBar.contains(QStringLiteral("comboLabelRightInset")));
    QVERIFY(!modeBar.contains(QStringLiteral("resolvedBackgroundColor")));

    QVERIFY(editorViewBar.contains(QStringLiteral("LV.ComboBox {")));
    QVERIFY(editorViewBar.contains(QStringLiteral("text: editorViewBar.activeViewText")));
    QVERIFY(editorViewBar.contains(QStringLiteral("LV.ContextMenu {")));
    QVERIFY(editorViewBar.contains(QStringLiteral("requestViewModeChange(index)")));
    QVERIFY(editorViewBar.contains(QStringLiteral("keyVisible: false")));
    QVERIFY(editorViewBar.contains(QStringLiteral("label: \"Presentation\"")));
    QVERIFY(!editorViewBar.contains(QStringLiteral("requestNextViewMode();")));
    QVERIFY(!editorViewBar.contains(QStringLiteral("comboLabelRightInset")));
    QVERIFY(!editorViewBar.contains(QStringLiteral("resolvedBackgroundColor")));
}

void NavigationQmlFramesTest::navigationApplicationControlBar_mustMatchFigmaChildOrder()
{
    const QString applicationControlBar = readQml(
        QStringLiteral("view/panels/navigation/control/NavigationApplicationControlBar.qml"));

    QVERIFY(!applicationControlBar.isEmpty());

    const int appControlIndex = applicationControlBar.indexOf(QStringLiteral("NavigationAppControlBar {"));
    const int exportIndex = applicationControlBar.indexOf(QStringLiteral("NavigationExportBar {"));
    const int addNewIndex = applicationControlBar.indexOf(QStringLiteral("NavigationAddNewBar {"));
    const int preferenceIndex = applicationControlBar.indexOf(QStringLiteral("NavigationPreferenceBar {"));

    QVERIFY(appControlIndex >= 0);
    QVERIFY(exportIndex >= 0);
    QVERIFY(addNewIndex >= 0);
    QVERIFY(preferenceIndex >= 0);

    QVERIFY(appControlIndex < exportIndex);
    QVERIFY(exportIndex < addNewIndex);
    QVERIFY(addNewIndex < preferenceIndex);
    QVERIFY(applicationControlBar.contains(QStringLiteral("LV.IconMenuButton {")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("iconName: \"generalprojectStructure\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"iconName\": \"generalprojectStructure\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"iconName\": \"pin\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"iconName\": \"toolwindownotifications\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"iconName\": \"startTimer\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"iconName\": \"generalupload\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"iconName\": \"generalprint\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"iconName\": \"mailer\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"iconName\": \"addFile\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"iconName\": \"audioToAudio\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"iconName\": \"columnIndex\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"label\": \"Show Structure\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"label\": \"Pin Window\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"label\": \"Alerts\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"label\": \"Timer\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"label\": \"Export\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"label\": \"Print\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"label\": \"Mail\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"label\": \"New File\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"label\": \"Preferences\"")));
    QVERIFY(applicationControlBar.contains(
        QStringLiteral("applicationControlBar.detailPanelCollapsed ? \"Show Detail Panel\" : \"Hide Detail Panel\"")));
    QVERIFY(!applicationControlBar.contains(QStringLiteral("iconName: \"generalsearch\"")));
    QVERIFY(!applicationControlBar.contains(QStringLiteral("NavigationCalendarBar {")));
    QVERIFY(!applicationControlBar.contains(QStringLiteral("\"label\": \"Sync Files\"")));
    QVERIFY(!applicationControlBar.contains(QStringLiteral("\"label\": \"Todo List\"")));
    QVERIFY(!applicationControlBar.contains(QStringLiteral("\"label\": \"Daily\"")));
    QVERIFY(!applicationControlBar.contains(QStringLiteral("\"label\": \"Weekly\"")));
    QVERIFY(!applicationControlBar.contains(QStringLiteral("\"label\": \"Monthly\"")));
    QVERIFY(!applicationControlBar.contains(QStringLiteral("\"label\": \"Yearly\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"keyVisible\": false")));
    QVERIFY(applicationControlBar.contains(
        QStringLiteral("applicationControlMenuButton.width")));
    QVERIFY(applicationControlBar.contains(
        QStringLiteral("applicationControlMenuButton.height + applicationControlBar.menuYOffset")));
    QVERIFY(!applicationControlBar.contains(QStringLiteral("openFor(applicationControlMenuButton, 0,")));
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
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("LV.ListFooter {")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("toolbarItems: []")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("spacing: sidebarHierarchyView.toolbarButtonSpacing")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("width: 78")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("anchors.bottomMargin: sidebarHierarchyView.verticalInset")));
    QVERIFY(sidebarHierarchyView.contains(
        QStringLiteral("anchors.bottomMargin: sidebarHierarchyView.verticalInset + (sidebarHierarchyView.footerVisible ? hierarchyFooter.implicitHeight : 0)")));
}

QTEST_APPLESS_MAIN(NavigationQmlFramesTest)

#include "test_navigation_qml_frames.moc"
