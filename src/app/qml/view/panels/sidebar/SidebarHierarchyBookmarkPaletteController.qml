import QtQuick
import LVRS 1.0 as LV

QtObject {
    id: bookmarkPaletteController

    required property var bookmarkCanvas
    required property var hostView
    required property var itemLocator

    function resolveThemeColorToken(tokenName) {
        const normalizedToken = tokenName === undefined || tokenName === null ? "" : String(tokenName).trim();
        if (!normalizedToken.length)
            return LV.Theme.bodyColor;
        if (LV.Theme[normalizedToken] !== undefined)
            return LV.Theme[normalizedToken];
        const accentTokens = LV.Theme.accentPaletteTokens !== undefined && LV.Theme.accentPaletteTokens !== null
            ? LV.Theme.accentPaletteTokens
            : [];
        for (let index = 0; index < accentTokens.length; ++index) {
            const token = accentTokens[index];
            if (!token || token.name === undefined || token.color === undefined)
                continue;
            if (String(token.name) === normalizedToken)
                return token.color;
        }
        return LV.Theme.bodyColor;
    }

    function bookmarkPaletteColorTokenForLabel(label) {
        const normalizedLabel = label === undefined || label === null ? "" : String(label).trim().toLowerCase();
        switch (normalizedLabel) {
        case "red":
            return "accentRed";
        case "orange":
            return "accentLightOrangeVivid";
        case "amber":
            return "accentLightAmberVivid";
        case "yellow":
            return "accentYellow";
        case "green":
            return "accentGreen";
        case "teal":
            return "accentDimTeal";
        case "blue":
            return "accentBlue";
        case "indigo":
            return "accentLighterIndigo";
        case "purple":
            return "accentPurple";
        case "pink":
            return "accentLightRose";
        default:
            return "";
        }
    }

    function bookmarkPaletteColorForLabel(label) {
        const colorToken = bookmarkPaletteController.bookmarkPaletteColorTokenForLabel(label);
        return colorToken.length
            ? bookmarkPaletteController.resolveThemeColorToken(colorToken)
            : LV.Theme.bodyColor;
    }

    function applyBookmarkPaletteVisuals() {
        if (!hostView.bookmarkPaletteVisualsEnabled)
            return;
        const hierarchyItems = itemLocator.collectHierarchyItems();
        for (let index = 0; index < hierarchyItems.length; ++index) {
            const item = hierarchyItems[index];
            if (!item)
                continue;
            const bookmarkColor = bookmarkPaletteController.bookmarkPaletteColorForLabel(item.text);
            item.textColorNormal = bookmarkColor;
            item.textColorDisabled = bookmarkColor;
        }
    }

    function drawBookmarkGlyph(context, x, y, size, color) {
        const left = x + size * 0.25;
        const right = x + size * 0.75;
        const top = y + size * 0.09375;
        const bottom = y + size * 0.875;
        const notchY = y + size * 0.65625;
        const centerX = x + size * 0.5;
        context.beginPath();
        context.moveTo(left, top);
        context.lineTo(right, top);
        context.lineTo(right, bottom);
        context.lineTo(centerX, notchY);
        context.lineTo(left, bottom);
        context.closePath();
        context.fillStyle = color;
        context.fill();
    }

    function scheduleBookmarkPaletteVisualRefresh() {
        Qt.callLater(function () {
            Qt.callLater(function () {
                bookmarkPaletteController.applyBookmarkPaletteVisuals();
                bookmarkCanvas.requestPaint();
            });
        });
    }
}
