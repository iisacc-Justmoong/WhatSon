#include "app/models/editor/input/ContentsRemainingInputControllers.hpp"

#include "app/models/editor/resource/ContentsEditorDynamicObjectSupport.hpp"

#include <QtMath>

namespace
{
QUrl helperUrl(const char* fileName)
{
    return QUrl(QStringLiteral("qrc:/qt/qml/WhatSon/App/view/content/editor/") + QString::fromLatin1(fileName));
}
}

ContentsAgendaBlockController::ContentsAgendaBlockController(QObject* parent)
    : ContentsQmlBackedInputControllerBase(parent)
{
}
QObject* ContentsAgendaBlockController::agendaBlock() const noexcept { return m_agendaBlock; }
void ContentsAgendaBlockController::setAgendaBlock(QObject* value) { if (m_agendaBlock == value) return; m_agendaBlock = value; syncHelperProperty("agendaBlock", QVariant::fromValue(value)); emit agendaBlockChanged(); }
QObject* ContentsAgendaBlockController::taskRepeater() const noexcept { return m_taskRepeater; }
void ContentsAgendaBlockController::setTaskRepeater(QObject* value) { if (m_taskRepeater == value) return; m_taskRepeater = value; syncHelperProperty("taskRepeater", QVariant::fromValue(value)); emit taskRepeaterChanged(); }
bool ContentsAgendaBlockController::hasFocusedTaskRow() const { return invokeHelperBool("hasFocusedTaskRow"); }
bool ContentsAgendaBlockController::focusedTaskInputMethodComposing() const { return invokeHelperBool("focusedTaskInputMethodComposing"); }
QString ContentsAgendaBlockController::focusedTaskPreeditText() const { return invokeHelperString("focusedTaskPreeditText"); }
bool ContentsAgendaBlockController::nativeCompositionActive() const { return invokeHelperBool("nativeCompositionActive"); }
int ContentsAgendaBlockController::currentFocusedTaskLineNumber() const { return invokeHelperInt("currentFocusedTaskLineNumber", {}, 1); }
QVariantMap ContentsAgendaBlockController::currentCursorRowRect() const { return invokeHelperVariantMap("currentCursorRowRect"); }
QString ContentsAgendaBlockController::currentVisiblePlainText() const { return invokeHelperString("currentVisiblePlainText"); }
QString ContentsAgendaBlockController::visiblePlainText() const { return invokeHelperString("visiblePlainText"); }
int ContentsAgendaBlockController::representativeCharCount(const QVariant& lineText) const { return invokeHelperInt("representativeCharCount", {lineText}); }
bool ContentsAgendaBlockController::focusLastTask() const { return invokeHelperBool("focusLastTask"); }
bool ContentsAgendaBlockController::focusTaskBoundary(const int taskIndex, const QString& side) const { return invokeHelperBool("focusTaskBoundary", {taskIndex, side}); }
bool ContentsAgendaBlockController::focusBoundary(const QString& side) const { return invokeHelperBool("focusBoundary", {side}); }
bool ContentsAgendaBlockController::focusFirstTask() const { return invokeHelperBool("focusFirstTask"); }
bool ContentsAgendaBlockController::focusTaskAtSourceOffset(const int sourceOffset) const { return invokeHelperBool("focusTaskAtSourceOffset", {sourceOffset}); }
bool ContentsAgendaBlockController::applyFocusRequest(const QVariantMap& request) const { return invokeHelperBool("applyFocusRequest", {request}); }
bool ContentsAgendaBlockController::clearSelection(const bool preserveFocusedEditor) const { return invokeHelperBool("clearSelection", {preserveFocusedEditor}); }
int ContentsAgendaBlockController::shortcutInsertionSourceOffset() const { return invokeHelperInt("shortcutInsertionSourceOffset"); }
QUrl ContentsAgendaBlockController::helperSourceUrl() const { return helperUrl("ContentsAgendaBlockController.qml"); }

