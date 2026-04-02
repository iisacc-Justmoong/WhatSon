#include "LibraryNoteMutationViewModel.hpp"

#include <QtGlobal>

LibraryNoteMutationViewModel::LibraryNoteMutationViewModel(QObject* parent)
    : QObject(parent)
{
}

LibraryNoteMutationViewModel::~LibraryNoteMutationViewModel()
{
    disconnectSourceViewModel();
}

void LibraryNoteMutationViewModel::setSourceViewModel(QObject* viewModel)
{
    if (m_sourceViewModel == viewModel)
    {
        return;
    }

    disconnectSourceViewModel();
    m_sourceViewModel = viewModel;
    m_sourceCapability = qobject_cast<ILibraryNoteMutationCapability*>(viewModel);

    if (m_sourceViewModel != nullptr && m_sourceCapability != nullptr)
    {
        m_sourceDestroyedConnection = connect(
            m_sourceViewModel,
            &QObject::destroyed,
            this,
            &LibraryNoteMutationViewModel::handleSourceDestroyed);
        m_noteDeletedConnection = connect(
            m_sourceViewModel,
            SIGNAL(noteDeleted(QString)),
            this,
            SIGNAL(noteDeleted(QString)));
        m_emptyNoteCreatedConnection = connect(
            m_sourceViewModel,
            SIGNAL(emptyNoteCreated(QString)),
            this,
            SIGNAL(emptyNoteCreated(QString)));
        m_hubFilesystemMutatedConnection = connect(
            m_sourceViewModel,
            SIGNAL(hubFilesystemMutated()),
            this,
            SIGNAL(hubFilesystemMutated()));
    }
    else if (m_sourceViewModel != nullptr)
    {
        qWarning("LibraryNoteMutationViewModel sourceViewModel does not implement ILibraryNoteMutationCapability.");
    }

    emit sourceViewModelChanged();
}

QObject* LibraryNoteMutationViewModel::sourceViewModel() const noexcept
{
    return m_sourceViewModel;
}

bool LibraryNoteMutationViewModel::createEmptyNote()
{
    return m_sourceCapability != nullptr && m_sourceCapability->createEmptyNote();
}

bool LibraryNoteMutationViewModel::clearNoteFoldersById(const QString& noteId)
{
    return m_sourceCapability != nullptr && m_sourceCapability->clearNoteFoldersById(noteId);
}

bool LibraryNoteMutationViewModel::deleteNoteById(const QString& noteId)
{
    return m_sourceCapability != nullptr && m_sourceCapability->deleteNoteById(noteId);
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
    m_sourceCapability = nullptr;
    m_sourceViewModel = nullptr;
}
