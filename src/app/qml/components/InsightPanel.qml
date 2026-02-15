import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

LV.AppCard {
    id: root

    title: "Strategic Insight"

    LV.VStack {
        spacing: LV.Theme.gap8
        width: parent ? parent.width : implicitWidth

        LV.AppCard {
            implicitHeight: 150
            subtitle: "Brand-Editor Link"
            title: "Core Proposal"

            LV.Label {
                style: description
                text: "Combining brand keyword density with editing template usage can reduce document production time by 18%."
                width: parent ? parent.width : implicitWidth
                wrapMode: Text.WordWrap
            }
        }
        LV.AppCard {
            implicitHeight: 250
            subtitle: "Priority Order"
            title: "Work Queue"

            LV.VStack {
                spacing: LV.Theme.gap6
                width: parent ? parent.width : implicitWidth

                LV.ListItem {
                    detail: "High"
                    enabled: false
                    label: "Brand Voice Guide v2 approval pending"
                    showChevron: false
                }
                LV.ListItem {
                    detail: "Medium"
                    enabled: false
                    label: "3 campaign copy drafts require review"
                    showChevron: false
                }
                LV.ListItem {
                    detail: "Low"
                    enabled: false
                    label: "12 knowledge notes require tag normalization"
                    showChevron: false
                }
                LV.Spacer {
                }
            }
        }
        LV.Spacer {
        }
    }
}