ContentsAgendaTaskRowController::ContentsAgendaTaskRowController(QObject* parent)
    : ContentsQmlBackedInputControllerBase(parent)
{
}
QObject* ContentsAgendaTaskRowController::agendaBlock() const noexcept { return m_agendaBlock; }
void ContentsAgendaTaskRowController::setAgendaBlock(QObject* value) { if (m_agendaBlock == value) return; m_agendaBlock = value; syncHelperProperty("agendaBlock", QVariant::fromValue(value)); emit agendaBlockChanged(); }
QObject* ContentsAgendaTaskRowController::taskRow() const noexcept { return m_taskRow; }
void ContentsAgendaTaskRowController::setTaskRow(QObject* value) { if (m_taskRow == value) return; m_taskRow = value; syncHelperProperty("taskRow", QVariant::fromValue(value)); emit taskRowChanged(); }
QObject* ContentsAgendaTaskRowController::taskEditor() const noexcept { return m_taskEditor; }
void ContentsAgendaTaskRowController::setTaskEditor(QObject* value) { if (m_taskEditor == value) return; m_taskEditor = value; syncHelperProperty("taskEditor", QVariant::fromValue(value)); emit taskEditorChanged(); }
void ContentsAgendaTaskRowController::focusEditor(const QVariant& cursorPosition) const { invokeHelperVoid("focusEditor", {cursorPosition}); }
bool ContentsAgendaTaskRowController::clearSelection(const bool preserveFocusedEditor) const { return invokeHelperBool("clearSelection", {preserveFocusedEditor}); }
QString ContentsAgendaTaskRowController::currentEditorPlainText() const { return invokeHelperString("currentEditorPlainText"); }
void ContentsAgendaTaskRowController::syncLiveTaskTextFromHost() const { invokeHelperVoid("syncLiveTaskTextFromHost"); }
QVariantMap ContentsAgendaTaskRowController::currentCursorRowRect() const { return invokeHelperVariantMap("currentCursorRowRect"); }
bool ContentsAgendaTaskRowController::nativeCompositionActive() const { return invokeHelperBool("nativeCompositionActive"); }
QVariantList ContentsAgendaTaskRowController::logicalLineLayoutEntries() const { return invokeHelperVariantList("logicalLineLayoutEntries"); }
QString ContentsAgendaTaskRowController::visiblePlainText() const { return invokeHelperString("visiblePlainText"); }
int ContentsAgendaTaskRowController::representativeCharCount(const QVariant& lineText) const { return invokeHelperInt("representativeCharCount", {lineText}); }
bool ContentsAgendaTaskRowController::cursorOnFirstVisualRow() const { return invokeHelperBool("cursorOnFirstVisualRow"); }
bool ContentsAgendaTaskRowController::cursorOnLastVisualRow() const { return invokeHelperBool("cursorOnLastVisualRow"); }
bool ContentsAgendaTaskRowController::applyFocusRequest(const QVariantMap& request) const { return invokeHelperBool("applyFocusRequest", {request}); }
int ContentsAgendaTaskRowController::shortcutInsertionSourceOffset() const { return invokeHelperInt("shortcutInsertionSourceOffset"); }
bool ContentsAgendaTaskRowController::handleToggleChanged(const bool checked) const { return invokeHelperBool("handleToggleChanged", {checked}); }
bool ContentsAgendaTaskRowController::handleKeyPress(QObject* event) const { return invokeHelperBool("handleKeyPress", {QVariant::fromValue(event)}); }
QUrl ContentsAgendaTaskRowController::helperSourceUrl() const { return helperUrl("ContentsAgendaTaskRowController.qml"); }

