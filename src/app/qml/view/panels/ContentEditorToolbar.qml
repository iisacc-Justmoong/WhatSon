pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

LV.HStack {
    id: editorToolbar

    readonly property string figmaNodeId: "399:9846"
    readonly property int figmaWidth: LV.Theme.scaleMetric(784)
    readonly property int figmaHeight: LV.Theme.gap20
    readonly property int toolbarPadding: LV.Theme.gap2
    readonly property int figmaStyleBarWidth: LV.Theme.scaleMetric(664)
    readonly property int figmaModeBarWidth: LV.Theme.scaleMetric(44)
    readonly property int figmaComboHeight: LV.Theme.scaleMetric(18)
    readonly property int figmaComboLargeWidth: LV.Theme.scaleMetric(97)
    readonly property int figmaComboSmallWidth: LV.Theme.scaleMetric(54)
    readonly property int figmaComboWeightWidth: LV.Theme.scaleMetric(88)
    readonly property int figmaIconButtonWidth: LV.Theme.gap20
    readonly property int figmaIconMenuButtonWidth: LV.Theme.scaleMetric(34)
    readonly property color figmaPanelBackground10: LV.Theme.panelBackground10
    readonly property color figmaPanelBackground12: LV.Theme.panelBackground12
    readonly property color figmaAccent: LV.Theme.primary
    readonly property color figmaGray11: "#CED0D6"
    readonly property color figmaGreen6: "#57965C"
    readonly property color figmaHighlight: "#FFD60A"
    readonly property color figmaHighlightOverlay: "#54FFD91A"
    readonly property color figmaTitleHeader: "#E6FFFFFF"
    readonly property int figmaBodyTextSize: LV.Theme.textBody
    readonly property int figmaBodyLineHeight: LV.Theme.textBodyLineHeight
    readonly property int figmaBodyWeight: LV.Theme.textBodyWeight
    readonly property int responsiveToolbarItemCount: 7
    readonly property real responsiveAvailableContentWidth: Math.max(LV.Theme.gapNone, editorToolbar.width - editorToolbar.toolbarPadding * 2)
    readonly property int firstVisibleToolbarItemIndex: editorToolbar.firstVisibleToolbarItemIndexForWidth(editorToolbar.responsiveAvailableContentWidth)
    readonly property bool formatButtonsEnabled: editorToolbar.enabled && !editorToolbar.editorReadOnly
    property bool editorReadOnly: false

    signal formatTagRequested(string tagName)
    signal toolbarActionRequested(string actionName)

    objectName: "EditorToolbar"
    Layout.minimumWidth: LV.Theme.gapNone
    spacing: LV.Theme.gapNone
    implicitWidth: editorToolbar.figmaWidth + editorToolbar.toolbarPadding * 2
    implicitHeight: editorToolbar.figmaHeight + editorToolbar.toolbarPadding * 2
    height: editorToolbar.implicitHeight
    clip: true

    function toolbarResponsiveItemWidth(itemIndex) {
        switch (itemIndex) {
        case 0:
        case 1:
            return editorToolbar.figmaComboLargeWidth;
        case 2:
        case 6:
            return editorToolbar.figmaComboSmallWidth;
        case 3:
            return editorToolbar.figmaComboWeightWidth;
        case 4:
            return LV.Theme.scaleMetric(130);
        case 5:
            return LV.Theme.scaleMetric(72);
        default:
            return LV.Theme.gapNone;
        }
    }

    function toolbarResponsiveItemsWidthFromIndex(firstVisibleIndex) {
        const normalizedFirstVisibleIndex = Math.max(0, Math.min(editorToolbar.responsiveToolbarItemCount, Number(firstVisibleIndex) || 0));
        let visibleWidth = LV.Theme.gapNone;

        for (let itemIndex = normalizedFirstVisibleIndex; itemIndex < editorToolbar.responsiveToolbarItemCount; ++itemIndex)
            visibleWidth += editorToolbar.toolbarResponsiveItemWidth(itemIndex);

        const visibleItemCount = editorToolbar.responsiveToolbarItemCount - normalizedFirstVisibleIndex;
        if (visibleItemCount > 1)
            visibleWidth += (visibleItemCount - 1) * LV.Theme.gap12;

        return visibleWidth;
    }

    function toolbarRequiredWidthFromIndex(firstVisibleIndex) {
        return editorToolbar.toolbarResponsiveItemsWidthFromIndex(firstVisibleIndex) + editorToolbar.figmaModeBarWidth;
    }

    function firstVisibleToolbarItemIndexForWidth(availableWidth) {
        const normalizedAvailableWidth = Math.max(LV.Theme.gapNone, Number(availableWidth) || LV.Theme.gapNone);

        for (let firstVisibleIndex = 0; firstVisibleIndex < editorToolbar.responsiveToolbarItemCount; ++firstVisibleIndex) {
            if (editorToolbar.toolbarRequiredWidthFromIndex(firstVisibleIndex) <= normalizedAvailableWidth)
                return firstVisibleIndex;
        }

        return editorToolbar.responsiveToolbarItemCount;
    }

    function toolbarResponsiveItemVisible(itemIndex) {
        return itemIndex >= editorToolbar.firstVisibleToolbarItemIndex;
    }

    component ToolbarComboBox: LV.ComboBox {
        id: toolbarComboBox

        property string actionName: ""
        property string figmaComponentName: "ComboBox/Tone=primary, Arrow=Down"
        property string figmaLabelNodeId: ""
        property string figmaNodeId: ""
        property string figmaStepperNodeId: ""
        property int preferredToolbarWidth: LV.Theme.gapNone

        arrow: LV.Stepper.Down
        Layout.minimumWidth: LV.Theme.gapNone
        Layout.preferredHeight: toolbarComboBox.visible ? editorToolbar.figmaComboHeight : LV.Theme.gapNone
        Layout.preferredWidth: toolbarComboBox.visible ? toolbarComboBox.preferredToolbarWidth : LV.Theme.gapNone
        height: editorToolbar.figmaComboHeight
        tone: LV.ComboBox.Tone.Primary
        width: toolbarComboBox.preferredToolbarWidth

        onClicked: editorToolbar.toolbarActionRequested(toolbarComboBox.actionName)
    }

    component ToolbarGlyphButton: LV.IconButton {
        id: glyphButton

        property string figmaComponentName: "Button/Type=IconButton, Kind=borderless"
        property string figmaIconComponentName: ""
        property string figmaIconNodeId: ""
        property string figmaNodeId: ""
        property string formatTag: ""
        property string glyph: ""
        property bool glyphBold: false
        property bool glyphItalic: false
        property bool glyphStrikeout: false
        property bool glyphUnderline: false

        Layout.preferredHeight: editorToolbar.figmaHeight
        Layout.preferredWidth: editorToolbar.figmaIconButtonWidth
        enabled: editorToolbar.formatButtonsEnabled
        height: editorToolbar.figmaHeight
        iconSize: LV.Theme.iconSm
        tone: LV.AbstractButton.Borderless
        width: editorToolbar.figmaIconButtonWidth

        contentItem: LV.Label {
            color: glyphButton.effectiveEnabled ? editorToolbar.figmaTitleHeader : LV.Theme.disabledColor
            font.italic: glyphButton.glyphItalic
            font.pixelSize: editorToolbar.figmaBodyTextSize
            font.strikeout: glyphButton.glyphStrikeout
            font.underline: glyphButton.glyphUnderline
            font.weight: glyphButton.glyphBold ? Font.Bold : editorToolbar.figmaBodyWeight
            height: LV.Theme.iconSm
            horizontalAlignment: Text.AlignHCenter
            lineHeight: editorToolbar.figmaBodyLineHeight
            lineHeightMode: Text.FixedHeight
            style: body
            text: glyphButton.glyph
            verticalAlignment: Text.AlignVCenter
            width: LV.Theme.iconSm
        }

        onClicked: {
            if (glyphButton.formatTag.length > 0)
                editorToolbar.formatTagRequested(glyphButton.formatTag);
        }
    }

    component ToolbarSwatchMenuButton: LV.IconMenuButton {
        id: swatchButton

        property string actionName: ""
        property string figmaComponentName: "Button/Type=IconMenuButton, Kind=borderless"
        property string figmaIconComponentName: ""
        property string figmaIconNodeId: ""
        property string figmaNodeId: ""
        property string formatTag: ""
        property string swatchGlyph: "\u25CF"
        property color swatchColor: editorToolbar.figmaHighlight

        Layout.preferredHeight: editorToolbar.figmaHeight
        Layout.preferredWidth: editorToolbar.figmaIconMenuButtonWidth
        backgroundColorPressed: swatchButton.formatTag === "highlight"
                                ? editorToolbar.figmaHighlightOverlay
                                : LV.Theme.accentBlueMuted
        enabled: editorToolbar.formatButtonsEnabled
        height: editorToolbar.figmaHeight
        iconGlyph: swatchButton.swatchGlyph
        iconSize: LV.Theme.iconSm
        textColor: swatchButton.swatchColor
        tone: LV.AbstractButton.Borderless
        width: editorToolbar.figmaIconMenuButtonWidth

        onClicked: {
            if (swatchButton.formatTag.length > 0) {
                editorToolbar.formatTagRequested(swatchButton.formatTag);
                return;
            }
            editorToolbar.toolbarActionRequested(swatchButton.actionName);
        }
    }

    component ToolbarModeButton: LV.IconButton {
        id: modeButton

        property string actionName: ""
        property string figmaComponentName: "Button/Type=IconButton, Kind=borderless"
        property string figmaIconComponentName: ""
        property string figmaIconNodeId: ""
        property string figmaNodeId: ""

        Layout.preferredHeight: editorToolbar.figmaHeight
        Layout.preferredWidth: editorToolbar.figmaIconButtonWidth
        height: editorToolbar.figmaHeight
        iconSize: LV.Theme.iconSm
        tone: LV.AbstractButton.Borderless
        width: editorToolbar.figmaIconButtonWidth

        onClicked: editorToolbar.toolbarActionRequested(modeButton.actionName)
    }

    Item {
        id: editorToolbarContentFrame

        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.margins: editorToolbar.toolbarPadding

        LV.HStack {
            id: editorToolbarContent

            anchors.fill: parent
            objectName: "EditorToolbarContent"
            spacing: LV.Theme.gapNone

            LV.HStack {
                id: styleBar

                readonly property string figmaNodeId: "398:8628"

                Layout.preferredHeight: editorToolbar.figmaHeight
                Layout.preferredWidth: visible ? implicitWidth : LV.Theme.gapNone
                Layout.minimumWidth: LV.Theme.gapNone
                objectName: "StyleBar"
                spacing: LV.Theme.gap12
                visible: editorToolbar.firstVisibleToolbarItemIndex < editorToolbar.responsiveToolbarItemCount

                ToolbarComboBox {
                    actionName: "editor.toolbar.style"
                    figmaLabelNodeId: "I397:8570;254:868"
                    figmaNodeId: "397:8570"
                    figmaStepperNodeId: "I397:8570;254:869"
                    objectName: "style"
                    preferredToolbarWidth: editorToolbar.figmaComboLargeWidth
                    text: "Title"
                    visible: editorToolbar.toolbarResponsiveItemVisible(0)
                }

                ToolbarComboBox {
                    actionName: "editor.toolbar.font"
                    figmaLabelNodeId: "I399:8668;254:868"
                    figmaNodeId: "399:8668"
                    figmaStepperNodeId: "I399:8668;254:869"
                    objectName: "font"
                    preferredToolbarWidth: editorToolbar.figmaComboLargeWidth
                    text: "Pretendard"
                    visible: editorToolbar.toolbarResponsiveItemVisible(1)
                }

                ToolbarComboBox {
                    actionName: "editor.toolbar.font-size"
                    figmaLabelNodeId: "I399:8663;254:868"
                    figmaNodeId: "399:8663"
                    figmaStepperNodeId: "I399:8663;254:869"
                    objectName: "fontSize"
                    preferredToolbarWidth: editorToolbar.figmaComboSmallWidth
                    text: "12"
                    visible: editorToolbar.toolbarResponsiveItemVisible(2)
                }

                ToolbarComboBox {
                    actionName: "editor.toolbar.font-weight"
                    figmaLabelNodeId: "I399:8673;254:868"
                    figmaNodeId: "399:8673"
                    figmaStepperNodeId: "I399:8673;254:869"
                    objectName: "fontWeight"
                    preferredToolbarWidth: editorToolbar.figmaComboWeightWidth
                    text: "Regular"
                    visible: editorToolbar.toolbarResponsiveItemVisible(3)
                }

                LV.HStack {
                    id: formatBar

                    readonly property string figmaNodeId: "398:8627"

                    Layout.preferredHeight: editorToolbar.figmaHeight
                    Layout.preferredWidth: visible ? LV.Theme.scaleMetric(130) : LV.Theme.gapNone
                    Layout.minimumWidth: LV.Theme.gapNone
                    objectName: "formatBar"
                    spacing: LV.Theme.gap4
                    visible: editorToolbar.toolbarResponsiveItemVisible(4)

                    ToolbarGlyphButton {
                        figmaIconComponentName: "bold/Theme=Light"
                        figmaIconNodeId: "I397:8586;203:4888"
                        figmaNodeId: "397:8586"
                        formatTag: "bold"
                        glyph: "B"
                        glyphBold: true
                        objectName: "Button"
                    }

                    ToolbarGlyphButton {
                        figmaIconComponentName: "italic"
                        figmaIconNodeId: "I397:8596;203:4888"
                        figmaNodeId: "397:8596"
                        formatTag: "italic"
                        glyph: "I"
                        glyphItalic: true
                        objectName: "Button"
                    }

                    ToolbarGlyphButton {
                        figmaIconComponentName: "underline"
                        figmaIconNodeId: "I398:8603;203:4888"
                        figmaNodeId: "398:8603"
                        formatTag: "underline"
                        glyph: "U"
                        glyphUnderline: true
                        objectName: "Button"
                    }

                    ToolbarGlyphButton {
                        figmaIconComponentName: "strikethrogh"
                        figmaIconNodeId: "I398:8611;203:4888"
                        figmaNodeId: "398:8611"
                        formatTag: "strikethrough"
                        glyph: "S"
                        glyphStrikeout: true
                        objectName: "Button"
                    }

                    ToolbarSwatchMenuButton {
                        actionName: "editor.toolbar.highlight-menu"
                        figmaIconComponentName: "highlight"
                        figmaIconNodeId: "I398:8629;203:4994"
                        figmaNodeId: "398:8629"
                        formatTag: "highlight"
                        objectName: "Button"
                        swatchColor: editorToolbar.figmaHighlight
                        swatchGlyph: "\u25CF"
                    }
                }

                LV.HStack {
                    id: colorBar

                    readonly property string figmaNodeId: "399:9827"

                    Layout.preferredHeight: editorToolbar.figmaHeight
                    Layout.preferredWidth: visible ? LV.Theme.scaleMetric(72) : LV.Theme.gapNone
                    Layout.minimumWidth: LV.Theme.gapNone
                    objectName: "colorBar"
                    spacing: LV.Theme.gap4
                    visible: editorToolbar.toolbarResponsiveItemVisible(5)

                    ToolbarSwatchMenuButton {
                        actionName: "editor.toolbar.color"
                        figmaIconComponentName: "color"
                        figmaIconNodeId: "I399:8683;203:4994"
                        figmaNodeId: "399:8683"
                        objectName: "Button"
                        swatchColor: editorToolbar.figmaHighlight
                        swatchGlyph: "\u25CF"
                    }

                    ToolbarSwatchMenuButton {
                        actionName: "editor.toolbar.background"
                        figmaIconComponentName: "background"
                        figmaIconNodeId: "I399:8690;203:4994"
                        figmaNodeId: "399:8690"
                        objectName: "Button"
                        swatchColor: editorToolbar.figmaTitleHeader
                        swatchGlyph: "\u25A0"
                    }
                }

                ToolbarComboBox {
                    actionName: "editor.toolbar.line-height"
                    figmaLabelNodeId: "I399:8678;254:868"
                    figmaNodeId: "399:8678"
                    figmaStepperNodeId: "I399:8678;254:869"
                    objectName: "lineHeight"
                    preferredToolbarWidth: editorToolbar.figmaComboSmallWidth
                    text: "1.1"
                    visible: editorToolbar.toolbarResponsiveItemVisible(6)
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.minimumWidth: LV.Theme.gapNone
            }

            LV.HStack {
                id: modeBar

                readonly property string figmaNodeId: "400:8662"

                Layout.preferredHeight: editorToolbar.figmaHeight
                Layout.preferredWidth: editorToolbar.figmaModeBarWidth
                objectName: "Frame 1000000891"
                spacing: LV.Theme.gap4

                ToolbarModeButton {
                    actionName: "editor.toolbar.add"
                    figmaIconComponentName: "general / add/Theme = Light"
                    figmaIconNodeId: "I399:9835;203:4888"
                    figmaNodeId: "399:9835"
                    iconName: "generaladd"
                    objectName: "ToggleMode"
                }

                ToolbarModeButton {
                    actionName: "editor.toolbar.renderer-kit"
                    backgroundColor: editorToolbar.figmaPanelBackground12
                    backgroundColorDisabled: editorToolbar.figmaPanelBackground12
                    backgroundColorHover: editorToolbar.figmaPanelBackground12
                    backgroundColorPressed: editorToolbar.figmaPanelBackground12
                    figmaComponentName: "Button/Type=IconButton, Kind=default"
                    figmaIconComponentName: "rendererKit/Theme = Light"
                    figmaIconNodeId: "I400:8656;203:4881"
                    figmaNodeId: "400:8656"
                    iconName: "rendererKit"
                    objectName: "ToggleMode"
                    tone: LV.AbstractButton.Default
                }
            }
        }
    }
}
