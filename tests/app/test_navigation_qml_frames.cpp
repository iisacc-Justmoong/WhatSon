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
    void mainQml_ios_mustForceEdgeToEdgeFullWindowArea();
    void mainQml_mustNotExposeGlobalResourceDropSurface();
    void macNativeMenuBar_mustExposeResourceImportAction();
    void navigationBar_mustComposePropertiesFrame();
    void navigationBar_mustSwitchApplicationBarsByNavigationMode();
    void navigationPropertiesFrame_mustComposeSeparatedChildFrames();
    void navigationChildFrames_mustBindPanelViewModelContracts();
    void navigationSelectionBars_mustUseContextMenuCombos();
    void navigationApplicationControlBar_mustMatchFigmaChildOrder();
    void navigationApplicationViewAndEditBars_mustMatchFigmaChildOrder();
    void navigationYearCalendar_mustOpenContentOverlay();
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
        QStringLiteral("interactionController.resolveOwnedWritableViewModel(")));
    QVERIFY(interactionController.contains(
        QStringLiteral("interactionController.navigationModeViewId,")));
    QVERIFY(interactionController.contains(QStringLiteral("navigationModeViewModel.requestNextMode();")));
}

void NavigationQmlFramesTest::mainQml_mustBindNewShortcutForNoteCreation()
{
    const QString mainQml = readQml(QStringLiteral("Main.qml"));
    const QString interactionController = readQml(QStringLiteral("MainWindowInteractionController.qml"));

    QVERIFY(!mainQml.isEmpty());
    QVERIFY(!interactionController.isEmpty());
    QVERIFY(mainQml.contains(QStringLiteral("readonly property int libraryHierarchyIndex: 0")));
    QVERIFY(interactionController.contains(QStringLiteral("property int libraryHierarchyIndex: 0")));
    QVERIFY(interactionController.contains(QStringLiteral("property string libraryNoteMutationViewId: \"\"")));
    QVERIFY(interactionController.contains(QStringLiteral("property string sidebarHierarchyViewId: \"\"")));
    QVERIFY(interactionController.contains(QStringLiteral("property string navigationModeViewId: \"\"")));
    QVERIFY(interactionController.contains(QStringLiteral("readonly property string addNewPanelKey: \"navigation.NavigationAddNewBar\"")));
    QVERIFY(interactionController.contains(QStringLiteral("readonly property string libraryNoteMutationViewModelKey: \"libraryNoteMutationViewModel\"")));
    QVERIFY(interactionController.contains(QStringLiteral("readonly property string navigationModeViewModelKey: \"navigationModeViewModel\"")));
    QVERIFY(interactionController.contains(QStringLiteral("readonly property string sidebarHierarchyViewModelKey: \"sidebarHierarchyViewModel\"")));
    QVERIFY(interactionController.contains(QStringLiteral("function createNoteFromShortcut()")));
    QVERIFY(interactionController.contains(QStringLiteral("function resolveOwnedWritableViewModel(viewId, viewModelKey)")));
    QVERIFY(interactionController.contains(QStringLiteral("function resolvePanelViewModel(panelKey)")));
    QVERIFY(interactionController.contains(QStringLiteral("function resolveLibraryNoteCreationViewModel()")));
    QVERIFY(interactionController.contains(QStringLiteral(
        "sidebarHierarchyViewModel.setActiveHierarchyIndex(interactionController.libraryHierarchyIndex);")));
    QVERIFY(interactionController.contains(QStringLiteral(
        "noteMutationViewModel.createEmptyNote()")));
    QVERIFY(interactionController.contains(QStringLiteral("interactionController.resolveLibraryNoteCreationViewModel()")));
    QVERIFY(interactionController.contains(QStringLiteral("interactionController.resolvePanelViewModel(interactionController.addNewPanelKey)")));
    QVERIFY(mainQml.contains(QStringLiteral("sequence: StandardKey.New")));
    QVERIFY(mainQml.contains(QStringLiteral("context: Qt.ApplicationShortcut")));
    QVERIFY(mainQml.contains(QStringLiteral("enabled: applicationWindow.isDesktopPlatform")));
    QVERIFY(mainQml.contains(QStringLiteral("onActivated: windowInteractions.createNoteFromShortcut()")));
    QVERIFY(mainQml.contains(QStringLiteral("libraryNoteMutationViewId: applicationWindow.libraryNoteMutationViewId")));
    QVERIFY(mainQml.contains(QStringLiteral("navigationModeViewId: applicationWindow.navigationModeViewId")));
    QVERIFY(mainQml.contains(QStringLiteral("sidebarHierarchyViewId: applicationWindow.sidebarHierarchyViewId")));
    QVERIFY(mainQml.contains(QStringLiteral("libraryHierarchyIndex: applicationWindow.libraryHierarchyIndex")));
    QVERIFY(mainQml.contains(QStringLiteral("readonly property var registeredViewModelKeys: LV.ViewModels.keys")));
    QVERIFY(mainQml.contains(QStringLiteral("return LV.ViewModels.get(\"libraryHierarchyViewModel\");")));
    QVERIFY(mainQml.contains(QStringLiteral("return LV.ViewModels.get(\"libraryNoteMutationViewModel\");")));
    QVERIFY(mainQml.contains(QStringLiteral("return LV.ViewModels.get(\"sidebarHierarchyViewModel\");")));
    QVERIFY(mainQml.contains(QStringLiteral("bindOwnedViewModel(applicationWindow.libraryNoteMutationViewId, \"libraryNoteMutationViewModel\")")));
}