ContentsCalloutBlockController::ContentsCalloutBlockController(QObject* parent)
    : ContentsQmlBackedInputControllerBase(parent)
{
}
QObject* ContentsCalloutBlockController::calloutBlock() const noexcept { return m_calloutBlock; }
void ContentsCalloutBlockController::setCalloutBlock(QObject* value) { if (m_calloutBlock == value) return; m_calloutBlock = value; syncHelperProperty("calloutBlock", QVariant::fromValue(value)); emit calloutBlockChanged(); }
QObject* ContentsCalloutBlockController::calloutEditor() const noexcept { return m_calloutEditor; }
void ContentsCalloutBlockController::setCalloutEditor(QObject* value) { if (m_calloutEditor == value) return; m_calloutEditor = value; syncHelperProperty("calloutEditor", QVariant::fromValue(value)); emit calloutEditorChanged(); }
QString ContentsCalloutBlockController::currentEditorPlainText() const { return invokeHelperString("currentEditorPlainText"); }
int ContentsCalloutBlockController::currentEditorLogicalLineNumber() const { return invokeHelperInt("currentEditorLogicalLineNumber", {}, 1); }
QVariantMap ContentsCalloutBlockController::currentCursorRowRect() const { return invokeHelperVariantMap("currentCursorRowRect"); }
bool ContentsCalloutBlockController::nativeCompositionActive() const { return invokeHelperBool("nativeCompositionActive"); }
void ContentsCalloutBlockController::syncLiveTextFromHost() const { invokeHelperVoid("syncLiveTextFromHost"); }
QVariantList ContentsCalloutBlockController::logicalLineLayoutEntries() const { return invokeHelperVariantList("logicalLineLayoutEntries"); }
QString ContentsCalloutBlockController::visiblePlainText() const { return invokeHelperString("visiblePlainText"); }
int ContentsCalloutBlockController::representativeCharCount(const QVariant& lineText) const { return invokeHelperInt("representativeCharCount", {lineText}); }
bool ContentsCalloutBlockController::cursorOnFirstVisualRow() const { return invokeHelperBool("cursorOnFirstVisualRow"); }
bool ContentsCalloutBlockController::cursorOnLastVisualRow() const { return invokeHelperBool("cursorOnLastVisualRow"); }
void ContentsCalloutBlockController::focusEditor(const QVariant& cursorPosition) const { invokeHelperVoid("focusEditor", {cursorPosition}); }
bool ContentsCalloutBlockController::clearSelection(const bool preserveFocusedEditor) const { return invokeHelperBool("clearSelection", {preserveFocusedEditor}); }
bool ContentsCalloutBlockController::applyFocusRequest(const QVariantMap& request) const { return invokeHelperBool("applyFocusRequest", {request}); }
int ContentsCalloutBlockController::shortcutInsertionSourceOffset() const { return invokeHelperInt("shortcutInsertionSourceOffset"); }
bool ContentsCalloutBlockController::handleTagManagementKeyPress(QObject* event) const { return invokeHelperBool("handleTagManagementKeyPress", {QVariant::fromValue(event)}); }
QUrl ContentsCalloutBlockController::helperSourceUrl() const { return helperUrl("ContentsCalloutBlockController.qml"); }

ContentsDocumentBlockController::ContentsDocumentBlockController(QObject* parent)
    : ContentsQmlBackedInputControllerBase(parent)
{
}
QObject* ContentsDocumentBlockController::documentBlock() const noexcept { return m_documentBlock; }
void ContentsDocumentBlockController::setDocumentBlock(QObject* value) { if (m_documentBlock == value) return; m_documentBlock = value; syncHelperProperty("documentBlock", QVariant::fromValue(value)); emit documentBlockChanged(); }
QObject* ContentsDocumentBlockController::blockLoader() const noexcept { return m_blockLoader; }
void ContentsDocumentBlockController::setBlockLoader(QObject* value) { if (m_blockLoader == value) return; m_blockLoader = value; syncHelperProperty("blockLoader", QVariant::fromValue(value)); emit blockLoaderChanged(); }
QVariantMap ContentsDocumentBlockController::currentCursorRowRect() const { return invokeHelperVariantMap("currentCursorRowRect"); }
QVariantList ContentsDocumentBlockController::logicalLineLayoutEntries() const { return invokeHelperVariantList("logicalLineLayoutEntries"); }
bool ContentsDocumentBlockController::applyFocusRequest(const QVariantMap& request) const { return invokeHelperBool("applyFocusRequest", {request}); }
bool ContentsDocumentBlockController::handleAtomicTagDeleteKeyPress(QObject* event) const { return invokeHelperBool("handleAtomicTagDeleteKeyPress", {QVariant::fromValue(event)}); }
QVariantMap ContentsDocumentBlockController::inlineFormatSelectionSnapshot() const { return invokeHelperVariantMap("inlineFormatSelectionSnapshot"); }
bool ContentsDocumentBlockController::clearSelection(const bool preserveFocusedEditor) const { return invokeHelperBool("clearSelection", {preserveFocusedEditor}); }
int ContentsDocumentBlockController::shortcutInsertionSourceOffset() const { return invokeHelperInt("shortcutInsertionSourceOffset"); }
QString ContentsDocumentBlockController::visiblePlainText() const { return invokeHelperString("visiblePlainText"); }
int ContentsDocumentBlockController::representativeCharCount(const QVariant& lineText) const { return invokeHelperInt("representativeCharCount", {lineText}); }
bool ContentsDocumentBlockController::handleAtomicTagManagementKeyPress(QObject* event) const { return invokeHelperBool("handleAtomicTagManagementKeyPress", {QVariant::fromValue(event)}); }
void ContentsDocumentBlockController::handleAtomicTap(const int tapCount) const { invokeHelperVoid("handleAtomicTap", {tapCount}); }
QUrl ContentsDocumentBlockController::helperSourceUrl() const { return helperUrl("ContentsDocumentBlockController.qml"); }

