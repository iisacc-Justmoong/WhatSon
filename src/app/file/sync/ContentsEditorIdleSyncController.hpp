#pragma once

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QtTypes>

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

private slots:
    void handleFetchTimerTimeout();
    void handleContentPersistenceContractAvailabilityChanged();
    void handlePersistenceFinished(
        const QString& noteId,
        const QString& text,
        bool success,
        const QString& errorMessage);

private:
    bool stageEditorSnapshot(const QString& noteId, const QString& text, bool requestImmediateFetch);
    bool enqueueNextBufferedPersistenceIfNeeded();
    void ensureFetchTimerRunning();
    void markNoteDirty(const QString& noteId);
    void removeNoteFromDirtyOrder(const QString& noteId);

    QPointer<ContentsNoteManagementCoordinator> m_noteManagementCoordinator;
    QTimer m_fetchTimer;
    QHash<QString, QString> m_bufferedTextByNote;
    QHash<QString, QString> m_lastPersistedTextByNote;
    QStringList m_dirtyNoteOrder;
    QString m_inFlightNoteId;
    QString m_inFlightText;
    bool m_persistenceInFlight = false;
};
