pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Dialogs
import QtQuick.Window
import LVRS 1.0 as LV

Window {
    id: root

    readonly property url appIconSource: "qrc:/whatson/AppIcon.png"
    readonly property int compactMinHeight: 420
    readonly property int compactMinWidth: 620
    readonly property url currentFolderUrl: hubSessionController ? hubSessionController.currentFolderUrl : ""
    property int defaultHeight: compactMinHeight
    property int defaultWidth: compactMinWidth
    readonly property int designHeight: 542
    readonly property int designWidth: 867
    readonly property int dragRegionHeight: 72
    readonly property string fallbackHubName: "WhatSon Hub"
    readonly property int fixedHeight: Math.max(defaultHeight, minHeight)
    readonly property int fixedWidth: Math.max(defaultWidth, minWidth)
    property Window hostWindow: null
    property string hubName: ""
    property url hubPreviewSource: ""
    property var hubSessionController: null
    readonly property real layoutScale: Math.min(1, Math.min(root.width / root.designWidth, root.height / root.designHeight))
    readonly property color linkColor: LV.Theme.accent
    readonly property int mainPanelHorizontalInset: Math.max(24, Math.round(32 * root.layoutScale))
    readonly property int mainPanelIconSize: Math.max(188, Math.min(300, Math.round(300 * root.layoutScale)))
    readonly property int mainPanelMaxWidth: 300
    readonly property int mainPanelMinWidth: 220
    readonly property int mainPanelSpacing: Math.max(12, Math.round(16 * root.layoutScale))
    readonly property color mainSurfaceColor: root.panelColor
    property int minHeight: compactMinHeight
    property int minWidth: compactMinWidth
    property color panelColor: LV.Theme.panelBackground06
    readonly property color previewPlaceholderColor: "#C4C4C4"
    readonly property string resolvedHubName: {
        const value = root.hubName === undefined || root.hubName === null ? "" : String(root.hubName).trim();
        if (value.length > 0)
            return value;
        const controllerValue = root.hubSessionController ? String(root.hubSessionController.currentHubName || "").trim() : "";
        return controllerValue.length > 0 ? controllerValue : root.fallbackHubName;
    }
    readonly property string resolvedVersionText: {
        const value = root.versionText === undefined || root.versionText === null ? "" : String(root.versionText).trim();
        return value.length > 0 ? value : "Version: 1.0.0";
    }
    readonly property color secondarySurfaceColor: root.sidePanelColor
    property color sidePanelColor: LV.Theme.panelBackground10
    readonly property int sidePanelMaxWidth: 306
    readonly property int sidePanelMinWidth: 236
    property bool standaloneMode: false
    property string versionText: "Version: 1.0.0"
    readonly property string statusText: {
        if (hubSessionController && hubSessionController.busy)
            return "Preparing WhatSon Hub...";
        if (hubSessionController && hubSessionController.lastError.length > 0)
            return hubSessionController.lastError;
        return "";
    }
    readonly property color statusTextColor: hubSessionController && hubSessionController.lastError.length > 0 ? LV.Theme.danger : LV.Theme.descriptionColor

    signal createFileRequested
    signal dismissed
    signal selectFileRequested
    signal viewHookRequested

    function recenter() {
        if (hostWindow) {
            x = hostWindow.x + Math.round((hostWindow.width - width) / 2);
            y = hostWindow.y + Math.round((hostWindow.height - height) / 2);
            return;
        }
        var targetScreen = root.screen;
        if (!targetScreen)
            return;
        x = Math.round(Screen.virtualX + (Screen.width - width) / 2);
        y = Math.round(Screen.virtualY + (Screen.height - height) / 2);
    }

    color: "transparent"
    flags: Qt.Dialog | Qt.FramelessWindowHint
    height: fixedHeight
    maximumHeight: fixedHeight
    maximumWidth: fixedWidth
    minimumHeight: fixedHeight
    minimumWidth: fixedWidth
    modality: Qt.ApplicationModal
    title: "WhatSon Onboarding"
    transientParent: hostWindow
    visible: false
    width: fixedWidth

    onClosing: function (close) {
        dismissed();
    }
    onHeightChanged: {
        if (visible)
            recenter();
    }
    onHostWindowChanged: recenter()
    onVisibleChanged: {
        if (visible)
            recenter();
    }
    onWidthChanged: {
        if (visible)
            recenter();
    }

    Connections {
        function onHubLoaded(hubPath) {
            if (!root.standaloneMode)
                root.close();
        }

        target: root.hubSessionController
    }
    FileDialog {
        id: createHubDialog

        currentFolder: root.currentFolderUrl
        currentFile: root.suggestedCreateHubFileUrl
        defaultSuffix: "wshub"
        fileMode: FileDialog.SaveFile
        nameFilters: ["WhatSon Hub (*.wshub)"]
        selectedFile: root.suggestedCreateHubFileUrl
        title: "Create WhatSon Hub"

        onAccepted: {
            if (root.hubSessionController) {
                root.hubSessionController.createHubAtUrl(selectedFile);
            } else {
                root.createFileRequested();
            }
        }
    }
    FolderDialog {
        id: selectHubDialog

        currentFolder: root.currentFolderUrl
        title: "Select WhatSon Hub"

        onAccepted: {
            if (root.hubSessionController) {
                root.hubSessionController.loadHubFromUrl(selectedFolder);
            } else {
                root.selectFileRequested();
            }
        }
    }
    Rectangle {
        id: windowFrame

        anchors.fill: parent
        antialiasing: true
        clip: true
        color: root.mainSurfaceColor
        radius: 32

        MouseArea {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            cursorShape: Qt.SizeAllCursor
            height: root.dragRegionHeight

            onPressed: function (mouse) {
                if (mouse.button === Qt.LeftButton && typeof root.startSystemMove === "function")
                    root.startSystemMove();
            }
        }
        Item {
            id: closeColumn

            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.top: parent.top
            width: 48

            Item {
                id: closeButton

                anchors.left: parent.left
                anchors.leftMargin: 16
                anchors.top: parent.top
                anchors.topMargin: 16
                height: 16
                width: 16

                Rectangle {
                    anchors.fill: parent
                    color: closeMouseArea.containsMouse ? Qt.rgba(1, 1, 1, 0.08) : "transparent"
                    radius: 4
                }
                Canvas {
                    anchors.fill: parent
                    antialiasing: true
                    opacity: closeMouseArea.pressed ? 0.68 : closeMouseArea.containsMouse ? 0.84 : 1

                    onPaint: {
                        const ctx = getContext("2d");
                        ctx.clearRect(0, 0, width, height);
                        ctx.strokeStyle = LV.Theme.accentGray;
                        ctx.lineCap = "round";
                        ctx.lineWidth = 1.6;
                        ctx.beginPath();
                        ctx.moveTo(5, 5);
                        ctx.lineTo(11, 11);
                        ctx.moveTo(11, 5);
                        ctx.lineTo(5, 11);
                        ctx.stroke();
                    }
                }
                MouseArea {
                    id: closeMouseArea

                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true

                    onClicked: root.close()
                }
            }
        }
        Item {
            id: rightPanel

            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.top: parent.top
            width: Math.max(root.sidePanelMinWidth, Math.min(root.sidePanelMaxWidth, Math.round(parent.width * root.sidePanelMaxWidth / root.designWidth)))

            Rectangle {
                anchors.fill: parent
                antialiasing: true
                color: root.secondarySurfaceColor
                radius: windowFrame.radius
            }
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.rightMargin: windowFrame.radius
                anchors.top: parent.top
                color: root.secondarySurfaceColor
            }
            Item {
                id: previewGroup

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 32
                height: 188
                width: Math.min(158, Math.max(140, parent.width - 32))

                Rectangle {
                    id: previewPlaceholder

                    anchors.horizontalCenter: parent.horizontalCenter
                    color: root.previewPlaceholderColor
                    height: Math.min(parent.width, 130)
                    width: height

                    Image {
                        anchors.fill: parent
                        fillMode: Image.PreserveAspectCrop
                        mipmap: true
                        smooth: true
                        root.hubPreviewSource
                        sourceSize.height: height
                        sourceSize.width: width
                        visible: root.hubPreviewSource.toString().length > 0
                    }
                }
                LV.Label {
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: LV.Theme.textPrimary
                    font.weight: Font.Bold
                    height: implicitHeight
                    horizontalAlignment: Text.AlignHCenter
                    maximumLineCount: 2
                    style: title
                    text: root.resolvedHubName
                    verticalAlignment: Text.AlignVCenter
                    width: parent.width
                    wrapMode: Text.WordWrap
                }
            }
            Item {
                id: actionGroup

                anchors.bottom: parent.bottom
                anchors.bottomMargin: 32
                anchors.horizontalCenter: parent.horizontalCenter
                height: actionColumn.implicitHeight
                width: Math.min(220, Math.max(createHubAction.implicitWidth, selectHubAction.implicitWidth, rightPanel.width - 32))

                Column {
                    id: actionColumn

                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    spacing: 10
                    width: parent.width

                    ActionLink {
                        id: createHubAction

                        enabled: !root.hubSessionController || !root.hubSessionController.busy
                        label: "Create new WhatSon Hub"
                        width: parent.width

                        onTriggered: {
                            if (root.hubSessionController) {
                                root.hubSessionController.clearLastError();
                                createHubDialog.open();
                            } else {
                                root.createFileRequested();
                            }
                        }
                    }
                    ActionLink {
                        id: selectHubAction

                        enabled: !root.hubSessionController || !root.hubSessionController.busy
                        label: "Select WhatSon Hub"
                        width: parent.width

                        onTriggered: {
                            if (root.hubSessionController) {
                                root.hubSessionController.clearLastError();
                                selectHubDialog.open();
                            } else {
                                root.selectFileRequested();
                            }
                        }
                    }
                    LV.Label {
                        color: root.statusTextColor
                        horizontalAlignment: Text.AlignHCenter
                        maximumLineCount: 4
                        style: description
                        text: root.statusText
                        visible: text.length > 0
                        width: parent.width
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }
        Item {
            id: appPanel

            anchors.bottom: parent.bottom
            anchors.left: closeColumn.right
            anchors.right: rightPanel.left
            anchors.top: parent.top

            Item {
                id: appPanelContent

                anchors.centerIn: parent
                height: contentColumn.implicitHeight
                width: Math.max(root.mainPanelMinWidth, Math.min(root.mainPanelMaxWidth, appPanel.width - root.mainPanelHorizontalInset * 2))

                Column {
                    id: contentColumn

                    anchors.centerIn: parent
                    spacing: root.mainPanelSpacing
                    width: parent.width

                    Image {
                        anchors.horizontalCenter: parent.horizontalCenter
                        fillMode: Image.PreserveAspectFit
                        height: root.mainPanelIconSize
                        mipmap: true
                        smooth: true
                        root.appIconSource
                        sourceSize.height: height
                        sourceSize.width: width
                        width: root.mainPanelIconSize
                    }
                    LV.Label {
                        anchors.horizontalCenter: parent.horizontalCenter
                        color: LV.Theme.textPrimary
                        font.pixelSize: Math.max(40, Math.round(48 * root.layoutScale))
                        font.styleName: "Bold"
                        font.weight: Font.Bold
                        horizontalAlignment: Text.AlignHCenter
                        lineHeight: font.pixelSize
                        lineHeightMode: Text.FixedHeight
                        style: title
                        text: "WhatSon"
                        verticalAlignment: Text.AlignVCenter
                        width: parent.width
                    }
                    LV.Label {
                        anchors.horizontalCenter: parent.horizontalCenter
                        color: LV.Theme.descriptionColor
                        font.styleName: "SemiBold"
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignHCenter
                        style: description
                        text: root.resolvedVersionText
                        width: parent.width
                    }
                }
            }
        }
    }

    component ActionLink: Item {
        id: actionLink

        property bool enabled: true
        property string label: ""

        signal triggered

        implicitHeight: actionText.implicitHeight
        implicitWidth: actionText.implicitWidth

        LV.Label {
            id: actionText

            anchors.centerIn: parent
            color: actionLink.enabled ? LV.Theme.accent : LV.Theme.descriptionColor
            font.weight: Font.DemiBold
            opacity: !actionLink.enabled ? 0.42 : actionMouseArea.pressed ? 0.68 : actionMouseArea.containsMouse ? 0.84 : 1
            style: header2
            text: actionLink.label
        }
        MouseArea {
            id: actionMouseArea

            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            enabled: actionLink.enabled
            hoverEnabled: true

            onClicked: actionLink.triggered()
        }
    }
}
