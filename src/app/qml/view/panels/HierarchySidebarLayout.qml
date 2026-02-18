import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: root

    readonly property int activeToolbarIndex: sidebarHierarchyStore.activeIndex
    readonly property int availableContentWidth: Math.max(0, width - horizontalInset * 2)
    readonly property int contentWidth: Math.max(minContentWidth, availableContentWidth)
    readonly property int footerHeight: 24
    readonly property int footerWidth: 84
    readonly property int horizontalInset: LV.Theme.gap8
    property color panelColor: LV.Theme.panelBackground04
    readonly property int rowHeight: 28
    readonly property int searchHeight: LV.Theme.gap18
    readonly property int toolbarButtonSize: LV.Theme.gap20
    readonly property int toolbarCount: toolbarIconNames.length
    readonly property var toolbarIconNames: (typeof sidebarHierarchyStore !== "undefined" && sidebarHierarchyStore && sidebarHierarchyStore.toolbarIconNames) ? sidebarHierarchyStore.toolbarIconNames : []
    readonly property var toolbarItems: {
        var items = [];
        for (var i = 0; i < toolbarIconNames.length; ++i)
            items.push({
                "id": i,
                "iconName": toolbarIconNames[i],
                "selected": i === root.activeToolbarIndex
            });
        return items;
    }
    readonly property int verticalInset: LV.Theme.gap2

    clip: true

    Rectangle {
        anchors.fill: parent
        color: root.panelColor
    }
    Item {
        id: hierarchyContents

        anchors.bottom: hierarchyFooter.top
        anchors.left: parent.left
        anchors.leftMargin: root.horizontalInset
        anchors.right: parent.right
        anchors.rightMargin: root.horizontalInset
        anchors.top: parent.top
        anchors.topMargin: root.verticalInset

        LV.VStack {
            anchors.fill: parent
            spacing: LV.Theme.gap2

            LV.HierarchyToolbar {
                Layout.alignment: Qt.AlignLeft
                Layout.maximumWidth: root.minContentWidth
                Layout.minimumWidth: root.minContentWidth
                Layout.preferredHeight: root.toolbarButtonSize
                Layout.preferredWidth: root.minContentWidth
                activeButtonId: root.activeToolbarIndex
                backgroundColor: "transparent"
                backgroundOpacity: 1.0
                buttonItems: root.toolbarItems
                horizontalPadding: 0
                spacing: root.toolbarSpacing
                verticalPadding: 0

                onActiveChanged: function (button, buttonId, index) {
                    if (index >= 0 && sidebarHierarchyStore.activeIndex !== index)
                        sidebarHierarchyStore.activeIndex = index;
                }
            }
            LV.InputField {
                Layout.fillWidth: true
                Layout.preferredHeight: root.searchHeight
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
                        model: sidebarHierarchyStore.itemModel

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
                            rowHeight: root.rowHeight
                            rowRightPadding: LV.Theme.gap8
                            showChevron: model.showChevron
                            textColorNormal: model.accent ? LV.Theme.accentBlue : LV.Theme.bodyColor
                        }
                    }
                }
            }
        }
    }
    Item {
        id: hierarchyFooter

        anchors.bottom: parent.bottom
        anchors.bottomMargin: root.verticalInset
        anchors.left: parent.left
        anchors.leftMargin: root.horizontalInset
        height: root.footerHeight
        width: Math.min(root.footerWidth, root.contentWidth)

        LV.ListFooter {
            anchors.fill: parent
            button1: ({
                    "type": "icon",
                    "iconName": "addFile",
                    "tone": LV.AbstractButton.Borderless,
                    "backgroundColor": "transparent",
                    "backgroundColorHover": LV.Theme.surfaceAlt,
                    "backgroundColorPressed": LV.Theme.surfaceAlt
                })
            button2: ({
                    "type": "icon",
                    "iconName": "delete",
                    "tone": LV.AbstractButton.Borderless,
                    "backgroundColor": "transparent",
                    "backgroundColorHover": LV.Theme.surfaceAlt,
                    "backgroundColorPressed": LV.Theme.surfaceAlt
                })
            button3: ({
                    "type": "menu",
                    "iconName": "settings",
                    "tone": LV.AbstractButton.Default,
                    "backgroundColor": LV.Theme.panelBackground12,
                    "backgroundColorHover": LV.Theme.panelBackground12,
                    "backgroundColorPressed": LV.Theme.panelBackground12,
                    "cornerRadius": LV.Theme.radiusSm
                })
            horizontalPadding: LV.Theme.gap2
            spacing: 0
            verticalPadding: LV.Theme.gap2
        }
    }
}
