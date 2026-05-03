#pragma once

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariantMap>
#include <QVector>

class IActiveHierarchyContextSource;
class ContentsEditorSessionController;

class NoteActiveStateTracker final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* hierarchyContextSource READ hierarchyContextSource WRITE setHierarchyContextSource NOTIFY hierarchyContextSourceChanged)
    Q_PROPERTY(QObject* editorSession READ editorSession WRITE setEditorSession NOTIFY editorSessionChanged)
    Q_PROPERTY(int activeHierarchyIndex READ activeHierarchyIndex NOTIFY activeHierarchyIndexChanged)
    Q_PROPERTY(QObject* activeHierarchyController READ activeHierarchyController NOTIFY activeHierarchyControllerChanged)
    Q_PROPERTY(QObject* activeNoteListModel READ activeNoteListModel NOTIFY activeNoteListModelChanged)
    Q_PROPERTY(QVariantMap activeNoteEntry READ activeNoteEntry NOTIFY activeNoteEntryChanged)
    Q_PROPERTY(QString activeNoteId READ activeNoteId NOTIFY activeNoteIdChanged)
    Q_PROPERTY(QString activeNoteDirectoryPath READ activeNoteDirectoryPath NOTIFY activeNoteDirectoryPathChanged)
    Q_PROPERTY(QString activeNoteBodyText READ activeNoteBodyText NOTIFY activeNoteBodyTextChanged)
    Q_PROPERTY(bool hasActiveNote READ hasActiveNote NOTIFY hasActiveNoteChanged)

public:
    explicit NoteActiveStateTracker(QObject* parent = nullptr);
    ~NoteActiveStateTracker() override;

    QObject* hierarchyContextSource() const noexcept;
    void setHierarchyContextSource(QObject* source);

    QObject* editorSession() const noexcept;
    void setEditorSession(QObject* session);

    int activeHierarchyIndex() const noexcept;
    QObject* activeHierarchyController() const noexcept;
    QObject* activeNoteListModel() const noexcept;
    QVariantMap activeNoteEntry() const;
    QString activeNoteId() const;
    QString activeNoteDirectoryPath() const;
    QString activeNoteBodyText() const;
    bool hasActiveNote() const noexcept;

    Q_INVOKABLE QVariantMap readActiveNoteEntry() const;
    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool syncEditorSessionFromActiveNote();

signals:
    void hierarchyContextSourceChanged();
    void editorSessionChanged();
    void activeHierarchyIndexChanged();
    void activeHierarchyControllerChanged();
    void activeNoteListModelChanged();
    void activeNoteEntryChanged();
    void activeNoteIdChanged();
    void activeNoteDirectoryPathChanged();
    void activeNoteBodyTextChanged();
    void hasActiveNoteChanged();
    void activeNoteStateChanged();

private slots:
    void handleHierarchyContextDestroyed();
    void handleActiveNoteListModelDestroyed();
    void handleEditorSessionDestroyed();
    void synchronizeActiveBindings();
    void refreshActiveNoteState();

private:
    void disconnectHierarchyContextSource();
    void disconnectActiveNoteListModel();
    void disconnectEditorSession();
    void setActiveNoteListModel(QObject* model);
    void setActiveNoteState(
        QVariantMap noteEntry,
        QString noteId,
        QString noteDirectoryPath,
        QString noteBodyText);

    QPointer<IActiveHierarchyContextSource> m_hierarchyContextSource;
    QPointer<ContentsEditorSessionController> m_editorSession;
    QPointer<QObject> m_activeHierarchyController;
    QPointer<QObject> m_activeNoteListModel;
    QVector<QMetaObject::Connection> m_hierarchyConnections;
    QVector<QMetaObject::Connection> m_noteListConnections;
    QMetaObject::Connection m_editorSessionDestroyedConnection;
    QVariantMap m_activeNoteEntry;
    QString m_activeNoteId;
    QString m_activeNoteDirectoryPath;
    QString m_activeNoteBodyText;
    int m_activeHierarchyIndex = 0;
};