ContentsDocumentTextBlockController::ContentsDocumentTextBlockController(QObject* parent)
    : ContentsQmlBackedInputControllerBase(parent)
{
    QObject::connect(this, &ContentsQmlBackedInputControllerBase::helperReady, this, [this]() {
        connectHelperPropertyNotify("sourceContainsInlineStyleTags", "sourceContainsInlineStyleTagsChanged()");
        connectHelperPropertyNotify("authoritativePlainText", "authoritativePlainTextChanged()");
        emit sourceContainsInlineStyleTagsChanged();
        emit authoritativePlainTextChanged();
    });
}
QObject* ContentsDocumentTextBlockController::textBlock() const noexcept { return m_textBlock; }
void ContentsDocumentTextBlockController::setTextBlock(QObject* value) { if (m_textBlock == value) return; m_textBlock = value; syncHelperProperty("textBlock", QVariant::fromValue(value)); emit textBlockChanged(); }
QObject* ContentsDocumentTextBlockController::blockEditor() const noexcept { return m_blockEditor; }
void ContentsDocumentTextBlockController::setBlockEditor(QObject* value) { if (m_blockEditor == value) return; m_blockEditor = value; syncHelperProperty("blockEditor", QVariant::fromValue(value)); emit blockEditorChanged(); }
QObject* ContentsDocumentTextBlockController::inlineStyleRenderer() const noexcept { return m_inlineStyleRenderer; }
void ContentsDocumentTextBlockController::setInlineStyleRenderer(QObject* value) { if (m_inlineStyleRenderer == value) return; m_inlineStyleRenderer = value; syncHelperProperty("inlineStyleRenderer", QVariant::fromValue(value)); emit inlineStyleRendererChanged(); }
QObject* ContentsDocumentTextBlockController::plainTextSourceMutator() const noexcept { return m_plainTextSourceMutator; }
void ContentsDocumentTextBlockController::setPlainTextSourceMutator(QObject* value) { if (m_plainTextSourceMutator == value) return; m_plainTextSourceMutator = value; syncHelperProperty("plainTextSourceMutator", QVariant::fromValue(value)); emit plainTextSourceMutatorChanged(); }
bool ContentsDocumentTextBlockController::sourceContainsInlineStyleTags() const { return helperProperty("sourceContainsInlineStyleTags").toBool(); }
QString ContentsDocumentTextBlockController::authoritativePlainText() const { return helperProperty("authoritativePlainText").toString(); }
QString ContentsDocumentTextBlockController::authoritativeSourceText() const { return invokeHelperString("authoritativeSourceText"); }
QString ContentsDocumentTextBlockController::visiblePlainText() const { return invokeHelperString("visiblePlainText"); }
int ContentsDocumentTextBlockController::representativeCharCount(const QVariant& lineText) const { return invokeHelperInt("representativeCharCount", {lineText}); }
QString ContentsDocumentTextBlockController::currentEditorPlainText() const { return invokeHelperString("currentEditorPlainText"); }
bool ContentsDocumentTextBlockController::nativeCompositionActive() const { return invokeHelperBool("nativeCompositionActive"); }
void ContentsDocumentTextBlockController::syncLiveEditSnapshotFromHost() const { invokeHelperVoid("syncLiveEditSnapshotFromHost"); }
QVariantList ContentsDocumentTextBlockController::logicalLineLayoutEntries() const { return invokeHelperVariantList("logicalLineLayoutEntries"); }
int ContentsDocumentTextBlockController::currentEditorLogicalLineNumber() const { return invokeHelperInt("currentEditorLogicalLineNumber", {}, 1); }
QVariantMap ContentsDocumentTextBlockController::currentCursorRowRect() const { return invokeHelperVariantMap("currentCursorRowRect"); }
bool ContentsDocumentTextBlockController::cursorOnFirstVisualRow() const { return invokeHelperBool("cursorOnFirstVisualRow"); }
bool ContentsDocumentTextBlockController::cursorOnLastVisualRow() const { return invokeHelperBool("cursorOnLastVisualRow"); }
QVariantMap ContentsDocumentTextBlockController::computePlainTextReplacementDelta(const QVariant& previousText, const QVariant& nextText) const { return invokeHelperVariantMap("computePlainTextReplacementDelta", {previousText, nextText}); }
void ContentsDocumentTextBlockController::focusEditor(const QVariant& cursorPosition) const { invokeHelperVoid("focusEditor", {cursorPosition}); }
bool ContentsDocumentTextBlockController::restoreEditorSelection(const int selectionStart, const int selectionEnd, const int cursorPosition) const { return invokeHelperBool("restoreEditorSelection", {selectionStart, selectionEnd, cursorPosition}); }
bool ContentsDocumentTextBlockController::clearSelection(const bool preserveFocusedEditor) const { return invokeHelperBool("clearSelection", {preserveFocusedEditor}); }
QVariantMap ContentsDocumentTextBlockController::inlineFormatSelectionSnapshot() const { return invokeHelperVariantMap("inlineFormatSelectionSnapshot"); }
bool ContentsDocumentTextBlockController::applyFocusRequest(const QVariantMap& request) const { return invokeHelperBool("applyFocusRequest", {request}); }
int ContentsDocumentTextBlockController::shortcutInsertionSourceOffset() const { return invokeHelperInt("shortcutInsertionSourceOffset"); }
bool ContentsDocumentTextBlockController::handleBoundaryDeletionKeyPress(QObject* event) const { return invokeHelperBool("handleBoundaryDeletionKeyPress", {QVariant::fromValue(event)}); }
QUrl ContentsDocumentTextBlockController::helperSourceUrl() const { return helperUrl("ContentsDocumentTextBlockController.qml"); }

