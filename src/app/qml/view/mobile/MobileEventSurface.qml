pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV

Item {
    id: mobileEventSurface

    property color canvasColor: LV.Theme.panelBackground01
    property bool eventSurfaceEnabled: false
    property bool runtimeHitTransparent: true

    enabled: false
    objectName: "mobileEventSurface"
    visible: false
}
