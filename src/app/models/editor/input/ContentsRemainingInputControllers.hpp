#pragma once

#include "app/models/editor/input/ContentsQmlBackedInputControllerBase.hpp"

#include <QtGlobal>
#include <qqmlregistration.h>

class ContentsAgendaBlockController : public ContentsQmlBackedInputControllerBase
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ContentsAgendaBlockController)
    Q_PROPERTY(QObject* agendaBlock READ agendaBlock WRITE setAgendaBlock NOTIFY agendaBlockChanged FINAL)
    Q_PROPERTY(QObject* taskRepeater READ taskRepeater WRITE setTaskRepeater NOTIFY taskRepeaterChanged FINAL)

public:
    explicit ContentsAgendaBlockController(QObject* parent = nullptr);

    QObject* agendaBlock() const noexcept;
    void setAgendaBlock(QObject* value);
    QObject* taskRepeater() const noexcept;
    void setTaskRepeater(QObject* value);

    Q_INVOKABLE bool hasFocusedTaskRow() const;
    Q_INVOKABLE bool focusedTaskInputMethodComposing() const;
    Q_INVOKABLE QString focusedTaskPreeditText() const;
    Q_INVOKABLE bool nativeCompositionActive() const;
    Q_INVOKABLE int currentFocusedTaskLineNumber() const;
    Q_INVOKABLE QVariantMap currentCursorRowRect() const;
    Q_INVOKABLE QString currentVisiblePlainText() const;
    Q_INVOKABLE QString visiblePlainText() const;
    Q_INVOKABLE int representativeCharCount(const QVariant& lineText) const;
    Q_INVOKABLE bool focusLastTask() const;
    Q_INVOKABLE bool focusTaskBoundary(int taskIndex, const QString& side) const;
    Q_INVOKABLE bool focusBoundary(const QString& side) const;
    Q_INVOKABLE bool focusFirstTask() const;
    Q_INVOKABLE bool focusTaskAtSourceOffset(int sourceOffset) const;
    Q_INVOKABLE bool applyFocusRequest(const QVariantMap& request) const;
    Q_INVOKABLE bool clearSelection(bool preserveFocusedEditor) const;
    Q_INVOKABLE int shortcutInsertionSourceOffset() const;

signals:
    void agendaBlockChanged();
    void taskRepeaterChanged();

protected:
    QUrl helperSourceUrl() const override;

private:
    QObject* m_agendaBlock = nullptr;
    QObject* m_taskRepeater = nullptr;
};

class ContentsAgendaTaskRowController : public ContentsQmlBackedInputControllerBase
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ContentsAgendaTaskRowController)
    Q_PROPERTY(QObject* agendaBlock READ agendaBlock WRITE setAgendaBlock NOTIFY agendaBlockChanged FINAL)
    Q_PROPERTY(QObject* taskRow READ taskRow WRITE setTaskRow NOTIFY taskRowChanged FINAL)
    Q_PROPERTY(QObject* taskEditor READ taskEditor WRITE setTaskEditor NOTIFY taskEditorChanged FINAL)

public:
    explicit ContentsAgendaTaskRowController(QObject* parent = nullptr);

    QObject* agendaBlock() const noexcept;
    void setAgendaBlock(QObject* value);
    QObject* taskRow() const noexcept;
    void setTaskRow(QObject* value);
    QObject* taskEditor() const noexcept;
    void setTaskEditor(QObject* value);

    Q_INVOKABLE void focusEditor(const QVariant& cursorPosition) const;
    Q_INVOKABLE bool clearSelection(bool preserveFocusedEditor) const;
    Q_INVOKABLE QString currentEditorPlainText() const;
    Q_INVOKABLE void syncLiveTaskTextFromHost() const;
    Q_INVOKABLE QVariantMap currentCursorRowRect() const;
    Q_INVOKABLE bool nativeCompositionActive() const;
    Q_INVOKABLE QVariantList logicalLineLayoutEntries() const;
    Q_INVOKABLE QString visiblePlainText() const;
    Q_INVOKABLE int representativeCharCount(const QVariant& lineText) const;
    Q_INVOKABLE bool cursorOnFirstVisualRow() const;
    Q_INVOKABLE bool cursorOnLastVisualRow() const;
    Q_INVOKABLE bool applyFocusRequest(const QVariantMap& request) const;
    Q_INVOKABLE int shortcutInsertionSourceOffset() const;
    Q_INVOKABLE bool handleToggleChanged(bool checked) const;
    Q_INVOKABLE bool handleKeyPress(QObject* event) const;

