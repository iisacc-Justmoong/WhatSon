pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: overlayHost

    required property var contentsView
    required property var resourceImportController

    ContentsDisplayExceptionOverlay {
        anchors.fill: parent
        contentsView: overlayHost.contentsView
    }

    ContentsDisplayResourceImportConflictAlert {
        contentsView: overlayHost.contentsView
        resourceImportController: overlayHost.resourceImportController
    }
}
