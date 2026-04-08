#pragma once

#include "file/note/WhatSonLocalNoteDocument.hpp"

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QtTypes>
#include <QVector>

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
            contentPersistenceContractAvailable READ contentPersistenceContractAvailable NOTIFY contentPersistenceContractAvailableChanged)
    Q_PROPERTY(
        bool
            directPersistenceContractAvailable READ directPersistenceAvailable NOTIFY contentPersistenceContractAvailableChanged)



    Q_PROPERTY(QString selectedNoteId READ selectedNoteId NOTIFY selectedNoteIdChanged)
    Q_PROPERTY(QString selectedNoteBodyText READ selectedNoteBodyText NOTIFY selectedNoteBodyTextChanged)
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
    int visibleNoteCount() const noexcept;

    Q_INVOKABLE bool persistEditorTextForNote(const QString& noteId, const QString& text);
    Q_INVOKABLE bool refreshSelectedNoteSnapshot();

    signals  :

    void editorTextPersistenceFinished(
        const QString& noteId,
        const QString& text,
        bool success,
        const QString& errorMessage);

    void noteListModelChanged();
    void contentViewModelChanged();
    void noteSelectionContractAvailableChanged();
    void noteCountContractAvailableChanged();
    void contentPersistenceContractAvailableChanged();
    void selectedNoteIdChanged();
    void selectedNoteBodyTextChanged();
    void visibleNoteCountChanged();

private
    slots  :



    void handleNoteListSelectionChanged();
    void handleNoteListCountChanged();
    void handleNoteListDestroyed();
    void handleContentViewModelDestroyed();

private:
    static bool hasReadableProperty(const QObject* object, const char* propertyName);
    static bool hasInvokableMethod(const QObject* object, const char* methodSignature);
    static QString readStringProperty(const QObject* object, const char* propertyName);
    static int readIntProperty(const QObject* object, const char* propertyName);

    struct DirectPersistenceRequest final
    {
        quint64 sequence = 0;
        QString noteId;
        QString noteDirectoryPath;
        QString text;
    };

    struct DirectPersistenceResult final
    {
        quint64 sequence = 0;
        QString noteId;
        QString text;
        WhatSonLocalNoteDocument persistedDocument;
        QString errorMessage;
        bool success = false;
    };

    QString resolveNoteDirectoryPathForNote(const QString& noteId) const;
    bool ensureBoundNotePersistenceSession(const QString& noteId, QString* errorMessage = nullptr);
    bool enqueueDirectPersistenceRequest(const QString& noteId, const QString& text);
    void dispatchNextDirectPersistenceRequest();
    static DirectPersistenceResult performDirectPersistence(DirectPersistenceRequest request);
    void handleDirectPersistenceFinished(DirectPersistenceResult result);
    int findPendingDirectPersistenceRequestIndex(const QString& noteId) const;
    bool applyPersistedBodyStateToContentViewModel(
        const QString& noteId,
        const WhatSonLocalNoteDocument& document) const;
    void requestTrackedStatisticsRefresh(const QString& noteId, bool incrementOpenCount) const;
    void resetBoundNotePersistenceSession();
    void refreshNoteSelectionState();
    void refreshNoteCountState();
    void refreshContentPersistenceState();
    void disconnectNoteListModel();
    void disconnectContentViewModel();

    QPointer<QObject> m_noteListModel;
    QPointer<QObject> m_contentViewModel;
    bool m_noteSelectionContractAvailable = false;
    bool m_noteCountContractAvailable = false;
    bool m_contentPersistenceContractAvailable = false;
    bool m_directPersistenceContractAvailable = false;
    QString m_selectedNoteId;
    QString m_selectedNoteBodyText;
    int m_visibleNoteCount = 0;
    QString m_boundNoteId;
    QString m_boundNoteDirectoryPath;
    bool m_directPersistenceInFlight = false;
    quint64 m_nextDirectPersistenceSequence = 1;
    DirectPersistenceRequest m_activeDirectPersistenceRequest;
    QVector<DirectPersistenceRequest> m_pendingDirectPersistenceRequests;
    QMetaObject::Connection m_noteListDestroyedConnection;
    QMetaObject::Connection m_currentNoteIdChangedConnection;
    QMetaObject::Connection m_currentBodyTextChangedConnection;
    QMetaObject::Connection m_itemCountChangedConnection;
    QMetaObject::Connection m_contentViewModelDestroyedConnection;
};