signals:
    void agendaBlockChanged();
    void taskRowChanged();
    void taskEditorChanged();

protected:
    QUrl helperSourceUrl() const override;

private:
    QObject* m_agendaBlock = nullptr;
    QObject* m_taskRow = nullptr;
    QObject* m_taskEditor = nullptr;
};

class ContentsCalloutBlockController : public ContentsQmlBackedInputControllerBase
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ContentsCalloutBlockController)
    Q_PROPERTY(QObject* calloutBlock READ calloutBlock WRITE setCalloutBlock NOTIFY calloutBlockChanged FINAL)
    Q_PROPERTY(QObject* calloutEditor READ calloutEditor WRITE setCalloutEditor NOTIFY calloutEditorChanged FINAL)

public:
    explicit ContentsCalloutBlockController(QObject* parent = nullptr);

    QObject* calloutBlock() const noexcept;
    void setCalloutBlock(QObject* value);
    QObject* calloutEditor() const noexcept;
    void setCalloutEditor(QObject* value);

    Q_INVOKABLE QString currentEditorPlainText() const;
    Q_INVOKABLE int currentEditorLogicalLineNumber() const;
    Q_INVOKABLE QVariantMap currentCursorRowRect() const;
    Q_INVOKABLE bool nativeCompositionActive() const;
    Q_INVOKABLE void syncLiveTextFromHost() const;
    Q_INVOKABLE QVariantList logicalLineLayoutEntries() const;
    Q_INVOKABLE QString visiblePlainText() const;
    Q_INVOKABLE int representativeCharCount(const QVariant& lineText) const;
    Q_INVOKABLE bool cursorOnFirstVisualRow() const;
    Q_INVOKABLE bool cursorOnLastVisualRow() const;
    Q_INVOKABLE void focusEditor(const QVariant& cursorPosition) const;
    Q_INVOKABLE bool clearSelection(bool preserveFocusedEditor) const;
    Q_INVOKABLE bool applyFocusRequest(const QVariantMap& request) const;
    Q_INVOKABLE int shortcutInsertionSourceOffset() const;
    Q_INVOKABLE bool handleTagManagementKeyPress(QObject* event) const;

signals:
    void calloutBlockChanged();
    void calloutEditorChanged();

protected:
    QUrl helperSourceUrl() const override;

private:
    QObject* m_calloutBlock = nullptr;
    QObject* m_calloutEditor = nullptr;
};

class ContentsDocumentBlockController : public ContentsQmlBackedInputControllerBase
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ContentsDocumentBlockController)
    Q_PROPERTY(QObject* documentBlock READ documentBlock WRITE setDocumentBlock NOTIFY documentBlockChanged FINAL)
    Q_PROPERTY(QObject* blockLoader READ blockLoader WRITE setBlockLoader NOTIFY blockLoaderChanged FINAL)

public:
    explicit ContentsDocumentBlockController(QObject* parent = nullptr);

    QObject* documentBlock() const noexcept;
    void setDocumentBlock(QObject* value);
    QObject* blockLoader() const noexcept;
    void setBlockLoader(QObject* value);

    Q_INVOKABLE QVariantMap currentCursorRowRect() const;
    Q_INVOKABLE QVariantList logicalLineLayoutEntries() const;
    Q_INVOKABLE bool applyFocusRequest(const QVariantMap& request) const;
    Q_INVOKABLE bool handleAtomicTagDeleteKeyPress(QObject* event) const;
    Q_INVOKABLE QVariantMap inlineFormatSelectionSnapshot() const;
    Q_INVOKABLE bool clearSelection(bool preserveFocusedEditor) const;
    Q_INVOKABLE int shortcutInsertionSourceOffset() const;
    Q_INVOKABLE QString visiblePlainText() const;
    Q_INVOKABLE int representativeCharCount(const QVariant& lineText) const;
    Q_INVOKABLE bool handleAtomicTagManagementKeyPress(QObject* event) const;
    Q_INVOKABLE void handleAtomicTap(int tapCount) const;

