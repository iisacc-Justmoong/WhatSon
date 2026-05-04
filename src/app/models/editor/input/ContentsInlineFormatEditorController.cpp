#include "app/models/editor/input/ContentsInlineFormatEditorController.hpp"

namespace
{
QUrl helperUrl()
{
    return QUrl(QStringLiteral("qrc:/qt/qml/WhatSon/App/view/contents/editor/ContentsInlineFormatEditorController.qml"));
}
}

ContentsInlineFormatEditorController::ContentsInlineFormatEditorController(QObject* parent)
    : ContentsQmlBackedInputControllerBase(parent)
{
}

QObject* ContentsInlineFormatEditorController::control() const noexcept { return m_control; }
void ContentsInlineFormatEditorController::setControl(QObject* value)
{
    if (m_control == value)
        return;
    m_control = value;
    syncHelperProperty("control", QVariant::fromValue(value));
    emit controlChanged();
}

QObject* ContentsInlineFormatEditorController::textInput() const noexcept { return m_textInput; }
void ContentsInlineFormatEditorController::setTextInput(QObject* value)
{
    if (m_textInput == value)
        return;
    m_textInput = value;
    syncHelperProperty("textInput", QVariant::fromValue(value));
    emit textInputChanged();
}

void ContentsInlineFormatEditorController::forceActiveFocus() const { invokeHelperVoid("forceActiveFocus"); }
QString ContentsInlineFormatEditorController::currentPlainText() const { return invokeHelperString("currentPlainText"); }
QVariantMap ContentsInlineFormatEditorController::selectionSnapshot() const { return invokeHelperVariantMap("selectionSnapshot"); }
void ContentsInlineFormatEditorController::clearSelection() const { invokeHelperVoid("clearSelection"); }
void ContentsInlineFormatEditorController::clearCachedSelectionSnapshot() const { invokeHelperVoid("clearCachedSelectionSnapshot"); }
QVariantMap ContentsInlineFormatEditorController::cacheCurrentSelectionSnapshot() const { return invokeHelperVariantMap("cacheCurrentSelectionSnapshot"); }
void ContentsInlineFormatEditorController::maybeDiscardCachedSelectionSnapshot() const { invokeHelperVoid("maybeDiscardCachedSelectionSnapshot"); }
QVariantMap ContentsInlineFormatEditorController::inlineFormatSelectionSnapshot() const { return invokeHelperVariantMap("inlineFormatSelectionSnapshot"); }
bool ContentsInlineFormatEditorController::nativeCompositionActive() const { return invokeHelperBool("nativeCompositionActive"); }
int ContentsInlineFormatEditorController::clampLogicalPosition(const int position, const int maximumLength) const
{
    return invokeHelperInt("clampLogicalPosition", {position, maximumLength});
}
int ContentsInlineFormatEditorController::setCursorPositionPreservingNativeInput(const int position) const
{
    return invokeHelperInt("setCursorPositionPreservingNativeInput", {position}, position);
}
bool ContentsInlineFormatEditorController::selectionCursorUsesStartEdge(
    const int cursorPosition,
    const int selectionStart,
    const int selectionEnd) const
{
    return invokeHelperBool("selectionCursorUsesStartEdge", {cursorPosition, selectionStart, selectionEnd});
}
bool ContentsInlineFormatEditorController::restoreSelectionRange(
    const int selectionStart,
    const int selectionEnd,
    const int cursorPosition) const
{
    return invokeHelperBool("restoreSelectionRange", {selectionStart, selectionEnd, cursorPosition});
}
QVariantMap ContentsInlineFormatEditorController::programmaticTextSyncPolicy(const QString& nextText) const
{
    return invokeHelperVariantMap("programmaticTextSyncPolicy", {nextText});
}
bool ContentsInlineFormatEditorController::canDeferProgrammaticTextSync(const QString& nextText) const
{
    return invokeHelperBool("canDeferProgrammaticTextSync", {nextText});
}
bool ContentsInlineFormatEditorController::shouldRejectFocusedProgrammaticTextSync(const QString& nextText) const
{
    return invokeHelperBool("shouldRejectFocusedProgrammaticTextSync", {nextText});
}
void ContentsInlineFormatEditorController::flushDeferredProgrammaticText(const bool force) const
{
    invokeHelperVoid("flushDeferredProgrammaticText", {force});
}
void ContentsInlineFormatEditorController::clearDeferredProgrammaticText() const
{
    invokeHelperVoid("clearDeferredProgrammaticText");
}
bool ContentsInlineFormatEditorController::dispatchCommittedTextEditedIfReady() const
{
    return invokeHelperBool("dispatchCommittedTextEditedIfReady");
}
void ContentsInlineFormatEditorController::applyImmediateProgrammaticText(const QString& nextText) const
{
    invokeHelperVoid("applyImmediateProgrammaticText", {nextText});
}
void ContentsInlineFormatEditorController::setProgrammaticText(const QString& nextText) const
{
    invokeHelperVoid("setProgrammaticText", {nextText});
}
void ContentsInlineFormatEditorController::scheduleCommittedTextEditedDispatch() const
{
    invokeHelperVoid("scheduleCommittedTextEditedDispatch");
}
QUrl ContentsInlineFormatEditorController::helperSourceUrl() const { return helperUrl(); }
