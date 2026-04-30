#pragma once

#include <QObject>
#include <QJSValue>
#include <QPointer>
#include <QString>
#include <QVariant>
#include <QVariantList>

class ContentsDisplayInteractionViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* contentsView READ contentsView WRITE setContentsView NOTIFY contentsViewChanged)
    Q_PROPERTY(QObject* contentsAgendaBackend READ contentsAgendaBackend WRITE setContentsAgendaBackend NOTIFY contentsAgendaBackendChanged)
    Q_PROPERTY(QObject* contextMenuCoordinator READ contextMenuCoordinator WRITE setContextMenuCoordinator NOTIFY contextMenuCoordinatorChanged)
    Q_PROPERTY(QObject* editOperationCoordinator READ editOperationCoordinator WRITE setEditOperationCoordinator NOTIFY editOperationCoordinatorChanged)
    Q_PROPERTY(QObject* editorInputPolicyAdapter READ editorInputPolicyAdapter WRITE setEditorInputPolicyAdapter NOTIFY editorInputPolicyAdapterChanged)
    Q_PROPERTY(QObject* editorProjection READ editorProjection WRITE setEditorProjection NOTIFY editorProjectionChanged)
    Q_PROPERTY(QObject* editorSelectionController READ editorSelectionController WRITE setEditorSelectionController NOTIFY editorSelectionControllerChanged)
    Q_PROPERTY(QObject* editorSession READ editorSession WRITE setEditorSession NOTIFY editorSessionChanged)
    Q_PROPERTY(QObject* editorTypingController READ editorTypingController WRITE setEditorTypingController NOTIFY editorTypingControllerChanged)
    Q_PROPERTY(QObject* editorViewport READ editorViewport WRITE setEditorViewport NOTIFY editorViewportChanged)
    Q_PROPERTY(QObject* eventPump READ eventPump WRITE setEventPump NOTIFY eventPumpChanged)
    Q_PROPERTY(QObject* minimapLayer READ minimapLayer WRITE setMinimapLayer NOTIFY minimapLayerChanged)
    Q_PROPERTY(QObject* mutationViewModel READ mutationViewModel WRITE setMutationViewModel NOTIFY mutationViewModelChanged)
    Q_PROPERTY(QObject* panelViewModel READ panelViewModel WRITE setPanelViewModel NOTIFY panelViewModelChanged)
    Q_PROPERTY(QObject* presentationViewModel READ presentationViewModel WRITE setPresentationViewModel NOTIFY presentationViewModelChanged)
    Q_PROPERTY(QObject* presentationRefreshController READ presentationRefreshController WRITE setPresentationRefreshController NOTIFY presentationRefreshControllerChanged)
    Q_PROPERTY(QObject* resourceImportController READ resourceImportController WRITE setResourceImportController NOTIFY resourceImportControllerChanged)
    Q_PROPERTY(QObject* selectionMountViewModel READ selectionMountViewModel WRITE setSelectionMountViewModel NOTIFY selectionMountViewModelChanged)
    Q_PROPERTY(QObject* structuredDocumentFlow READ structuredDocumentFlow WRITE setStructuredDocumentFlow NOTIFY structuredDocumentFlowChanged)

