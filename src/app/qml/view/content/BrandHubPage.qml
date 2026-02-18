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
            text: "Brand Intelligence Hub"
        }
        LV.Label {
            style: description
            text: "A brand knowledge hub that manages tone and manner, restricted expressions, and core messages as organization-wide rules."
        }
        LV.HStack {
            Layout.fillWidth: true
            spacing: LV.Theme.gap8

            MetricCard {
                Layout.fillWidth: true
                hint: "2 added this month"
                title: "Brand Rule Sets"
                value: "14"
            }
            MetricCard {
                Layout.fillWidth: true
                hint: "Top team: Product Marketing"
                title: "Compliance Rate"
                value: "92%"
            }
            MetricCard {
                Layout.fillWidth: true
                hint: "2 critical"
                title: "Review Alerts"
                value: "11"
            }
        }
        InfoListCard {
            Layout.fillHeight: true
            Layout.fillWidth: true
            entries: ["Core values: accuracy, trust, execution", "Restricted expression: exaggerated absolute claims", "Preferred vocabulary: evidence-based, metric-centered writing", "Voice spectrum: technical but avoids excessive jargon"]
            title: "Core Brand Signals"
        }
    }
}
