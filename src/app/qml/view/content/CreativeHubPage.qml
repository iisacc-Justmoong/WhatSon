import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import "../../components"

Item {
    id: root

    LV.VStack {
        anchors.fill: parent
        spacing: LV.Theme.gap10

        LV.Label {
            style: title
            text: "Creative Ops Dashboard"
        }
        LV.Label {
            style: description
            text: "An operating layer that connects campaign briefs, copy, and visual planning documents into one workflow."
        }
        LV.HStack {
            Layout.fillWidth: true
            spacing: LV.Theme.gap8

            MetricCard {
                Layout.fillWidth: true
                hint: "+3 vs yesterday"
                title: "Active Projects"
                value: "28"
            }
            MetricCard {
                Layout.fillWidth: true
                hint: "4 near deadline"
                title: "Draft Completion"
                value: "76%"
            }
            MetricCard {
                Layout.fillWidth: true
                hint: "-0.6 days vs last week"
                title: "Approval Lead Time"
                value: "2.4 days"
            }
        }
        InfoListCard {
            Layout.fillHeight: true
            Layout.fillWidth: true
            entries: ["Brand repositioning brief - Editor stage", "Sales page key sentence set - Review stage", "Launch campaign Q&A guide - Approval stage", "Design copy set A/B - Revision stage"]
            title: "Today’s Creative Flow"
        }
    }
}
