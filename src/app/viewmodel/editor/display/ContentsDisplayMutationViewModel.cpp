#include "app/viewmodel/editor/display/ContentsDisplayMutationViewModel.hpp"

bool ContentsDisplayMutationViewModel::queueStructuredInlineFormatWrap(const QString& tagName)
{
    return invokeControllerBool("queueStructuredInlineFormatWrap", {tagName});
}

bool ContentsDisplayMutationViewModel::queueInlineFormatWrap(const QString& tagName)
{
    return invokeControllerBool("queueInlineFormatWrap", {tagName});
}

bool ContentsDisplayMutationViewModel::queueAgendaShortcutInsertion()
{
    return invokeControllerBool("queueAgendaShortcutInsertion");
}

bool ContentsDisplayMutationViewModel::queueCalloutShortcutInsertion()
{
    return invokeControllerBool("queueCalloutShortcutInsertion");
}

bool ContentsDisplayMutationViewModel::queueBreakShortcutInsertion()
{
    return invokeControllerBool("queueBreakShortcutInsertion");
}

void ContentsDisplayMutationViewModel::focusStructuredBlockSourceOffset(const int sourceOffset)
{
    invokeControllerVoid("focusStructuredBlockSourceOffset", {sourceOffset});
}

bool ContentsDisplayMutationViewModel::applyDocumentSourceMutation(
    const QString& nextSourceText,
    const QVariant& focusRequest)
{
    return invokeControllerBool("applyDocumentSourceMutation", {nextSourceText, focusRequest});
}

bool ContentsDisplayMutationViewModel::setAgendaTaskDone(
    const int taskOpenTagStart,
    const int taskOpenTagEnd,
    const bool checked)
{
    return invokeControllerBool("setAgendaTaskDone", {taskOpenTagStart, taskOpenTagEnd, checked});
}

bool ContentsDisplayMutationViewModel::persistEditorTextImmediately(const QString& nextText)
{
    return invokeControllerBool("persistEditorTextImmediately", {nextText});
}

QVariant ContentsDisplayMutationViewModel::selectedEditorRange()
{
    return invokeController("selectedEditorRange");
}

bool ContentsDisplayMutationViewModel::wrapSelectedEditorTextWithTag(
    const QString& tagName,
    const QVariant& explicitSelectionRange)
{
    return invokeControllerBool("wrapSelectedEditorTextWithTag", {tagName, explicitSelectionRange});
}
