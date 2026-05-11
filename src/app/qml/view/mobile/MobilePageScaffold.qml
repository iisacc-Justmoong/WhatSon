import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import "../panels" as PanelView

Rectangle {
    id: mobilePageScaffold

    readonly property var activePageRouter: bodyRouter
    readonly property var bodyItem: bodyRouter.currentPageItem
    readonly property real bodyWidth: bodyRouter.width
    property string bodyInitialPath: "/"
    property var bodyRoutes: []
    readonly property int contentBottomPadding: LV.Theme.gap16
    readonly property int contentHorizontalPadding: LV.Theme.gap16
    readonly property int contentTopPadding: LV.Theme.gap24 + LV.Theme.gap24
    readonly property int sectionSpacing: LV.Theme.gap2
    property color canvasColor: LV.Theme.panelBackground01
    property bool compactAddFolderVisible: true
    property bool compactDetailPanelVisible: false
    property bool compactEditorViewVisible: false
    property string compactLeadingActionIconName: "generalchevronLeft"
    property bool compactLeadingActionVisible: false
    property bool compactNoteListControlsVisible: false
    property bool compactSettingsVisible: true
    property color controlSurfaceColor: LV.Theme.panelBackground10
    property bool detailPanelCollapsed: true
    property var editorViewModeController: null
    property var navigationModeController: null
    property string statusPlaceholderText: ""
    property string statusSearchText: ""
    property var windowInteractions: null

    signal compactAddFolderRequested
    signal compactLeadingActionRequested
    signal createNoteRequested
    signal agendaRequested
    signal dayCalendarRequested
    signal monthCalendarRequested
    signal statusSearchSubmitted(string text)
    signal statusSearchTextEdited(string text)
    signal toggleDetailPanelRequested
    signal viewHookRequested
    signal weekCalendarRequested
    signal yearCalendarRequested

    function requestViewHook() {
        viewHookRequested();
    }

    color: mobilePageScaffold.canvasColor

    ColumnLayout {
        anchors.bottomMargin: mobilePageScaffold.contentBottomPadding
        anchors.fill: parent
        anchors.leftMargin: mobilePageScaffold.contentHorizontalPadding
        anchors.rightMargin: mobilePageScaffold.contentHorizontalPadding
        anchors.topMargin: mobilePageScaffold.contentTopPadding
        spacing: mobilePageScaffold.sectionSpacing

        PanelView.NavigationBarLayout {
            compactAddFolderVisible: mobilePageScaffold.compactAddFolderVisible
            compactDetailPanelVisible: mobilePageScaffold.compactDetailPanelVisible
            compactEditorViewVisible: mobilePageScaffold.compactEditorViewVisible
            compactLeadingActionIconName: mobilePageScaffold.compactLeadingActionIconName
            compactLeadingActionVisible: mobilePageScaffold.compactLeadingActionVisible
            compactMode: true
            compactNoteListControlsVisible: mobilePageScaffold.compactNoteListControlsVisible
            compactSettingsVisible: mobilePageScaffold.compactSettingsVisible
            compactSurfaceColor: mobilePageScaffold.controlSurfaceColor
            detailPanelCollapsed: mobilePageScaffold.detailPanelCollapsed
            editorViewModeController: mobilePageScaffold.editorViewModeController
            navigationModeController: mobilePageScaffold.navigationModeController

            onCompactAddFolderRequested: mobilePageScaffold.compactAddFolderRequested()
            onCompactLeadingActionRequested: mobilePageScaffold.compactLeadingActionRequested()
            onAgendaRequested: mobilePageScaffold.agendaRequested()
            onDayCalendarRequested: mobilePageScaffold.dayCalendarRequested()
            onMonthCalendarRequested: mobilePageScaffold.monthCalendarRequested()
            onToggleDetailPanelRequested: mobilePageScaffold.toggleDetailPanelRequested()
            onViewHookRequested: mobilePageScaffold.requestViewHook()
            onWeekCalendarRequested: mobilePageScaffold.weekCalendarRequested()
            onYearCalendarRequested: mobilePageScaffold.yearCalendarRequested()
        }
        LV.PageRouter {
            id: bodyRouter

            Layout.fillHeight: true
            Layout.fillWidth: true
            initialPath: mobilePageScaffold.bodyInitialPath
            registerAsGlobalNavigator: false
            routes: mobilePageScaffold.bodyRoutes
        }
        PanelView.StatusBarLayout {
            compactFieldColor: mobilePageScaffold.controlSurfaceColor
            compactMode: true
            compactToolbarText: mobilePageScaffold.statusPlaceholderText
            searchText: mobilePageScaffold.statusSearchText

            onCreateNoteRequested: mobilePageScaffold.createNoteRequested()
            onSearchSubmitted: function (text) {
                mobilePageScaffold.statusSearchText = text;
                mobilePageScaffold.statusSearchSubmitted(text);
            }
            onSearchTextEdited: function (text) {
                mobilePageScaffold.statusSearchText = text;
                mobilePageScaffold.statusSearchTextEdited(text);
            }
            onViewHookRequested: mobilePageScaffold.requestViewHook()
        }
    }
}
