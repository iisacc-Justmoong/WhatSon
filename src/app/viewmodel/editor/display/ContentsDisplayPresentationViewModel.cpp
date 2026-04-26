#include "app/viewmodel/editor/display/ContentsDisplayPresentationViewModel.hpp"

void ContentsDisplayPresentationViewModel::logEditorCreationState(const QString& reason)
{
    invokeControllerVoid("logEditorCreationState", {reason});
}

void ContentsDisplayPresentationViewModel::commitDocumentPresentationRefresh()
{
    invokeControllerVoid("commitDocumentPresentationRefresh");
}

bool ContentsDisplayPresentationViewModel::documentPresentationRenderDirty()
{
    return invokeControllerBool("documentPresentationRenderDirty");
}

void ContentsDisplayPresentationViewModel::refreshInlineResourcePresentation()
{
    invokeControllerVoid("refreshInlineResourcePresentation");
}

void ContentsDisplayPresentationViewModel::requestViewHook(const QString& reason)
{
    invokeControllerVoid("requestViewHook", {reason});
}

void ContentsDisplayPresentationViewModel::applyPresentationRefreshPlan(const QVariant& plan)
{
    invokeControllerVoid("applyPresentationRefreshPlan", {plan});
}

void ContentsDisplayPresentationViewModel::scheduleDeferredDocumentPresentationRefresh()
{
    invokeControllerVoid("scheduleDeferredDocumentPresentationRefresh");
}

void ContentsDisplayPresentationViewModel::scheduleDocumentPresentationRefresh(const bool forceImmediate)
{
    invokeControllerVoid("scheduleDocumentPresentationRefresh", {forceImmediate});
}