ContentsEditorSelectionController::ContentsEditorSelectionController(QObject* parent)
    : ContentsQmlBackedInputControllerBase(parent)
{
    QObject::connect(this, &ContentsQmlBackedInputControllerBase::helperReady, this, [this]() {
        connectHelperPropertyNotify("contextMenuSelectionEnd", "contextMenuSelectionEndChanged()");
        connectHelperPropertyNotify("contextMenuSelectionCursorPosition", "contextMenuSelectionCursorPositionChanged()");
        connectHelperPropertyNotify("contextMenuSelectionStart", "contextMenuSelectionStartChanged()");
        connectHelperPropertyNotify("contextMenuSelectionText", "contextMenuSelectionTextChanged()");
        emit contextMenuItemsChanged();
        emit contextMenuSelectionEndChanged();
        emit contextMenuSelectionCursorPositionChanged();
        emit contextMenuSelectionStartChanged();
        emit contextMenuSelectionTextChanged();
    });
}
QObject* ContentsEditorSelectionController::contentEditor() const noexcept { return m_contentEditor; }
void ContentsEditorSelectionController::setContentEditor(QObject* value) { if (m_contentEditor == value) return; m_contentEditor = value; syncHelperProperty("contentEditor", QVariant::fromValue(value)); emit contentEditorChanged(); }
QVariantList ContentsEditorSelectionController::contextMenuItems() const
{
    return WhatSon::Editor::DynamicObjectSupport::normalizeSequentialVariant(helperProperty("contextMenuItems"));
}
int ContentsEditorSelectionController::contextMenuSelectionEnd() const { return helperObject() ? helperProperty("contextMenuSelectionEnd").toInt() : m_contextMenuSelectionEnd; }
void ContentsEditorSelectionController::setContextMenuSelectionEnd(const int value) { if (!helperObject() && m_contextMenuSelectionEnd == value) return; m_contextMenuSelectionEnd = value; syncHelperProperty("contextMenuSelectionEnd", value); emit contextMenuSelectionEndChanged(); }
double ContentsEditorSelectionController::contextMenuSelectionCursorPosition() const { return helperObject() ? helperProperty("contextMenuSelectionCursorPosition").toDouble() : m_contextMenuSelectionCursorPosition; }
void ContentsEditorSelectionController::setContextMenuSelectionCursorPosition(const double value) { if (!helperObject() && qFuzzyCompare(m_contextMenuSelectionCursorPosition, value)) return; m_contextMenuSelectionCursorPosition = value; syncHelperProperty("contextMenuSelectionCursorPosition", value); emit contextMenuSelectionCursorPositionChanged(); }
int ContentsEditorSelectionController::contextMenuSelectionStart() const { return helperObject() ? helperProperty("contextMenuSelectionStart").toInt() : m_contextMenuSelectionStart; }
void ContentsEditorSelectionController::setContextMenuSelectionStart(const int value) { if (!helperObject() && m_contextMenuSelectionStart == value) return; m_contextMenuSelectionStart = value; syncHelperProperty("contextMenuSelectionStart", value); emit contextMenuSelectionStartChanged(); }
QString ContentsEditorSelectionController::contextMenuSelectionText() const { return helperObject() ? helperProperty("contextMenuSelectionText").toString() : m_contextMenuSelectionText; }
void ContentsEditorSelectionController::setContextMenuSelectionText(const QString& value) { if (!helperObject() && m_contextMenuSelectionText == value) return; m_contextMenuSelectionText = value; syncHelperProperty("contextMenuSelectionText", value); emit contextMenuSelectionTextChanged(); }
QObject* ContentsEditorSelectionController::editorSession() const noexcept { return m_editorSession; }
void ContentsEditorSelectionController::setEditorSession(QObject* value) { if (m_editorSession == value) return; m_editorSession = value; syncHelperProperty("editorSession", QVariant::fromValue(value)); emit editorSessionChanged(); }
QObject* ContentsEditorSelectionController::editorViewport() const noexcept { return m_editorViewport; }
void ContentsEditorSelectionController::setEditorViewport(QObject* value) { if (m_editorViewport == value) return; m_editorViewport = value; syncHelperProperty("editorViewport", QVariant::fromValue(value)); emit editorViewportChanged(); }
QObject* ContentsEditorSelectionController::selectionBridge() const noexcept { return m_selectionBridge; }
void ContentsEditorSelectionController::setSelectionBridge(QObject* value) { if (m_selectionBridge == value) return; m_selectionBridge = value; syncHelperProperty("selectionBridge", QVariant::fromValue(value)); emit selectionBridgeChanged(); }
QObject* ContentsEditorSelectionController::selectionContextMenu() const noexcept { return m_selectionContextMenu; }
void ContentsEditorSelectionController::setSelectionContextMenu(QObject* value) { if (m_selectionContextMenu == value) return; m_selectionContextMenu = value; syncHelperProperty("selectionContextMenu", QVariant::fromValue(value)); emit selectionContextMenuChanged(); }
QObject* ContentsEditorSelectionController::textMetricsBridge() const noexcept { return m_textMetricsBridge; }
void ContentsEditorSelectionController::setTextMetricsBridge(QObject* value) { if (m_textMetricsBridge == value) return; m_textMetricsBridge = value; syncHelperProperty("textMetricsBridge", QVariant::fromValue(value)); emit textMetricsBridgeChanged(); }
QObject* ContentsEditorSelectionController::view() const noexcept { return m_view; }
void ContentsEditorSelectionController::setView(QObject* value) { if (m_view == value) return; m_view = value; syncHelperProperty("view", QVariant::fromValue(value)); emit viewChanged(); }
QVariantMap ContentsEditorSelectionController::contextMenuEditorSelectionRange() const { return invokeHelperVariantMap("contextMenuEditorSelectionRange"); }
QVariantMap ContentsEditorSelectionController::contextMenuEditorSelectionSnapshot() const { return invokeHelperVariantMap("contextMenuEditorSelectionSnapshot"); }
QVariantMap ContentsEditorSelectionController::currentInlineFormatSelectionSnapshot() const { return invokeHelperVariantMap("currentInlineFormatSelectionSnapshot"); }
int ContentsEditorSelectionController::currentEditorCursorPosition() const { return invokeHelperInt("currentEditorCursorPosition"); }
QString ContentsEditorSelectionController::currentSelectedEditorText() const { return invokeHelperString("currentSelectedEditorText"); }
QVariantMap ContentsEditorSelectionController::selectedEditorRange() const { return invokeHelperVariantMap("selectedEditorRange"); }
QVariantMap ContentsEditorSelectionController::inferSelectionRangeFromSelectedText(const QVariant& selectedText, const QVariant& cursorPosition) const { return invokeHelperVariantMap("inferSelectionRangeFromSelectedText", {selectedText, cursorPosition}); }
QVariantMap ContentsEditorSelectionController::inlineStyleWrapTags(const QString& styleTag) const { return invokeHelperVariantMap("inlineStyleWrapTags", {styleTag}); }
QString ContentsEditorSelectionController::normalizeInlineStyleTag(const QString& tagName) const { return invokeHelperString("normalizeInlineStyleTag", {tagName}); }
bool ContentsEditorSelectionController::queueInlineFormatWrap(const QString& tagName) const { return invokeHelperBool("queueInlineFormatWrap", {tagName}); }
bool ContentsEditorSelectionController::wrapSelectedEditorTextWithTag(const QString& tagName, const QVariantMap& explicitSelectionRange) const { return invokeHelperBool("wrapSelectedEditorTextWithTag", {tagName, explicitSelectionRange}); }
bool ContentsEditorSelectionController::openEditorSelectionContextMenu(const qreal localX, const qreal localY) const { return invokeHelperBool("openEditorSelectionContextMenu", {localX, localY}); }
bool ContentsEditorSelectionController::primeContextMenuSelectionSnapshot() const { return invokeHelperBool("primeContextMenuSelectionSnapshot"); }
void ContentsEditorSelectionController::handleSelectionContextMenuEvent(const QString& eventName) const { invokeHelperVoid("handleSelectionContextMenuEvent", {eventName}); }
void ContentsEditorSelectionController::resetEditorSelectionCache() const { invokeHelperVoid("resetEditorSelectionCache"); }
QUrl ContentsEditorSelectionController::helperSourceUrl() const { return helperUrl("ContentsEditorSelectionController.qml"); }

