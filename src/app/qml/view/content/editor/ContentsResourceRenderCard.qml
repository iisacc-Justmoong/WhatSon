pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: resourceCard

    property var resourceEntry: ({})
    property bool inlinePresentation: false
    property color borderColor: "#334E5157"
    property color cardColor: "#E61A1D22"

    readonly property string resourceDisplayName: resourceEntry.displayName !== undefined ? String(resourceEntry.displayName) : ""
    readonly property string resourceFormat: resourceEntry.format !== undefined ? String(resourceEntry.format) : ""
    readonly property string resourcePath: resourceEntry.resourcePath !== undefined ? String(resourceEntry.resourcePath) : ""
    readonly property string resourcePreviewText: resourceEntry.previewText !== undefined ? String(resourceEntry.previewText) : ""
    readonly property string resourceRenderMode: resourceEntry.renderMode !== undefined ? String(resourceEntry.renderMode) : ""
    readonly property string resourceResolvedPath: resourceEntry.resolvedPath !== undefined ? String(resourceEntry.resolvedPath) : ""
    readonly property string resourceSource: resourceEntry.source !== undefined ? String(resourceEntry.source) : ""
    readonly property string resourceType: resourceEntry.type !== undefined ? String(resourceEntry.type) : ""
    readonly property string resourceOpenTarget: resourceCard.resourceSource.length > 0
                                               ? resourceCard.resourceSource
                                               : resourceCard.resourceResolvedPath
    readonly property bool resourceOpenable: resourceCard.resourceOpenTarget.length > 0
    readonly property string resourceFileName: {
        if (resourceCard.resourceDisplayName.length > 0)
            return resourceCard.resourceDisplayName;
        if (resourceCard.resourcePath.length > 0)
            return resourceCard.resourcePath;
        return resourceCard.resourceModeTitle;
    }
    readonly property string resourceModeTitle: {
        if (resourceCard.resourceRenderMode === "image")
            return "Image Resource";
        if (resourceCard.resourceRenderMode === "video")
            return "Video Resource";
        if (resourceCard.resourceRenderMode === "audio")
            return "Audio Resource";
        if (resourceCard.resourceRenderMode === "pdf")
            return "PDF Resource";
        if (resourceCard.resourceRenderMode === "text")
            return "Text Resource";
        return "Document Resource";
    }
    readonly property string resourcePreviewTitle: {
        if (resourceCard.resourceRenderMode === "video")
            return "Video";
        if (resourceCard.resourceRenderMode === "audio")
            return "Audio";
        if (resourceCard.resourceRenderMode === "pdf")
            return "PDF";
        if (resourceCard.resourceRenderMode === "document")
            return "File";
        return resourceCard.resourceModeTitle;
    }
    readonly property bool inlineImagePresentation: resourceCard.inlinePresentation
                                                   && resourceCard.resourceRenderMode === "image"
    readonly property real inlineImageFrameWidth: Math.max(
                                                      120,
                                                      Math.min(
                                                          Number(resourceCard.width) || 0,
                                                          Number(inlineImageFrame.implicitWidth) || 480))
    readonly property real previewHeight: resourceCard.inlinePresentation
                                          ? 88
                                          : (resourceCard.resourceRenderMode === "text" ? 96 : 72)

    border.color: resourceCard.inlineImagePresentation ? "transparent" : resourceCard.borderColor
    border.width: resourceCard.inlineImagePresentation ? 0 : Math.max(1, Math.round(LV.Theme.strokeThin))
    clip: false
    color: resourceCard.inlineImagePresentation ? "transparent" : resourceCard.cardColor
    implicitHeight: resourceCard.inlineImagePresentation
                    ? inlineImageFrame.implicitHeight
                    : resourceRow.implicitHeight + LV.Theme.gap2 * 2
    height: implicitHeight
    radius: resourceCard.inlineImagePresentation ? 0 : LV.Theme.radiusSm
    width: parent ? parent.width : 0

    Item {
        id: inlineImageViewport

        anchors.fill: parent
        visible: resourceCard.inlineImagePresentation

        ContentsImageResourceFrame {
            id: inlineImageFrame

            anchors.horizontalCenter: parent.horizontalCenter
            fileNameText: resourceCard.resourceFileName
            menuButtonEnabled: resourceCard.resourceOpenable
            panelColor: resourceCard.cardColor
            resourceTitle: "Image"
            width: resourceCard.inlineImageFrameWidth

            onMenuRequested: {
                if (resourceCard.resourceOpenable)
                    Qt.openUrlExternally(resourceCard.resourceOpenTarget);
            }

            ContentsResourceViewer {
                id: inlineResourceViewer

                anchors.fill: parent
                imageFillMode: Image.PreserveAspectCrop
                resourceEntry: resourceCard.resourceEntry
            }

            LV.Label {
                anchors.centerIn: parent
                color: LV.Theme.textSecondary
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 3
                style: body
                text: inlineResourceViewer.renderFailureReason.length > 0
                          ? inlineResourceViewer.renderFailureReason
                          : resourceCard.resourceFileName
                visible: !inlineResourceViewer.resourceRenderable
                width: Math.max(120, parent.width - LV.Theme.gap4 * 2)
                wrapMode: Text.Wrap
            }
        }
    }

    RowLayout {
        id: resourceRow

        anchors.fill: parent
        anchors.margins: LV.Theme.gap2
        spacing: LV.Theme.gap2
        visible: !resourceCard.inlineImagePresentation

        Rectangle {
            Layout.alignment: Qt.AlignTop
            Layout.preferredHeight: resourceCard.previewHeight
            Layout.preferredWidth: 120
            color: "#CC0F141A"
            radius: LV.Theme.radiusSm

            Image {
                anchors.fill: parent
                anchors.margins: 1
                asynchronous: true
                cache: true
                fillMode: Image.PreserveAspectFit
                source: resourceCard.resourceOpenTarget
                visible: resourceCard.resourceRenderMode === "image" && resourceCard.resourceOpenTarget.length > 0
            }
            LV.Label {
                anchors.fill: parent
                anchors.margins: LV.Theme.gap2
                color: LV.Theme.textSecondary
                elide: Text.ElideRight
                horizontalAlignment: resourceCard.resourceRenderMode === "text" ? Text.AlignLeft : Text.AlignHCenter
                style: body
                text: resourceCard.resourceRenderMode === "text"
                          ? (resourceCard.resourcePreviewText.length > 0 ? resourceCard.resourcePreviewText : "No text preview")
                          : resourceCard.resourcePreviewTitle
                verticalAlignment: resourceCard.resourceRenderMode === "text" ? Text.AlignTop : Text.AlignVCenter
                visible: resourceCard.resourceRenderMode !== "image"
                wrapMode: Text.Wrap
            }
        }
        LV.VStack {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: LV.Theme.gap2

            LV.Label {
                color: LV.Theme.textPrimary
                elide: resourceCard.inlinePresentation ? Text.ElideRight : Text.ElideNone
                style: body
                text: resourceCard.resourceDisplayName.length > 0 ? resourceCard.resourceDisplayName : resourceCard.resourceModeTitle
                wrapMode: resourceCard.inlinePresentation ? Text.NoWrap : Text.Wrap
            }
            LV.Label {
                color: LV.Theme.textSecondary
                elide: resourceCard.inlinePresentation ? Text.ElideRight : Text.ElideNone
                style: body
                text: resourceCard.resourceModeTitle
                wrapMode: resourceCard.inlinePresentation ? Text.NoWrap : Text.Wrap
            }
            LV.Label {
                color: LV.Theme.textSecondary
                elide: resourceCard.inlinePresentation ? Text.ElideRight : Text.ElideNone
                style: body
                text: "type=" + resourceCard.resourceType + "  format=" + resourceCard.resourceFormat
                wrapMode: resourceCard.inlinePresentation ? Text.NoWrap : Text.Wrap
            }
            LV.Label {
                color: LV.Theme.textTertiary
                elide: Text.ElideMiddle
                style: body
                text: resourceCard.resourcePath
                wrapMode: resourceCard.inlinePresentation ? Text.NoWrap : Text.Wrap
            }
            LV.IconButton {
                iconName: "generalshow"
                iconSize: 14
                visible: resourceCard.resourceOpenable

                onClicked: Qt.openUrlExternally(resourceCard.resourceOpenTarget)
            }
        }
    }
}
