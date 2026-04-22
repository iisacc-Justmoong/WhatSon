#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

class ContentsWsnBodyBlockParser;

class ContentsDisplayNoteBodyMountCoordinator : public QObject
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
            selectedNoteBodyResolved READ selectedNoteBodyResolved WRITE setSelectedNoteBodyResolved
                NOTIFY selectedNoteBodyResolvedChanged)
    Q_PROPERTY(
        bool
            selectedNoteBodyLoading READ selectedNoteBodyLoading WRITE setSelectedNoteBodyLoading
                NOTIFY selectedNoteBodyLoadingChanged)
    Q_PROPERTY(QString editorBoundNoteId READ editorBoundNoteId WRITE setEditorBoundNoteId NOTIFY editorBoundNoteIdChanged)
    Q_PROPERTY(
        bool
            editorSessionBoundToSelectedNote READ editorSessionBoundToSelectedNote
                WRITE setEditorSessionBoundToSelectedNote NOTIFY editorSessionBoundToSelectedNoteChanged)
    Q_PROPERTY(bool pendingBodySave READ pendingBodySave WRITE setPendingBodySave NOTIFY pendingBodySaveChanged)
    Q_PROPERTY(QString editorText READ editorText WRITE setEditorText NOTIFY editorTextChanged)
    Q_PROPERTY(
        bool
            inlineDocumentSurfaceRequested READ inlineDocumentSurfaceRequested
                WRITE setInlineDocumentSurfaceRequested NOTIFY inlineDocumentSurfaceRequestedChanged)
    Q_PROPERTY(
        bool
            inlineDocumentSurfaceReady READ inlineDocumentSurfaceReady
                WRITE setInlineDocumentSurfaceReady NOTIFY inlineDocumentSurfaceReadyChanged)
    Q_PROPERTY(
        bool
            inlineDocumentSurfaceLoading READ inlineDocumentSurfaceLoading
                WRITE setInlineDocumentSurfaceLoading NOTIFY inlineDocumentSurfaceLoadingChanged)
    Q_PROPERTY(
        bool
            structuredDocumentSurfaceRequested READ structuredDocumentSurfaceRequested
                WRITE setStructuredDocumentSurfaceRequested NOTIFY structuredDocumentSurfaceRequestedChanged)
    Q_PROPERTY(
        bool
            structuredDocumentSurfaceReady READ structuredDocumentSurfaceReady
                WRITE setStructuredDocumentSurfaceReady NOTIFY structuredDocumentSurfaceReadyChanged)
    Q_PROPERTY(bool mountPending READ mountPending NOTIFY mountStateChanged)
    Q_PROPERTY(bool parseMounted READ parseMounted NOTIFY mountStateChanged)
    Q_PROPERTY(bool sourceMounted READ sourceMounted NOTIFY mountStateChanged)
    Q_PROPERTY(bool noteMounted READ noteMounted NOTIFY mountStateChanged)
    Q_PROPERTY(bool mountFailed READ mountFailed NOTIFY mountStateChanged)
    Q_PROPERTY(bool surfaceVisible READ surfaceVisible NOTIFY mountStateChanged)
    Q_PROPERTY(QString mountFailureReason READ mountFailureReason NOTIFY mountStateChanged)
    Q_PROPERTY(QString mountFailureMessage READ mountFailureMessage NOTIFY mountStateChanged)
    Q_PROPERTY(QString exceptionReason READ exceptionReason NOTIFY mountStateChanged)
    Q_PROPERTY(QString exceptionTitle READ exceptionTitle NOTIFY mountStateChanged)
    Q_PROPERTY(QString exceptionMessage READ exceptionMessage NOTIFY mountStateChanged)
    Q_PROPERTY(bool exceptionVisible READ exceptionVisible NOTIFY mountStateChanged)
    Q_PROPERTY(bool commandSurfaceEnabled READ commandSurfaceEnabled NOTIFY mountStateChanged)

