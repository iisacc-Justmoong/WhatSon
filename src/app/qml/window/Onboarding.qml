pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import LVRS 1.0 as LV
import "." as WindowView

Window {
    id: root

    readonly property int desktopDesignHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(542)))
    readonly property int desktopDesignWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(867)))
    readonly property int desktopMinHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(420)))
    readonly property int desktopMinWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(620)))
    readonly property bool isMobilePlatform: Qt.platform.os === "android" || Qt.platform.os === "ios"
    readonly property int mobileDesignHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(762)))
    readonly property int mobileDesignWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(470)))
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
    property var hostWindow: null
    property var hubSessionController: null
    property int minHeight: compactMinHeight
    property int minWidth: compactMinWidth
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

    WindowView.OnboardingContent {
        anchors.fill: parent
        hostWindow: root.hostWindow
        hubSessionController: root.hubSessionController
        panelColor: root.panelColor
        sidePanelColor: root.sidePanelColor
        standaloneMode: root.standaloneMode
        versionText: root.versionText

        onCompleted: {
            root.close()
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
