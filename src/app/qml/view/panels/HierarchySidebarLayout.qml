import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: hierarchyView

    readonly property int activeToolbarIndex: hierarchyView.hierarchyStore ? hierarchyView.hierarchyStore.activeIndex : -1
    readonly property int availableContentWidth: Math.max(0, width - horizontalInset * 2)
    readonly property int contentWidth: Math.max(minContentWidth, availableContentWidth)
    readonly property int footerHeight: 24
    readonly property int footerWidth: 84
    readonly property var hierarchyStore: (typeof sidebarHierarchyStore !== "undefined" && sidebarHierarchyStore) ? sidebarHierarchyStore : null
    readonly property int horizontalInset: (typeof LV.Theme.gap8 === "number" && isFinite(LV.Theme.gap8)) ? LV.Theme.gap8 : 8
    readonly property int minContentWidth: Math.max(152, toolbarMinWidth)
    property color panelColor: LV.Theme.panelBackground04
    readonly property int rowHeight: 28
    readonly property int searchHeight: (typeof LV.Theme.gap18 === "number" && isFinite(LV.Theme.gap18)) ? LV.Theme.gap18 : 18
    readonly property int toolbarButtonSize: (typeof LV.Theme.gap20 === "number" && isFinite(LV.Theme.gap20)) ? LV.Theme.gap20 : 20
    readonly property int toolbarCount: toolbarIconNames.length
    readonly property var toolbarIconNames: hierarchyView.hierarchyStore ? hierarchyView.hierarchyStore.toolbarIconNames : []
    readonly property var toolbarItems: {
        var items = [];
        for (var i = 0; i < toolbarIconNames.length; ++i)
            items.push({
                "id": i,
                "iconName": toolbarIconNames[i],
                "selected": i === hierarchyView.activeToolbarIndex
            });
        return items;
    }
    readonly property int toolbarMinWidth: toolbarCount > 0 ? toolbarCount * toolbarButtonSize + (toolbarCount - 1) * toolbarSpacing : toolbarButtonSize
    readonly property int toolbarSpacing: (typeof LV.Theme.gap2 === "number" && isFinite(LV.Theme.gap2)) ? LV.Theme.gap2 : 2
    readonly property int verticalInset: (typeof LV.Theme.gap2 === "number" && isFinite(LV.Theme.gap2)) ? LV.Theme.gap2 : 2

    clip: true

    Rectangle {
        anchors.fill: parent
        color: hierarchyView.panelColor
    }
    Item {
        id: hierarchyContents

        anchors.bottom: hierarchyFooter.top
        anchors.left: parent.left
        anchors.leftMargin: hierarchyView.horizontalInset
        anchors.right: parent.right
        anchors.rightMargin: hierarchyView.horizontalInset
        anchors.top: parent.top
        anchors.topMargin: hierarchyView.verticalInset

        LV.VStack {
            anchors.fill: parent
            spacing: LV.Theme.gap2

            LV.HierarchyToolbar {
                id: hierarchyHeaderToolbar

                Layout.alignment: Qt.AlignLeft
                Layout.maximumWidth: hierarchyView.minContentWidth
                Layout.minimumWidth: hierarchyView.minContentWidth
                Layout.preferredHeight: hierarchyView.toolbarButtonSize
                Layout.preferredWidth: hierarchyView.minContentWidth
                activeButtonId: hierarchyView.activeToolbarIndex
                backgroundColor: "transparent"
                backgroundOpacity: 1.0
                buttonItems: hierarchyView.toolbarItems
                horizontalPadding: 0
                spacing: hierarchyView.toolbarSpacing
                verticalPadding: 0

                onActiveChanged: function (button, buttonId, index) {
                    if (index >= 0 && hierarchyView.hierarchyStore && hierarchyView.hierarchyStore.activeIndex !== index)
                        hierarchyView.hierarchyStore.activeIndex = index;
                }
            }
            LV.InputField {
                id: searchBar

                Layout.fillWidth: true
                Layout.preferredHeight: hierarchyView.searchHeight
                mode: searchMode
            }
            Flickable {
                id: hierarchyViewport

                Layout.fillHeight: true
                Layout.fillWidth: true
                boundsBehavior: Flickable.StopAtBounds
                clip: true
                contentHeight: hierarchyList.implicitHeight
                contentWidth: width
                interactive: contentHeight > height

                LV.HierarchyList {
                    id: hierarchyList

                    keyboardNavigationEnabled: false
                    rowSpacing: 0
                    width: hierarchyViewport.width

                    Repeater {
                        model: hierarchyView.hierarchyStore ? hierarchyView.hierarchyStore.itemModel : null

                        delegate: LV.HierarchyItem {
                            baseLeftPadding: LV.Theme.gap8
                            chevronColor: LV.Theme.darkGrey10
                            chevronSize: LV.Theme.iconSm
                            expanded: model.expanded
                            iconPlaceholderColor: model.accent ? LV.Theme.accentYellow : LV.Theme.accentGrayLight
                            iconSize: LV.Theme.iconSm
                            indentLevel: model.indentLevel
                            indentStep: 13
                            itemWidth: hierarchyViewport.width
                            label: model.label
                            leadingSpacing: LV.Theme.gap2
                            rowBackgroundColor: "transparent"
                            rowBackgroundColorHover: "transparent"
                            rowBackgroundColorPressed: "transparent"
                            rowHeight: hierarchyView.rowHeight
                            rowRightPadding: LV.Theme.gap8
                            showChevron: model.showChevron
                            textColorNormal: model.accent ? LV.Theme.accentBlue : LV.Theme.bodyColor
                        }
                    }
                }
            }
        }
    }
    LV.ListFooter {
        id: hierarchyFooter

        anchors.bottom: parent.bottom
        anchors.bottomMargin: hierarchyView.verticalInset
        anchors.left: parent.left
        anchors.leftMargin: hierarchyView.horizontalInset
        button1: ({
                "type": "icon",
                "iconName": "addFile",
                "tone": LV.AbstractButton.Borderless
            })
        button2: ({
                "type": "icon",
                "iconName": "generaldelete",
                "tone": LV.AbstractButton.Borderless
            })
        button3: ({
                "type": "menu",
                "iconName": "settings",
                "tone": LV.AbstractButton.Default,
                "backgroundColor": LV.Theme.panelBackground12,
                "cornerRadius": LV.Theme.radiusSm
            })
        height: hierarchyView.footerHeight
        horizontalPadding: LV.Theme.gap2
        verticalPadding: LV.Theme.gap2
        width: Math.min(hierarchyView.footerWidth, hierarchyView.contentWidth)
    }
}
