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
    readonly property var styleTagStyleValues: [
        "Title",
        "Title2",
        "Subtitle",
        "Header",
        "Header2",
        "Body",
        "Description",
        "Caption",
        "Footnote"
    ]
    property bool editorReadOnly: false
    property string selectedStyleTagStyleValue: "Title"

    signal formatTagRequested(string tagName)
    signal styleTagStyleRequested(string styleValue)
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

    function styleTagStylePreviewDescriptor(styleValue) {
        const normalizedStyleValue = styleValue === undefined || styleValue === null
                ? "Body"
                : String(styleValue).trim();

        if (normalizedStyleValue === "Title") {
            return {
                previewTokenName: "LV.Theme.textTitle",
                previewPixelSize: LV.Theme.textTitle,
                previewWeight: LV.Theme.textTitleWeight,
                previewStyleName: LV.Theme.textTitleStyleName,
                previewLineHeight: LV.Theme.textTitleLineHeight,
                previewLetterSpacing: LV.Theme.textTitleLetterSpacing,
                previewColor: LV.Theme.titleHeaderColor
            };
        }
        if (normalizedStyleValue === "Title2") {
            return {
                previewTokenName: "LV.Theme.textTitle2",
                previewPixelSize: LV.Theme.textTitle2,
                previewWeight: LV.Theme.textTitle2Weight,
                previewStyleName: LV.Theme.textTitle2StyleName,
                previewLineHeight: LV.Theme.textTitle2LineHeight,
                previewLetterSpacing: LV.Theme.textTitle2LetterSpacing,
                previewColor: LV.Theme.titleHeaderColor
            };
        }
        if (normalizedStyleValue === "Subtitle") {
            return {
                previewTokenName: "LV.Theme.textBodyLg",
                previewPixelSize: LV.Theme.textBodyLg,
                previewWeight: LV.Theme.textBodyLgWeight,
                previewStyleName: LV.Theme.textBodyLgStyleName,
                previewLineHeight: LV.Theme.textHeader2LineHeight,
                previewLetterSpacing: LV.Theme.textHeader2LetterSpacing,
                previewColor: LV.Theme.bodyColor
            };
        }
        if (normalizedStyleValue === "Header") {
            return {
                previewTokenName: "LV.Theme.textHeader",
                previewPixelSize: LV.Theme.textHeader,
                previewWeight: LV.Theme.textHeaderWeight,
                previewStyleName: LV.Theme.textHeaderStyleName,
                previewLineHeight: LV.Theme.textHeaderLineHeight,
                previewLetterSpacing: LV.Theme.textHeaderLetterSpacing,
                previewColor: LV.Theme.titleHeaderColor
            };
        }
        if (normalizedStyleValue === "Header2") {
            return {
                previewTokenName: "LV.Theme.textHeader2",
                previewPixelSize: LV.Theme.textHeader2,
                previewWeight: LV.Theme.textHeader2Weight,
                previewStyleName: LV.Theme.textHeader2StyleName,
                previewLineHeight: LV.Theme.textHeader2LineHeight,
                previewLetterSpacing: LV.Theme.textHeader2LetterSpacing,
                previewColor: LV.Theme.titleHeaderColor
            };
        }
        if (normalizedStyleValue === "Description") {
            return {
                previewTokenName: "LV.Theme.textDescription",
                previewPixelSize: LV.Theme.textDescription,
                previewWeight: LV.Theme.textDescriptionWeight,
                previewStyleName: LV.Theme.textDescriptionStyleName,
                previewLineHeight: LV.Theme.textDescriptionLineHeight,
                previewLetterSpacing: LV.Theme.textDescriptionLetterSpacing,
                previewColor: LV.Theme.descriptionColor
            };
        }
        if (normalizedStyleValue === "Caption") {
            return {
                previewTokenName: "LV.Theme.textCaption",
                previewPixelSize: LV.Theme.textCaption,
                previewWeight: LV.Theme.textCaptionWeight,
                previewStyleName: LV.Theme.textCaptionStyleName,
                previewLineHeight: LV.Theme.textCaptionLineHeight,
                previewLetterSpacing: LV.Theme.textCaptionLetterSpacing,
                previewColor: LV.Theme.captionColor
            };
        }
        if (normalizedStyleValue === "Footnote") {
            return {
                previewTokenName: "LV.Theme.textDisabled",
                previewPixelSize: LV.Theme.textDisabled,
                previewWeight: LV.Theme.textDisabledWeight,
                previewStyleName: LV.Theme.textDisabledStyleName,
                previewLineHeight: LV.Theme.textDisabledLineHeight,
                previewLetterSpacing: LV.Theme.textDisabledLetterSpacing,
                previewColor: LV.Theme.disabledColor
            };
        }
        return {
            previewTokenName: "LV.Theme.textBody",
            previewPixelSize: LV.Theme.textBody,
            previewWeight: LV.Theme.textBodyWeight,
            previewStyleName: LV.Theme.textBodyStyleName,
            previewLineHeight: LV.Theme.textBodyLineHeight,
            previewLetterSpacing: LV.Theme.textBodyLetterSpacing,
            previewColor: LV.Theme.bodyColor
        };
    }

    function styleTagStyleMenuPreferredWidth() {
        let maxPreviewWidth = editorToolbar.figmaComboLargeWidth;
        for (let valueIndex = 0; valueIndex < editorToolbar.styleTagStyleValues.length; ++valueIndex) {
            const styleValue = String(editorToolbar.styleTagStyleValues[valueIndex]);
            const descriptor = editorToolbar.styleTagStylePreviewDescriptor(styleValue);
            maxPreviewWidth = Math.max(
                        maxPreviewWidth,
                        Math.ceil(styleValue.length * descriptor.previewPixelSize * 0.62)
                        + LV.Theme.gap16);
        }
        return maxPreviewWidth;
    }

    function styleTagStyleMenuItems() {
        const result = [];
        for (let valueIndex = 0; valueIndex < editorToolbar.styleTagStyleValues.length; ++valueIndex) {
            const styleValue = String(editorToolbar.styleTagStyleValues[valueIndex]);
            const descriptor = editorToolbar.styleTagStylePreviewDescriptor(styleValue);
            result.push({
                id: "style-tag-style-" + styleValue,
                label: styleValue,
                styleValue: styleValue,
                previewTokenName: descriptor.previewTokenName,
                previewPixelSize: descriptor.previewPixelSize,
                previewWeight: descriptor.previewWeight,
                previewStyleName: descriptor.previewStyleName,
                previewLineHeight: descriptor.previewLineHeight,
                previewLetterSpacing: descriptor.previewLetterSpacing,
                previewColor: descriptor.previewColor,
                eventName: "editor.toolbar.style",
                eventPayload: ({
                    style: styleValue
                })
            });
        }
        return result;
    }

    function openStyleTagStyleContextMenu(anchorItem) {
        if (!anchorItem || editorToolbar.editorReadOnly || !anchorItem.visible)
            return false;

        styleTagStyleContextMenu.items = editorToolbar.styleTagStyleMenuItems();
        styleTagStyleContextMenu.itemWidth = editorToolbar.styleTagStyleMenuPreferredWidth();
        styleTagStyleContextMenu.selectedIndex = editorToolbar.styleTagStyleValues.indexOf(editorToolbar.selectedStyleTagStyleValue);
        styleTagStyleContextMenu.openFor(anchorItem, LV.Theme.gapNone, anchorItem.height);
        editorToolbar.toolbarActionRequested("editor.toolbar.style");
        return true;
    }

    component ToolbarComboBox: LV.ComboBox {
        id: toolbarComboBox

        property string actionName: ""
        property string figmaComponentName: "ComboBox/Tone=primary, Arrow=Down"
        property string figmaLabelNodeId: ""
        property string figmaNodeId: ""
        property string figmaStepperNodeId: ""
        property int preferredToolbarWidth: LV.Theme.gapNone
        property bool opensStyleTagStyleMenu: false

        arrow: LV.Stepper.Down
        Layout.minimumWidth: LV.Theme.gapNone
        Layout.preferredHeight: toolbarComboBox.visible ? editorToolbar.figmaComboHeight : LV.Theme.gapNone
        Layout.preferredWidth: toolbarComboBox.visible ? toolbarComboBox.preferredToolbarWidth : LV.Theme.gapNone
        height: editorToolbar.figmaComboHeight
        tone: LV.ComboBox.Tone.Primary
        width: toolbarComboBox.preferredToolbarWidth

        onClicked: {
            if (toolbarComboBox.opensStyleTagStyleMenu) {
                editorToolbar.openStyleTagStyleContextMenu(toolbarComboBox);
                return;
            }
            if (toolbarComboBox.actionName.length > 0)
                editorToolbar.toolbarActionRequested(toolbarComboBox.actionName);
        }
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

    Component {
        id: styleTagStyleMenuItemDelegate

        LV.MenuItem {
            id: styleMenuItem

            property var modelData: ({})
            property int index: modelData.index === undefined ? -1 : modelData.index
            property var entry: modelData.entry
            property color previewColor: styleMenuItem.entry
                    && styleMenuItem.entry.previewColor !== undefined
                    ? styleMenuItem.entry.previewColor
                    : LV.Theme.bodyColor
            property real previewLetterSpacing: styleMenuItem.entry
                    && styleMenuItem.entry.previewLetterSpacing !== undefined
                    ? Number(styleMenuItem.entry.previewLetterSpacing) || 0
                    : LV.Theme.textBodyLetterSpacing
            property int previewLineHeight: styleMenuItem.entry
                    && styleMenuItem.entry.previewLineHeight !== undefined
                    ? Math.max(1, Math.round(Number(styleMenuItem.entry.previewLineHeight) || 1))
                    : LV.Theme.textBodyLineHeight
            property int previewPixelSize: styleMenuItem.entry
                    && styleMenuItem.entry.previewPixelSize !== undefined
                    ? Math.max(1, Math.round(Number(styleMenuItem.entry.previewPixelSize) || 1))
                    : LV.Theme.textBody
            property string previewStyleName: styleMenuItem.entry
                    && styleMenuItem.entry.previewStyleName !== undefined
                    ? String(styleMenuItem.entry.previewStyleName)
                    : LV.Theme.textBodyStyleName
            property int previewWeight: styleMenuItem.entry
                    && styleMenuItem.entry.previewWeight !== undefined
                    ? Math.round(Number(styleMenuItem.entry.previewWeight) || LV.Theme.textBodyWeight)
                    : LV.Theme.textBodyWeight

            enabled: styleMenuItem.modelData.enabled !== false
            itemHeight: Math.max(LV.Theme.gap20, styleMenuItem.previewLineHeight + LV.Theme.gap8)
            itemWidth: styleTagStyleContextMenu.minimumItemWidth
            label: styleMenuItem.entry && styleMenuItem.entry.label !== undefined
                    ? String(styleMenuItem.entry.label)
                    : ""
            state: styleMenuItem.modelData.state === undefined
                    ? styleMenuItem.defaultState
                    : styleMenuItem.modelData.state
            width: parent ? parent.width : styleTagStyleContextMenu.resolvedItemWidth

            contentItem: Item {
                implicitHeight: stylePreviewLabel.implicitHeight
                implicitWidth: stylePreviewLabel.implicitWidth

                LV.Label {
                    id: stylePreviewLabel

                    color: styleMenuItem.effectiveEnabled
                           ? styleMenuItem.previewColor
                           : LV.Theme.disabledColor
                    elide: Text.ElideRight
                    font.letterSpacing: styleMenuItem.previewLetterSpacing
                    font.pixelSize: styleMenuItem.previewPixelSize
                    font.styleName: styleMenuItem.previewStyleName
                    font.weight: styleMenuItem.previewWeight
                    height: implicitHeight
                    lineHeight: styleMenuItem.previewLineHeight
                    lineHeightMode: Text.FixedHeight
                    style: body
                    text: styleMenuItem.label
                    verticalAlignment: Text.AlignVCenter
                    width: parent.width
                    y: Math.round((parent.height - height) * 0.5)
                }
            }

            onClicked: styleTagStyleContextMenu.triggerEntry(styleMenuItem.index)
        }
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
                    enabled: editorToolbar.formatButtonsEnabled
                    figmaLabelNodeId: "I397:8570;254:868"
                    figmaNodeId: "397:8570"
                    figmaStepperNodeId: "I397:8570;254:869"
                    objectName: "style"
                    opensStyleTagStyleMenu: true
                    preferredToolbarWidth: editorToolbar.figmaComboLargeWidth
                    text: editorToolbar.selectedStyleTagStyleValue
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

    LV.ContextMenu {
        id: styleTagStyleContextMenu

        autoCloseOnTrigger: true
        itemDelegate: styleTagStyleMenuItemDelegate
        itemWidth: editorToolbar.styleTagStyleMenuPreferredWidth()
        items: editorToolbar.styleTagStyleMenuItems()
        modal: false
        objectName: "styleTagStyleContextMenu"
        parent: editorToolbar.Window.window ? editorToolbar.Window.window.contentItem : null
        selectedIndex: editorToolbar.styleTagStyleValues.indexOf(editorToolbar.selectedStyleTagStyleValue)

        onItemTriggered: function(index, item) {
            const styleValue = item && item.styleValue !== undefined && item.styleValue !== null
                    ? String(item.styleValue)
                    : "";
            if (styleValue.length === 0)
                return;

            editorToolbar.selectedStyleTagStyleValue = styleValue;
            editorToolbar.styleTagStyleRequested(styleValue);
        }
    }
}
