#pragma once

#include <QMetaObject>
#include <QObject>
#include <QPointer>

class LibraryHierarchyViewModel;

class LibraryNoteMutationViewModel final : public QObject
{
    Q_OBJECT

public:
    explicit LibraryNoteMutationViewModel(QObject* parent = nullptr);
    ~LibraryNoteMutationViewModel() override;

    void setSourceViewModel(LibraryHierarchyViewModel* viewModel);
    LibraryHierarchyViewModel* sourceViewModel() const noexcept;

    Q_INVOKABLE bool createEmptyNote();
    Q_INVOKABLE bool clearNoteFoldersById(const QString& noteId);
    Q_INVOKABLE bool deleteNoteById(const QString& noteId);

signals:
    void sourceViewModelChanged();
    void noteDeleted(const QString& noteId);
    void emptyNoteCreated(const QString& noteId);
    void hubFilesystemMutated();

private slots:
    void handleSourceDestroyed();

private:
    void disconnectSourceViewModel();

    QPointer<LibraryHierarchyViewModel> m_sourceViewModel;
    QMetaObject::Connection m_sourceDestroyedConnection;
    QMetaObject::Connection m_noteDeletedConnection;
    QMetaObject::Connection m_emptyNoteCreatedConnection;
    QMetaObject::Connection m_hubFilesystemMutatedConnection;
};