ContentsEditorTypingController::ContentsEditorTypingController(QObject* parent)
    : ContentsQmlBackedInputControllerBase(parent)
{
}
QObject* ContentsEditorTypingController::view() const noexcept { return m_view; }
void ContentsEditorTypingController::setView(QObject* value) { if (m_view == value) return; m_view = value; syncHelperProperty("view", QVariant::fromValue(value)); emit viewChanged(); }
QObject* ContentsEditorTypingController::contentEditor() const noexcept { return m_contentEditor; }
void ContentsEditorTypingController::setContentEditor(QObject* value) { if (m_contentEditor == value) return; m_contentEditor = value; syncHelperProperty("contentEditor", QVariant::fromValue(value)); emit contentEditorChanged(); }
QObject* ContentsEditorTypingController::editorSession() const noexcept { return m_editorSession; }
void ContentsEditorTypingController::setEditorSession(QObject* value) { if (m_editorSession == value) return; m_editorSession = value; syncHelperProperty("editorSession", QVariant::fromValue(value)); emit editorSessionChanged(); }
QObject* ContentsEditorTypingController::plainTextSourceMutator() const noexcept { return m_plainTextSourceMutator; }
void ContentsEditorTypingController::setPlainTextSourceMutator(QObject* value) { if (m_plainTextSourceMutator == value) return; m_plainTextSourceMutator = value; syncHelperProperty("plainTextSourceMutator", QVariant::fromValue(value)); emit plainTextSourceMutatorChanged(); }
QObject* ContentsEditorTypingController::textMetricsBridge() const noexcept { return m_textMetricsBridge; }
void ContentsEditorTypingController::setTextMetricsBridge(QObject* value) { if (m_textMetricsBridge == value) return; m_textMetricsBridge = value; syncHelperProperty("textMetricsBridge", QVariant::fromValue(value)); emit textMetricsBridgeChanged(); }
QObject* ContentsEditorTypingController::agendaBackend() const noexcept { return m_agendaBackend; }
void ContentsEditorTypingController::setAgendaBackend(QObject* value) { if (m_agendaBackend == value) return; m_agendaBackend = value; syncHelperProperty("agendaBackend", QVariant::fromValue(value)); emit agendaBackendChanged(); }
QObject* ContentsEditorTypingController::calloutBackend() const noexcept { return m_calloutBackend; }
void ContentsEditorTypingController::setCalloutBackend(QObject* value) { if (m_calloutBackend == value) return; m_calloutBackend = value; syncHelperProperty("calloutBackend", QVariant::fromValue(value)); emit calloutBackendChanged(); }
void ContentsEditorTypingController::synchronizeLiveEditingStateFromPresentation() const { invokeHelperVoid("synchronizeLiveEditingStateFromPresentation"); }
QString ContentsEditorTypingController::currentEditorPlainText() const { return invokeHelperString("currentEditorPlainText"); }
bool ContentsEditorTypingController::queueAgendaShortcutInsertion() const { return invokeHelperBool("queueAgendaShortcutInsertion"); }
bool ContentsEditorTypingController::queueCalloutShortcutInsertion() const { return invokeHelperBool("queueCalloutShortcutInsertion"); }
bool ContentsEditorTypingController::queueBreakShortcutInsertion() const { return invokeHelperBool("queueBreakShortcutInsertion"); }
bool ContentsEditorTypingController::handleEditorTextEdited() const { return invokeHelperBool("handleEditorTextEdited"); }
int ContentsEditorTypingController::logicalOffsetForSourceOffset(const int sourceOffset) const { return invokeHelperInt("logicalOffsetForSourceOffset", {sourceOffset}); }
int ContentsEditorTypingController::sourceOffsetForLogicalOffset(const int logicalOffset) const { return invokeHelperInt("sourceOffsetForLogicalOffset", {logicalOffset}); }
QUrl ContentsEditorTypingController::helperSourceUrl() const { return helperUrl("ContentsEditorTypingController.qml"); }

