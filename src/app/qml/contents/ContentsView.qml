pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV

Rectangle {
    id: contentsView

    readonly property int defaultDesignHeight: LV.Theme.dialogMaxWidth + LV.Theme.dialogMinWidth + LV.Theme.gap16
    readonly property int defaultDesignWidth: LV.Theme.dialogMaxWidth + LV.Theme.inputWidthMd + LV.Theme.buttonMinWidth + LV.Theme.gap2
    readonly property int defaultActiveLineNumber: gutterLayoutMetrics.defaultActiveLineNumber
    readonly property int defaultLineNumberCount: gutterLayoutMetrics.designLineNumberCount
    readonly property int defaultMinimapRowCount: minimapLayoutMetrics.designRowCount
    readonly property string defaultEditorText: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Integer malesuada, purus vitae faucibus facilisis, massa lectus dignissim metus, vel tempor elit libero sed nisl. Donec interdum, metus ac facilisis consequat, augue lorem interdum lorem, sed tristique erat risus sed nibh. Curabitur interdum sapien sed neque luctus, vitae hendrerit erat posuere. Praesent vel lacus ut arcu vulputate dignissim.\n" + "Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Aenean vel metus ut nisl convallis volutpat. Nam interdum sapien non turpis gravida, at placerat magna facilisis. Sed dignissim orci id justo dignissim, vel tincidunt risus malesuada. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Morbi non libero at nulla faucibus pulvinar.\n" + "Aliquam erat volutpat. Nulla facilisi. Proin luctus, sapien sed faucibus cursus, sapien est interdum risus, quis gravida lorem elit vitae arcu. Suspendisse potenti. Mauris porta, sem vitae malesuada tempus, augue nisl posuere libero, sit amet interdum erat lacus vel lectus. Cras dignissim, elit sed pretium dictum, lacus augue tincidunt turpis, vitae fermentum lorem arcu vitae justo.\n" + "Donec luctus felis a lectus luctus, non convallis mi vehicula. Etiam vel nunc vitae risus dictum consequat. Phasellus sit amet ligula id est fermentum congue. Maecenas tristique purus vitae neque malesuada, non efficitur ligula faucibus. Quisque non risus vitae arcu convallis ultrices. Sed vitae magna sed massa feugiat dictum. Curabitur ut erat ac massa vehicula vulputate.Lorem ipsum dolor sit amet, consectetur adipiscing elit. Integer malesuada, purus vitae faucibus facilisis, massa lectus dignissim metus, vel tempor elit libero sed nisl. Donec interdum, metus ac facilisis consequat, augue lorem interdum lorem, sed tristique erat risus sed nibh. Curabitur interdum sapien sed neque luctus, vitae hendrerit erat posuere. Praesent vel lacus ut arcu vulputate dignissim.\n" + "Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Aenean vel metus ut nisl convallis volutpat. Nam interdum sapien non turpis gravida, at placerat magna facilisis. Sed dignissim orci id justo dignissim, vel tincidunt risus malesuada. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Morbi non libero at nulla faucibus pulvinar.\n" + "Aliquam erat volutpat. Nulla facilisi. Proin luctus, sapien sed faucibus cursus, sapien est interdum risus, quis gravida lorem elit vitae arcu. Suspendisse potenti. Mauris porta, sem vitae malesuada tempus, augue nisl posuere libero, sit amet interdum erat lacus vel lectus. Cras dignissim, elit sed pretium dictum, lacus augue tincidunt turpis, vitae fermentum lorem arcu vitae justo.\n" + "Donec luctus felis a lectus luctus, non convallis mi vehicula. Etiam vel nunc vitae risus dictum consequat. Phasellus sit amet ligula id est fermentum congue. Maecenas tristique purus vitae neque malesuada, non efficitur ligula faucibus. Quisque non risus vitae arcu convallis ultrices. Sed vitae magna sed massa feugiat dictum. Curabitur ut erat ac massa vehicula vulputate."
    property int activeLineNumber: defaultActiveLineNumber
    property string editorText: defaultEditorText
    property color editorTextColor: LV.Theme.bodyColor
    property color gutterColor: LV.Theme.panelBackground02
    property int lineNumberCount: defaultLineNumberCount
    property color lineNumberActiveColor: LV.Theme.captionColor
    property color lineNumberColor: LV.Theme.disabledColor
    property color minimapLineColor: LV.Theme.captionColor
    property int minimapRowCount: defaultMinimapRowCount
    property color surfaceColor: LV.Theme.panelBackground02

    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        contentsView.viewHookRequested(hookReason);
    }

    ContentsGutterLayoutMetrics {
        id: gutterLayoutMetrics

        controlHeightMd: LV.Theme.controlHeightMd
        controlHeightSm: LV.Theme.controlHeightSm
        dialogMaxWidth: LV.Theme.dialogMaxWidth
        gapNone: LV.Theme.gapNone
        gap2: LV.Theme.gap2
        gap3: LV.Theme.gap3
        gap5: LV.Theme.gap5
        gap7: LV.Theme.gap7
        gap14: LV.Theme.gap14
        gap20: LV.Theme.gap20
        gap24: LV.Theme.gap24
        inputWidthMd: LV.Theme.inputWidthMd
        objectName: "contentsViewGutterLayoutMetrics"
        strokeThin: LV.Theme.strokeThin
    }

    ContentsGutterLineNumberGeometry {
        id: gutterLineNumberGeometry

        fallbackLineHeight: LV.Theme.textBodyLineHeight
        fallbackTopInset: LV.Theme.gap16
        lineNumberBaseOffset: gutterLayoutMetrics.lineNumberBaseOffset
        lineNumberCount: contentsView.lineNumberCount
        objectName: "contentsViewGutterLineNumberGeometry"
        sourceText: contentsView.editorText
    }

    ContentsGutterMarkerGeometry {
        id: gutterMarkerGeometry

        cursorPosition: LV.Theme.gapNone
        editorMounted: false
        lineNumberBaseOffset: gutterLayoutMetrics.lineNumberBaseOffset
        lineNumberEntries: gutterLineNumberGeometry.lineNumberEntries
        markerHeight: LV.Theme.textBodyLineHeight
        objectName: "contentsViewGutterMarkerGeometry"
        savedSourceText: contentsView.editorText
        sourceText: contentsView.editorText
    }

    ContentsMinimapLayoutMetrics {
        id: minimapLayoutMetrics

        buttonMinWidth: LV.Theme.buttonMinWidth
        gapNone: LV.Theme.gapNone
        gap8: LV.Theme.gap8
        gap12: LV.Theme.gap12
        gap20: LV.Theme.gap20
        gap24: LV.Theme.gap24
        objectName: "contentsViewMinimapLayoutMetrics"
        strokeThin: LV.Theme.strokeThin
    }

    clip: true
    color: contentsView.surfaceColor
    implicitHeight: contentsView.defaultDesignHeight
    implicitWidth: contentsView.defaultDesignWidth
    objectName: "figma-155-4561-ContentsView"

    LV.HStack {
        anchors.fill: parent
        objectName: "figma-155-5344-HStack"
        spacing: LV.Theme.gapNone

        Gutter {
            id: gutterPane

            Layout.fillHeight: true
            Layout.preferredWidth: gutterLayoutMetrics.defaultGutterWidth
            activeLineNumber: contentsView.activeLineNumber
            gutterColor: contentsView.gutterColor
            iconRailX: gutterLayoutMetrics.iconRailX
            lineNumberBaseOffset: gutterLayoutMetrics.lineNumberBaseOffset
            lineNumberActiveColor: contentsView.lineNumberActiveColor
            lineNumberColor: contentsView.lineNumberColor
            lineNumberCount: contentsView.lineNumberCount
            lineNumberEntries: gutterLineNumberGeometry.lineNumberEntries
            markerEntries: gutterMarkerGeometry.markerEntries

            onViewHookRequested: function (reason) {
                contentsView.viewHookRequested(reason);
            }
        }
        EditorView {
            id: editorPane

            Layout.fillHeight: true
            Layout.fillWidth: true
            editorText: contentsView.editorText
            textColor: contentsView.editorTextColor

            onViewHookRequested: function (reason) {
                contentsView.viewHookRequested(reason);
            }
        }
        Minimap {
            id: minimapPane

            Layout.fillHeight: true
            Layout.preferredWidth: minimapLayoutMetrics.defaultMinimapWidth
            lineColor: contentsView.minimapLineColor
            rowCount: contentsView.minimapRowCount

            onViewHookRequested: function (reason) {
                contentsView.viewHookRequested(reason);
            }
        }
    }
}
