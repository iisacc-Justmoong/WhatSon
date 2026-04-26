#include "app/viewmodel/editor/display/ContentsDisplayGeometryViewModel.hpp"

#include <algorithm>

void ContentsDisplayGeometryViewModel::refreshMinimapSnapshot()
{
    invokeControllerVoid("refreshMinimapSnapshot");
}

void ContentsDisplayGeometryViewModel::refreshMinimapCursorTracking(const QVariant& rowsOverride)
{
    invokeControllerVoid("refreshMinimapCursorTracking", {rowsOverride});
}

void ContentsDisplayGeometryViewModel::refreshMinimapViewportTracking(const QVariant& trackHeightOverride)
{
    invokeControllerVoid("refreshMinimapViewportTracking", {trackHeightOverride});
}

void ContentsDisplayGeometryViewModel::resetNoteEntryLineGeometryState()
{
    invokeControllerVoid("resetNoteEntryLineGeometryState");
}

void ContentsDisplayGeometryViewModel::resetGutterRefreshState()
{
    invokeControllerVoid("resetGutterRefreshState");
}

bool ContentsDisplayGeometryViewModel::refreshLiveLogicalLineMetrics()
{
    return invokeControllerBool("refreshLiveLogicalLineMetrics");
}

int ContentsDisplayGeometryViewModel::activeLogicalLineCountSnapshot()
{
    return std::max(1, invokeController("activeLogicalLineCountSnapshot").toInt());
}

void ContentsDisplayGeometryViewModel::scheduleGutterRefresh(const int passCount, const QString& reason)
{
    invokeControllerVoid("scheduleGutterRefresh", {passCount, reason});
}

void ContentsDisplayGeometryViewModel::scheduleNoteEntryGutterRefresh(const QString& noteId)
{
    invokeControllerVoid("scheduleNoteEntryGutterRefresh", {noteId});
}

void ContentsDisplayGeometryViewModel::scheduleCursorDrivenUiRefresh()
{
    invokeControllerVoid("scheduleCursorDrivenUiRefresh");
}

void ContentsDisplayGeometryViewModel::scheduleViewportGutterRefresh()
{
    invokeControllerVoid("scheduleViewportGutterRefresh");
}

void ContentsDisplayGeometryViewModel::scheduleMinimapSnapshotRefresh(const bool forceFull)
{
    invokeControllerVoid("scheduleMinimapSnapshotRefresh", {forceFull});
}

void ContentsDisplayGeometryViewModel::scrollEditorViewportToMinimapPosition(const double localY)
{
    invokeControllerVoid("scrollEditorViewportToMinimapPosition", {localY});
}

void ContentsDisplayGeometryViewModel::correctTypingViewport(const bool forceAnchor)
{
    invokeControllerVoid("correctTypingViewport", {forceAnchor});
}

void ContentsDisplayGeometryViewModel::scheduleTypingViewportCorrection(const bool forceAnchor)
{
    invokeControllerVoid("scheduleTypingViewportCorrection", {forceAnchor});
}
