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
            text: "Knowledge Repository"
        }
        LV.Label {
            style: description
            text: "A text knowledge hub that links organizational information assets, research records, and policy documents through tags."
        }
        LV.HStack {
            Layout.fillWidth: true
            spacing: LV.Theme.gap8

            MetricCard {
                Layout.fillWidth: true
                hint: "+57 this week"
                title: "Registered Docs"
                value: "1,842"
            }
            MetricCard {
                Layout.fillWidth: true
                hint: "Cross-reference trend rising"
                title: "Reuse Rate"
                value: "64%"
            }
            MetricCard {
                Layout.fillWidth: true
                hint: "Waiting for auto-tagging"
                title: "Unclassified"
                value: "37"
            }
        }
        InfoListCard {
            Layout.fillHeight: true
            Layout.fillWidth: true
            entries: ["Market Research 2026 Q1 - Consumer Perception Analysis", "New Feature FAQ - Sales Response Document", "Content Quality Standard - Editorial Guide", "AI Usage Policy - Risk Control Clauses"]
            title: "Recent Updates"
        }
    }
}
