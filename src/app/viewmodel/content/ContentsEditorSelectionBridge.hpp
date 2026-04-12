#pragma once

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QtTypes>

class ContentsEditorIdleSyncController;

class ContentsEditorSelectionBridge : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* noteListModel READ noteListModel WRITE setNoteListModel NOTIFY noteListModelChanged)
    Q_PROPERTY(QObject* contentViewModel READ contentViewModel WRITE setContentViewModel NOTIFY contentViewModelChanged)
    Q_PROPERTY(
        bool
            noteSelectionContractAvailable READ noteSelectionContractAvailable NOTIFY noteSelectionContractAvailableChanged)
    Q_PROPERTY(bool noteCountContractAvailable READ noteCountContractAvailable NOTIFY noteCountContractAvailableChanged)
    Q_PROPERTY(
        bool
            contentPersistenceContractAvailable READ contentPersistenceContractAvailable
                NOTIFY contentPersistenceContractAvailableChanged)
    Q_PROPERTY(
        bool
            directPersistenceContractAvailable READ directPersistenceAvailable
                NOTIFY contentPersistenceContractAvailableChanged)
    Q_PROPERTY(QString selectedNoteId READ selectedNoteId NOTIFY selectedNoteIdChanged)
    Q_PROPERTY(QString selectedNoteBodyText READ selectedNoteBodyText NOTIFY selectedNoteBodyTextChanged)
    Q_PROPERTY(bool selectedNoteBodyLoading READ selectedNoteBodyLoading NOTIFY selectedNoteBodyLoadingChanged)
    Q_PROPERTY(int visibleNoteCount READ visibleNoteCount NOTIFY visibleNoteCountChanged)

public:
    explicit ContentsEditorSelectionBridge(QObject* parent = nullptr);
    ~ContentsEditorSelectionBridge() override;

    QObject* noteListModel() const noexcept;
    void setNoteListModel(QObject* model);

    QObject* contentViewModel() const noexcept;
    void setContentViewModel(QObject* model);

    bool noteSelectionContractAvailable() const noexcept;
    bool noteCountContractAvailable() const noexcept;
    bool contentPersistenceContractAvailable() const noexcept;
    bool directPersistenceAvailable() const noexcept;
    QString selectedNoteId() const;
    QString selectedNoteBodyText() const;
    bool selectedNoteBodyLoading() const noexcept;
    int visibleNoteCount() const noexcept;

    Q_INVOKABLE bool persistEditorTextForNote(const QString& noteId, const QString& text);
    Q_INVOKABLE bool stageEditorTextForIdleSync(const QString& noteId, const QString& text);
    Q_INVOKABLE bool flushEditorTextForNote(const QString& noteId, const QString& text);
    Q_INVOKABLE bool reconcileViewSessionAndRefreshSnapshotForNote(
        const QString& noteId,
        const QString& viewSessionText);
    Q_INVOKABLE bool refreshSelectedNoteSnapshot();

signals:
    void editorTextPersistenceQueued(const QString& noteId, const QString& text);
    void editorTextPersistenceFinished(
        const QString& noteId,
        const QString& text,
        bool success,
        const QString& errorMessage);
    void viewSessionSnapshotReconciled(
        const QString& noteId,
        bool refreshed,
        bool success,
        const QString& errorMessage);

    void noteListModelChanged();
    void contentViewModelChanged();
    void noteSelectionContractAvailableChanged();
    void noteCountContractAvailableChanged();
    void contentPersistenceContractAvailableChanged();
    void selectedNoteIdChanged();
    void selectedNoteBodyTextChanged();
    void selectedNoteBodyLoadingChanged();
    void visibleNoteCountChanged();

private slots:
    void handleNoteListSelectionChanged();
    void handleNoteBodyTextLoaded(
        quint64 sequence,
        const QString& noteId,
        const QString& text,
        bool success,
        const QString& errorMessage);
    void handleViewSessionSnapshotReconciledInternal(
        const QString& noteId,
        bool refreshed,
        bool success,
        const QString& errorMessage);
    void flushPendingNoteSelectionRefresh();
    void handleNoteListCountChanged();
    void handleNoteListDestroyed();
    void handleContentViewModelDestroyed();

private:
    static bool hasReadableProperty(const QObject* object, const char* propertyName);
    static QString readStringProperty(const QObject* object, const char* propertyName);
    static int readIntProperty(const QObject* object, const char* propertyName);
    void setSelectedNoteBodyState(QString bodyText, bool loading);
    void startSelectedNoteBodyLoad(const QString& noteId, bool clearCachedBody);
    void scheduleNoteSelectionRefresh();
    void refreshNoteSelectionState();
    void refreshNoteCountState();
    void disconnectNoteListModel();
    void disconnectContentViewModel();

    QPointer<QObject> m_noteListModel;
    QPointer<QObject> m_contentViewModel;
    QPointer<ContentsEditorIdleSyncController> m_idleSyncController;
    bool m_noteSelectionContractAvailable = false;
    bool m_noteCountContractAvailable = false;
    bool m_noteSelectionRefreshQueued = false;
    QString m_selectedNoteId;
    QString m_selectedNoteBodyText;
    QString m_selectedNoteBodySnapshotNoteId;
    quint64 m_selectedNoteBodyRequestSequence = 0;
    bool m_selectedNoteBodyLoading = false;
    int m_visibleNoteCount = 0;
    QMetaObject::Connection m_noteListDestroyedConnection;
    QMetaObject::Connection m_currentNoteIdChangedConnection;
    QMetaObject::Connection m_itemCountChangedConnection;
    QMetaObject::Connection m_contentViewModelDestroyedConnection;
};
