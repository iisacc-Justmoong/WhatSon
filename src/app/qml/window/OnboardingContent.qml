pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Dialogs
import QtQuick.Window
import LVRS 1.0 as LV

Item {
    id: root

    readonly property url appIconSource: "qrc:/whatson/AppIcon.png"
    readonly property int appIconSize: Math.max(0, Math.round(LV.Theme.scaleMetric(144)))
    readonly property int closeButtonInset: LV.Theme.gap16
    readonly property int closeButtonSize: Math.max(0, Math.round(LV.Theme.scaleMetric(16)))
    readonly property int closeColumnWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(48)))
    readonly property url currentFolderUrl: hubSessionController ? hubSessionController.currentFolderUrl : ""
    readonly property string defaultCreateHubFileName: "Untitled.wshub"
    readonly property int desktopActionWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(180)))
    readonly property int desktopContentWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(209)))
    readonly property int dragRegionHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(72)))
    readonly property bool hasHubSelectionCandidates: root.hubSessionController && root.hubSessionController.hubSelectionCandidateNames.length > 0
    readonly property string onboardingSessionState: {
        if (!root.hubSessionController || root.hubSessionController.sessionState === undefined)
            return "idle";
        return String(root.hubSessionController.sessionState).trim();
    }
    readonly property bool useAndroidExistingHubFileFlow: Qt.platform.os === "android"
    readonly property bool isMobilePlatform: Qt.platform.os === "android" || Qt.platform.os === "ios"
    readonly property bool useMobileCreateDirectoryFlow: root.isMobilePlatform
    readonly property color linkColor: LV.Theme.accent
    readonly property color mainSurfaceColor: root.panelColor
    readonly property int mobileActionSpacing: LV.Theme.gap24
    readonly property int mobileActionWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(180)))
    readonly property int mobileContentSpacing: Math.max(0, Math.round(LV.Theme.scaleMetric(32)))
    readonly property int mobileContentWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(209)))
    readonly property int mobileDesignHeight: 762
    readonly property int mobileDesignWidth: 470
    readonly property int mobileSurfaceRadius: LV.Theme.radiusXl * 2
    readonly property int mobileVersionWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(75)))
    readonly property var owningWindow: root.hostWindow ? root.hostWindow : root.Window.window
    readonly property int panelCornerRadius: LV.Theme.radiusXl * 2
    readonly property int panelTextHorizontalInset: LV.Theme.gap16
    readonly property int titleLineHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(26)))
    readonly property int titleTextSize: Math.max(0, Math.round(LV.Theme.scaleMetric(48)))
    readonly property int versionLineHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
    readonly property int versionTextSize: Math.max(0, Math.round(LV.Theme.scaleMetric(12)))
    readonly property int actionTextLineHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(15)))
    readonly property int actionTextSize: Math.max(0, Math.round(LV.Theme.scaleMetric(15)))
    readonly property int statusLabelLineHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(15)))
    readonly property int availableScreenHeight: {
        const targetScreen = root.owningWindow && root.owningWindow.screen ? root.owningWindow.screen : null;
        return targetScreen ? Math.round(targetScreen.height) : root.mobileDesignHeight;
    }
    readonly property int availableScreenWidth: {
        const targetScreen = root.owningWindow && root.owningWindow.screen ? root.owningWindow.screen : null;
        return targetScreen ? Math.round(targetScreen.width) : root.mobileDesignWidth;
    }
    readonly property bool useMobileLayout: {
        if (root.hostWindow && root.hostWindow.adaptiveMobileLayout !== undefined)
            return Boolean(root.hostWindow.adaptiveMobileLayout) || root.isMobilePlatform;
        return root.isMobilePlatform;
    }
    readonly property bool useRoundedWindowFrame: !root.useMobileLayout
    property var hostWindow: null
    property var hubSessionController: null
    property color panelColor: LV.Theme.panelBackground06
    property bool autoCompleteOnHubLoaded: true
    property var createHubDialogInstance: null
    readonly property int rightPanelWidth: Math.max(
                                               Math.max(0, Math.round(LV.Theme.scaleMetric(214))),
                                               Math.min(
                                                   Math.max(0, Math.round(LV.Theme.scaleMetric(306))),
                                                   Math.round(root.width * Math.max(0, Math.round(LV.Theme.scaleMetric(306))) / 867)))
    readonly property color secondarySurfaceColor: root.sidePanelColor
    readonly property string resolvedVersionText: {
        const value = root.versionText === undefined || root.versionText === null ? "" : String(root.versionText).trim();
        return value.length > 0 ? value : "Version: 1.0.0";
    }
    readonly property string mobileSelectionAssistText: {
        if (root.statusText.length > 0)
            return root.statusText;
        if (root.useAndroidExistingHubFileFlow)
            return "On Android, choose the .wshub package directly from the native picker.";
        if (root.hasHubSelectionCandidates)
            return "Choose the WhatSon Hub package found in the selected folder.";
        return "On mobile, choose the folder that contains your WhatSon Hub.";
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
    property var selectHubFileDialogInstance: null
    property bool standaloneMode: false
    readonly property string statusText: {
        if (root.hubSessionController && root.onboardingSessionState === "routingWorkspace")
            return "Opening WhatSon workspace...";
        if (root.hubSessionController && root.onboardingSessionState === "loadingHub")
            return "Loading WhatSon Hub...";
        if (root.hubSessionController && root.onboardingSessionState === "resolvingSelection")
            return "Resolving WhatSon Hub...";
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

    signal completed
    signal createFileRequested
    signal dismissRequested
    signal requestWindowMove
    signal selectFileRequested
    signal viewHookRequested

    function ensureCreateHubDialog() {
        if (root.useMobileCreateDirectoryFlow)
            return null;
        if (!root.createHubDialogInstance)
            root.createHubDialogInstance = createHubDialogComponent.createObject(root);
        return root.createHubDialogInstance;
    }

    function ensureSelectHubFileDialog() {
        if (!root.useAndroidExistingHubFileFlow)
            return null;
        if (!root.selectHubFileDialogInstance)
            root.selectHubFileDialogInstance = selectHubFileDialogComponent.createObject(root);
        return root.selectHubFileDialogInstance;
    }

    function openCreateHubDialog() {
        const dialog = root.ensureCreateHubDialog();
        if (dialog && dialog.open)
            dialog.open();
    }

    function openSelectHubFileDialog() {
        const dialog = root.ensureSelectHubFileDialog();
        if (dialog && dialog.open)
            dialog.open();
    }

    Connections {
        target: root.hubSessionController

        function onHubLoaded(hubPath) {
            if (!root.standaloneMode && root.autoCompleteOnHubLoaded)
                root.completed();
        }
    }

    Component {
        id: createHubDialogComponent

        FileDialog {
            currentFile: root.suggestedCreateHubFileUrl
            currentFolder: root.currentFolderUrl
            defaultSuffix: "wshub"
            fileMode: FileDialog.SaveFile
            nameFilters: ["WhatSon Hub (*.wshub)"]
            title: "Create WhatSon Hub"

            onAccepted: {
                if (root.hubSessionController) {
                    root.hubSessionController.createHubAtUrl(selectedFile);
                } else {
                    root.createFileRequested();
                }
            }
        }
    }

    FolderDialog {
        id: createHubDirectoryDialog

        currentFolder: root.currentFolderUrl
        title: "Choose Folder for New WhatSon Hub"

        onAccepted: {
            if (root.hubSessionController) {
                root.hubSessionController.createHubInDirectoryUrl(selectedFolder, root.defaultCreateHubFileName);
            } else {
                root.createFileRequested();
            }
        }
    }

    FolderDialog {
        id: selectHubDialog

        currentFolder: root.currentFolderUrl
        title: root.isMobilePlatform ? "Choose Folder Containing WhatSon Hub" : "Select WhatSon Hub"

        onAccepted: {
            if (root.hubSessionController) {
                if (root.isMobilePlatform)
                    root.hubSessionController.prepareHubSelectionFromUrl(selectedFolder);
                else
                    root.hubSessionController.loadHubFromUrl(selectedFolder);
            } else {
                root.selectFileRequested();
            }
        }
    }

    Component {
        id: selectHubFileDialogComponent

        FileDialog {
            currentFolder: root.currentFolderUrl
            fileMode: FileDialog.OpenFile
            nameFilters: ["WhatSon Hub (*.wshub)"]
            title: "Select WhatSon Hub"

            onAccepted: {
                if (root.hubSessionController) {
                    root.hubSessionController.loadHubFromUrl(selectedFile);
                } else {
                    root.selectFileRequested();
                }
            }
        }
    }

    Rectangle {
        id: windowFrame

        anchors.fill: parent
        antialiasing: root.useRoundedWindowFrame
        clip: root.useRoundedWindowFrame
        color: root.mainSurfaceColor
        radius: root.useRoundedWindowFrame ? root.panelCornerRadius : 0

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
                    if (mouse.button === Qt.LeftButton)
                        root.requestWindowMove();
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
                    anchors.leftMargin: root.closeButtonInset
                    anchors.top: parent.top
                    anchors.topMargin: root.closeButtonInset
                    height: root.closeButtonSize
                    width: root.closeButtonSize

                    Rectangle {
                        anchors.fill: parent
                        color: closeMouseArea.containsMouse ? LV.Theme.panelBackground08 : "transparent"
                        radius: LV.Theme.radiusSm
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
                            ctx.lineWidth = Math.max(1, width * 0.1);
                            const start = width * 0.3125;
                            const end = width * 0.6875;
                            ctx.beginPath();
                            ctx.moveTo(start, start);
                            ctx.lineTo(end, end);
                            ctx.moveTo(end, start);
                            ctx.lineTo(start, end);
                            ctx.stroke();
                        }
                    }

                    MouseArea {
                        id: closeMouseArea

                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true

                        onClicked: root.dismissRequested()
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
                    lineHeight: root.statusLabelLineHeight
                    lineHeightMode: Text.FixedHeight
                    maximumLineCount: 1
                    style: header2
                    text: root.selectedHubStatusText
                    width: Math.max(0, parent.width - root.panelTextHorizontalInset * 2)
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
                    width: root.desktopContentWidth

                    Column {
                        id: appPanelColumn

                        anchors.centerIn: parent
                        spacing: root.mobileContentSpacing
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
                            font.pixelSize: root.titleTextSize
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
                            font.pixelSize: root.versionTextSize
                            font.styleName: "SemiBold"
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignHCenter
                            lineHeight: root.versionLineHeight
                            lineHeightMode: Text.FixedHeight
                            style: description
                            text: root.resolvedVersionText
                            width: parent.width
                        }

                        Column {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: root.mobileActionSpacing
                            width: root.desktopActionWidth

                            ActionLink {
                                id: createHubAction

                                enabled: !root.hubSessionController || !root.hubSessionController.busy
                                label: "Create new WhatSon Hub"
                                width: parent.width

                                onTriggered: {
                                    root.viewHookRequested();
                                    if (root.hubSessionController) {
                                        root.hubSessionController.clearLastError();
                                        root.hubSessionController.clearHubSelectionCandidates();
                                        root.openCreateHubDialog();
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
                                        root.hubSessionController.clearHubSelectionCandidates();
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
                anchors.centerIn: parent
                height: mobileAppColumn.implicitHeight
                width: root.mobileContentWidth

                Item {
                    anchors.centerIn: parent
                    height: mobileAppColumn.implicitHeight
                    width: parent.width

                    Column {
                        id: mobileAppColumn

                        anchors.centerIn: parent
                        spacing: root.mobileContentSpacing
                        width: parent.width

                        Column {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: root.mobileContentSpacing
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
                                color: LV.Theme.titleHeaderColor
                                font.pixelSize: root.titleTextSize
                                font.styleName: "Bold"
                                font.weight: Font.Bold
                                horizontalAlignment: Text.AlignHCenter
                                lineHeight: root.titleLineHeight
                                lineHeightMode: Text.FixedHeight
                                maximumLineCount: 1
                                style: title
                                text: "WhatSon"
                                width: parent.width
                                wrapMode: Text.NoWrap
                            }
                            LV.Label {
                                anchors.horizontalCenter: parent.horizontalCenter
                                color: LV.Theme.descriptionColor
                                font.pixelSize: root.versionTextSize
                                font.styleName: "SemiBold"
                                font.weight: Font.DemiBold
                                horizontalAlignment: Text.AlignHCenter
                                lineHeight: root.versionLineHeight
                                lineHeightMode: Text.FixedHeight
                                style: description
                                text: root.resolvedVersionText
                                width: root.mobileVersionWidth
                            }
                        }

                        Column {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: root.mobileActionSpacing
                            width: root.mobileActionWidth

                            ActionLink {
                                enabled: !root.hubSessionController || !root.hubSessionController.busy
                                label: "Create new WhatSon Hub"
                                width: parent.width

                                onTriggered: {
                                    root.viewHookRequested();
                                    if (root.hubSessionController) {
                                        root.hubSessionController.clearLastError();
                                        root.hubSessionController.clearHubSelectionCandidates();
                                        createHubDirectoryDialog.open();
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
                                        root.hubSessionController.clearHubSelectionCandidates();
                                        if (root.useAndroidExistingHubFileFlow)
                                            root.openSelectHubFileDialog();
                                        else
                                            selectHubDialog.open();
                                    } else {
                                        root.selectFileRequested();
                                    }
                                }
                            }
                        }

                        LV.Label {
                            anchors.horizontalCenter: parent.horizontalCenter
                            color: root.statusText.length > 0 ? root.statusTextColor : LV.Theme.descriptionColor
                            horizontalAlignment: Text.AlignHCenter
                            maximumLineCount: 4
                            style: description
                            text: root.mobileSelectionAssistText
                            width: parent.width
                            wrapMode: Text.WordWrap
                        }

                        Column {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: root.mobileActionSpacing
                            visible: root.hasHubSelectionCandidates
                            width: root.mobileActionWidth

                            Repeater {
                                model: root.hubSessionController ? root.hubSessionController.hubSelectionCandidateNames : []

                                delegate: ActionLink {
                                    required property int index
                                    required property string modelData

                                    enabled: !root.hubSessionController || !root.hubSessionController.busy
                                    label: modelData
                                    width: parent.width

                                    onTriggered: {
                                        if (root.hubSessionController)
                                            root.hubSessionController.loadHubSelectionCandidate(index);
                                    }
                                }
                            }
                        }
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
            color: actionLink.enabled ? root.linkColor : LV.Theme.descriptionColor
            font.pixelSize: root.actionTextSize
            font.styleName: "SemiBold"
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
            lineHeight: root.actionTextLineHeight
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

    Component.onDestruction: {
        if (root.createHubDialogInstance) {
            root.createHubDialogInstance.destroy();
            root.createHubDialogInstance = null;
        }
        if (root.selectHubFileDialogInstance) {
            root.selectHubFileDialogInstance.destroy();
            root.selectHubFileDialogInstance = null;
        }
    }
}
