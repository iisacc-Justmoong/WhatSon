#pragma once

#include <QObject>
#include <QPointer>
#include <QString>

class ContentsAgendaBackend;
class ContentsEditorSelectionBridge;

class ContentsEditorSessionController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString editorBoundNoteId READ editorBoundNoteId WRITE setEditorBoundNoteId NOTIFY editorBoundNoteIdChanged)
    Q_PROPERTY(
        QString
            editorBoundNoteDirectoryPath READ editorBoundNoteDirectoryPath WRITE setEditorBoundNoteDirectoryPath
                NOTIFY editorBoundNoteDirectoryPathChanged)
    Q_PROPERTY(QString editorText READ editorText WRITE setEditorText NOTIFY editorTextChanged)
    Q_PROPERTY(
        bool
            localEditorAuthority READ localEditorAuthority WRITE setLocalEditorAuthority
                NOTIFY localEditorAuthorityChanged)
    Q_PROPERTY(
        double
            lastLocalEditTimestampMs READ lastLocalEditTimestampMs WRITE setLastLocalEditTimestampMs
                NOTIFY lastLocalEditTimestampMsChanged)
    Q_PROPERTY(bool pendingBodySave READ pendingBodySave WRITE setPendingBodySave NOTIFY pendingBodySaveChanged)
    Q_PROPERTY(
        int
            typingIdleThresholdMs READ typingIdleThresholdMs WRITE setTypingIdleThresholdMs
                NOTIFY typingIdleThresholdMsChanged)
    Q_PROPERTY(QObject* selectionBridge READ selectionBridge WRITE setSelectionBridge NOTIFY selectionBridgeChanged)
    Q_PROPERTY(QObject* agendaBackend READ agendaBackend WRITE setAgendaBackend NOTIFY agendaBackendChanged)
    Q_PROPERTY(
        bool
            syncingEditorTextFromModel READ syncingEditorTextFromModel WRITE setSyncingEditorTextFromModel
                NOTIFY syncingEditorTextFromModelChanged)

public:
    explicit ContentsEditorSessionController(QObject* parent = nullptr);
    ~ContentsEditorSessionController() override;

    QString editorBoundNoteId() const;
    void setEditorBoundNoteId(const QString& noteId);

    QString editorBoundNoteDirectoryPath() const;
    void setEditorBoundNoteDirectoryPath(const QString& noteDirectoryPath);

    QString editorText() const;
    void setEditorText(const QString& text);

    bool localEditorAuthority() const noexcept;
    void setLocalEditorAuthority(bool localEditorAuthority);

    double lastLocalEditTimestampMs() const noexcept;
    void setLastLocalEditTimestampMs(double timestampMs);

    bool pendingBodySave() const noexcept;
    void setPendingBodySave(bool pendingBodySave);

    int typingIdleThresholdMs() const noexcept;
    void setTypingIdleThresholdMs(int thresholdMs);

    QObject* selectionBridge() const noexcept;
    void setSelectionBridge(QObject* selectionBridge);

    QObject* agendaBackend() const noexcept;
    void setAgendaBackend(QObject* agendaBackend);

    bool syncingEditorTextFromModel() const noexcept;
    void setSyncingEditorTextFromModel(bool syncing);

    Q_INVOKABLE bool flushPendingEditorText();
    Q_INVOKABLE bool isTypingSessionActive() const;
    Q_INVOKABLE bool requestSyncEditorTextFromSelection(
        const QString& noteId,
        const QString& text,
        const QString& bodyNoteId,
        const QString& noteDirectoryPath = QString());
    Q_INVOKABLE void markLocalEditorAuthority();
    Q_INVOKABLE bool scheduleEditorPersistence();
    Q_INVOKABLE bool persistEditorTextImmediately();
    Q_INVOKABLE bool persistEditorTextImmediatelyWithText(const QString& text);

signals:
    void editorBoundNoteIdChanged();
    void editorBoundNoteDirectoryPathChanged();
    void editorTextChanged();
    void localEditorAuthorityChanged();
    void lastLocalEditTimestampMsChanged();
    void pendingBodySaveChanged();
    void typingIdleThresholdMsChanged();
    void selectionBridgeChanged();
    void agendaBackendChanged();
    void syncingEditorTextFromModelChanged();

    void editorTextSynchronized();

private slots:
    void handleEditorTextPersistenceFinished(
        const QString& noteId,
        const QString& text,
        bool success,
        const QString& errorMessage);

private:
    static QString normalizedNoteId(const QString& noteId);
    static QString normalizedNoteDirectoryPath(const QString& noteDirectoryPath);

    bool enqueueEditorPersistence(const QString& noteId, const QString& bodyText, bool immediateFlush);
    QString normalizeAgendaPlaceholderDates(const QString& text) const;
    QString normalizeStructuredEmptyBlockAnchors(const QString& text) const;
    QString normalizedEditorText(const QString& text) const;
    QString normalizeModifiedEditorText(const QString& text);
    bool shouldAcceptModelBodyText(
        const QString& noteId,
        const QString& noteDirectoryPath,
        const QString& text) const;
    bool queueCurrentEditorTextForPersistence(bool immediateFlush, const QString& rawBodyText);
    void releaseSyncGuard();
    void syncEditorTextFromSelection(
        const QString& noteId,
        const QString& noteDirectoryPath,
        const QString& text);
    double currentTimestampMs() const noexcept;

    QPointer<ContentsEditorSelectionBridge> m_selectionBridge;
    QPointer<ContentsAgendaBackend> m_agendaBackend;
    QString m_editorBoundNoteId;
    QString m_editorBoundNoteDirectoryPath;
    QString m_editorText;
    bool m_localEditorAuthority = false;
    double m_lastLocalEditTimestampMs = 0;
    bool m_pendingBodySave = false;
    int m_typingIdleThresholdMs = 1000;
    bool m_syncingEditorTextFromModel = false;
};
