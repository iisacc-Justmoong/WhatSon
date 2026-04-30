#pragma once

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>

class DetailCurrentNoteContextBridge final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* noteListModel READ noteListModel WRITE setNoteListModel NOTIFY noteListModelChanged)
    Q_PROPERTY(QObject* noteDirectorySourceController READ noteDirectorySourceController WRITE setNoteDirectorySourceController NOTIFY noteDirectorySourceControllerChanged)
    Q_PROPERTY(QString currentNoteId READ currentNoteId NOTIFY currentNoteIdChanged)
    Q_PROPERTY(QString currentNoteDirectoryPath READ currentNoteDirectoryPath NOTIFY currentNoteDirectoryPathChanged)

public:
    explicit DetailCurrentNoteContextBridge(QObject* parent = nullptr);

    QObject* noteListModel() const noexcept;
    void setNoteListModel(QObject* noteListModel);
    QObject* noteDirectorySourceController() const noexcept;
    void setNoteDirectorySourceController(QObject* sourceController);

    QString currentNoteId() const;
    QString currentNoteDirectoryPath() const;

signals:
    void noteListModelChanged();
    void noteDirectorySourceControllerChanged();
    void currentNoteIdChanged();
    void currentNoteDirectoryPathChanged();

private slots:
    void refreshContext();

private:
    void disconnectNoteListModelSignals();

    QPointer<QObject> m_noteListModel;
    QPointer<QObject> m_noteDirectorySourceController;
    QMetaObject::Connection m_noteListModelDestroyedConnection;
    QMetaObject::Connection m_currentIndexChangedConnection;
    QMetaObject::Connection m_currentNoteEntryChangedConnection;
    QMetaObject::Connection m_currentNoteIdChangedConnection;
    QMetaObject::Connection m_currentNoteDirectoryPathChangedConnection;
    QString m_currentNoteId;
    QString m_currentNoteDirectoryPath;
};
