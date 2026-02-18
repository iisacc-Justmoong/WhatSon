import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: root

    LV.HStack {
        anchors.fill: parent
        spacing: LV.Theme.gap10

        LV.AppCard {
            Layout.fillHeight: true
            Layout.fillWidth: true
            subtitle: "A body editing space that references brand rules and knowledge context at the same time."
            title: "Editor Studio"

            LV.TextEditor {
                editorHeight: 480
                mode: plainTextMode
                placeholderText: "Write the document using the LVRS block component policy."
                showRenderedOutput: false
                text: "Document Title: Spring 2026 Campaign Message Architecture\n\nCore Narrative:\nThe customer problem does not start from missing features, but from the absence of a trustworthy information flow. This document defines message principles that combine product value and brand trust in one context."
                width: parent ? parent.width : implicitWidth
            }
        }
        LV.AppCard {
            Layout.fillHeight: true
            Layout.preferredWidth: 300
            subtitle: "Execution Criteria"
            title: "Editing Rules"

            LV.VStack {
                spacing: LV.Theme.gap6
                width: parent ? parent.width : implicitWidth

                LV.ListItem {
                    enabled: false
                    label: "1. Separate facts from interpretation"
                    showChevron: false
                }
                LV.ListItem {
                    enabled: false
                    label: "2. Auto-check restricted brand expressions"
                    showChevron: false
                }
                LV.ListItem {
                    enabled: false
                    label: "3. Attach evidence document links as body annotations"
                    showChevron: false
                }
                LV.Spacer {
                }
            }
        }
    }
}
