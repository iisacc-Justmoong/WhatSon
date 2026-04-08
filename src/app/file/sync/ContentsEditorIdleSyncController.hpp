#pragma once

#include <QObject>
#include <QPointer>
#include <QThread>
#include <QString>
#include <QtTypes>

class ContentsEditorIdleSyncWorker;
class ContentsNoteManagementCoordinator;

class ContentsEditorIdleSyncController final : public QObject
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
    explicit ContentsEditorIdleSyncController(QObject* parent = nullptr);
    ~ContentsEditorIdleSyncController() override;

    QObject* contentViewModel() const noexcept;
    void setContentViewModel(QObject* model);

    bool contentPersistenceContractAvailable() const noexcept;
    bool directPersistenceAvailable() const noexcept;

    Q_INVOKABLE bool persistEditorTextForNote(const QString& noteId, const QString& text);
    Q_INVOKABLE bool stageEditorTextForIdleSync(const QString& noteId, const QString& text);
    Q_INVOKABLE bool flushEditorTextForNote(const QString& noteId, const QString& text);
    bool refreshNoteSnapshotForNote(const QString& noteId);
    void bindSelectedNote(const QString& noteId);
    void clearSelectedNote();

signals:
    void contentPersistenceContractAvailableChanged();
    void editorTextPersistenceQueued(const QString& noteId, const QString& text);
    void editorTextPersistenceFinished(
        const QString& noteId,
        const QString& text,
        bool success,
        const QString& errorMessage);

    void idleMonitorStageRequested(quint64 revision);
    void idleMonitorClearRequested();

private slots:
    void handleIdleRevisionReached(quint64 revision);
    void handlePersistenceFinished(
        const QString& noteId,
        const QString& text,
        bool success,
        const QString& errorMessage);

private:
    bool stageEditorSnapshot(const QString& noteId, const QString& text, bool flushImmediately);
    bool enqueueStagedPersistenceIfNeeded();

    QPointer<ContentsNoteManagementCoordinator> m_noteManagementCoordinator;
    QThread m_idleMonitorThread;
    ContentsEditorIdleSyncWorker* m_idleMonitorWorker = nullptr;
    QString m_stagedNoteId;
    QString m_stagedText;
    quint64 m_latestStagedRevision = 0;
    quint64 m_latestIdleRevision = 0;
    quint64 m_forceFlushRevision = 0;
    quint64 m_lastQueuedRevision = 0;
    quint64 m_lastCompletedRevision = 0;
    bool m_persistenceInFlight = false;
};