ContentsInlineFormatEditorController::ContentsInlineFormatEditorController(QObject* parent)
    : ContentsQmlBackedInputControllerBase(parent)
{
}
QObject* ContentsInlineFormatEditorController::control() const noexcept { return m_control; }
void ContentsInlineFormatEditorController::setControl(QObject* value) { if (m_control == value) return; m_control = value; syncHelperProperty("control", QVariant::fromValue(value)); emit controlChanged(); }
QObject* ContentsInlineFormatEditorController::textInput() const noexcept { return m_textInput; }
void ContentsInlineFormatEditorController::setTextInput(QObject* value) { if (m_textInput == value) return; m_textInput = value; syncHelperProperty("textInput", QVariant::fromValue(value)); emit textInputChanged(); }
void ContentsInlineFormatEditorController::forceActiveFocus() const { invokeHelperVoid("forceActiveFocus"); }
QString ContentsInlineFormatEditorController::currentPlainText() const { return invokeHelperString("currentPlainText"); }
QVariantMap ContentsInlineFormatEditorController::selectionSnapshot() const { return invokeHelperVariantMap("selectionSnapshot"); }
void ContentsInlineFormatEditorController::clearSelection() const { invokeHelperVoid("clearSelection"); }
void ContentsInlineFormatEditorController::clearCachedSelectionSnapshot() const { invokeHelperVoid("clearCachedSelectionSnapshot"); }
QVariantMap ContentsInlineFormatEditorController::cacheCurrentSelectionSnapshot() const { return invokeHelperVariantMap("cacheCurrentSelectionSnapshot"); }
void ContentsInlineFormatEditorController::maybeDiscardCachedSelectionSnapshot() const { invokeHelperVoid("maybeDiscardCachedSelectionSnapshot"); }
QVariantMap ContentsInlineFormatEditorController::inlineFormatSelectionSnapshot() const { return invokeHelperVariantMap("inlineFormatSelectionSnapshot"); }
bool ContentsInlineFormatEditorController::nativeCompositionActive() const { return invokeHelperBool("nativeCompositionActive"); }
int ContentsInlineFormatEditorController::clampLogicalPosition(const int position, const int maximumLength) const { return invokeHelperInt("clampLogicalPosition", {position, maximumLength}); }
int ContentsInlineFormatEditorController::setCursorPositionPreservingNativeInput(const int position) const { return invokeHelperInt("setCursorPositionPreservingNativeInput", {position}, position); }
bool ContentsInlineFormatEditorController::selectionCursorUsesStartEdge(const int cursorPosition, const int selectionStart, const int selectionEnd) const { return invokeHelperBool("selectionCursorUsesStartEdge", {cursorPosition, selectionStart, selectionEnd}); }
bool ContentsInlineFormatEditorController::restoreSelectionRange(const int selectionStart, const int selectionEnd, const int cursorPosition) const { return invokeHelperBool("restoreSelectionRange", {selectionStart, selectionEnd, cursorPosition}); }
QVariantMap ContentsInlineFormatEditorController::programmaticTextSyncPolicy(const QString& nextText) const { return invokeHelperVariantMap("programmaticTextSyncPolicy", {nextText}); }
bool ContentsInlineFormatEditorController::canDeferProgrammaticTextSync(const QString& nextText) const { return invokeHelperBool("canDeferProgrammaticTextSync", {nextText}); }
bool ContentsInlineFormatEditorController::shouldRejectFocusedProgrammaticTextSync(const QString& nextText) const { return invokeHelperBool("shouldRejectFocusedProgrammaticTextSync", {nextText}); }
void ContentsInlineFormatEditorController::flushDeferredProgrammaticText(const bool force) const { invokeHelperVoid("flushDeferredProgrammaticText", {force}); }
void ContentsInlineFormatEditorController::clearDeferredProgrammaticText() const { invokeHelperVoid("clearDeferredProgrammaticText"); }
bool ContentsInlineFormatEditorController::dispatchCommittedTextEditedIfReady() const { return invokeHelperBool("dispatchCommittedTextEditedIfReady"); }
void ContentsInlineFormatEditorController::setProgrammaticText(const QString& nextText) const { invokeHelperVoid("setProgrammaticText", {nextText}); }
void ContentsInlineFormatEditorController::scheduleCommittedTextEditedDispatch() const { invokeHelperVoid("scheduleCommittedTextEditedDispatch"); }
QUrl ContentsInlineFormatEditorController::helperSourceUrl() const { return helperUrl("ContentsInlineFormatEditorController.qml"); }
