pragma ComponentBehavior: Bound

import QtQuick
import WhatSon.App.Internal 1.0
import LVRS 1.0 as LV

Item {
    id: mobileEventSurface

    property color canvasColor: LV.Theme.panelBackground01
    property alias controller: mobileEventController
    property bool eventSurfaceEnabled: true

    signal touchClassified(var eventData)
    signal scrollClassified(var eventData)
    signal gestureClassified(var eventData)
    signal classificationChanged(var eventData)

    clip: true
    objectName: "mobileEventSurface"

    function pointSnapshot(touchPoints) {
        const points = [];
        for (let pointIndex = 0; pointIndex < touchPoints.length; ++pointIndex) {
            const point = touchPoints[pointIndex];
            points.push({
                "id": point.pointId !== undefined ? Number(point.pointId) : pointIndex,
                "x": Number(point.x) || 0,
                "y": Number(point.y) || 0
            });
        }
        return points;
    }

    MobileEventSurfaceController {
        id: mobileEventController

        onClassificationChanged: function (eventData) {
            mobileEventSurface.classificationChanged(eventData);
        }
        onGestureRecognized: function (eventData) {
            mobileEventSurface.gestureClassified(eventData);
        }
        onScrollRecognized: function (eventData) {
            mobileEventSurface.scrollClassified(eventData);
        }
        onTouchRecognized: function (eventData) {
            mobileEventSurface.touchClassified(eventData);
        }
    }

    MultiPointTouchArea {
        id: mobileTouchSurface

        anchors.fill: parent
        enabled: mobileEventSurface.eventSurfaceEnabled
        maximumTouchPoints: 5
        minimumTouchPoints: 1
        mouseEnabled: false

        property int activeSessionId: -1

        onCanceled: {
            if (activeSessionId >= 0)
                mobileEventController.cancelTouchSequence(activeSessionId);
            activeSessionId = -1;
        }
        onPressed: function (touchPoints) {
            activeSessionId = mobileEventController.nextSessionId();
            mobileEventController.beginTouchSequence(
                        activeSessionId,
                        mobileEventSurface.pointSnapshot(touchPoints));
        }
        onReleased: function (touchPoints) {
            if (activeSessionId >= 0)
                mobileEventController.endTouchSequence(
                            activeSessionId,
                            mobileEventSurface.pointSnapshot(touchPoints),
                            false);
            activeSessionId = -1;
        }
        onUpdated: function (touchPoints) {
            if (activeSessionId < 0)
                return;
            mobileEventController.updateTouchSequence(
                        activeSessionId,
                        mobileEventSurface.pointSnapshot(touchPoints));
        }
    }
}