signals:
    void documentBlockChanged();
    void blockLoaderChanged();

protected:
    QUrl helperSourceUrl() const override;

private:
    QObject* m_documentBlock = nullptr;
    QObject* m_blockLoader = nullptr;
};

class ContentsDocumentTextBlockController : public ContentsQmlBackedInputControllerBase
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ContentsDocumentTextBlockController)
    Q_PROPERTY(QObject* textBlock READ textBlock WRITE setTextBlock NOTIFY textBlockChanged FINAL)
    Q_PROPERTY(QObject* blockEditor READ blockEditor WRITE setBlockEditor NOTIFY blockEditorChanged FINAL)
    Q_PROPERTY(QObject* inlineStyleRenderer READ inlineStyleRenderer WRITE setInlineStyleRenderer NOTIFY inlineStyleRendererChanged FINAL)
    Q_PROPERTY(QObject* plainTextSourceMutator READ plainTextSourceMutator WRITE setPlainTextSourceMutator NOTIFY plainTextSourceMutatorChanged FINAL)
    Q_PROPERTY(bool sourceContainsInlineStyleTags READ sourceContainsInlineStyleTags NOTIFY sourceContainsInlineStyleTagsChanged FINAL)
    Q_PROPERTY(QString authoritativePlainText READ authoritativePlainText NOTIFY authoritativePlainTextChanged FINAL)

public:
    explicit ContentsDocumentTextBlockController(QObject* parent = nullptr);

    QObject* textBlock() const noexcept;
    void setTextBlock(QObject* value);
    QObject* blockEditor() const noexcept;
    void setBlockEditor(QObject* value);
    QObject* inlineStyleRenderer() const noexcept;
    void setInlineStyleRenderer(QObject* value);
    QObject* plainTextSourceMutator() const noexcept;
    void setPlainTextSourceMutator(QObject* value);

    bool sourceContainsInlineStyleTags() const;
    QString authoritativePlainText() const;

    Q_INVOKABLE QString authoritativeSourceText() const;
    Q_INVOKABLE QString visiblePlainText() const;
    Q_INVOKABLE int representativeCharCount(const QVariant& lineText) const;
    Q_INVOKABLE QString currentEditorPlainText() const;
    Q_INVOKABLE bool nativeCompositionActive() const;
    Q_INVOKABLE void syncLiveEditSnapshotFromHost() const;
    Q_INVOKABLE QVariantList logicalLineLayoutEntries() const;
    Q_INVOKABLE int currentEditorLogicalLineNumber() const;
    Q_INVOKABLE QVariantMap currentCursorRowRect() const;
    Q_INVOKABLE bool cursorOnFirstVisualRow() const;
    Q_INVOKABLE bool cursorOnLastVisualRow() const;
    Q_INVOKABLE QVariantMap computePlainTextReplacementDelta(const QVariant& previousText, const QVariant& nextText) const;
    Q_INVOKABLE void focusEditor(const QVariant& cursorPosition) const;
    Q_INVOKABLE bool restoreEditorSelection(int selectionStart, int selectionEnd, int cursorPosition) const;
    Q_INVOKABLE bool clearSelection(bool preserveFocusedEditor) const;
    Q_INVOKABLE QVariantMap inlineFormatSelectionSnapshot() const;
    Q_INVOKABLE bool applyFocusRequest(const QVariantMap& request) const;
    Q_INVOKABLE int shortcutInsertionSourceOffset() const;
    Q_INVOKABLE bool handleBoundaryDeletionKeyPress(QObject* event) const;

