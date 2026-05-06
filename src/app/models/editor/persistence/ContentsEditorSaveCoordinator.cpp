#include "app/models/editor/persistence/ContentsEditorSaveCoordinator.hpp"

#include "app/models/editor/persistence/ContentsEditorPersistenceController.hpp"
#include "app/models/editor/session/ContentsEditorSessionController.hpp"
#include "app/models/file/WhatSonDebugTrace.hpp"

#include <QDir>

ContentsEditorSaveCoordinator::ContentsEditorSaveCoordinator(QObject* parent)
    : QObject(parent)
    , m_persistenceController(new ContentsEditorPersistenceController(this))
{
    connect(
        m_persistenceController,
        &ContentsEditorPersistenceController::editorTextPersistenceFinished,
        this,
        &ContentsEditorSaveCoordinator::handleEditorTextPersistenceFinished);
    WhatSon::Debug::traceEditorSelf(this, QStringLiteral("editorSaveCoordinator"), QStringLiteral("ctor"));
}

ContentsEditorSaveCoordinator::~ContentsEditorSaveCoordinator()
{
    WhatSon::Debug::traceEditorSelf(this, QStringLiteral("editorSaveCoordinator"), QStringLiteral("dtor"));
}

QObject* ContentsEditorSaveCoordinator::contentController() const noexcept
{
    return m_contentController.data();
}

void ContentsEditorSaveCoordinator::setContentController(QObject* contentController)
{
    if (m_contentController == contentController)
    {
        return;
    }

    m_contentController = contentController;
    m_persistenceController->setContentController(contentController);
    emit contentControllerChanged();
}

QObject* ContentsEditorSaveCoordinator::editorSession() const noexcept
{
    return m_editorSession.data();
}

void ContentsEditorSaveCoordinator::setEditorSession(QObject* editorSession)
{
    auto* const nextSession = qobject_cast<ContentsEditorSessionController*>(editorSession);
    if (m_editorSession == nextSession)
    {
        return;
    }

    m_editorSession = nextSession;
    emit editorSessionChanged();
}

bool ContentsEditorSaveCoordinator::commitEditedSourceText(const QString& text)
{
    auto* const currentSession = session();
    if (currentSession == nullptr || !currentSession->commitRawEditorTextMutation(text))
    {
        return false;
    }

    return scheduleEditorPersistence();
}

bool ContentsEditorSaveCoordinator::scheduleEditorPersistence()
{
    auto* const currentSession = session();
    if (currentSession == nullptr)
    {
        return false;
    }

    const QString noteId = normalizedNoteId(currentSession->editorBoundNoteId());
    if (noteId.isEmpty())
    {
        currentSession->setPendingBodySave(false);
        return false;
    }

    return enqueueEditorPersistence(
        noteId,
        normalizedNoteDirectoryPath(currentSession->editorBoundNoteDirectoryPath()),
        currentSession->editorText(),
        false);
}

bool ContentsEditorSaveCoordinator::flushPendingEditorText()
{
    auto* const currentSession = session();
    if (currentSession == nullptr || !currentSession->pendingBodySave())
    {
        return true;
    }

    return persistEditorTextImmediately();
}

bool ContentsEditorSaveCoordinator::persistEditorTextImmediately()
{
    auto* const currentSession = session();
    if (currentSession == nullptr)
    {
        return false;
    }

    return persistEditorTextImmediatelyWithText(currentSession->editorText());
}

bool ContentsEditorSaveCoordinator::persistEditorTextImmediatelyWithText(const QString& text)
{
    auto* const currentSession = session();
    if (currentSession == nullptr)
    {
        return false;
    }

    const QString noteId = normalizedNoteId(currentSession->editorBoundNoteId());
    if (noteId.isEmpty())
    {
        currentSession->setPendingBodySave(false);
        return false;
    }

    return enqueueEditorPersistence(
        noteId,
        normalizedNoteDirectoryPath(currentSession->editorBoundNoteDirectoryPath()),
        text,
        true);
}

bool ContentsEditorSaveCoordinator::syncEditorSessionFromSelection(
    const QString& noteId,
    const QString& text,
    const QString& bodyNoteId,
    const QString& noteDirectoryPath)
{
    auto* const currentSession = session();
    if (currentSession == nullptr)
    {
        return false;
    }

    const QString nextNoteId = normalizedNoteId(noteId);
    const QString currentNoteId = normalizedNoteId(currentSession->editorBoundNoteId());
    const QString nextNoteDirectoryPath = normalizedNoteDirectoryPath(noteDirectoryPath);
    const QString currentNoteDirectoryPath = normalizedNoteDirectoryPath(currentSession->editorBoundNoteDirectoryPath());
    if ((currentNoteId != nextNoteId || currentNoteDirectoryPath != nextNoteDirectoryPath)
        && currentSession->pendingBodySave()
        && !flushPendingEditorText())
    {
        return false;
    }

    return currentSession->requestSyncEditorTextFromSelection(
        noteId,
        text,
        bodyNoteId,
        noteDirectoryPath);
}

void ContentsEditorSaveCoordinator::handleEditorTextPersistenceFinished(
    const QString& noteId,
    const QString& text,
    const bool success,
    const QString& errorMessage)
{
    Q_UNUSED(errorMessage)

    auto* const currentSession = session();
    if (currentSession == nullptr || !success)
    {
        return;
    }

    if (normalizedNoteId(currentSession->editorBoundNoteId()) == normalizedNoteId(noteId)
        && currentSession->editorText() == text)
    {
        currentSession->setPendingBodySave(false);
    }
}

QString ContentsEditorSaveCoordinator::normalizedNoteId(const QString& noteId)
{
    return noteId.trimmed();
}

QString ContentsEditorSaveCoordinator::normalizedNoteDirectoryPath(const QString& noteDirectoryPath)
{
    const QString normalizedPath = QDir::cleanPath(noteDirectoryPath.trimmed());
    return normalizedPath == QStringLiteral(".") ? QString() : normalizedPath;
}

bool ContentsEditorSaveCoordinator::enqueueEditorPersistence(
    const QString& noteId,
    const QString& noteDirectoryPath,
    const QString& text,
    const bool immediateFlush)
{
    if (noteId.isEmpty())
    {
        return false;
    }

    const bool accepted = !noteDirectoryPath.isEmpty()
        ? (immediateFlush
                ? m_persistenceController->flushEditorTextForNoteAtPath(noteId, noteDirectoryPath, text)
                : m_persistenceController->stageEditorTextForIdleSyncAtPath(noteId, noteDirectoryPath, text))
        : (immediateFlush
                ? m_persistenceController->flushEditorTextForNote(noteId, text)
                : m_persistenceController->stageEditorTextForIdleSync(noteId, text));

    WhatSon::Debug::traceEditorSelf(
        this,
        QStringLiteral("editorSaveCoordinator"),
        immediateFlush ? QStringLiteral("flushEditorPersistence") : QStringLiteral("stageEditorPersistence"),
        QStringLiteral("accepted=%1 noteId=%2 noteDirectoryPath=%3 %4")
            .arg(accepted)
            .arg(noteId)
            .arg(noteDirectoryPath)
            .arg(WhatSon::Debug::summarizeText(text)));
    return accepted;
}

ContentsEditorSessionController* ContentsEditorSaveCoordinator::session() const noexcept
{
    return m_editorSession.data();
}
