import QtQuick
import LVRS 1.0 as LV

Item {
    id: detailFileStatForm

    property var fileStatViewModel: null
    readonly property var summaryLines: detailFileStatForm.resolveTextLines(detailFileStatForm.fileStatViewModel ? detailFileStatForm.fileStatViewModel.summaryLines : [])
    readonly property var textMetricLines: detailFileStatForm.resolveTextLines(detailFileStatForm.fileStatViewModel ? detailFileStatForm.fileStatViewModel.textMetricLines : [])
    readonly property var activityLines: detailFileStatForm.resolveTextLines(detailFileStatForm.fileStatViewModel ? detailFileStatForm.fileStatViewModel.activityLines : [])

    function resolveTextLines(values) {
        if (!values)
            return [];
        if (Array.isArray(values))
            return values;
        if (values.length !== undefined)
            return Array.prototype.slice.call(values);
        return [];
    }

    component FileStatLineLabel: LV.Label {
        required property string lineText

        color: LV.Theme.descriptionColor
        style: description
        text: lineText
        width: parent ? parent.width : 0
        wrapMode: Text.WordWrap
    }

    component FileStatTextSection: Item {
        id: fileStatTextSection

        required property string sectionObjectName
        required property var lines

        readonly property var normalizedLines: detailFileStatForm.resolveTextLines(lines)
        implicitHeight: sectionColumn.implicitHeight
        implicitWidth: sectionColumn.implicitWidth

        Column {
            id: sectionColumn

            anchors.fill: parent
            spacing: 0

            Repeater {
                model: fileStatTextSection.normalizedLines.length

                delegate: FileStatLineLabel {
                    required property int index

                    lineText: String(fileStatTextSection.normalizedLines[index] || "")
                    objectName: fileStatTextSection.sectionObjectName + "Line" + index
                    visible: lineText.length > 0
                    width: parent ? parent.width : fileStatTextSection.width
                }
            }
        }
    }

    objectName: "DetailFileStatForm"

    Flickable {
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds
        boundsMovement: Flickable.StopAtBounds
        clip: true
        contentHeight: statsColumn.y + statsColumn.implicitHeight + 2
        contentWidth: width
        interactive: contentHeight > height

        Column {
            id: statsColumn

            x: 8
            y: 2
            objectName: "Form"
            spacing: 10
            width: Math.max(0, parent.width - 16)

            FileStatTextSection {
                objectName: "SummaryStatsSection"
                lines: detailFileStatForm.summaryLines
                sectionObjectName: "SummaryStats"
                width: parent.width
            }
            FileStatTextSection {
                objectName: "TextMetricStatsSection"
                lines: detailFileStatForm.textMetricLines
                sectionObjectName: "TextMetricStats"
                width: parent.width
            }
            FileStatTextSection {
                objectName: "ActivityStatsSection"
                lines: detailFileStatForm.activityLines
                sectionObjectName: "ActivityStats"
                width: parent.width
            }
        }
    }
}
