pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: exceptionOverlay

    required property var contentsView

    visible: contentsView.noteDocumentExceptionVisible
    z: 1

    Column {
        anchors.centerIn: parent
        spacing: LV.Theme.gap4
        width: Math.max(
                   0,
                   Math.min(
                       parent ? Number(parent.width) || 0 : 0,
                       Math.round(LV.Theme.scaleMetric(360))))

        LV.Label {
            color: LV.Theme.titleHeaderColor
            horizontalAlignment: Text.AlignHCenter
            style: body
            text: exceptionOverlay.contentsView.noteDocumentExceptionTitle
            width: parent.width
            wrapMode: Text.Wrap
        }
        LV.Label {
            color: LV.Theme.descriptionColor
            horizontalAlignment: Text.AlignHCenter
            style: caption
            text: exceptionOverlay.contentsView.noteDocumentExceptionMessage
            width: parent.width
            wrapMode: Text.Wrap
        }
    }
}
