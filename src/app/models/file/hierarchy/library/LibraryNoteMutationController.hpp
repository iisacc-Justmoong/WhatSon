#pragma once

#include "app/models/file/hierarchy/IHierarchyCapabilities.hpp"

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QVariantList>

class LibraryNoteMutationController final : public QObject
{
    Q_OBJECT

public:
    explicit LibraryNoteMutationController(QObject* parent = nullptr);
    ~LibraryNoteMutationController() override;

    void setSourceController(QObject* controller);
    QObject* sourceController() const noexcept;

    Q_INVOKABLE bool createEmptyNote();
    Q_INVOKABLE bool clearNoteFoldersById(const QString& noteId);
    Q_INVOKABLE bool clearNoteFoldersByIds(const QVariantList& noteIds);
    Q_INVOKABLE bool deleteNoteById(const QString& noteId);
    Q_INVOKABLE bool deleteNotesByIds(const QVariantList& noteIds);

signals:
    void sourceControllerChanged();
    void noteDeleted(const QString& noteId);
    void emptyNoteCreated(const QString& noteId);
    void hubFilesystemMutated();

private slots:
    void handleSourceDestroyed();

private:
    void disconnectSourceController();

    QPointer<QObject> m_sourceController;
    ILibraryNoteMutationCapability* m_sourceCapability = nullptr;
    QMetaObject::Connection m_sourceDestroyedConnection;
    QMetaObject::Connection m_noteDeletedConnection;
    QMetaObject::Connection m_emptyNoteCreatedConnection;
    QMetaObject::Connection m_hubFilesystemMutatedConnection;
};