void NavigationQmlFramesTest::mainQml_ios_mustForceEdgeToEdgeFullWindowArea()
{
    const QString mainQml = readQml(QStringLiteral("Main.qml"));

    QVERIFY(!mainQml.isEmpty());
    QVERIFY(mainQml.contains(QStringLiteral("delegateMobileInsetsToSystem: false")));
    QVERIFY(mainQml.contains(QStringLiteral("delegateMobileWindowingToSystem: false")));
    QVERIFY(mainQml.contains(QStringLiteral("forceFullWindowAreaOnMobile: applicationWindow.isMobilePlatform")));
    QVERIFY(mainQml.contains(QStringLiteral("usePlatformSafeMargin: false")));
    QVERIFY(!mainQml.contains(QStringLiteral("readonly property bool useIosSystemWindowingPolicy")));
}

void NavigationQmlFramesTest::mainQml_mustNotExposeGlobalResourceDropSurface()
{
    const QString mainQml = readQml(QStringLiteral("Main.qml"));

    QVERIFY(!mainQml.isEmpty());
    QVERIFY(!mainQml.contains(QStringLiteral("rootResourcesImportViewModel")));
    QVERIFY(!mainQml.contains(QStringLiteral("resourceImportDropActive")));
    QVERIFY(!mainQml.contains(QStringLiteral("resourceImportStatusText")));
    QVERIFY(!mainQml.contains(QStringLiteral("showResourceImportStatus(")));
    QVERIFY(!mainQml.contains(QStringLiteral("canAcceptResourceDrop(")));
    QVERIFY(!mainQml.contains(QStringLiteral("droppedUrls(")));
    QVERIFY(!mainQml.contains(QStringLiteral("LV.ViewModels.set(\"resourcesImportViewModel\", resourcesImportViewModel);")));
    QVERIFY(!mainQml.contains(QStringLiteral("Drop files to import resources")));
}

