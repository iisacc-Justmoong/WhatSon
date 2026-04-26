#pragma once

#include "app/viewmodel/editor/display/ContentsDisplayControllerBridgeViewModel.hpp"

#include <QString>
#include <QVariant>

class ContentsDisplayGeometryViewModel : public ContentsDisplayControllerBridgeViewModel
{
    Q_OBJECT

public:
    using ContentsDisplayControllerBridgeViewModel::ContentsDisplayControllerBridgeViewModel;

public slots:
    void refreshMinimapSnapshot();
    void refreshMinimapCursorTracking(const QVariant& rowsOverride);
    void refreshMinimapViewportTracking(const QVariant& trackHeightOverride);
    void resetNoteEntryLineGeometryState();
    void resetGutterRefreshState();
    bool refreshLiveLogicalLineMetrics();
    int activeLogicalLineCountSnapshot();
    void scheduleGutterRefresh(int passCount, const QString& reason);
    void scheduleNoteEntryGutterRefresh(const QString& noteId);
    void scheduleCursorDrivenUiRefresh();
    void scheduleViewportGutterRefresh();
    void scheduleMinimapSnapshotRefresh(bool forceFull);
    void scrollEditorViewportToMinimapPosition(double localY);
    void correctTypingViewport(bool forceAnchor);
    void scheduleTypingViewportCorrection(bool forceAnchor);
};
