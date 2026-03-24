pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: detailContents

    readonly property string figmaNodeId: "155:4582"
    property var activeContentViewModel: null
    property string activeStateName: "properties"
    readonly property var folderItems: ["Label", "Label", "Label", "Label", "Label"]
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("detail.DetailContents") : null
    readonly property string resolvedActiveStateName: detailContents.normalizeStateName(detailContents.activeStateName)
    readonly property var tagItems: ["Label", "Label", "Label", "Label"]

    signal viewHookRequested

    function normalizeStateName(value) {
        const normalized = value === undefined || value === null ? "" : String(value).trim();
        switch (normalized) {
        case "properties":
            return "properties";
        case "fileStat":
            return "fileStat";
        case "insert":
            return "insert";
        case "fileHistory":
            return "fileHistory";
        case "layer":
            return "layer";
        case "help":
            return normalized;
        default:
            return "properties";
        }
    }
    function placeholderHeadingForState(stateName) {
        switch (stateName) {
        case "fileStat":
            return "Statistics";
        case "insert":
            return "Insert";
        case "fileHistory":
            return "History";
        case "layer":
            return "Layer";
        case "help":
            return "Help";
        default:
            return "Properties";
        }
    }
    function placeholderSummaryForState(stateName) {
        switch (stateName) {
        case "fileStat":
            return "Detail statistics content will be mounted in this dedicated panel form.";
        case "insert":
            return "Insert controls will be mounted in this dedicated panel form.";
        case "fileHistory":
            return "History controls will be mounted in this dedicated panel form.";
        case "layer":
            return "Layer controls will be mounted in this dedicated panel form.";
        case "help":
            return "Help content will be mounted in this dedicated panel form.";
        default:
            return "Properties content remains the default detail panel form.";
        }
    }
    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    objectName: "DetailContents"
    state: detailContents.resolvedActiveStateName

    component DetailSectionTitle: LV.Label {
        required property string figmaTextNodeId

        property color labelColor: LV.Theme.captionColor

        color: labelColor
        readonly property string figmaNodeId: figmaTextNodeId
        style: caption
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.NoWrap
    }

    component DetailListRow: Item {
        required property string rowLabel
        required property string rowObjectName

        objectName: rowObjectName
        implicitHeight: 20
        implicitWidth: 170

        Row {
            anchors.fill: parent
            anchors.bottomMargin: 2
            anchors.leftMargin: 4
            anchors.rightMargin: 4
            anchors.topMargin: 2
            spacing: 1

            Image {
                fillMode: Image.PreserveAspectFit
                height: 16
                smooth: true
                source: LV.Theme.iconPath("nodesfolder")
                sourceSize.height: 16
                sourceSize.width: 16
                width: 16
            }
            DetailSectionTitle {
                figmaTextNodeId: ""
                labelColor: LV.Theme.bodyColor
                style: body
                text: rowLabel
            }
        }
    }

    component DetailComboSection: Item {
        id: comboSection
        required property string comboNodeId
        required property string fieldObjectName
        required property string frameNodeId
        required property string labelNodeId
        required property string labelText
        required property string valueText

        readonly property string figmaNodeId: frameNodeId
        implicitHeight: 33
        implicitWidth: 178
        objectName: fieldObjectName

        Column {
            anchors.fill: parent
            spacing: 4

            DetailSectionTitle {
                figmaTextNodeId: comboSection.labelNodeId
                labelColor: LV.Theme.accentWhite
                objectName: comboSection.fieldObjectName + "Label"
                text: comboSection.labelText
                width: parent.width
            }
            LV.ComboBox {
                readonly property string figmaNodeId: comboSection.comboNodeId

                objectName: comboSection.fieldObjectName + "ComboBox"
                text: comboSection.valueText
                width: parent.width

                onClicked: detailContents.requestViewHook(comboSection.fieldObjectName + ".combo")
            }
        }
    }

    component DetailListSection: Item {
        id: listSection
        required property string footerActionPrefix
        required property string frameNodeId
        required property string itemsNodeId
        required property var listItems
        required property string listNodeId
        required property string listObjectName
        required property string titleNodeId
        required property string titleText

        readonly property string figmaNodeId: frameNodeId
        implicitHeight: 155
        implicitWidth: 178
        objectName: listObjectName

        Column {
            anchors.fill: parent
            spacing: 4

            DetailSectionTitle {
                figmaTextNodeId: listSection.titleNodeId
                objectName: listSection.listObjectName + "Label"
                text: listSection.titleText
                width: parent.width
            }
            Rectangle {
                id: smallList

                readonly property string figmaNodeId: listSection.listNodeId

                color: LV.Theme.panelBackground03
                height: 140
                objectName: listSection.listObjectName + "SmallList"
                width: parent.width

                Item {
                    readonly property string figmaNodeId: listSection.itemsNodeId

                    clip: true
                    height: 116
                    objectName: listSection.listObjectName + "Items"
                    width: parent.width

                    Column {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        spacing: 0

                        Repeater {
                            model: listSection.listItems.length

                            delegate: DetailListRow {
                                required property int index

                                rowLabel: String(listSection.listItems[index])
                                rowObjectName: listSection.listObjectName + "Item" + index
                            }
                        }
                    }
                }
                LV.ListFooter {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    button1: ({
                            type: "icon",
                            iconName: "addFile",
                            backgroundColor: "transparent",
                            backgroundColorDisabled: "transparent",
                            backgroundColorHover: "transparent",
                            backgroundColorPressed: "transparent",
                            horizontalPadding: 2,
                            onClicked: function () {
                                detailContents.requestViewHook(listSection.footerActionPrefix + ".add");
                            },
                            verticalPadding: 2
                        })
                    button2: ({
                            type: "icon",
                            iconName: "trash",
                            backgroundColor: "transparent",
                            backgroundColorDisabled: "transparent",
                            backgroundColorHover: "transparent",
                            backgroundColorPressed: "transparent",
                            horizontalPadding: 2,
                            onClicked: function () {
                                detailContents.requestViewHook(listSection.footerActionPrefix + ".delete");
                            },
                            verticalPadding: 2
                        })
                    button3: ({
                            type: "menu",
                            iconName: "settings",
                            backgroundColor: "transparent",
                            backgroundColorDisabled: "transparent",
                            backgroundColorHover: "transparent",
                            backgroundColorPressed: "transparent",
                            bottomPadding: 2,
                            leftPadding: 2,
                            onClicked: function () {
                                detailContents.requestViewHook(listSection.footerActionPrefix + ".settings");
                            },
                            rightPadding: 4,
                            topPadding: 2
                    })
                    height: 24
                    horizontalPadding: 2
                    objectName: listSection.listObjectName + "Footer"
                    spacing: 0
                    verticalPadding: 2
                    width: 78
                }
            }
        }
    }

    component DetailPlaceholderForm: Item {
        anchors.fill: parent
        objectName: detailContents.resolvedActiveStateName + "Form"

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            anchors.topMargin: 2
            spacing: 10

            DetailSectionTitle {
                figmaTextNodeId: ""
                labelColor: LV.Theme.accentWhite
                objectName: detailContents.resolvedActiveStateName + "Title"
                style: body
                text: detailContents.placeholderHeadingForState(detailContents.resolvedActiveStateName)
                width: parent.width
            }
            Rectangle {
                color: LV.Theme.panelBackground03
                height: 72
                radius: LV.Theme.radiusControl
                width: parent.width

                DetailSectionTitle {
                    anchors.fill: parent
                    anchors.bottomMargin: 8
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    anchors.topMargin: 8
                    figmaTextNodeId: ""
                    labelColor: LV.Theme.bodyColor
                    objectName: detailContents.resolvedActiveStateName + "Description"
                    style: body
                    text: detailContents.placeholderSummaryForState(detailContents.resolvedActiveStateName)
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.WordWrap
                }
            }
        }
    }

    Flickable {
        id: propertiesFlickable

        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds
        boundsMovement: Flickable.StopAtBounds
        clip: true
        contentHeight: formColumn.y + formColumn.implicitHeight + 2
        contentWidth: width
        interactive: contentHeight > height
        visible: detailContents.resolvedActiveStateName === "properties"

        Column {
            id: formColumn

            readonly property string figmaNodeId: "155:4583"

            x: 8
            y: 2
            objectName: "Form"
            spacing: 10
            width: Math.max(0, propertiesFlickable.width - 16)

            DetailComboSection {
                comboNodeId: "178:5496"
                fieldObjectName: "Projects"
                frameNodeId: "178:5494"
                labelNodeId: "178:5495"
                labelText: "Projects"
                valueText: "${project}"
                width: parent.width
            }
            DetailComboSection {
                comboNodeId: "155:4586"
                fieldObjectName: "Bookmark"
                frameNodeId: "155:4584"
                labelNodeId: "155:4585"
                labelText: "Bookmark"
                valueText: "${bookmark}"
                width: parent.width
            }
            DetailListSection {
                footerActionPrefix: "folders"
                frameNodeId: "155:4587"
                itemsNodeId: "I155:4589;179:438"
                listItems: detailContents.folderItems
                listNodeId: "155:4589"
                listObjectName: "FoldersList"
                titleNodeId: "155:4588"
                titleText: "Folders"
                width: parent.width
            }
            DetailListSection {
                footerActionPrefix: "tags"
                frameNodeId: "155:4590"
                itemsNodeId: "I155:4592;179:438"
                listItems: detailContents.tagItems
                listNodeId: "155:4592"
                listObjectName: "TagsList"
                titleNodeId: "155:4591"
                titleText: "Tags"
                width: parent.width
            }
            DetailComboSection {
                comboNodeId: "178:5503"
                fieldObjectName: "Progress"
                frameNodeId: "178:5501"
                labelNodeId: "178:5502"
                labelText: "Progress"
                valueText: "${progress}"
                width: parent.width
            }
        }
    }
    DetailPlaceholderForm {
        visible: detailContents.resolvedActiveStateName !== "properties"
    }

    states: [
        State {
            name: "properties"
        },
        State {
            name: "fileStat"
        },
        State {
            name: "insert"
        },
        State {
            name: "fileHistory"
        },
        State {
            name: "layer"
        },
        State {
            name: "help"
        }
    ]
}