signals:
    void textBlockChanged();
    void blockEditorChanged();
    void inlineStyleRendererChanged();
    void plainTextSourceMutatorChanged();
    void sourceContainsInlineStyleTagsChanged();
    void authoritativePlainTextChanged();

protected:
    QUrl helperSourceUrl() const override;

private:
    QObject* m_textBlock = nullptr;
    QObject* m_blockEditor = nullptr;
    QObject* m_inlineStyleRenderer = nullptr;
    QObject* m_plainTextSourceMutator = nullptr;
};

class ContentsEditorSelectionController : public ContentsQmlBackedInputControllerBase
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ContentsEditorSelectionController)
    Q_PROPERTY(QObject* contentEditor READ contentEditor WRITE setContentEditor NOTIFY contentEditorChanged FINAL)
    Q_PROPERTY(QVariantList contextMenuItems READ contextMenuItems NOTIFY contextMenuItemsChanged FINAL)
    Q_PROPERTY(int contextMenuSelectionEnd READ contextMenuSelectionEnd WRITE setContextMenuSelectionEnd NOTIFY contextMenuSelectionEndChanged FINAL)
    Q_PROPERTY(double contextMenuSelectionCursorPosition READ contextMenuSelectionCursorPosition WRITE setContextMenuSelectionCursorPosition NOTIFY contextMenuSelectionCursorPositionChanged FINAL)
    Q_PROPERTY(int contextMenuSelectionStart READ contextMenuSelectionStart WRITE setContextMenuSelectionStart NOTIFY contextMenuSelectionStartChanged FINAL)
    Q_PROPERTY(QString contextMenuSelectionText READ contextMenuSelectionText WRITE setContextMenuSelectionText NOTIFY contextMenuSelectionTextChanged FINAL)
    Q_PROPERTY(QObject* editorSession READ editorSession WRITE setEditorSession NOTIFY editorSessionChanged FINAL)
    Q_PROPERTY(QObject* editorViewport READ editorViewport WRITE setEditorViewport NOTIFY editorViewportChanged FINAL)
    Q_PROPERTY(QObject* selectionBridge READ selectionBridge WRITE setSelectionBridge NOTIFY selectionBridgeChanged FINAL)
    Q_PROPERTY(QObject* selectionContextMenu READ selectionContextMenu WRITE setSelectionContextMenu NOTIFY selectionContextMenuChanged FINAL)
    Q_PROPERTY(QObject* textMetricsBridge READ textMetricsBridge WRITE setTextMetricsBridge NOTIFY textMetricsBridgeChanged FINAL)
    Q_PROPERTY(QObject* view READ view WRITE setView NOTIFY viewChanged FINAL)

public:
    explicit ContentsEditorSelectionController(QObject* parent = nullptr);

    QObject* contentEditor() const noexcept;
    void setContentEditor(QObject* value);
    QVariantList contextMenuItems() const;
    int contextMenuSelectionEnd() const;
    void setContextMenuSelectionEnd(int value);
    double contextMenuSelectionCursorPosition() const;
    void setContextMenuSelectionCursorPosition(double value);
    int contextMenuSelectionStart() const;
    void setContextMenuSelectionStart(int value);
    QString contextMenuSelectionText() const;
    void setContextMenuSelectionText(const QString& value);
    QObject* editorSession() const noexcept;
    void setEditorSession(QObject* value);
    QObject* editorViewport() const noexcept;
    void setEditorViewport(QObject* value);
    QObject* selectionBridge() const noexcept;
    void setSelectionBridge(QObject* value);
    QObject* selectionContextMenu() const noexcept;
    void setSelectionContextMenu(QObject* value);
    QObject* textMetricsBridge() const noexcept;
    void setTextMetricsBridge(QObject* value);
    QObject* view() const noexcept;
    void setView(QObject* value);

    Q_INVOKABLE QVariantMap contextMenuEditorSelectionRange() const;
    Q_INVOKABLE QVariantMap contextMenuEditorSelectionSnapshot() const;
    Q_INVOKABLE QVariantMap currentInlineFormatSelectionSnapshot() const;
    Q_INVOKABLE int currentEditorCursorPosition() const;
    Q_INVOKABLE QString currentSelectedEditorText() const;
    Q_INVOKABLE QVariantMap selectedEditorRange() const;
    Q_INVOKABLE QVariantMap inferSelectionRangeFromSelectedText(const QVariant& selectedText, const QVariant& cursorPosition) const;
    Q_INVOKABLE QVariantMap inlineStyleWrapTags(const QString& styleTag) const;
    Q_INVOKABLE QString normalizeInlineStyleTag(const QString& tagName) const;
    Q_INVOKABLE bool queueInlineFormatWrap(const QString& tagName) const;
    Q_INVOKABLE bool wrapSelectedEditorTextWithTag(const QString& tagName, const QVariantMap& explicitSelectionRange = {}) const;
    Q_INVOKABLE bool openEditorSelectionContextMenu(qreal localX, qreal localY) const;
    Q_INVOKABLE bool primeContextMenuSelectionSnapshot() const;
    Q_INVOKABLE void handleSelectionContextMenuEvent(const QString& eventName) const;
    Q_INVOKABLE void resetEditorSelectionCache() const;

