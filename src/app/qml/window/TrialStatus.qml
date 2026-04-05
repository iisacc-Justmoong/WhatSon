pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import LVRS 1.0 as LV

Window {
    id: root

    readonly property int fixedHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(372)))
    readonly property int fixedWidth: Math.max(0, Math.round(LV.Theme.scaleMetric(452)))
    readonly property bool hasTrialPolicy: root.trialActivationPolicy !== null
    readonly property bool bypassedByAuthentication: root.hasTrialPolicy
                                                    && root.trialActivationPolicy.bypassedByAuthentication !== undefined
                                                    ? Boolean(root.trialActivationPolicy.bypassedByAuthentication)
                                                    : false
    readonly property bool trialActive: root.hasTrialPolicy && root.trialActivationPolicy.active !== undefined
                                        ? Boolean(root.trialActivationPolicy.active)
                                        : false
    readonly property int remainingDays: root.hasTrialPolicy && root.trialActivationPolicy.remainingDays !== undefined
                                         ? Number(root.trialActivationPolicy.remainingDays)
                                         : 0
    readonly property int elapsedDays: root.hasTrialPolicy && root.trialActivationPolicy.elapsedDays !== undefined
                                       ? Number(root.trialActivationPolicy.elapsedDays)
                                       : 0
    readonly property int trialLengthDays: root.hasTrialPolicy
                                           && root.trialActivationPolicy.trialLengthDays !== undefined
                                           ? Number(root.trialActivationPolicy.trialLengthDays)
                                           : 0
    readonly property color chromeColor: LV.Theme.panelBackground06
    readonly property int contentInset: LV.Theme.gap20
    readonly property int dividerHeight: Math.max(1, Math.round(LV.Theme.strokeThin))
    readonly property int headlineTextSize: Math.max(0, Math.round(LV.Theme.scaleMetric(18)))
    readonly property color surfaceColor: LV.Theme.panelBackground10
    readonly property int primaryMetricTextSize: Math.max(0, Math.round(LV.Theme.scaleMetric(36)))
    readonly property int summaryLineHeight: Math.max(0, Math.round(LV.Theme.scaleMetric(22)))
    readonly property int summaryTextSize: Math.max(0, Math.round(LV.Theme.scaleMetric(14)))
    readonly property int surfaceRadius: Math.max(0, Math.round(LV.Theme.scaleMetric(24)))
    readonly property int trialDetailTextSize: Math.max(0, Math.round(LV.Theme.scaleMetric(13)))
    readonly property int trialSectionTitleSize: Math.max(0, Math.round(LV.Theme.scaleMetric(14)))
    readonly property color headlineColor: {
        if (root.bypassedByAuthentication)
            return LV.Theme.accent;
        if (root.trialActive)
            return LV.Theme.titleHeaderColor;
        return LV.Theme.danger;
    }
    readonly property string headlineText: {
        if (root.bypassedByAuthentication)
            return "Trial unlocked";
        if (root.trialActive)
            return "Trial active";
        return "Trial expired";
    }
    readonly property string primaryMetricText: {
        if (root.bypassedByAuthentication)
            return "Authenticated";
        return root.remainingDays + " days";
    }
    readonly property string summaryText: {
        if (root.bypassedByAuthentication)
            return "A registered session is active on this device, so the local trial limit is bypassed.";
        if (root.trialActive)
            return "WhatSon Trial is running. You can keep evaluating the desktop workspace until the remaining trial period ends.";
        return "The local WhatSon Trial period has ended on this device. Registration is required to continue beyond the expired trial window.";
    }
    property var hostWindow: null
    property var trialActivationPolicy: null

    function formattedDate(value) {
        if (value === undefined || value === null)
            return "Unknown";
        if (value instanceof Date)
            return Qt.formatDate(value, "yyyy-MM-dd");
        var text = String(value).trim();
        return text.length > 0 && text !== "Invalid Date" ? text : "Unknown";
    }

    function recenter() {
        if (root.hostWindow) {
            x = root.hostWindow.x + Math.round((root.hostWindow.width - root.width) / 2);
            y = root.hostWindow.y + Math.round((root.hostWindow.height - root.height) / 2);
            return;
        }
        var targetScreen = root.screen;
        if (!targetScreen)
            return;
        x = Math.round(targetScreen.virtualX + (targetScreen.width - root.width) / 2);
        y = Math.round(targetScreen.virtualY + (targetScreen.height - root.height) / 2);
    }

    color: root.chromeColor
    flags: Qt.Dialog
    height: fixedHeight
    maximumHeight: fixedHeight
    maximumWidth: fixedWidth
    minimumHeight: fixedHeight
    minimumWidth: fixedWidth
    title: "WhatSon Trial"
    transientParent: hostWindow
    visible: false
    width: fixedWidth

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

    Rectangle {
        anchors.fill: parent
        color: root.chromeColor

        Rectangle {
            anchors.fill: parent
            anchors.margins: LV.Theme.gap4
            color: root.surfaceColor
            radius: root.surfaceRadius

            Column {
                anchors.fill: parent
                anchors.margins: root.contentInset
                spacing: LV.Theme.gap4

                LV.Label {
                    color: root.headlineColor
                    font.bold: true
                    font.pixelSize: root.headlineTextSize
                    text: root.headlineText
                }

                LV.Label {
                    color: LV.Theme.titleHeaderColor
                    font.bold: true
                    font.pixelSize: root.primaryMetricTextSize
                    text: root.primaryMetricText
                }

                LV.Label {
                    color: LV.Theme.descriptionColor
                    font.pixelSize: root.summaryTextSize
                    lineHeight: root.summaryLineHeight
                    text: root.summaryText
                    width: parent.width
                    wrapMode: Text.WordWrap
                }

                Rectangle {
                    color: root.chromeColor
                    height: root.dividerHeight
                    width: parent.width
                }

                Column {
                    spacing: LV.Theme.gap2

                    LV.Label {
                        color: LV.Theme.titleHeaderColor
                        font.bold: true
                        font.pixelSize: root.trialSectionTitleSize
                        text: "Trial details"
                    }

                    LV.Label {
                        color: LV.Theme.descriptionColor
                        font.pixelSize: root.trialDetailTextSize
                        text: "Trial length: " + root.trialLengthDays + " days"
                    }

                    LV.Label {
                        color: LV.Theme.descriptionColor
                        font.pixelSize: root.trialDetailTextSize
                        text: "Elapsed: " + root.elapsedDays + " days"
                    }

                    LV.Label {
                        color: LV.Theme.descriptionColor
                        font.pixelSize: root.trialDetailTextSize
                        text: "Installed: " + root.formattedDate(root.hasTrialPolicy ? root.trialActivationPolicy.installDate : null)
                    }

                    LV.Label {
                        color: LV.Theme.descriptionColor
                        font.pixelSize: root.trialDetailTextSize
                        text: "Last active day: " + root.formattedDate(root.hasTrialPolicy ? root.trialActivationPolicy.lastActiveDate : null)
                    }
                }
            }
        }
    }
}