void NavigationQmlFramesTest::macNativeMenuBar_mustExposeResourceImportAction()
{
    const QString mainQml = readQml(QStringLiteral("Main.qml"));
    const QString nativeMenuBar = readQml(QStringLiteral("window/MacNativeMenuBar.qml"));

    QVERIFY(!mainQml.isEmpty());
    QVERIFY(!nativeMenuBar.isEmpty());
    QVERIFY(mainQml.contains(QStringLiteral("item.resourcesImportViewModel = resourcesImportViewModel")));
    QVERIFY(nativeMenuBar.contains(QStringLiteral("import QtQuick.Dialogs")));
    QVERIFY(nativeMenuBar.contains(QStringLiteral("property var resourcesImportViewModel: null")));
    QVERIFY(nativeMenuBar.contains(QStringLiteral("text: qsTr(\"Import File...\")")));
    QVERIFY(nativeMenuBar.contains(QStringLiteral("fileMode: FileDialog.OpenFiles")));
    QVERIFY(nativeMenuBar.contains(QStringLiteral("title: qsTr(\"Import Resource Files\")")));
    QVERIFY(nativeMenuBar.contains(QStringLiteral("root.resourcesImportViewModel.importUrls(selectedFiles)")));
    QVERIFY(nativeMenuBar.contains(QStringLiteral("importFailureDialog.open()")));
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
    const QString applicationCalendarBar = readQml(
        QStringLiteral("view/panels/navigation/NavigationApplicationCalendarBar.qml"));
    const QString applicationPreferenceBar = readQml(
        QStringLiteral("view/panels/navigation/NavigationApplicationPreferenceBar.qml"));

    QVERIFY(!navigationBarLayout.isEmpty());
    QVERIFY(!applicationViewBar.isEmpty());
    QVERIFY(!applicationEditBar.isEmpty());
    QVERIFY(!applicationCalendarBar.isEmpty());
    QVERIFY(!applicationPreferenceBar.isEmpty());
    QVERIFY(navigationBarLayout.contains(QStringLiteral("readonly property string activeNavigationModeName")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral("NavigationViewMode.NavigationApplicationViewBar {")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral("NavigationEditMode.NavigationApplicationEditBar {")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral("NavigationControlMode.NavigationApplicationControlBar {")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("NavigationShared.NavigationApplicationPreferenceBar {")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("NavigationShared.NavigationApplicationPreferenceBar {")));
    QVERIFY(applicationCalendarBar.contains(QStringLiteral("NavigationCalendarBar {")));
    QVERIFY(applicationPreferenceBar.contains(QStringLiteral("NavigationPreferenceBar {")));
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
    QVERIFY(modeBar.contains(QStringLiteral("iconName: \"generalshow\"")));
    QVERIFY(modeBar.contains(QStringLiteral("iconName: \"renameColumn\"")));
    QVERIFY(modeBar.contains(QStringLiteral("iconName: \"abstractClass\"")));
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
    QVERIFY(applicationControlBar.contains(QStringLiteral("property bool compactNoteListControlsVisible: false")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("iconName: \"toolwindowtodo\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("iconName: \"sortByType\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("iconName: \"cwmPermissionView\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"iconName\": \"toolwindowtodo\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"iconName\": \"sortByType\"")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("\"iconName\": \"cwmPermissionView\"")));
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
    QVERIFY(applicationControlBar.contains(QStringLiteral("compact-note-list-sort")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("compact-note-list-visibility")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("if (applicationControlBar.compactNoteListControlsVisible)")));
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
    QVERIFY(applicationControlBar.contains(QStringLiteral("leftPadding: LV.Theme.gap2")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("rightPadding: LV.Theme.gap4")));
    QVERIFY(applicationControlBar.contains(QStringLiteral("spacing: LV.Theme.gapNone")));
    QVERIFY(applicationControlBar.contains(
        QStringLiteral("applicationControlMenuButton.width")));
    QVERIFY(applicationControlBar.contains(
        QStringLiteral("applicationControlMenuButton.height + applicationControlBar.menuYOffset")));
    QVERIFY(applicationControlBar.contains(
        QStringLiteral("noteListApplicationControlMenuButton.height + applicationControlBar.menuYOffset")));
    QVERIFY(!applicationControlBar.contains(QStringLiteral("openFor(applicationControlMenuButton, 0,")));
}

