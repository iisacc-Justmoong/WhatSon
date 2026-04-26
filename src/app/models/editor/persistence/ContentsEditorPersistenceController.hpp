#pragma once

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QtTypes>

class ContentsNoteManagementCoordinator;

class ContentsEditorPersistenceController final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(
        bool
            contentPersistenceContractAvailable READ contentPersistenceContractAvailable
                NOTIFY contentPersistenceContractAvailableChanged)
    Q_PROPERTY(
        bool
            directPersistenceContractAvailable READ directPersistenceAvailable
                NOTIFY contentPersistenceContractAvailableChanged)

public:
    explicit ContentsEditorPersistenceController(QObject* parent = nullptr);
    ~ContentsEditorPersistenceController() override;

    QObject* contentViewModel() const noexcept;
    void setContentViewModel(QObject* model);

    bool contentPersistenceContractAvailable() const noexcept;
    bool directPersistenceAvailable() const noexcept;

    Q_INVOKABLE bool persistEditorTextForNote(const QString& noteId, const QString& text);
    Q_INVOKABLE bool stageEditorTextForPersistence(const QString& noteId, const QString& text);
    bool stageEditorTextForPersistenceAtPath(
        const QString& noteId,
        const QString& noteDirectoryPath,
        const QString& text);
    Q_INVOKABLE bool stageEditorTextForIdleSync(const QString& noteId, const QString& text);
    bool stageEditorTextForIdleSyncAtPath(
        const QString& noteId,
        const QString& noteDirectoryPath,
        const QString& text);
    Q_INVOKABLE bool flushEditorTextForNote(const QString& noteId, const QString& text);
    bool flushEditorTextForNoteAtPath(
        const QString& noteId,
        const QString& noteDirectoryPath,
        const QString& text);
    QString noteDirectoryPathForNote(const QString& noteId) const;
    bool pendingEditorTextForNote(const QString& noteId, QString* text = nullptr) const;
    quint64 loadNoteBodyTextForNote(const QString& noteId);
    quint64 loadNoteBodyTextForNoteAtPath(const QString& noteId, const QString& noteDirectoryPath);
    bool reconcileViewSessionAndRefreshSnapshotForNote(
        const QString& noteId,
        const QString& viewSessionText,
        bool preferViewSessionOnMismatch = false);
    bool reconcileViewSessionAndRefreshSnapshotForNoteAtPath(
        const QString& noteId,
        const QString& noteDirectoryPath,
        const QString& viewSessionText,
        bool preferViewSessionOnMismatch = false);
    bool refreshNoteSnapshotForNote(const QString& noteId);
    void bindSelectedNote(const QString& noteId);
    void bindSelectedNoteAtPath(const QString& noteId, const QString& noteDirectoryPath);
    void clearSelectedNote();

signals:
    void contentPersistenceContractAvailableChanged();
    void editorTextPersistenceQueued(const QString& noteId, const QString& text);
    void editorTextPersistenceFinished(
        const QString& noteId,
        const QString& text,
        bool success,
        const QString& errorMessage);
    void noteBodyTextLoaded(
        quint64 sequence,
        const QString& noteId,
        const QString& text,
        bool success,
        const QString& errorMessage);
    void viewSessionSnapshotReconciled(
        const QString& noteId,
        bool refreshed,
        bool success,
        const QString& errorMessage);

private slots:
    void handleFetchTimerTimeout();
    void handleContentPersistenceContractAvailabilityChanged();
    void handlePersistenceFinished(
        const QString& noteId,
        const QString& text,
        bool success,
        const QString& errorMessage);

private:
    struct BufferedSnapshot final
    {
        QString text;
        QString noteDirectoryPath;
        bool directPersistenceReady = false;
    };

    bool stageEditorSnapshot(
        const QString& noteId,
        const QString& noteDirectoryPath,
        const QString& text,
        bool requestImmediateFetch);
    bool enqueueNextBufferedPersistenceIfNeeded();
    void ensureFetchTimerRunning();
    void markNoteDirty(const QString& noteId);
    void removeNoteFromDirtyOrder(const QString& noteId);

    QPointer<ContentsNoteManagementCoordinator> m_noteManagementCoordinator;
    QTimer m_fetchTimer;
    QHash<QString, BufferedSnapshot> m_bufferedSnapshotsByNote;
    QHash<QString, QString> m_lastPersistedTextByNote;
    QStringList m_dirtyNoteOrder;
    QString m_inFlightNoteId;
    QString m_inFlightNoteDirectoryPath;
    QString m_inFlightText;
    bool m_persistenceInFlight = false;
};
