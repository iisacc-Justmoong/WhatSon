#include "LibraryNoteMutationViewModel.hpp"

#include "LibraryHierarchyViewModel.hpp"

LibraryNoteMutationViewModel::LibraryNoteMutationViewModel(QObject* parent)
    : QObject(parent)
{
}

LibraryNoteMutationViewModel::~LibraryNoteMutationViewModel()
{
    disconnectSourceViewModel();
}

void LibraryNoteMutationViewModel::setSourceViewModel(LibraryHierarchyViewModel* viewModel)
{
    if (m_sourceViewModel == viewModel)
    {
        return;
    }

    disconnectSourceViewModel();
    m_sourceViewModel = viewModel;

    if (m_sourceViewModel != nullptr)
    {
        m_sourceDestroyedConnection = connect(
            m_sourceViewModel,
            &QObject::destroyed,
            this,
            &LibraryNoteMutationViewModel::handleSourceDestroyed);
        m_noteDeletedConnection = connect(
            m_sourceViewModel,
            &LibraryHierarchyViewModel::noteDeleted,
            this,
            &LibraryNoteMutationViewModel::noteDeleted);
        m_emptyNoteCreatedConnection = connect(
            m_sourceViewModel,
            &LibraryHierarchyViewModel::emptyNoteCreated,
            this,
            &LibraryNoteMutationViewModel::emptyNoteCreated);
        m_hubFilesystemMutatedConnection = connect(
            m_sourceViewModel,
            &LibraryHierarchyViewModel::hubFilesystemMutated,
            this,
            &LibraryNoteMutationViewModel::hubFilesystemMutated);
    }

    emit sourceViewModelChanged();
}

LibraryHierarchyViewModel* LibraryNoteMutationViewModel::sourceViewModel() const noexcept
{
    return m_sourceViewModel;
}

bool LibraryNoteMutationViewModel::createEmptyNote()
{
    return m_sourceViewModel != nullptr && m_sourceViewModel->createEmptyNote();
}

bool LibraryNoteMutationViewModel::clearNoteFoldersById(const QString& noteId)
{
    return m_sourceViewModel != nullptr && m_sourceViewModel->clearNoteFoldersById(noteId);
}

bool LibraryNoteMutationViewModel::deleteNoteById(const QString& noteId)
{
    return m_sourceViewModel != nullptr && m_sourceViewModel->deleteNoteById(noteId);
}

void LibraryNoteMutationViewModel::handleSourceDestroyed()
{
    disconnectSourceViewModel();
    emit sourceViewModelChanged();
}

void LibraryNoteMutationViewModel::disconnectSourceViewModel()
{
    if (m_sourceDestroyedConnection)
    {
        QObject::disconnect(m_sourceDestroyedConnection);
    }
    if (m_noteDeletedConnection)
    {
        QObject::disconnect(m_noteDeletedConnection);
    }
    if (m_emptyNoteCreatedConnection)
    {
        QObject::disconnect(m_emptyNoteCreatedConnection);
    }
    if (m_hubFilesystemMutatedConnection)
    {
        QObject::disconnect(m_hubFilesystemMutatedConnection);
    }

    m_sourceDestroyedConnection = {};
    m_noteDeletedConnection = {};
    m_emptyNoteCreatedConnection = {};
    m_hubFilesystemMutatedConnection = {};
    m_sourceViewModel = nullptr;
}