void NavigationQmlFramesTest::navigationApplicationViewAndEditBars_mustMatchFigmaChildOrder()
{
    const QString applicationViewBar = readQml(
        QStringLiteral("view/panels/navigation/view/NavigationApplicationViewBar.qml"));
    const QString applicationEditBar = readQml(
        QStringLiteral("view/panels/navigation/edit/NavigationApplicationEditBar.qml"));
    const QString applicationCalendarBar = readQml(
        QStringLiteral("view/panels/navigation/NavigationApplicationCalendarBar.qml"));
    const QString applicationAddNewBar = readQml(
        QStringLiteral("view/panels/navigation/NavigationApplicationAddNewBar.qml"));
    const QString applicationPreferenceBar = readQml(
        QStringLiteral("view/panels/navigation/NavigationApplicationPreferenceBar.qml"));

    QVERIFY(!applicationViewBar.isEmpty());
    QVERIFY(!applicationEditBar.isEmpty());
    QVERIFY(!applicationCalendarBar.isEmpty());
    QVERIFY(!applicationAddNewBar.isEmpty());
    QVERIFY(!applicationPreferenceBar.isEmpty());

    const int viewCalendarIndex = applicationViewBar.indexOf(QStringLiteral("NavigationShared.NavigationApplicationCalendarBar {"));
    const int viewAddNewIndex = applicationViewBar.indexOf(QStringLiteral("NavigationShared.NavigationApplicationAddNewBar {"));
    const int viewPreferenceIndex = applicationViewBar.indexOf(QStringLiteral("NavigationShared.NavigationApplicationPreferenceBar {"));
    QVERIFY(viewCalendarIndex >= 0);
    QVERIFY(viewAddNewIndex >= 0);
    QVERIFY(viewPreferenceIndex >= 0);
    QVERIFY(viewCalendarIndex < viewAddNewIndex);
    QVERIFY(viewAddNewIndex < viewPreferenceIndex);
    QVERIFY(applicationViewBar.contains(QStringLiteral("LV.IconMenuButton {")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("id: applicationViewContextMenu")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("iconName: \"toolWindowCheckDetails\"")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("\"iconName\": \"toolWindowCheckDetails\"")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("\"iconName\": \"newUIlightThemeSelected\"")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("\"iconName\": \"table\"")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("\"iconName\": \"pnpm\"")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("\"iconName\": \"runshowCurrentFrame\"")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("\"iconName\": \"addFile\"")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("\"iconName\": \"audioToAudio\"")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("\"iconName\": \"columnIndex\"")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("\"label\": \"Todo List\"")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("\"label\": \"Daily Calendar\"")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("\"label\": \"Weekly Calendar\"")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("\"label\": \"Monthly Calendar\"")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("\"label\": \"Yearly Calendar\"")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("\"label\": \"New File\"")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("\"label\": \"Preferences\"")));
    QVERIFY(applicationViewBar.contains(
        QStringLiteral("applicationViewBar.detailPanelCollapsed ? \"Show Detail Panel\" : \"Hide Detail Panel\"")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("leftPadding: LV.Theme.gap2")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("rightPadding: LV.Theme.gap4")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("spacing: LV.Theme.gapNone")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("applicationViewMenuButton.width")));
    QVERIFY(applicationViewBar.contains(
        QStringLiteral("applicationViewMenuButton.height + applicationViewBar.menuYOffset")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("\"keyVisible\": false")));
    QVERIFY(!applicationViewBar.contains(QStringLiteral("NavigationAppControlBar {")));
    QVERIFY(!applicationViewBar.contains(QStringLiteral("NavigationExportBar {")));
    QVERIFY(applicationCalendarBar.contains(QStringLiteral("NavigationCalendarBar {")));
    QVERIFY(applicationAddNewBar.contains(QStringLiteral("NavigationAddNewBar {")));
    QVERIFY(applicationPreferenceBar.contains(QStringLiteral("NavigationPreferenceBar {")));

    const int editCalendarIndex = applicationEditBar.indexOf(QStringLiteral("NavigationShared.NavigationApplicationCalendarBar {"));
    const int editAddNewIndex = applicationEditBar.indexOf(QStringLiteral("NavigationShared.NavigationApplicationAddNewBar {"));
    const int editPreferenceIndex = applicationEditBar.indexOf(QStringLiteral("NavigationShared.NavigationApplicationPreferenceBar {"));
    QVERIFY(editCalendarIndex >= 0);
    QVERIFY(editAddNewIndex >= 0);
    QVERIFY(editPreferenceIndex >= 0);
    QVERIFY(editCalendarIndex < editAddNewIndex);
    QVERIFY(editAddNewIndex < editPreferenceIndex);
    QVERIFY(applicationEditBar.contains(QStringLiteral("LV.IconMenuButton {")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("id: applicationEditContextMenu")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("iconName: \"toolWindowCheckDetails\"")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("\"iconName\": \"toolWindowCheckDetails\"")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("\"iconName\": \"newUIlightThemeSelected\"")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("\"iconName\": \"table\"")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("\"iconName\": \"pnpm\"")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("\"iconName\": \"runshowCurrentFrame\"")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("\"iconName\": \"addFile\"")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("\"iconName\": \"audioToAudio\"")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("\"iconName\": \"columnIndex\"")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("\"label\": \"Todo List\"")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("\"label\": \"Daily Calendar\"")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("\"label\": \"Weekly Calendar\"")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("\"label\": \"Monthly Calendar\"")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("\"label\": \"Yearly Calendar\"")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("\"label\": \"New File\"")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("\"label\": \"Preferences\"")));
    QVERIFY(applicationEditBar.contains(
        QStringLiteral("applicationEditBar.detailPanelCollapsed ? \"Show Detail Panel\" : \"Hide Detail Panel\"")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("leftPadding: LV.Theme.gap2")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("rightPadding: LV.Theme.gap4")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("spacing: LV.Theme.gapNone")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("applicationEditMenuButton.width")));
    QVERIFY(applicationEditBar.contains(
        QStringLiteral("applicationEditMenuButton.height + applicationEditBar.menuYOffset")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("\"keyVisible\": false")));
    QVERIFY(!applicationEditBar.contains(QStringLiteral("NavigationAppControlBar {")));
    QVERIFY(!applicationEditBar.contains(QStringLiteral("NavigationExportBar {")));
    QVERIFY(applicationCalendarBar.contains(QStringLiteral("NavigationCalendarBar {")));
    QVERIFY(applicationAddNewBar.contains(QStringLiteral("NavigationAddNewBar {")));
    QVERIFY(applicationPreferenceBar.contains(QStringLiteral("NavigationPreferenceBar {")));
}