public:
    explicit ContentsDisplayNoteBodyMountCoordinator(QObject* parent = nullptr);
    ~ContentsDisplayNoteBodyMountCoordinator() override;

    bool visible() const noexcept;
    void setVisible(bool visible);

    QString selectedNoteId() const;
    void setSelectedNoteId(const QString& noteId);

    QString selectedNoteBodyNoteId() const;
    void setSelectedNoteBodyNoteId(const QString& noteId);

    QString selectedNoteBodyText() const;
    void setSelectedNoteBodyText(const QString& text);

    bool selectedNoteBodyResolved() const noexcept;
    void setSelectedNoteBodyResolved(bool resolved);

    bool selectedNoteBodyLoading() const noexcept;
    void setSelectedNoteBodyLoading(bool loading);

    QString editorBoundNoteId() const;
    void setEditorBoundNoteId(const QString& noteId);

    bool editorSessionBoundToSelectedNote() const noexcept;
    void setEditorSessionBoundToSelectedNote(bool bound);

    bool pendingBodySave() const noexcept;
    void setPendingBodySave(bool pending);

    QString editorText() const;
    void setEditorText(const QString& text);

    bool inlineDocumentSurfaceRequested() const noexcept;
    void setInlineDocumentSurfaceRequested(bool requested);

    bool inlineDocumentSurfaceReady() const noexcept;
    void setInlineDocumentSurfaceReady(bool ready);

    bool inlineDocumentSurfaceLoading() const noexcept;
    void setInlineDocumentSurfaceLoading(bool loading);

    bool structuredDocumentSurfaceRequested() const noexcept;
    void setStructuredDocumentSurfaceRequested(bool requested);

    bool structuredDocumentSurfaceReady() const noexcept;
    void setStructuredDocumentSurfaceReady(bool ready);

    bool mountPending() const noexcept;
    bool parseMounted() const noexcept;
    bool sourceMounted() const noexcept;
    bool noteMounted() const noexcept;
    bool mountFailed() const noexcept;
    bool surfaceVisible() const noexcept;
    QString mountFailureReason() const;
    QString mountFailureMessage() const;
    QString exceptionReason() const;
    QString exceptionTitle() const;
    QString exceptionMessage() const;
    bool exceptionVisible() const noexcept;
    bool commandSurfaceEnabled() const noexcept;

    Q_INVOKABLE void scheduleMount(const QVariantMap& options);
    Q_INVOKABLE void handleSnapshotRefreshFinished(const QString& noteId, bool success);
    Q_INVOKABLE QVariantMap currentMountState() const;

signals:
    void visibleChanged();
    void selectedNoteIdChanged();
    void selectedNoteBodyNoteIdChanged();
    void selectedNoteBodyTextChanged();
    void selectedNoteBodyResolvedChanged();
    void selectedNoteBodyLoadingChanged();
    void editorBoundNoteIdChanged();
    void editorSessionBoundToSelectedNoteChanged();
    void pendingBodySaveChanged();
    void editorTextChanged();
    void inlineDocumentSurfaceRequestedChanged();
    void inlineDocumentSurfaceReadyChanged();
    void inlineDocumentSurfaceLoadingChanged();
    void structuredDocumentSurfaceRequestedChanged();
    void structuredDocumentSurfaceReadyChanged();
    void mountStateChanged();

    void mountFlushRequested(const QVariantMap& plan);

private:
    static QString normalizeNoteId(const QString& noteId);

    bool canFlushMountImmediately() const noexcept;
    bool parserAcceptsSource(const QString& sourceText) const;
    bool documentSurfaceRequested() const noexcept;
    bool documentSurfaceReady() const noexcept;
    bool documentSurfaceLoading() const noexcept;
    bool selectionSnapshotReady() const noexcept;
    bool documentSourceReady() const noexcept;
    bool refreshAttemptedForSelectedNote() const noexcept;
    void flushMount();
    void resetCurrentSelectionMountTracking();
    void setPendingMountNoteId(const QString& noteId);

    bool m_visible = true;
    QString m_selectedNoteId;
    QString m_selectedNoteBodyNoteId;
    QString m_selectedNoteBodyText;
    bool m_selectedNoteBodyResolved = false;
    bool m_selectedNoteBodyLoading = false;
    QString m_editorBoundNoteId;
    bool m_editorSessionBoundToSelectedNote = false;
    bool m_pendingBodySave = false;
    QString m_editorText;
    bool m_inlineDocumentSurfaceRequested = false;
    bool m_inlineDocumentSurfaceReady = false;
    bool m_inlineDocumentSurfaceLoading = false;
    bool m_structuredDocumentSurfaceRequested = false;
    bool m_structuredDocumentSurfaceReady = false;

    QString m_pendingMountNoteId;
    QString m_snapshotRefreshAttemptedNoteId;
    QString m_focusEditorOnMountNoteId;

    bool m_mountQueued = false;
    bool m_mountResetSnapshotPending = false;
    bool m_mountScheduleReconcilePending = false;
    bool m_mountFocusEditorPending = false;
    bool m_mountFallbackRefreshPending = false;
    bool m_mountForceVisualRefreshPending = false;
};
