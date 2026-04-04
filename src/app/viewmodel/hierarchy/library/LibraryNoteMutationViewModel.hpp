#pragma once

#include "viewmodel/hierarchy/IHierarchyCapabilities.hpp"

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QVariantList>

class LibraryNoteMutationViewModel final : public QObject
{
    Q_OBJECT

public:
    explicit LibraryNoteMutationViewModel(QObject* parent = nullptr);
    ~LibraryNoteMutationViewModel() override;

    void setSourceViewModel(QObject* viewModel);
    QObject* sourceViewModel() const noexcept;

    Q_INVOKABLE bool createEmptyNote();
    Q_INVOKABLE bool clearNoteFoldersById(const QString& noteId);
    Q_INVOKABLE bool clearNoteFoldersByIds(const QVariantList& noteIds);
    Q_INVOKABLE bool deleteNoteById(const QString& noteId);
    Q_INVOKABLE bool deleteNotesByIds(const QVariantList& noteIds);

signals:
    void sourceViewModelChanged();
    void noteDeleted(const QString& noteId);
    void emptyNoteCreated(const QString& noteId);
    void hubFilesystemMutated();

private slots:
    void handleSourceDestroyed();

private:
    void disconnectSourceViewModel();

    QPointer<QObject> m_sourceViewModel;
    ILibraryNoteMutationCapability* m_sourceCapability = nullptr;
    QMetaObject::Connection m_sourceDestroyedConnection;
    QMetaObject::Connection m_noteDeletedConnection;
    QMetaObject::Connection m_emptyNoteCreatedConnection;
    QMetaObject::Connection m_hubFilesystemMutatedConnection;
};