void NavigationQmlFramesTest::navigationYearCalendar_mustOpenContentOverlay()
{
    const QString mainQml = readQml(QStringLiteral("Main.qml"));
    const QString navigationBarLayout = readQml(QStringLiteral("view/panels/NavigationBarLayout.qml"));
    const QString applicationViewBar = readQml(
        QStringLiteral("view/panels/navigation/view/NavigationApplicationViewBar.qml"));
    const QString applicationEditBar = readQml(
        QStringLiteral("view/panels/navigation/edit/NavigationApplicationEditBar.qml"));
    const QString navigationCalendarBar = readQml(
        QStringLiteral("view/panels/navigation/NavigationCalendarBar.qml"));
    const QString bodyLayout = readQml(QStringLiteral("view/panels/BodyLayout.qml"));
    const QString contentViewLayout = readQml(QStringLiteral("view/panels/ContentViewLayout.qml"));
    const QString dayCalendarPage = readQml(QStringLiteral("view/calendar/DayCalendarPage.qml"));
    const QString weekCalendarPage = readQml(QStringLiteral("view/calendar/WeekCalendarPage.qml"));
    const QString mobileScaffold = readQml(QStringLiteral("view/mobile/MobilePageScaffold.qml"));
    const QString mobileHierarchyPage = readQml(QStringLiteral("view/mobile/pages/MobileHierarchyPage.qml"));

    QVERIFY(!mainQml.isEmpty());
    QVERIFY(!navigationBarLayout.isEmpty());
    QVERIFY(!applicationViewBar.isEmpty());
    QVERIFY(!applicationEditBar.isEmpty());
    QVERIFY(!navigationCalendarBar.isEmpty());
    QVERIFY(!bodyLayout.isEmpty());
    QVERIFY(!contentViewLayout.isEmpty());
    QVERIFY(!dayCalendarPage.isEmpty());
    QVERIFY(!weekCalendarPage.isEmpty());
    QVERIFY(!mobileScaffold.isEmpty());
    QVERIFY(!mobileHierarchyPage.isEmpty());

    QVERIFY(applicationViewBar.contains(QStringLiteral("signal viewHookRequested(string reason)")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("signal viewHookRequested(string reason)")));
    QVERIFY(applicationViewBar.contains(QStringLiteral("onViewHookRequested: function (reason)")));
    QVERIFY(applicationEditBar.contains(QStringLiteral("onViewHookRequested: function (reason)")));
    QVERIFY(navigationCalendarBar.contains(QStringLiteral("signal viewHookRequested(string reason)")));
    QVERIFY(navigationCalendarBar.contains(QStringLiteral(
        "onClicked: calendarBar.requestViewHook(\"open-daily-calendar\")")));
    QVERIFY(navigationCalendarBar.contains(QStringLiteral(
        "onClicked: calendarBar.requestViewHook(\"open-weekly-calendar\")")));
    QVERIFY(navigationCalendarBar.contains(QStringLiteral(
        "onClicked: calendarBar.requestViewHook(\"open-monthly-calendar\")")));
    QVERIFY(navigationCalendarBar.contains(QStringLiteral(
        "onClicked: calendarBar.requestViewHook(\"open-yearly-calendar\")")));

    QVERIFY(navigationBarLayout.contains(QStringLiteral("signal dayCalendarRequested")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral(
        "if (hookReason.indexOf(\"daily-calendar\") >= 0)")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral(
        "navigationBar.dayCalendarRequested();")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral("signal monthCalendarRequested")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral(
        "if (hookReason.indexOf(\"monthly-calendar\") >= 0)")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral(
        "navigationBar.monthCalendarRequested();")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral("signal weekCalendarRequested")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral(
        "if (hookReason.indexOf(\"weekly-calendar\") >= 0)")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral(
        "navigationBar.weekCalendarRequested();")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral("signal yearCalendarRequested")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral(
        "if (hookReason.indexOf(\"yearly-calendar\") >= 0)")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral(
        "navigationBar.yearCalendarRequested();")));

    QVERIFY(mainQml.contains(QStringLiteral("property bool dayCalendarOverlayVisible: false")));
    QVERIFY(mainQml.contains(QStringLiteral("property bool monthCalendarOverlayVisible: false")));
    QVERIFY(mainQml.contains(QStringLiteral("property bool weekCalendarOverlayVisible: false")));
    QVERIFY(mainQml.contains(QStringLiteral("property bool yearCalendarOverlayVisible: false")));
    QVERIFY(mainQml.contains(QStringLiteral(
        "onDayCalendarRequested: {")));
    QVERIFY(mainQml.contains(QStringLiteral(
        "applicationWindow.dayCalendarOverlayVisible = true;")));
    QVERIFY(mainQml.contains(QStringLiteral(
        "dayCalendarOverlayVisible: applicationWindow.dayCalendarOverlayVisible")));
    QVERIFY(mainQml.contains(QStringLiteral(
        "dayCalendarViewModel: applicationWindow.rootDayCalendarViewModel")));
    QVERIFY(mainQml.contains(QStringLiteral(
        "onDayCalendarOverlayDismissRequested: applicationWindow.dayCalendarOverlayVisible = false")));
    QVERIFY(mainQml.contains(QStringLiteral(
        "onMonthCalendarRequested: {")));
    QVERIFY(mainQml.contains(QStringLiteral(
        "applicationWindow.monthCalendarOverlayVisible = true;")));
    QVERIFY(mainQml.contains(QStringLiteral(
        "monthCalendarOverlayVisible: applicationWindow.monthCalendarOverlayVisible")));
    QVERIFY(mainQml.contains(QStringLiteral(
        "monthCalendarViewModel: applicationWindow.rootMonthCalendarViewModel")));
    QVERIFY(mainQml.contains(QStringLiteral(
        "onMonthCalendarOverlayDismissRequested: applicationWindow.monthCalendarOverlayVisible = false")));
    QVERIFY(mainQml.contains(QStringLiteral(
        "onWeekCalendarRequested: {")));
    QVERIFY(mainQml.contains(QStringLiteral(
        "applicationWindow.weekCalendarOverlayVisible = true;")));
    QVERIFY(mainQml.contains(QStringLiteral(
        "weekCalendarOverlayVisible: applicationWindow.weekCalendarOverlayVisible")));
    QVERIFY(mainQml.contains(QStringLiteral(
        "weekCalendarViewModel: applicationWindow.rootWeekCalendarViewModel")));
    QVERIFY(mainQml.contains(QStringLiteral(
        "onWeekCalendarOverlayDismissRequested: applicationWindow.weekCalendarOverlayVisible = false")));
    QVERIFY(mainQml.contains(QStringLiteral(
        "onYearCalendarRequested: {")));
    QVERIFY(mainQml.contains(QStringLiteral(
        "applicationWindow.yearCalendarOverlayVisible = true;")));
    QVERIFY(mainQml.contains(QStringLiteral(
        "yearCalendarOverlayVisible: applicationWindow.yearCalendarOverlayVisible")));
    QVERIFY(mainQml.contains(QStringLiteral(
        "yearCalendarViewModel: applicationWindow.rootYearCalendarViewModel")));
    QVERIFY(mainQml.contains(QStringLiteral(
        "onYearCalendarOverlayDismissRequested: applicationWindow.yearCalendarOverlayVisible = false")));

    QVERIFY(bodyLayout.contains(QStringLiteral("property bool dayCalendarOverlayVisible: false")));
    QVERIFY(bodyLayout.contains(QStringLiteral("property var dayCalendarViewModel: null")));
    QVERIFY(bodyLayout.contains(QStringLiteral("signal dayCalendarOverlayDismissRequested")));
    QVERIFY(bodyLayout.contains(QStringLiteral("property bool monthCalendarOverlayVisible: false")));
    QVERIFY(bodyLayout.contains(QStringLiteral("property var monthCalendarViewModel: null")));
    QVERIFY(bodyLayout.contains(QStringLiteral("signal monthCalendarOverlayDismissRequested")));
    QVERIFY(bodyLayout.contains(QStringLiteral("property bool weekCalendarOverlayVisible: false")));
    QVERIFY(bodyLayout.contains(QStringLiteral("property var weekCalendarViewModel: null")));
    QVERIFY(bodyLayout.contains(QStringLiteral("signal weekCalendarOverlayDismissRequested")));
    QVERIFY(bodyLayout.contains(QStringLiteral("property bool yearCalendarOverlayVisible: false")));
    QVERIFY(bodyLayout.contains(QStringLiteral("property var yearCalendarViewModel: null")));
    QVERIFY(bodyLayout.contains(QStringLiteral("signal yearCalendarOverlayDismissRequested")));
    QVERIFY(contentViewLayout.contains(QStringLiteral("import \"../calendar\" as CalendarView")));
    QVERIFY(contentViewLayout.contains(QStringLiteral("signal dayCalendarOverlayCloseRequested")));
    QVERIFY(contentViewLayout.contains(QStringLiteral("signal monthCalendarOverlayCloseRequested")));
    QVERIFY(contentViewLayout.contains(QStringLiteral("signal weekCalendarOverlayCloseRequested")));
    QVERIFY(contentViewLayout.contains(QStringLiteral("signal yearCalendarOverlayCloseRequested")));
    QVERIFY(contentViewLayout.contains(QStringLiteral("CalendarView.DayCalendarPage {")));
    QVERIFY(contentViewLayout.contains(QStringLiteral(
        "dayCalendarViewModel: contentViewLayout.dayCalendarViewModel")));
    QVERIFY(contentViewLayout.contains(QStringLiteral("CalendarView.MonthCalendarPage {")));
    QVERIFY(contentViewLayout.contains(QStringLiteral(
        "monthCalendarViewModel: contentViewLayout.monthCalendarViewModel")));
    QVERIFY(contentViewLayout.contains(QStringLiteral("CalendarView.WeekCalendarPage {")));
    QVERIFY(contentViewLayout.contains(QStringLiteral("weekCalendarViewModel: contentViewLayout.weekCalendarViewModel")));
    QVERIFY(contentViewLayout.contains(QStringLiteral("CalendarView.YearCalendarPage {")));
    QVERIFY(contentViewLayout.contains(QStringLiteral("yearCalendarViewModel: contentViewLayout.yearCalendarViewModel")));
    QVERIFY(dayCalendarPage.contains(QStringLiteral("text: \"Today\"")));
    QVERIFY(dayCalendarPage.contains(QStringLiteral("model: dayCalendarPage.timeSlots")));
    QVERIFY(dayCalendarPage.contains(QStringLiteral("requestViewHook(\"previous-day\")")));
    QVERIFY(dayCalendarPage.contains(QStringLiteral("requestViewHook(\"next-day\")")));
    QVERIFY(weekCalendarPage.contains(QStringLiteral("text: \"Today\"")));
    QVERIFY(weekCalendarPage.contains(QStringLiteral("function entriesForHour(dayModel, hour)")));
    QVERIFY(weekCalendarPage.contains(QStringLiteral("model: weekCalendarPage.hourSlots")));

    QVERIFY(mobileScaffold.contains(QStringLiteral("signal dayCalendarRequested")));
    QVERIFY(mobileScaffold.contains(QStringLiteral(
        "onDayCalendarRequested: mobilePageScaffold.dayCalendarRequested()")));
    QVERIFY(mobileHierarchyPage.contains(QStringLiteral("signal dayCalendarRequested")));
    QVERIFY(mobileHierarchyPage.contains(QStringLiteral(
        "signal dayCalendarOverlayDismissRequested")));
    QVERIFY(mobileHierarchyPage.contains(QStringLiteral(
        "dayCalendarOverlayVisible: mobileHierarchyPage.dayCalendarOverlayVisible")));
    QVERIFY(mobileScaffold.contains(QStringLiteral("signal monthCalendarRequested")));
    QVERIFY(mobileScaffold.contains(QStringLiteral(
        "onMonthCalendarRequested: mobilePageScaffold.monthCalendarRequested()")));
    QVERIFY(mobileHierarchyPage.contains(QStringLiteral("signal monthCalendarRequested")));
    QVERIFY(mobileHierarchyPage.contains(QStringLiteral(
        "signal monthCalendarOverlayDismissRequested")));
    QVERIFY(mobileHierarchyPage.contains(QStringLiteral(
        "monthCalendarOverlayVisible: mobileHierarchyPage.monthCalendarOverlayVisible")));
    QVERIFY(mobileScaffold.contains(QStringLiteral("signal weekCalendarRequested")));
    QVERIFY(mobileScaffold.contains(QStringLiteral(
        "onWeekCalendarRequested: mobilePageScaffold.weekCalendarRequested()")));
    QVERIFY(mobileHierarchyPage.contains(QStringLiteral("signal weekCalendarRequested")));
    QVERIFY(mobileHierarchyPage.contains(QStringLiteral(
        "signal weekCalendarOverlayDismissRequested")));
    QVERIFY(mobileHierarchyPage.contains(QStringLiteral(
        "weekCalendarOverlayVisible: mobileHierarchyPage.weekCalendarOverlayVisible")));
    QVERIFY(mobileScaffold.contains(QStringLiteral("signal yearCalendarRequested")));
    QVERIFY(mobileScaffold.contains(QStringLiteral(
        "onYearCalendarRequested: mobilePageScaffold.yearCalendarRequested()")));
    QVERIFY(mobileHierarchyPage.contains(QStringLiteral("signal yearCalendarRequested")));
    QVERIFY(mobileHierarchyPage.contains(QStringLiteral(
        "signal yearCalendarOverlayDismissRequested")));
    QVERIFY(mobileHierarchyPage.contains(QStringLiteral(
        "yearCalendarOverlayVisible: mobileHierarchyPage.yearCalendarOverlayVisible")));
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
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("LV.ContextMenu {")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("id: hierarchyViewOptionsMenu")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("items: sidebarHierarchyView.hierarchyViewOptionsMenuItems")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("hierarchyViewOptionsMenu.openFor(hierarchyFooter, hierarchyFooter.width, hierarchyFooter.height + 2);")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("\"label\": \"Expand All\"")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("\"label\": \"Collapse All\"")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("\"eventName\": \"hierarchy.expandAll\"")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("\"eventName\": \"hierarchy.collapseAll\"")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("toolbarItems: []")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("spacing: sidebarHierarchyView.toolbarButtonSpacing")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("width: 78")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("anchors.bottomMargin: sidebarHierarchyView.verticalInset")));
    QVERIFY(sidebarHierarchyView.contains(
        QStringLiteral("anchors.bottomMargin: sidebarHierarchyView.verticalInset + (sidebarHierarchyView.footerVisible ? hierarchyFooter.implicitHeight : 0)")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("sidebarHierarchyView.hierarchyInteractionBridge.setAllItemsExpanded(true)")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("sidebarHierarchyView.hierarchyInteractionBridge.setAllItemsExpanded(false)")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("function handleHierarchyViewOptionsTrigger(index, eventName)")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("normalizedEventName === \"hierarchy.expandAll\"")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("normalizedEventName === \"hierarchy.collapseAll\"")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("onItemEventTriggered: function (eventName, payload, index, item)")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("sidebarHierarchyView.handleHierarchyViewOptionsTrigger(index, eventName);")));
}

QTEST_APPLESS_MAIN(NavigationQmlFramesTest)

#include "test_navigation_qml_frames.moc"
