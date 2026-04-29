#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

class ContentsDisplayRefreshCoordinator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool lineGeometryRefreshEnabled READ lineGeometryRefreshEnabled WRITE setLineGeometryRefreshEnabled NOTIFY lineGeometryRefreshEnabledChanged)
    Q_PROPERTY(bool minimapRefreshEnabled READ minimapRefreshEnabled WRITE setMinimapRefreshEnabled NOTIFY minimapRefreshEnabledChanged)
    Q_PROPERTY(bool preferNativeInputHandling READ preferNativeInputHandling WRITE setPreferNativeInputHandling NOTIFY preferNativeInputHandlingChanged)
    Q_PROPERTY(bool showEditorGutter READ showEditorGutter WRITE setShowEditorGutter NOTIFY showEditorGutterChanged)
    Q_PROPERTY(bool showPrintEditorLayout READ showPrintEditorLayout WRITE setShowPrintEditorLayout NOTIFY showPrintEditorLayoutChanged)
    Q_PROPERTY(bool editorInputFocused READ editorInputFocused WRITE setEditorInputFocused NOTIFY editorInputFocusedChanged)
    Q_PROPERTY(bool structuredHostGeometryActive READ structuredHostGeometryActive WRITE setStructuredHostGeometryActive NOTIFY structuredHostGeometryActiveChanged)
    Q_PROPERTY(int liveLogicalLineCount READ liveLogicalLineCount WRITE setLiveLogicalLineCount NOTIFY liveLogicalLineCountChanged)
    Q_PROPERTY(bool minimapSnapshotRefreshQueued READ minimapSnapshotRefreshQueued NOTIFY minimapSnapshotRefreshQueuedChanged)
    Q_PROPERTY(bool cursorDrivenUiRefreshQueued READ cursorDrivenUiRefreshQueued NOTIFY cursorDrivenUiRefreshQueuedChanged)
    Q_PROPERTY(bool viewportGutterRefreshQueued READ viewportGutterRefreshQueued NOTIFY viewportGutterRefreshQueuedChanged)
    Q_PROPERTY(bool typingViewportCorrectionQueued READ typingViewportCorrectionQueued NOTIFY typingViewportCorrectionQueuedChanged)
    Q_PROPERTY(bool typingViewportForceCorrectionRequested READ typingViewportForceCorrectionRequested NOTIFY typingViewportForceCorrectionRequestedChanged)
    Q_PROPERTY(int gutterRefreshPassesRemaining READ gutterRefreshPassesRemaining NOTIFY gutterRefreshPassesRemainingChanged)
    Q_PROPERTY(QString pendingNoteEntryGutterRefreshNoteId READ pendingNoteEntryGutterRefreshNoteId WRITE setPendingNoteEntryGutterRefreshNoteId NOTIFY pendingNoteEntryGutterRefreshNoteIdChanged)

public:
    explicit ContentsDisplayRefreshCoordinator(QObject* parent = nullptr);
    ~ContentsDisplayRefreshCoordinator() override;

    bool lineGeometryRefreshEnabled() const noexcept;
    void setLineGeometryRefreshEnabled(bool enabled);

    bool minimapRefreshEnabled() const noexcept;
    void setMinimapRefreshEnabled(bool enabled);

    bool preferNativeInputHandling() const noexcept;
    void setPreferNativeInputHandling(bool enabled);

    bool showEditorGutter() const noexcept;
    void setShowEditorGutter(bool enabled);

    bool showPrintEditorLayout() const noexcept;
    void setShowPrintEditorLayout(bool enabled);

    bool editorInputFocused() const noexcept;
    void setEditorInputFocused(bool focused);

    bool structuredHostGeometryActive() const noexcept;
    void setStructuredHostGeometryActive(bool active);

    int liveLogicalLineCount() const noexcept;
    void setLiveLogicalLineCount(int lineCount);

    bool minimapSnapshotRefreshQueued() const noexcept;
    bool cursorDrivenUiRefreshQueued() const noexcept;
    bool viewportGutterRefreshQueued() const noexcept;
    bool typingViewportCorrectionQueued() const noexcept;
    bool typingViewportForceCorrectionRequested() const noexcept;
    int gutterRefreshPassesRemaining() const noexcept;

    QString pendingNoteEntryGutterRefreshNoteId() const;
    void setPendingNoteEntryGutterRefreshNoteId(const QString& noteId);

    Q_INVOKABLE bool shouldScheduleGutterRefreshForReason(const QString& reason, int activeLogicalLineCount) const;
    Q_INVOKABLE QVariantMap scheduleGutterRefresh(int passCount, const QString& reason, int activeLogicalLineCount);
    Q_INVOKABLE QVariantMap scheduleNoteEntryGutterRefresh(const QString& noteId);
    Q_INVOKABLE QVariantMap scheduleCursorDrivenUiRefresh();
    Q_INVOKABLE QVariantMap scheduleViewportGutterRefresh();
    Q_INVOKABLE QVariantMap scheduleMinimapSnapshotRefresh(bool forceFull);
    Q_INVOKABLE QVariantMap scheduleTypingViewportCorrection(bool forceAnchor);

    Q_INVOKABLE void clearCursorDrivenUiRefreshQueued();
    Q_INVOKABLE void clearViewportGutterRefreshQueued();
    Q_INVOKABLE void clearMinimapSnapshotRefreshQueued();
    Q_INVOKABLE int takeGutterRefreshPassesRemaining();
    Q_INVOKABLE bool takeTypingViewportForceCorrectionRequested();
    Q_INVOKABLE void clearTypingViewportCorrectionQueued();
    Q_INVOKABLE void clearTypingViewportCorrectionState();

signals:
    void lineGeometryRefreshEnabledChanged();
    void minimapRefreshEnabledChanged();
    void preferNativeInputHandlingChanged();
    void showEditorGutterChanged();
    void showPrintEditorLayoutChanged();
    void editorInputFocusedChanged();
    void structuredHostGeometryActiveChanged();
    void liveLogicalLineCountChanged();
    void minimapSnapshotRefreshQueuedChanged();
    void cursorDrivenUiRefreshQueuedChanged();
    void viewportGutterRefreshQueuedChanged();
    void typingViewportCorrectionQueuedChanged();
    void typingViewportForceCorrectionRequestedChanged();
    void gutterRefreshPassesRemainingChanged();
    void pendingNoteEntryGutterRefreshNoteIdChanged();

private:
    void setMinimapSnapshotRefreshQueued(bool queued);
    void setCursorDrivenUiRefreshQueued(bool queued);
    void setViewportGutterRefreshQueued(bool queued);
    void setTypingViewportCorrectionQueued(bool queued);
    void setTypingViewportForceCorrectionRequested(bool requested);
    void setGutterRefreshPassesRemaining(int passes);
    static QString normalizeNoteId(const QString& noteId);

    bool m_lineGeometryRefreshEnabled = true;
    bool m_minimapRefreshEnabled = true;
    bool m_preferNativeInputHandling = false;
    bool m_showEditorGutter = true;
    bool m_showPrintEditorLayout = false;
    bool m_editorInputFocused = false;
    bool m_structuredHostGeometryActive = false;
    int m_liveLogicalLineCount = 1;
    bool m_minimapSnapshotRefreshQueued = false;
    bool m_cursorDrivenUiRefreshQueued = false;
    bool m_viewportGutterRefreshQueued = false;
    bool m_typingViewportCorrectionQueued = false;
    bool m_typingViewportForceCorrectionRequested = false;
    int m_gutterRefreshPassesRemaining = 0;
    QString m_pendingNoteEntryGutterRefreshNoteId;
};