public:
    explicit ContentsDisplayInteractionViewModel(QObject* parent = nullptr);
    ~ContentsDisplayInteractionViewModel() override;

    QObject* contentsView() const noexcept;
    void setContentsView(QObject* value);
    QObject* contentsAgendaBackend() const noexcept;
    void setContentsAgendaBackend(QObject* value);
    QObject* contextMenuCoordinator() const noexcept;
    void setContextMenuCoordinator(QObject* value);
    QObject* editOperationCoordinator() const noexcept;
    void setEditOperationCoordinator(QObject* value);
    QObject* editorInputPolicyAdapter() const noexcept;
    void setEditorInputPolicyAdapter(QObject* value);
    QObject* editorProjection() const noexcept;
    void setEditorProjection(QObject* value);
    QObject* editorSelectionController() const noexcept;
    void setEditorSelectionController(QObject* value);
    QObject* editorSession() const noexcept;
    void setEditorSession(QObject* value);
    QObject* editorTypingController() const noexcept;
    void setEditorTypingController(QObject* value);
    QObject* editorViewport() const noexcept;
    void setEditorViewport(QObject* value);
    QObject* eventPump() const noexcept;
    void setEventPump(QObject* value);
    QObject* minimapLayer() const noexcept;
    void setMinimapLayer(QObject* value);
    QObject* mutationViewModel() const noexcept;
    void setMutationViewModel(QObject* value);
    QObject* panelViewModel() const noexcept;
    void setPanelViewModel(QObject* value);
    QObject* presentationViewModel() const noexcept;
    void setPresentationViewModel(QObject* value);
    QObject* presentationRefreshController() const noexcept;
    void setPresentationRefreshController(QObject* value);
    QObject* resourceImportController() const noexcept;
    void setResourceImportController(QObject* value);
    QObject* selectionMountViewModel() const noexcept;
    void setSelectionMountViewModel(QObject* value);
    QObject* structuredDocumentFlow() const noexcept;
    void setStructuredDocumentFlow(QObject* value);

    Q_INVOKABLE void logEditorCreationState(const QString& reason) const;
    Q_INVOKABLE bool shouldFlushBlurredEditorState(const QString& scheduledNoteId) const;
    Q_INVOKABLE bool nativeEditorCompositionActive() const;
    Q_INVOKABLE bool nativeTextInputSessionOwnsKeyboard() const;
    Q_INVOKABLE void flushEditorStateAfterInputSettles(const QString& scheduledNoteId) const;
    Q_INVOKABLE void focusEditorForSelectedNoteId(const QString& noteId) const;
    Q_INVOKABLE void focusEditorForPendingNote() const;

    Q_INVOKABLE bool eventRequestsPasteShortcut(const QJSValue& event) const;
    Q_INVOKABLE QString inlineFormatShortcutTag(const QJSValue& event) const;
    Q_INVOKABLE bool handleInlineFormatTagShortcut(const QJSValue& event) const;
    Q_INVOKABLE bool clipboardImageAvailableForPaste() const;
    Q_INVOKABLE bool handleClipboardImagePasteShortcut(const QJSValue& event) const;
    Q_INVOKABLE bool handleTagManagementShortcutKeyPress(const QJSValue& event) const;
    Q_INVOKABLE bool handleSelectionContextMenuEvent(const QString& eventName) const;

    Q_INVOKABLE void commitDocumentPresentationRefresh() const;
    Q_INVOKABLE bool documentPresentationRenderDirty() const;
    Q_INVOKABLE void refreshInlineResourcePresentation() const;
    Q_INVOKABLE void requestViewHook(const QString& reason) const;
    Q_INVOKABLE QString encodeXmlAttributeValue(const QVariant& value) const;
    Q_INVOKABLE void resetStructuredSelectionContextMenuSnapshot() const;
    Q_INVOKABLE bool primeStructuredSelectionContextMenuSnapshot() const;
    Q_INVOKABLE bool handleStructuredSelectionContextMenuEvent(const QString& eventName) const;
    Q_INVOKABLE bool openEditorSelectionContextMenu(qreal localX, qreal localY) const;
    Q_INVOKABLE bool editorContextMenuPointerTriggerAccepted(const QString& triggerKind) const;
    Q_INVOKABLE bool requestEditorSelectionContextMenuFromPointer(qreal localX, qreal localY, const QString& triggerKind) const;
    Q_INVOKABLE bool editorSelectionContextMenuSnapshotValid() const;
    Q_INVOKABLE bool ensureEditorSelectionContextMenuSnapshot() const;
    Q_INVOKABLE bool primeEditorSelectionContextMenuSnapshot() const;

    Q_INVOKABLE void scheduleEditorEntrySnapshotReconcile() const;
    Q_INVOKABLE void pollSelectedNoteSnapshot() const;
    Q_INVOKABLE bool reconcileEditorEntrySnapshotOnce() const;
    Q_INVOKABLE bool persistEditorTextImmediately(const QString& nextText) const;
    Q_INVOKABLE bool queueStructuredInlineFormatWrap(const QString& tagName) const;
    Q_INVOKABLE bool queueInlineFormatWrap(const QString& tagName) const;
    Q_INVOKABLE bool queueAgendaShortcutInsertion() const;
    Q_INVOKABLE bool queueCalloutShortcutInsertion() const;
    Q_INVOKABLE bool queueBreakShortcutInsertion() const;
    Q_INVOKABLE void focusStructuredBlockSourceOffset(int sourceOffset) const;
    Q_INVOKABLE bool applyDocumentSourceMutation(const QString& nextSourceText, const QVariant& focusRequest) const;
    Q_INVOKABLE bool setAgendaTaskDone(int taskOpenTagStart, int taskOpenTagEnd, bool checked) const;
    Q_INVOKABLE QVariant selectedEditorRange() const;
    Q_INVOKABLE bool wrapSelectedEditorTextWithTag(const QString& tagName, const QVariant& explicitSelectionRange) const;
    Q_INVOKABLE void scheduleSelectionModelSync(const QVariant& options) const;
    Q_INVOKABLE bool executeSelectionDeliveryPlan(const QVariant& plan, const QString& fallbackKey) const;
    Q_INVOKABLE void scheduleEditorFocusForNote(const QString& noteId) const;
    Q_INVOKABLE void applyPresentationRefreshPlan(const QVariant& plan) const;
    Q_INVOKABLE void executeRefreshPlan(const QVariant& plan) const;
    Q_INVOKABLE void scheduleStructuredDocumentOpenLayoutRefresh(const QString& reason) const;
    Q_INVOKABLE void scheduleDeferredDocumentPresentationRefresh() const;
    Q_INVOKABLE void scheduleDocumentPresentationRefresh(bool forceImmediate) const;
    Q_INVOKABLE void resetEditorSelectionCache() const;

