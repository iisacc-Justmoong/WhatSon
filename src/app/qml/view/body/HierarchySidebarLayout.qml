import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: root

    readonly property int activeToolbarIndex: sidebarHierarchyStore.activeIndex
    readonly property int availableContentWidth: Math.max(0, width - horizontalInset * 2)
    readonly property int contentWidth: Math.max(minContentWidth, Math.min(maxContentWidth, availableContentWidth))
    readonly property int footerHeight: 24
    readonly property int footerWidth: 84
    readonly property int horizontalInset: LV.Theme.gap8
    readonly property int maxContentWidth: 200
    readonly property int minContentWidth: 136
    property color panelColor: LV.Theme.panelBackground04
    readonly property int rowHeight: 28
    readonly property int searchHeight: LV.Theme.gap18
    readonly property int toolbarButtonSize: LV.Theme.gap20
    readonly property int toolbarCount: toolbarIconNames.length
    readonly property real toolbarGap: toolbarCount > 1 ? Math.max(0, (contentWidth - toolbarButtonSize * toolbarCount) / (toolbarCount - 1)) : 0
    readonly property var toolbarIconNames: sidebarHierarchyStore.toolbarIconNames
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
        anchors.top: parent.top
        anchors.topMargin: root.verticalInset
        width: root.contentWidth

        LV.VStack {
            anchors.fill: parent
            spacing: LV.Theme.gap2

            LV.HStack {
                Layout.fillWidth: true
                Layout.preferredHeight: root.toolbarButtonSize
                alignmentName: "top"
                spacing: root.toolbarGap

                Repeater {
                    model: root.toolbarCount

                    delegate: LV.IconButton {
                        required property int index

                        Layout.preferredHeight: root.toolbarButtonSize
                        Layout.preferredWidth: root.toolbarButtonSize
                        backgroundColor: index === root.activeToolbarIndex ? LV.Theme.panelBackground12 : "transparent"
                        backgroundColorDisabled: "transparent"
                        backgroundColorHover: index === root.activeToolbarIndex ? LV.Theme.panelBackground12 : LV.Theme.surfaceAlt
                        backgroundColorPressed: index === root.activeToolbarIndex ? LV.Theme.panelBackground12 : LV.Theme.surfaceAlt
                        iconName: root.toolbarIconNames[index]
                        iconSize: LV.Theme.iconSm
                        textColor: index === root.activeToolbarIndex ? LV.Theme.accentBlue : LV.Theme.accentGrayLight
                        tone: LV.AbstractButton.Borderless

                        onClicked: sidebarHierarchyStore.activeIndex = index
                    }
                }
            }
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: root.searchHeight
                color: LV.Theme.panelBackground10
                radius: LV.Theme.radiusControl

                LV.HStack {
                    alignmentName: "center"
                    anchors.bottomMargin: LV.Theme.gap3
                    anchors.fill: parent
                    anchors.leftMargin: LV.Theme.gap7
                    anchors.rightMargin: LV.Theme.gap7
                    anchors.topMargin: LV.Theme.gap3
                    spacing: 0

                    LV.Label {
                        Layout.fillWidth: true
                        color: LV.Theme.titleHeaderColor
                        elide: Text.ElideRight
                        font.family: "Pretendard"
                        font.pixelSize: 12
                        font.weight: Font.Medium
                        lineHeight: 12
                        lineHeightMode: Text.FixedHeight
                        style: body
                        text: "Placeholder"
                        verticalAlignment: Text.AlignVCenter
                    }
                    Image {
                        Layout.preferredHeight: 12
                        Layout.preferredWidth: 12
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                        LV.Theme.iconPath("search")
                        sourceSize.height: 12
                        sourceSize.width: 12
                    }
                }
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
                            itemWidth: root.contentWidth
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

        LV.HStack {
            anchors.bottomMargin: LV.Theme.gap2
            anchors.fill: parent
            anchors.leftMargin: LV.Theme.gap2
            anchors.rightMargin: LV.Theme.gap2
            anchors.topMargin: LV.Theme.gap2
            spacing: 0

            LV.IconButton {
                Layout.preferredHeight: 20
                Layout.preferredWidth: 20
                backgroundColor: "transparent"
                backgroundColorHover: LV.Theme.surfaceAlt
                backgroundColorPressed: LV.Theme.surfaceAlt
                iconName: "addFile"
                iconSize: LV.Theme.iconSm
                tone: LV.AbstractButton.Borderless
            }
            LV.IconButton {
                Layout.preferredHeight: 20
                Layout.preferredWidth: 20
                backgroundColor: "transparent"
                backgroundColorHover: LV.Theme.surfaceAlt
                backgroundColorPressed: LV.Theme.surfaceAlt
                iconName: "delete"
                iconSize: LV.Theme.iconSm
                tone: LV.AbstractButton.Borderless
            }
            LV.IconMenuButton {
                Layout.preferredHeight: 20
                Layout.preferredWidth: 40
                backgroundColor: LV.Theme.panelBackground12
                backgroundColorHover: LV.Theme.panelBackground12
                backgroundColorPressed: LV.Theme.panelBackground12
                cornerRadius: LV.Theme.radiusSm
                iconName: "settings"
                iconSize: LV.Theme.iconSm
                tone: LV.AbstractButton.Default
            }
        }
    }
}
