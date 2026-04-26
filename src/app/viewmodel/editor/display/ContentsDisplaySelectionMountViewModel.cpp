#include "app/viewmodel/editor/display/ContentsDisplaySelectionMountViewModel.hpp"

bool ContentsDisplaySelectionMountViewModel::shouldFlushBlurredEditorState(const QString& scheduledNoteId)
{
    return invokeControllerBool("shouldFlushBlurredEditorState", {scheduledNoteId});
}

void ContentsDisplaySelectionMountViewModel::flushEditorStateAfterInputSettles(const QString& scheduledNoteId)
{
    invokeControllerVoid("flushEditorStateAfterInputSettles", {scheduledNoteId});
}

void ContentsDisplaySelectionMountViewModel::focusEditorForSelectedNoteId(const QString& noteId)
{
    invokeControllerVoid("focusEditorForSelectedNoteId", {noteId});
}

void ContentsDisplaySelectionMountViewModel::focusEditorForPendingNote()
{
    invokeControllerVoid("focusEditorForPendingNote");
}

void ContentsDisplaySelectionMountViewModel::scheduleEditorEntrySnapshotReconcile()
{
    invokeControllerVoid("scheduleEditorEntrySnapshotReconcile");
}

void ContentsDisplaySelectionMountViewModel::pollSelectedNoteSnapshot()
{
    invokeControllerVoid("pollSelectedNoteSnapshot");
}

bool ContentsDisplaySelectionMountViewModel::reconcileEditorEntrySnapshotOnce()
{
    return invokeControllerBool("reconcileEditorEntrySnapshotOnce");
}

void ContentsDisplaySelectionMountViewModel::scheduleSelectionModelSync(const QVariant& options)
{
    invokeControllerVoid("scheduleSelectionModelSync", {options});
}

bool ContentsDisplaySelectionMountViewModel::executeSelectionDeliveryPlan(
    const QVariant& plan,
    const QString& fallbackKey)
{
    return invokeControllerBool("executeSelectionDeliveryPlan", {plan, fallbackKey});
}

void ContentsDisplaySelectionMountViewModel::scheduleEditorFocusForNote(const QString& noteId)
{
    invokeControllerVoid("scheduleEditorFocusForNote", {noteId});
}

void ContentsDisplaySelectionMountViewModel::resetEditorSelectionCache()
{
    invokeControllerVoid("resetEditorSelectionCache");
}
