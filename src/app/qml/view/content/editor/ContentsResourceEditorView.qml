pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import WhatSon.App.Internal 1.0

Rectangle {
    id: resourceEditor

    property color displayColor: "transparent"
    property var resourceEntry: ({})

    readonly property string resourceId: resourceEditor.resourceEntry.noteId !== undefined
                                         ? String(resourceEditor.resourceEntry.noteId).trim()
                                         : ""
    readonly property string resourceDisplayName: {
        if (resourceEditor.resourceEntry.displayName !== undefined) {
            const displayName = String(resourceEditor.resourceEntry.displayName).trim();
            if (displayName.length > 0)
                return displayName;
        }
        if (resourceEditor.resourceEntry.resourcePath !== undefined) {
            const resourcePath = String(resourceEditor.resourceEntry.resourcePath).trim();
            if (resourcePath.length > 0)
                return resourcePath;
        }
        return "Resource Editor";
    }
    readonly property string resourceFormat: resourceEditor.resourceEntry.format !== undefined
                                             ? String(resourceEditor.resourceEntry.format).trim()
                                             : ""
    readonly property string resourcePath: resourceEditor.resourceEntry.resourcePath !== undefined
                                           ? String(resourceEditor.resourceEntry.resourcePath).trim()
                                           : ""
    readonly property string resourcePreviewText: resourceEditor.resourceEntry.previewText !== undefined
                                                  ? String(resourceEditor.resourceEntry.previewText).trim()
                                                  : ""
    readonly property string resourceRenderMode: resourceEditor.resourceEntry.renderMode !== undefined
                                                 ? String(resourceEditor.resourceEntry.renderMode).trim()
                                                 : ""
    readonly property string resourceResolvedPath: resourceEditor.resourceEntry.resolvedPath !== undefined
                                                   ? String(resourceEditor.resourceEntry.resolvedPath).trim()
                                                   : ""
    readonly property string resourceSource: resourceEditor.resourceEntry.source !== undefined
                                             ? String(resourceEditor.resourceEntry.source).trim()
                                             : ""
    readonly property string resourceType: resourceEditor.resourceEntry.type !== undefined
                                           ? String(resourceEditor.resourceEntry.type).trim()
                                           : ""
    readonly property bool hasResourceSelection: resourceEditor.resourceId.length > 0
    readonly property string primaryLocationText: resourceEditor.resourceResolvedPath.length > 0
                                                 ? resourceEditor.resourceResolvedPath
                                                 : (resourceEditor.resourceSource.length > 0
                                                    ? resourceEditor.resourceSource
                                                    : resourceEditor.resourcePath)
    readonly property string emptyStateMessage: "Select a resource from the list to preview it here."
    readonly property string unsupportedStateMessage: {
        if (!resourceEditor.hasResourceSelection)
            return resourceEditor.emptyStateMessage;
        if (resourceBitmapState.bitmapPreviewCandidate && resourceBitmapState.incompatibilityReason.length > 0)
            return resourceBitmapState.incompatibilityReason;
        return "The dedicated resource editor currently previews image resources first.";
    }

    signal viewHookRequested

    function requestViewHook(reason) {
        resourceEditor.viewHookRequested();
    }

    color: resourceEditor.displayColor
    clip: true

    ResourceBitmapViewer {
        id: resourceBitmapState

        resourceEntry: resourceEditor.resourceEntry
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: LV.Theme.gap4
        spacing: LV.Theme.gap4

        Rectangle {
            Layout.fillWidth: true
            color: LV.Theme.panelBackground03
            radius: LV.Theme.radiusSm

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: LV.Theme.gap4
                spacing: LV.Theme.gap3

                LV.Label {
                    Layout.fillWidth: true
                    color: LV.Theme.textPrimary
                    elide: Text.ElideRight
                    style: body
                    text: resourceEditor.resourceDisplayName
                }

                Flow {
                    Layout.fillWidth: true
                    spacing: LV.Theme.gap2

                    Rectangle {
                        visible: resourceEditor.resourceType.length > 0
                        color: LV.Theme.panelBackground08
                        height: typeChipLabel.implicitHeight + LV.Theme.gap2
                        radius: LV.Theme.radiusSm
                        width: typeChipLabel.implicitWidth + LV.Theme.gap4

                        LV.Label {
                            id: typeChipLabel

                            anchors.centerIn: parent
                            color: LV.Theme.textSecondary
                            style: caption
                            text: "Type " + resourceEditor.resourceType
                        }
                    }

                    Rectangle {
                        visible: resourceEditor.resourceFormat.length > 0
                        color: LV.Theme.panelBackground08
                        height: formatChipLabel.implicitHeight + LV.Theme.gap2
                        radius: LV.Theme.radiusSm
                        width: formatChipLabel.implicitWidth + LV.Theme.gap4

                        LV.Label {
                            id: formatChipLabel

                            anchors.centerIn: parent
                            color: LV.Theme.textSecondary
                            style: caption
                            text: "Format " + resourceEditor.resourceFormat
                        }
                    }

                    Rectangle {
                        visible: resourceEditor.resourceRenderMode.length > 0
                        color: LV.Theme.panelBackground08
                        height: renderModeChipLabel.implicitHeight + LV.Theme.gap2
                        radius: LV.Theme.radiusSm
                        width: renderModeChipLabel.implicitWidth + LV.Theme.gap4

                        LV.Label {
                            id: renderModeChipLabel

                            anchors.centerIn: parent
                            color: LV.Theme.textSecondary
                            style: caption
                            text: "Mode " + resourceEditor.resourceRenderMode
                        }
                    }
                }

                LV.Label {
                    Layout.fillWidth: true
                    color: LV.Theme.descriptionColor
                    elide: Text.ElideMiddle
                    style: caption
                    text: resourceEditor.primaryLocationText.length > 0
                          ? resourceEditor.primaryLocationText
                          : "No resource file is currently selected."
                    visible: text.length > 0
                }
            }
        }

        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: "#CC0F141A"
            radius: LV.Theme.radiusSm

            ContentsResourceViewer {
                anchors.fill: parent
                anchors.margins: LV.Theme.gap4
                imageAllowUpscale: false
                imageFillMode: Image.PreserveAspectFit
                resourceEntry: resourceEditor.resourceEntry
                visible: resourceEditor.hasResourceSelection && resourceBitmapState.bitmapRenderable
            }

            ColumnLayout {
                anchors.centerIn: parent
                spacing: LV.Theme.gap2
                visible: !resourceEditor.hasResourceSelection || !resourceBitmapState.bitmapRenderable
                width: Math.min(parent.width - LV.Theme.gap8, Math.max(220, parent.width * 0.7))

                LV.Label {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    color: LV.Theme.textPrimary
                    horizontalAlignment: Text.AlignHCenter
                    style: body
                    text: !resourceEditor.hasResourceSelection
                          ? "No Resource Selected"
                          : "Preview Unavailable"
                    wrapMode: Text.Wrap
                }

                LV.Label {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    color: LV.Theme.descriptionColor
                    horizontalAlignment: Text.AlignHCenter
                    maximumLineCount: 4
                    style: body
                    text: resourceEditor.unsupportedStateMessage
                    wrapMode: Text.Wrap
                }
            }
        }

        LV.Label {
            Layout.fillWidth: true
            color: LV.Theme.descriptionColor
            maximumLineCount: 3
            style: caption
            text: resourceEditor.resourcePreviewText.length > 0
                  ? resourceEditor.resourcePreviewText
                  : "The resource editor now owns direct resource preview for the Resources hierarchy."
            visible: text.length > 0
            wrapMode: Text.Wrap
        }
    }
}
