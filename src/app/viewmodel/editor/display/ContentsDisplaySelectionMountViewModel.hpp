#pragma once

#include "app/viewmodel/editor/display/ContentsDisplayControllerBridgeViewModel.hpp"

#include <QString>
#include <QVariant>

class ContentsDisplaySelectionMountViewModel : public ContentsDisplayControllerBridgeViewModel
{
    Q_OBJECT

public:
    using ContentsDisplayControllerBridgeViewModel::ContentsDisplayControllerBridgeViewModel;

public slots:
    bool shouldFlushBlurredEditorState(const QString& scheduledNoteId);
    void flushEditorStateAfterInputSettles(const QString& scheduledNoteId);
    void focusEditorForSelectedNoteId(const QString& noteId);
    void focusEditorForPendingNote();
    void scheduleEditorEntrySnapshotReconcile();
    void pollSelectedNoteSnapshot();
    bool reconcileEditorEntrySnapshotOnce();
    void scheduleSelectionModelSync(const QVariant& options);
    bool executeSelectionDeliveryPlan(const QVariant& plan, const QString& fallbackKey);
    void scheduleEditorFocusForNote(const QString& noteId);
    void resetEditorSelectionCache();
};
