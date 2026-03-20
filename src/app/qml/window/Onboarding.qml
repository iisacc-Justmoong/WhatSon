pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Dialogs
import QtQuick.Window
import LVRS 1.0 as LV

Window {
    id: root

    readonly property url appIconSource: "qrc:/whatson/AppIcon.png"
    readonly property int appIconSize: 144
    readonly property int closeColumnWidth: 48
    readonly property int desktopDesignHeight: 542
    readonly property int desktopDesignWidth: 867
    readonly property int desktopMinHeight: 420
    readonly property int desktopMinWidth: 620
    readonly property url currentFolderUrl: hubSessionController ? hubSessionController.currentFolderUrl : ""
    readonly property string defaultCreateHubFileName: "Untitled.wshub"
    readonly property int dragRegionHeight: 72
    readonly property bool isMobilePlatform: Qt.platform.os === "android" || Qt.platform.os === "ios"
    readonly property color linkColor: LV.Theme.accent
    readonly property color mainSurfaceColor: root.panelColor
    readonly property int mobileDesignHeight: 760
    readonly property int mobileDesignWidth: 470
    readonly property int mobileStatusPanelHeight: Math.max(172, Math.round(root.height * 0.5))
    readonly property int mobileTopContentWidth: Math.min(260, Math.max(220, root.width - 64))
    readonly property int availableScreenHeight: {
        const targetScreen = root.hostWindow && root.hostWindow.screen ? root.hostWindow.screen : root.screen;
        return targetScreen ? Math.round(targetScreen.height) : root.mobileDesignHeight;
    }
    readonly property int availableScreenWidth: {
        const targetScreen = root.hostWindow && root.hostWindow.screen ? root.hostWindow.screen : root.screen;
        return targetScreen ? Math.round(targetScreen.width) : root.mobileDesignWidth;
    }
    readonly property int mobileWindowHeight: Math.max(0, root.availableScreenHeight)
    readonly property int mobileWindowWidth: Math.max(0, root.availableScreenWidth)
    readonly property bool useMobileLayout: {
        if (root.hostWindow && root.hostWindow.adaptiveMobileLayout !== undefined)
            return Boolean(root.hostWindow.adaptiveMobileLayout) || root.isMobilePlatform;
        return root.isMobilePlatform;
    }
    readonly property int compactMinHeight: root.useMobileLayout ? root.mobileWindowHeight : root.desktopMinHeight
    readonly property int compactMinWidth: root.useMobileLayout ? root.mobileWindowWidth : root.desktopMinWidth
    property int defaultHeight: compactMinHeight
    property int defaultWidth: compactMinWidth
    readonly property int designHeight: root.useMobileLayout ? root.mobileDesignHeight : root.desktopDesignHeight
    readonly property int designWidth: root.useMobileLayout ? root.mobileDesignWidth : root.desktopDesignWidth
    readonly property int fixedHeight: Math.max(defaultHeight, minHeight)
    readonly property int fixedWidth: Math.max(defaultWidth, minWidth)
    readonly property int effectiveHeight: root.useMobileLayout ? root.compactMinHeight : root.fixedHeight
    readonly property int effectiveWidth: root.useMobileLayout ? root.compactMinWidth : root.fixedWidth
    property Window hostWindow: null
    property var hubSessionController: null
    property int minHeight: compactMinHeight
    property int minWidth: compactMinWidth
    property color panelColor: LV.Theme.panelBackground06
    readonly property int rightPanelWidth: Math.max(214, Math.min(306, Math.round(root.width * 306 / root.desktopDesignWidth)))
    readonly property color secondarySurfaceColor: root.sidePanelColor
    readonly property string resolvedVersionText: {
        const value = root.versionText === undefined || root.versionText === null ? "" : String(root.versionText).trim();
        return value.length > 0 ? value : "Version: 1.0.0";
    }
    readonly property string selectedHubStatusText: {
        if (root.hubSessionController) {
            if (root.hubSessionController.currentHubPathName !== undefined) {
                const pathName = String(root.hubSessionController.currentHubPathName).trim();
                if (pathName.length > 0)
                    return pathName;
            }
            if (root.hubSessionController.currentHubName !== undefined) {
                const hubName = String(root.hubSessionController.currentHubName).trim();
                if (hubName.length > 0)
                    return hubName;
            }
        }
        return "No WhatSon Hub Selected";
    }
    property color sidePanelColor: LV.Theme.panelBackground10
    property bool standaloneMode: false
    readonly property string statusText: {
        if (hubSessionController && hubSessionController.busy)
            return "Preparing WhatSon Hub...";
        if (hubSessionController && hubSessionController.lastError.length > 0)
            return hubSessionController.lastError;
        return "";
    }
    readonly property color statusTextColor: hubSessionController && hubSessionController.lastError.length > 0 ? LV.Theme.danger : LV.Theme.descriptionColor
    readonly property url suggestedCreateHubFileUrl: {
        const folderText = root.currentFolderUrl ? String(root.currentFolderUrl).trim() : "";
        if (folderText.length === 0)
            return "";
        const normalizedFolderText = folderText.endsWith("/") ? folderText.slice(0, -1) : folderText;
        return normalizedFolderText + "/" + root.defaultCreateHubFileName;
    }
    property string versionText: "Version: 1.0.0"

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
        x = Math.round(targetScreen.virtualX + (targetScreen.width - width) / 2);
        y = Math.round(targetScreen.virtualY + (targetScreen.height - height) / 2);
    }

    color: "transparent"
    flags: Qt.Dialog | Qt.FramelessWindowHint
    height: effectiveHeight
    maximumHeight: effectiveHeight
    maximumWidth: effectiveWidth
    minimumHeight: effectiveHeight
    minimumWidth: effectiveWidth
    modality: Qt.ApplicationModal
    title: "WhatSon Onboarding"
    transientParent: hostWindow
    visible: false
    width: effectiveWidth

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
        target: root.hubSessionController

        function onHubLoaded(hubPath) {
            if (!root.standaloneMode)
                root.close();
        }
    }

    FileDialog {
        id: createHubDialog

        currentFile: root.suggestedCreateHubFileUrl
        currentFolder: root.currentFolderUrl
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
        radius: root.useMobileLayout ? 0 : 32

        Item {
            anchors.fill: parent
            visible: !root.useMobileLayout

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
                width: root.closeColumnWidth

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

            Rectangle {
                id: rightPanel

                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.top: parent.top
                antialiasing: true
                color: root.secondarySurfaceColor
                radius: windowFrame.radius
                width: root.rightPanelWidth

                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.rightMargin: windowFrame.radius
                    anchors.top: parent.top
                    color: root.secondarySurfaceColor
                }

                LV.Label {
                    anchors.centerIn: parent
                    color: LV.Theme.textPrimary
                    elide: Text.ElideMiddle
                    font.styleName: "SemiBold"
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    lineHeight: 15
                    lineHeightMode: Text.FixedHeight
                    maximumLineCount: 1
                    style: header2
                    text: root.selectedHubStatusText
                    width: Math.max(0, parent.width - 32)
                    wrapMode: Text.NoWrap
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
                    height: appPanelColumn.implicitHeight
                    width: 209

                    Column {
                        id: appPanelColumn

                        anchors.centerIn: parent
                        spacing: 32
                        width: parent.width

                        Image {
                            anchors.horizontalCenter: parent.horizontalCenter
                            fillMode: Image.PreserveAspectFit
                            height: root.appIconSize
                            mipmap: true
                            smooth: true
                            source: root.appIconSource
                            sourceSize.height: root.appIconSize
                            sourceSize.width: root.appIconSize
                            width: root.appIconSize
                        }
                        LV.Label {
                            anchors.horizontalCenter: parent.horizontalCenter
                            color: LV.Theme.textPrimary
                            font.pixelSize: 48
                            font.styleName: "Bold"
                            font.weight: Font.Bold
                            horizontalAlignment: Text.AlignHCenter
                            style: title
                            text: "WhatSon"
                            width: parent.width
                        }
                        LV.Label {
                            anchors.horizontalCenter: parent.horizontalCenter
                            color: LV.Theme.descriptionColor
                            font.pixelSize: 12
                            font.styleName: "SemiBold"
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignHCenter
                            lineHeight: 12
                            lineHeightMode: Text.FixedHeight
                            style: description
                            text: root.resolvedVersionText
                            width: parent.width
                        }

                        Column {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 24
                            width: 180

                            ActionLink {
                                id: createHubAction

                                enabled: !root.hubSessionController || !root.hubSessionController.busy
                                label: "Create new WhatSon Hub"
                                width: parent.width

                                onTriggered: {
                                    root.viewHookRequested();
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
                                    root.viewHookRequested();
                                    if (root.hubSessionController) {
                                        root.hubSessionController.clearLastError();
                                        selectHubDialog.open();
                                    } else {
                                        root.selectFileRequested();
                                    }
                                }
                            }
                        }
                        LV.Label {
                            anchors.horizontalCenter: parent.horizontalCenter
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
        }

        Item {
            anchors.fill: parent
            visible: root.useMobileLayout

            Item {
                id: mobileAppPanel

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: mobileHubPanel.top

                Item {
                    anchors.centerIn: parent
                    height: mobileAppColumn.implicitHeight
                    width: root.mobileTopContentWidth

                    Column {
                        id: mobileAppColumn

                        anchors.centerIn: parent
                        spacing: 32
                        width: parent.width

                        Image {
                            anchors.horizontalCenter: parent.horizontalCenter
                            fillMode: Image.PreserveAspectFit
                            height: root.appIconSize
                            mipmap: true
                            smooth: true
                            source: root.appIconSource
                            sourceSize.height: root.appIconSize
                            sourceSize.width: root.appIconSize
                            width: root.appIconSize
                        }
                        LV.Label {
                            anchors.horizontalCenter: parent.horizontalCenter
                            color: LV.Theme.textPrimary
                            font.pixelSize: 48
                            font.styleName: "Bold"
                            font.weight: Font.Bold
                            horizontalAlignment: Text.AlignHCenter
                            style: title
                            text: "WhatSon"
                            width: parent.width
                        }
                        LV.Label {
                            anchors.horizontalCenter: parent.horizontalCenter
                            color: LV.Theme.descriptionColor
                            font.pixelSize: 12
                            font.styleName: "SemiBold"
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignHCenter
                            lineHeight: 12
                            lineHeightMode: Text.FixedHeight
                            style: description
                            text: root.resolvedVersionText
                            width: parent.width
                        }

                        Column {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 24
                            width: parent.width

                            ActionLink {
                                enabled: !root.hubSessionController || !root.hubSessionController.busy
                                label: "Create new WhatSon Hub"
                                width: parent.width

                                onTriggered: {
                                    root.viewHookRequested();
                                    if (root.hubSessionController) {
                                        root.hubSessionController.clearLastError();
                                        createHubDialog.open();
                                    } else {
                                        root.createFileRequested();
                                    }
                                }
                            }
                            ActionLink {
                                enabled: !root.hubSessionController || !root.hubSessionController.busy
                                label: "Select WhatSon Hub"
                                width: parent.width

                                onTriggered: {
                                    root.viewHookRequested();
                                    if (root.hubSessionController) {
                                        root.hubSessionController.clearLastError();
                                        selectHubDialog.open();
                                    } else {
                                        root.selectFileRequested();
                                    }
                                }
                            }
                        }
                        LV.Label {
                            anchors.horizontalCenter: parent.horizontalCenter
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

            Rectangle {
                id: mobileHubPanel

                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                color: root.secondarySurfaceColor
                height: root.mobileStatusPanelHeight

                LV.Label {
                    anchors.centerIn: parent
                    color: LV.Theme.textPrimary
                    elide: Text.ElideMiddle
                    font.styleName: "SemiBold"
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    lineHeight: 15
                    lineHeightMode: Text.FixedHeight
                    maximumLineCount: 2
                    style: header2
                    text: root.selectedHubStatusText
                    width: Math.max(0, parent.width - 32)
                    wrapMode: Text.WordWrap
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
            color: actionLink.enabled ? root.linkColor : LV.Theme.descriptionColor
            font.pixelSize: 15
            font.styleName: "SemiBold"
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
            lineHeight: 15
            lineHeightMode: Text.FixedHeight
            opacity: !actionLink.enabled ? 0.42 : actionMouseArea.pressed ? 0.68 : actionMouseArea.containsMouse ? 0.84 : 1
            style: header2
            text: actionLink.label
            width: parent.width
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
