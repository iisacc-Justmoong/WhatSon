pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: state

    property int documentPresentationRefreshIntervalMs: 120
    property string renderedEditorHtml: ""
}
