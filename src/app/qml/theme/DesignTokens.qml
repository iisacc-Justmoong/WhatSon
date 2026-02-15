import QtQuick
import LVRS 1.0 as LV

QtObject {
    readonly property color borderSoft: LV.Theme.strokeSoft
    readonly property color brandPrimary: LV.Theme.primary
    readonly property color brandSecondary: LV.Theme.accentBlue
    readonly property color inkMuted: LV.Theme.textSecondary
    readonly property color inkStrong: LV.Theme.textPrimary
    readonly property color success: LV.Theme.success
    readonly property color surfaceBase: LV.Theme.surfaceAlt
    readonly property color surfacePanel: LV.Theme.surfaceSolid
    readonly property color warning: LV.Theme.warning
}