signals:
    void contentsViewChanged();
    void contentsAgendaBackendChanged();
    void contextMenuCoordinatorChanged();
    void editOperationCoordinatorChanged();
    void editorInputPolicyAdapterChanged();
    void editorProjectionChanged();
    void editorSelectionControllerChanged();
    void editorSessionChanged();
    void editorTypingControllerChanged();
    void editorViewportChanged();
    void eventPumpChanged();
    void minimapLayerChanged();
    void mutationViewModelChanged();
    void panelViewModelChanged();
    void presentationViewModelChanged();
    void presentationRefreshControllerChanged();
    void resourceImportControllerChanged();
    void selectionMountViewModelChanged();
    void structuredDocumentFlowChanged();

private:
    QVariant invoke(QObject* target, const char* methodName, const QVariantList& arguments = {}) const;
    bool invokeBool(QObject* target, const char* methodName, const QVariantList& arguments = {}) const;
    void invokeVoid(QObject* target, const char* methodName, const QVariantList& arguments = {}) const;
    QVariant property(QObject* target, const char* propertyName) const;
    void setProperty(QObject* target, const char* propertyName, const QVariant& value) const;
    QString bodyTagShortcutKind(const QJSValue& event) const;
    int eventKey(const QJSValue& event) const;
    int eventModifiers(const QJSValue& event) const;
    QString eventText(const QJSValue& event) const;
    void setEventAccepted(const QJSValue& event) const;
    QString normalizedTextValue(const QVariant& value) const;
    QString normalizedTrimmedTextValue(const QVariant& value) const;
    QString selectedNoteDirectoryPath() const;
    bool editorSessionBoundToSelectedNote() const;
    bool ensureEditorSessionBoundForSourceMutation(const QString& currentSourceText) const;
    QString currentDocumentSourceText() const;
    bool focusRequestRequiresImmediatePersistence(const QVariant& focusRequest) const;
    bool persistRawEditorTextImmediately(const QString& nextSourceText) const;
    QString committedEditorText(const QString& fallbackText) const;
    bool commitRawEditorTextMutation(const QString& nextSourceText) const;
    void requestStructuredFocusLater(const QVariant& focusRequest) const;

    QPointer<QObject> m_contentsView;
    QPointer<QObject> m_contentsAgendaBackend;
    QPointer<QObject> m_contextMenuCoordinator;
    QPointer<QObject> m_editOperationCoordinator;
    QPointer<QObject> m_editorInputPolicyAdapter;
    QPointer<QObject> m_editorProjection;
    QPointer<QObject> m_editorSelectionController;
    QPointer<QObject> m_editorSession;
    QPointer<QObject> m_editorTypingController;
    QPointer<QObject> m_editorViewport;
    QPointer<QObject> m_eventPump;
    QPointer<QObject> m_minimapLayer;
    QPointer<QObject> m_mutationViewModel;
    QPointer<QObject> m_panelViewModel;
    QPointer<QObject> m_presentationViewModel;
    QPointer<QObject> m_presentationRefreshController;
    QPointer<QObject> m_resourceImportController;
    QPointer<QObject> m_selectionMountViewModel;
    QPointer<QObject> m_structuredDocumentFlow;
};
