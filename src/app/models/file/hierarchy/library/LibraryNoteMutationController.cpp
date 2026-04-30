#include "app/models/file/hierarchy/library/LibraryNoteMutationController.hpp"

#include "app/policy/ArchitecturePolicyLock.hpp"

#include <QStringList>
#include <QtGlobal>

namespace
{
    QStringList normalizedUniqueNoteIds(const QVariantList& noteIds)
    {
        QStringList normalized;
        normalized.reserve(noteIds.size());
        for (const QVariant& noteIdValue : noteIds)
        {
            const QString normalizedNoteId = noteIdValue.toString().trimmed();
            if (normalizedNoteId.isEmpty() || normalized.contains(normalizedNoteId))
            {
                continue;
            }

            normalized.push_back(normalizedNoteId);
        }

        return normalized;
    }
}

LibraryNoteMutationController::LibraryNoteMutationController(QObject* parent)
    : QObject(parent)
{
}

LibraryNoteMutationController::~LibraryNoteMutationController()
{
    disconnectSourceController();
}

void LibraryNoteMutationController::setSourceController(QObject* controller)
{
    if (controller != nullptr
        && !WhatSon::Policy::verifyMutableDependencyAllowed(
            WhatSon::Policy::Layer::View,
            WhatSon::Policy::Layer::Controller,
            QStringLiteral("LibraryNoteMutationController::setSourceController")))
    {
        return;
    }

    if (controller == nullptr
        && !WhatSon::Policy::verifyMutableWiringAllowed(
            QStringLiteral("LibraryNoteMutationController::setSourceController")))
    {
        return;
    }

    if (m_sourceController == controller)
    {
        return;
    }

    disconnectSourceController();
    m_sourceController = controller;
    m_sourceCapability = qobject_cast<ILibraryNoteMutationCapability*>(controller);

    if (m_sourceController != nullptr && m_sourceCapability != nullptr)
    {
        m_sourceDestroyedConnection = connect(
            m_sourceController,
            &QObject::destroyed,
            this,
            &LibraryNoteMutationController::handleSourceDestroyed);
        m_noteDeletedConnection = connect(
            m_sourceController,
            SIGNAL(noteDeleted(QString)),
            this,
            SIGNAL(noteDeleted(QString)));
        m_emptyNoteCreatedConnection = connect(
            m_sourceController,
            SIGNAL(emptyNoteCreated(QString)),
            this,
            SIGNAL(emptyNoteCreated(QString)));
        m_hubFilesystemMutatedConnection = connect(
            m_sourceController,
            SIGNAL(hubFilesystemMutated()),
            this,
            SIGNAL(hubFilesystemMutated()));
    }
    else if (m_sourceController != nullptr)
    {
        qWarning("LibraryNoteMutationController sourceController does not implement ILibraryNoteMutationCapability.");
    }

    emit sourceControllerChanged();
}

QObject* LibraryNoteMutationController::sourceController() const noexcept
{
    return m_sourceController;
}

bool LibraryNoteMutationController::createEmptyNote()
{
    return m_sourceCapability != nullptr && m_sourceCapability->createEmptyNote();
}

bool LibraryNoteMutationController::clearNoteFoldersById(const QString& noteId)
{
    return m_sourceCapability != nullptr && m_sourceCapability->clearNoteFoldersById(noteId);
}

bool LibraryNoteMutationController::clearNoteFoldersByIds(const QVariantList& noteIds)
{
    const QStringList normalizedNoteIds = normalizedUniqueNoteIds(noteIds);
    bool clearedAny = false;
    for (const QString& noteId : normalizedNoteIds)
    {
        if (clearNoteFoldersById(noteId))
        {
            clearedAny = true;
        }
    }

    return clearedAny;
}

bool LibraryNoteMutationController::deleteNoteById(const QString& noteId)
{
    return m_sourceCapability != nullptr && m_sourceCapability->deleteNoteById(noteId);
}

bool LibraryNoteMutationController::deleteNotesByIds(const QVariantList& noteIds)
{
    const QStringList normalizedNoteIds = normalizedUniqueNoteIds(noteIds);
    bool deletedAny = false;
    for (const QString& noteId : normalizedNoteIds)
    {
        if (deleteNoteById(noteId))
        {
            deletedAny = true;
        }
    }

    return deletedAny;
}

void LibraryNoteMutationController::handleSourceDestroyed()
{
    disconnectSourceController();
    emit sourceControllerChanged();
}

void LibraryNoteMutationController::disconnectSourceController()
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
    m_sourceController = nullptr;
}