signals:
    void contentEditorChanged();
    void contextMenuItemsChanged();
    void contextMenuSelectionEndChanged();
    void contextMenuSelectionCursorPositionChanged();
    void contextMenuSelectionStartChanged();
    void contextMenuSelectionTextChanged();
    void editorSessionChanged();
    void editorViewportChanged();
    void selectionBridgeChanged();
    void selectionContextMenuChanged();
    void textMetricsBridgeChanged();
    void viewChanged();

protected:
    QUrl helperSourceUrl() const override;

private:
    QObject* m_contentEditor = nullptr;
    int m_contextMenuSelectionEnd = -1;
    double m_contextMenuSelectionCursorPosition = qQNaN();
    int m_contextMenuSelectionStart = -1;
    QString m_contextMenuSelectionText;
    QObject* m_editorSession = nullptr;
    QObject* m_editorViewport = nullptr;
    QObject* m_selectionBridge = nullptr;
    QObject* m_selectionContextMenu = nullptr;
    QObject* m_textMetricsBridge = nullptr;
    QObject* m_view = nullptr;
};

class ContentsEditorTypingController : public ContentsQmlBackedInputControllerBase
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ContentsEditorTypingController)
    Q_PROPERTY(QObject* view READ view WRITE setView NOTIFY viewChanged FINAL)
    Q_PROPERTY(QObject* contentEditor READ contentEditor WRITE setContentEditor NOTIFY contentEditorChanged FINAL)
    Q_PROPERTY(QObject* editorSession READ editorSession WRITE setEditorSession NOTIFY editorSessionChanged FINAL)
    Q_PROPERTY(QObject* plainTextSourceMutator READ plainTextSourceMutator WRITE setPlainTextSourceMutator NOTIFY plainTextSourceMutatorChanged FINAL)
    Q_PROPERTY(QObject* textMetricsBridge READ textMetricsBridge WRITE setTextMetricsBridge NOTIFY textMetricsBridgeChanged FINAL)
    Q_PROPERTY(QObject* agendaBackend READ agendaBackend WRITE setAgendaBackend NOTIFY agendaBackendChanged FINAL)
    Q_PROPERTY(QObject* calloutBackend READ calloutBackend WRITE setCalloutBackend NOTIFY calloutBackendChanged FINAL)

