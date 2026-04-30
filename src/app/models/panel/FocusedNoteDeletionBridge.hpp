#pragma once

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>

class FocusedNoteDeletionBridge : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* noteListModel READ noteListModel WRITE setNoteListModel NOTIFY noteListModelChanged)
    Q_PROPERTY(QObject* deletionTarget READ deletionTarget WRITE setDeletionTarget NOTIFY deletionTargetChanged)
    Q_PROPERTY(QString focusedNoteId READ focusedNoteId WRITE setFocusedNoteId NOTIFY focusedNoteIdChanged)
    Q_PROPERTY(bool focusedNoteAvailable READ focusedNoteAvailable NOTIFY focusedNoteAvailableChanged)
    Q_PROPERTY(bool deleteContractAvailable READ deleteContractAvailable NOTIFY deleteContractAvailableChanged)

public:
    explicit FocusedNoteDeletionBridge(QObject* parent = nullptr);
    ~FocusedNoteDeletionBridge() override;

    QObject* noteListModel() const noexcept;
    void setNoteListModel(QObject* model);

    QObject* deletionTarget() const noexcept;
    void setDeletionTarget(QObject* target);

    QString focusedNoteId() const;
    void setFocusedNoteId(const QString& noteId);

    bool focusedNoteAvailable() const noexcept;
    bool deleteContractAvailable() const noexcept;

    Q_INVOKABLE bool deleteFocusedNote();

    signals  :


    void noteListModelChanged();
    void deletionTargetChanged();
    void focusedNoteIdChanged();
    void focusedNoteAvailableChanged();
    void deleteContractAvailableChanged();

private
    slots  :


    void handleNoteListSelectionChanged();
    void handleNoteListDestroyed();
    void handleDeletionTargetDestroyed();

private:
    static bool hasReadableProperty(const QObject* object, const char* propertyName);
    static bool hasInvokableMethod(const QObject* object, const char* methodSignature);
    static QString readStringProperty(const QObject* object, const char* propertyName);

    QString resolvedFocusedNoteId() const;
    void refreshFocusedNoteState();
    void refreshDeleteContractState();
    void disconnectNoteListModel();
    void disconnectDeletionTarget();

    QPointer<QObject> m_noteListModel;
    QPointer<QObject> m_deletionTarget;
    QString m_focusedNoteId;
    bool m_focusedNoteAvailable = false;
    bool m_deleteContractAvailable = false;
    QMetaObject::Connection m_noteListDestroyedConnection;
    QMetaObject::Connection m_currentNoteIdChangedConnection;
    QMetaObject::Connection m_deletionTargetDestroyedConnection;
};
