pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import LVRS 1.0 as LV
import "." as WindowView

Window {
    id: root

    readonly property int desktopDesignHeight: LV.Theme.scaffoldBlobPrimarySize + LV.Theme.gap20 + LV.Theme.gap2
    readonly property int desktopDesignWidth: LV.Theme.scaffoldBlobPrimarySize + LV.Theme.dialogMaxWidth - LV.Theme.gap12 - Math.round(LV.Theme.strokeThin)
    readonly property int desktopMinHeight: LV.Theme.scaffoldBlobSecondaryHeight + LV.Theme.gap20 + LV.Theme.gap20
    readonly property int desktopMinWidth: LV.Theme.scaffoldBlobSecondaryWidth - LV.Theme.gap20
    property int defaultHeight: root.desktopMinHeight
    property int defaultWidth: root.desktopMinWidth
    readonly property int designHeight: root.desktopDesignHeight
    readonly property int designWidth: root.desktopDesignWidth
    readonly property int fixedHeight: Math.max(defaultHeight, minHeight)
    readonly property int fixedWidth: Math.max(defaultWidth, minWidth)
    readonly property int effectiveHeight: root.fixedHeight
    readonly property int effectiveWidth: root.fixedWidth
    property var hostWindow: null
    property var hubSessionController: null
    property int minHeight: root.desktopMinHeight
    property int minWidth: root.desktopMinWidth
    property color panelColor: LV.Theme.panelBackground06
    property color sidePanelColor: LV.Theme.panelBackground10
    property bool standaloneMode: false
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

    color: LV.Theme.accentTransparent
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

    WindowView.OnboardingContent {
        anchors.fill: parent
        hostWindow: root.hostWindow
        hubSessionController: root.hubSessionController
        panelColor: root.panelColor
        sidePanelColor: root.sidePanelColor
        standaloneMode: root.standaloneMode
        versionText: root.versionText

        onCompleted: {
            root.close();
        }
        onCreateFileRequested: root.createFileRequested()
        onDismissRequested: root.close()
        onRequestWindowMove: {
            if (typeof root.startSystemMove === "function")
                root.startSystemMove();
        }
        onSelectFileRequested: root.selectFileRequested()
        onViewHookRequested: root.viewHookRequested()
    }
}
