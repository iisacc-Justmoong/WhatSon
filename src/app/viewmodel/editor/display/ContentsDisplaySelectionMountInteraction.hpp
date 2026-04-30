#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariant>
#include <QVariantList>

class ContentsDisplaySelectionMountInteraction : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* contentsView READ contentsView WRITE setContentsView NOTIFY contentsViewChanged)
    Q_PROPERTY(QObject* editorSelectionController READ editorSelectionController WRITE setEditorSelectionController NOTIFY editorSelectionControllerChanged)
    Q_PROPERTY(QObject* editorSession READ editorSession WRITE setEditorSession NOTIFY editorSessionChanged)
    Q_PROPERTY(QObject* editorTypingController READ editorTypingController WRITE setEditorTypingController NOTIFY editorTypingControllerChanged)
    Q_PROPERTY(QObject* noteBodyMountCoordinator READ noteBodyMountCoordinator WRITE setNoteBodyMountCoordinator NOTIFY noteBodyMountCoordinatorChanged)
    Q_PROPERTY(QObject* selectionBridge READ selectionBridge WRITE setSelectionBridge NOTIFY selectionBridgeChanged)
    Q_PROPERTY(QObject* selectionSyncCoordinator READ selectionSyncCoordinator WRITE setSelectionSyncCoordinator NOTIFY selectionSyncCoordinatorChanged)
    Q_PROPERTY(QObject* structuredDocumentFlow READ structuredDocumentFlow WRITE setStructuredDocumentFlow NOTIFY structuredDocumentFlowChanged)

public:
    explicit ContentsDisplaySelectionMountInteraction(QObject* parent = nullptr);
    ~ContentsDisplaySelectionMountInteraction() override;

    QObject* contentsView() const noexcept;
    void setContentsView(QObject* value);
    QObject* editorSelectionController() const noexcept;
    void setEditorSelectionController(QObject* value);
    QObject* editorSession() const noexcept;
    void setEditorSession(QObject* value);
    QObject* editorTypingController() const noexcept;
    void setEditorTypingController(QObject* value);
    QObject* noteBodyMountCoordinator() const noexcept;
    void setNoteBodyMountCoordinator(QObject* value);
    QObject* selectionBridge() const noexcept;
    void setSelectionBridge(QObject* value);
    QObject* selectionSyncCoordinator() const noexcept;
    void setSelectionSyncCoordinator(QObject* value);
    QObject* structuredDocumentFlow() const noexcept;
    void setStructuredDocumentFlow(QObject* value);

    Q_INVOKABLE bool shouldFlushBlurredEditorState(const QString& scheduledNoteId) const;
    Q_INVOKABLE void flushEditorStateAfterInputSettles(const QString& scheduledNoteId) const;
    Q_INVOKABLE void focusEditorForSelectedNoteId(const QString& noteId) const;
    Q_INVOKABLE void focusEditorForPendingNote() const;
    Q_INVOKABLE void scheduleEditorEntrySnapshotReconcile() const;
    Q_INVOKABLE void pollSelectedNoteSnapshot() const;
    Q_INVOKABLE bool reconcileEditorEntrySnapshotOnce() const;
    Q_INVOKABLE void scheduleSelectionModelSync(const QVariant& options) const;
    Q_INVOKABLE bool executeSelectionDeliveryPlan(const QVariant& plan, const QString& fallbackKey) const;
    Q_INVOKABLE void scheduleEditorFocusForNote(const QString& noteId) const;
    Q_INVOKABLE void resetEditorSelectionCache() const;

signals:
    void contentsViewChanged();
    void editorSelectionControllerChanged();
    void editorSessionChanged();
    void editorTypingControllerChanged();
    void noteBodyMountCoordinatorChanged();
    void selectionBridgeChanged();
    void selectionSyncCoordinatorChanged();
    void structuredDocumentFlowChanged();

private:
    QVariant invoke(QObject* target, const char* methodName, const QVariantList& arguments = {}) const;
    bool invokeBool(QObject* target, const char* methodName, const QVariantList& arguments = {}) const;
    void invokeVoid(QObject* target, const char* methodName, const QVariantList& arguments = {}) const;
    QVariant property(QObject* target, const char* propertyName) const;
    QString normalizedTextValue(const QVariant& value) const;
    QString normalizedTrimmedTextValue(const QVariant& value) const;
    QString selectedNoteId() const;
    QString editorBoundNoteId() const;
    QString selectedNoteDirectoryPath() const;
    QString sessionEditorText() const;
    bool requestSelectionSnapshotReconcile(const QString& noteId) const;
    void focusStructuredDocumentAtNoteEnd(const QString& noteId) const;

    QPointer<QObject> m_contentsView;
    QPointer<QObject> m_editorSelectionController;
    QPointer<QObject> m_editorSession;
    QPointer<QObject> m_editorTypingController;
    QPointer<QObject> m_noteBodyMountCoordinator;
    QPointer<QObject> m_selectionBridge;
    QPointer<QObject> m_selectionSyncCoordinator;
    QPointer<QObject> m_structuredDocumentFlow;
};