public:
    explicit ContentsEditorTypingController(QObject* parent = nullptr);

    QObject* view() const noexcept;
    void setView(QObject* value);
    QObject* contentEditor() const noexcept;
    void setContentEditor(QObject* value);
    QObject* editorSession() const noexcept;
    void setEditorSession(QObject* value);
    QObject* plainTextSourceMutator() const noexcept;
    void setPlainTextSourceMutator(QObject* value);
    QObject* textMetricsBridge() const noexcept;
    void setTextMetricsBridge(QObject* value);
    QObject* agendaBackend() const noexcept;
    void setAgendaBackend(QObject* value);
    QObject* calloutBackend() const noexcept;
    void setCalloutBackend(QObject* value);

    Q_INVOKABLE void synchronizeLiveEditingStateFromPresentation() const;
    Q_INVOKABLE QString currentEditorPlainText() const;
    Q_INVOKABLE bool queueAgendaShortcutInsertion() const;
    Q_INVOKABLE bool queueCalloutShortcutInsertion() const;
    Q_INVOKABLE bool queueBreakShortcutInsertion() const;
    Q_INVOKABLE bool handleEditorTextEdited() const;
    Q_INVOKABLE int logicalOffsetForSourceOffset(int sourceOffset) const;
    Q_INVOKABLE int sourceOffsetForLogicalOffset(int logicalOffset) const;

signals:
    void viewChanged();
    void contentEditorChanged();
    void editorSessionChanged();
    void plainTextSourceMutatorChanged();
    void textMetricsBridgeChanged();
    void agendaBackendChanged();
    void calloutBackendChanged();

protected:
    QUrl helperSourceUrl() const override;

private:
    QObject* m_view = nullptr;
    QObject* m_contentEditor = nullptr;
    QObject* m_editorSession = nullptr;
    QObject* m_plainTextSourceMutator = nullptr;
    QObject* m_textMetricsBridge = nullptr;
    QObject* m_agendaBackend = nullptr;
    QObject* m_calloutBackend = nullptr;
};

class ContentsInlineFormatEditorController : public ContentsQmlBackedInputControllerBase
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ContentsInlineFormatEditorController)
    Q_PROPERTY(QObject* control READ control WRITE setControl NOTIFY controlChanged FINAL)
    Q_PROPERTY(QObject* textInput READ textInput WRITE setTextInput NOTIFY textInputChanged FINAL)

public:
    explicit ContentsInlineFormatEditorController(QObject* parent = nullptr);

    QObject* control() const noexcept;
    void setControl(QObject* value);
    QObject* textInput() const noexcept;
    void setTextInput(QObject* value);

    Q_INVOKABLE void forceActiveFocus() const;
    Q_INVOKABLE QString currentPlainText() const;
    Q_INVOKABLE QVariantMap selectionSnapshot() const;
    Q_INVOKABLE void clearSelection() const;
    Q_INVOKABLE void clearCachedSelectionSnapshot() const;
    Q_INVOKABLE QVariantMap cacheCurrentSelectionSnapshot() const;
    Q_INVOKABLE void maybeDiscardCachedSelectionSnapshot() const;
    Q_INVOKABLE QVariantMap inlineFormatSelectionSnapshot() const;
    Q_INVOKABLE bool nativeCompositionActive() const;
    Q_INVOKABLE int clampLogicalPosition(int position, int maximumLength) const;
    Q_INVOKABLE int setCursorPositionPreservingNativeInput(int position) const;
    Q_INVOKABLE bool selectionCursorUsesStartEdge(int cursorPosition, int selectionStart, int selectionEnd) const;
    Q_INVOKABLE bool restoreSelectionRange(int selectionStart, int selectionEnd, int cursorPosition) const;
    Q_INVOKABLE QVariantMap programmaticTextSyncPolicy(const QString& nextText) const;
    Q_INVOKABLE bool canDeferProgrammaticTextSync(const QString& nextText) const;
    Q_INVOKABLE bool shouldRejectFocusedProgrammaticTextSync(const QString& nextText) const;
    Q_INVOKABLE void flushDeferredProgrammaticText(bool force) const;
    Q_INVOKABLE void clearDeferredProgrammaticText() const;
    Q_INVOKABLE bool dispatchCommittedTextEditedIfReady() const;
    Q_INVOKABLE void setProgrammaticText(const QString& nextText) const;
    Q_INVOKABLE void scheduleCommittedTextEditedDispatch() const;

signals:
    void controlChanged();
    void textInputChanged();

protected:
    QUrl helperSourceUrl() const override;

private:
    QObject* m_control = nullptr;
    QObject* m_textInput = nullptr;
};
