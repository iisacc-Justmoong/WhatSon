#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantMap>

class ContentsDisplaySelectionSyncCoordinator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(QString selectedNoteId READ selectedNoteId WRITE setSelectedNoteId NOTIFY selectedNoteIdChanged)
    Q_PROPERTY(
        QString
            selectedNoteBodyNoteId READ selectedNoteBodyNoteId WRITE setSelectedNoteBodyNoteId
                NOTIFY selectedNoteBodyNoteIdChanged)
    Q_PROPERTY(
        QString
            selectedNoteBodyText READ selectedNoteBodyText WRITE setSelectedNoteBodyText NOTIFY selectedNoteBodyTextChanged)
    Q_PROPERTY(
        bool
            selectedNoteBodyLoading READ selectedNoteBodyLoading WRITE setSelectedNoteBodyLoading
                NOTIFY selectedNoteBodyLoadingChanged)
    Q_PROPERTY(QString editorBoundNoteId READ editorBoundNoteId WRITE setEditorBoundNoteId NOTIFY editorBoundNoteIdChanged)
    Q_PROPERTY(
        bool
            editorSessionBoundToSelectedNote READ editorSessionBoundToSelectedNote
                WRITE setEditorSessionBoundToSelectedNote NOTIFY editorSessionBoundToSelectedNoteChanged)
    Q_PROPERTY(bool editorInputFocused READ editorInputFocused WRITE setEditorInputFocused NOTIFY editorInputFocusedChanged)
    Q_PROPERTY(bool pendingBodySave READ pendingBodySave WRITE setPendingBodySave NOTIFY pendingBodySaveChanged)
    Q_PROPERTY(
        bool
            typingSessionSyncProtected READ typingSessionSyncProtected WRITE setTypingSessionSyncProtected
                NOTIFY typingSessionSyncProtectedChanged)
    Q_PROPERTY(QString comparedSnapshotNoteId READ comparedSnapshotNoteId NOTIFY comparedSnapshotNoteIdChanged)
    Q_PROPERTY(QString pendingSnapshotNoteId READ pendingSnapshotNoteId NOTIFY pendingSnapshotNoteIdChanged)
    Q_PROPERTY(QString pendingEditorFocusNoteId READ pendingEditorFocusNoteId NOTIFY pendingEditorFocusNoteIdChanged)

public:
    explicit ContentsDisplaySelectionSyncCoordinator(QObject* parent = nullptr);
    ~ContentsDisplaySelectionSyncCoordinator() override;

    bool visible() const noexcept;
    void setVisible(bool visible);

    QString selectedNoteId() const;
    void setSelectedNoteId(const QString& noteId);

    QString selectedNoteBodyNoteId() const;
    void setSelectedNoteBodyNoteId(const QString& noteId);

    QString selectedNoteBodyText() const;
    void setSelectedNoteBodyText(const QString& text);

    bool selectedNoteBodyLoading() const noexcept;
    void setSelectedNoteBodyLoading(bool loading);

    QString editorBoundNoteId() const;
    void setEditorBoundNoteId(const QString& noteId);

    bool editorSessionBoundToSelectedNote() const noexcept;
    void setEditorSessionBoundToSelectedNote(bool bound);

    bool editorInputFocused() const noexcept;
    void setEditorInputFocused(bool focused);

    bool pendingBodySave() const noexcept;
    void setPendingBodySave(bool pending);

    bool typingSessionSyncProtected() const noexcept;
    void setTypingSessionSyncProtected(bool protectedSync);

    QString comparedSnapshotNoteId() const;
    QString pendingSnapshotNoteId() const;
    QString pendingEditorFocusNoteId() const;

    Q_INVOKABLE void scheduleSelectionSync(const QVariantMap& options);
    Q_INVOKABLE void scheduleSnapshotReconcile();
    Q_INVOKABLE QVariantMap snapshotPollPlan() const;
    Q_INVOKABLE QVariantMap snapshotReconcilePlan() const;
    Q_INVOKABLE void markSnapshotReconcileStarted(const QString& noteId);
    Q_INVOKABLE void handleSnapshotReconcileFinished(const QString& noteId, bool success);
    Q_INVOKABLE void invalidateComparedSnapshot();
    Q_INVOKABLE void scheduleEditorFocusForNote(const QString& noteId);
    Q_INVOKABLE QString takePendingEditorFocusNoteId(const QString& selectedNoteId);

signals:
    void visibleChanged();
    void selectedNoteIdChanged();
    void selectedNoteBodyNoteIdChanged();
    void selectedNoteBodyTextChanged();
    void selectedNoteBodyLoadingChanged();
    void editorBoundNoteIdChanged();
    void editorSessionBoundToSelectedNoteChanged();
    void editorInputFocusedChanged();
    void pendingBodySaveChanged();
    void typingSessionSyncProtectedChanged();
    void comparedSnapshotNoteIdChanged();
    void pendingSnapshotNoteIdChanged();
    void pendingEditorFocusNoteIdChanged();

    void selectionSyncFlushRequested(const QVariantMap& plan);
    void snapshotReconcileRequested();
    void editorFocusRequested();

private:
    static QString normalizeNoteId(const QString& value);

    bool canFlushSelectionSyncImmediately() const;
    QVariantMap buildSnapshotPlan(const QString& reason) const;
    void flushSelectionSync();
    void setComparedSnapshotNoteId(const QString& noteId);
    void setPendingSnapshotNoteId(const QString& noteId);
    void setPendingEditorFocusNoteId(const QString& noteId);

    bool m_visible = true;
    QString m_selectedNoteId;
    QString m_selectedNoteBodyNoteId;
    QString m_selectedNoteBodyText;
    bool m_selectedNoteBodyLoading = false;
    QString m_editorBoundNoteId;
    bool m_editorSessionBoundToSelectedNote = false;
    bool m_editorInputFocused = false;
    bool m_pendingBodySave = false;
    bool m_typingSessionSyncProtected = false;

    QString m_comparedSnapshotNoteId;
    QString m_pendingSnapshotNoteId;
    QString m_pendingEditorFocusNoteId;

    bool m_snapshotReconcileQueued = false;
    bool m_selectionSyncQueued = false;
    bool m_selectionSyncResetSnapshotPending = false;
    bool m_selectionSyncReconcilePending = false;
    bool m_selectionSyncFocusEditorPending = false;
    bool m_selectionSyncFallbackRefreshPending = false;
    bool m_selectionSyncForceVisualRefreshPending = false;
};
