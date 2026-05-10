#include "app/models/editor/NoteEditorDocumentSession.hpp"

#include "app/models/panel/NoteActiveStateTracker.hpp"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>

namespace
{
    QString normalizePath(const QString& path)
    {
        const QString trimmed = path.trimmed();
        return trimmed.isEmpty() ? QString() : QDir::cleanPath(trimmed);
    }

    QString sanitizeFileStem(const QString& value)
    {
        QString sanitized;
        const QString trimmed = value.trimmed();
        sanitized.reserve(trimmed.size());
        for (const QChar ch : trimmed)
        {
            if (ch.isLetterOrNumber() || ch == QLatin1Char('_') || ch == QLatin1Char('-'))
            {
                sanitized.push_back(ch);
            }
            else
            {
                sanitized.push_back(QLatin1Char('_'));
            }
        }
        return sanitized.trimmed().isEmpty() ? QStringLiteral("note") : sanitized;
    }

    QString shortHash(const QString& value)
    {
        return QString::fromLatin1(
            QCryptographicHash::hash(value.toUtf8(), QCryptographicHash::Sha256).toHex().left(16));
    }
} // namespace

NoteEditorDocumentSession::NoteEditorDocumentSession(QObject* parent)
    : QObject(parent)
    , m_noteManagementCoordinator(this)
{
    connect(
        &m_noteManagementCoordinator,
        &ContentsNoteManagementCoordinator::noteBodyTextLoaded,
        this,
        &NoteEditorDocumentSession::handleNoteBodyTextLoaded);
    connect(
        &m_noteManagementCoordinator,
        &ContentsNoteManagementCoordinator::editorTextPersistenceFinished,
        this,
        &NoteEditorDocumentSession::handleEditorTextPersistenceFinished);
}

NoteEditorDocumentSession::~NoteEditorDocumentSession() = default;

QObject* NoteEditorDocumentSession::noteActiveState() const noexcept
{
    return m_noteActiveState;
}

void NoteEditorDocumentSession::setNoteActiveState(QObject* noteActiveState)
{
    auto* typedState = qobject_cast<NoteActiveStateTracker*>(noteActiveState);
    if (m_noteActiveState == typedState)
    {
        return;
    }

    if (m_noteActiveState != nullptr)
    {
        disconnect(m_noteActiveState, nullptr, this, nullptr);
    }

    m_noteActiveState = typedState;
    if (m_noteActiveState != nullptr)
    {
        connect(
            m_noteActiveState,
            &NoteActiveStateTracker::activeNoteStateChanged,
            this,
            &NoteEditorDocumentSession::refreshFromActiveNoteState);
        connect(
            m_noteActiveState,
            &NoteActiveStateTracker::activeHierarchyControllerChanged,
            this,
            &NoteEditorDocumentSession::refreshContentController);
        connect(
            m_noteActiveState,
            &QObject::destroyed,
            this,
            &NoteEditorDocumentSession::handleNoteActiveStateDestroyed);
    }

    emit noteActiveStateChanged();
    refreshContentController();
    refreshFromActiveNoteState();
}

QString NoteEditorDocumentSession::editorFilePath() const
{
    return m_editorFilePath;
}

QString NoteEditorDocumentSession::activeNoteId() const
{
    return m_activeNoteId;
}

QString NoteEditorDocumentSession::activeNoteDirectoryPath() const
{
    return m_activeNoteDirectoryPath;
}

bool NoteEditorDocumentSession::hasActiveNote() const noexcept
{
    return !m_activeNoteId.trimmed().isEmpty();
}

bool NoteEditorDocumentSession::loading() const noexcept
{
    return m_loading;
}

bool NoteEditorDocumentSession::readOnly() const noexcept
{
    return m_readOnly;
}

QString NoteEditorDocumentSession::lastError() const
{
    return m_lastError;
}

void NoteEditorDocumentSession::setSessionRootPathForTests(const QString& sessionRootPath)
{
    m_sessionRootPathForTests = normalizePath(sessionRootPath);
}

bool NoteEditorDocumentSession::openNoteForEditing(
    const QString& noteId,
    const QString& noteDirectoryPath)
{
    const QString normalizedNoteId = noteId.trimmed();
    const QString normalizedNoteDirectoryPath = normalizePath(noteDirectoryPath);
    if (normalizedNoteId.isEmpty() || normalizedNoteDirectoryPath.isEmpty())
    {
        return clearEditor();
    }

    if (m_pendingLoadSequence == 0
        && m_activeNoteId == normalizedNoteId
        && m_activeNoteDirectoryPath == normalizedNoteDirectoryPath
        && !m_editorFilePath.trimmed().isEmpty()
        && QFileInfo::exists(m_editorFilePath))
    {
        setLoading(false);
        setReadOnly(false);
        setLastError(QString());
        return true;
    }

    setReadOnly(true);
    setLoading(true);
    setLastError(QString());

    m_pendingLoadNoteId = normalizedNoteId;
    m_pendingLoadNoteDirectoryPath = normalizedNoteDirectoryPath;
    m_pendingLoadSequence = m_noteManagementCoordinator.loadNoteBodyTextForNote(
        normalizedNoteId,
        normalizedNoteDirectoryPath);
    if (m_pendingLoadSequence == 0)
    {
        setLoading(false);
        switchToBlankEditorFile();
        setActiveNoteContext(QString(), QString());
        setLastError(QStringLiteral("Failed to enqueue note body load."));
        return false;
    }
    return true;
}

bool NoteEditorDocumentSession::clearEditor()
{
    m_pendingLoadSequence = 0;
    m_pendingLoadNoteId.clear();
    m_pendingLoadNoteDirectoryPath.clear();
    setActiveNoteContext(QString(), QString());
    setLoading(false);
    setReadOnly(true);
    setLastError(QString());
    switchToBlankEditorFile();
    return true;
}

bool NoteEditorDocumentSession::persistEditorFile(const QString& editorFilePath)
{
    const QString normalizedEditorFilePath = normalizePath(editorFilePath);
    if (normalizedEditorFilePath.isEmpty())
    {
        return false;
    }

    const auto contextIterator = m_editorFileContexts.constFind(normalizedEditorFilePath);
    if (contextIterator == m_editorFileContexts.constEnd()
        || contextIterator->noteId.trimmed().isEmpty()
        || contextIterator->noteDirectoryPath.trimmed().isEmpty())
    {
        return false;
    }

    QString sourceText;
    QString readError;
    if (!readEditorSourceFile(normalizedEditorFilePath, &sourceText, &readError))
    {
        setLastError(readError);
        emit editorSourcePersistFinished(contextIterator->noteId, false, readError);
        return false;
    }

    emit editorSourcePersistRequested(contextIterator->noteId, normalizedEditorFilePath);
    const bool enqueued = m_noteManagementCoordinator.persistEditorTextForNoteAtPath(
        contextIterator->noteId,
        contextIterator->noteDirectoryPath,
        sourceText);
    if (!enqueued)
    {
        const QString error = QStringLiteral("Failed to enqueue note body persistence.");
        setLastError(error);
        emit editorSourcePersistFinished(contextIterator->noteId, false, error);
    }
    return enqueued;
}

void NoteEditorDocumentSession::refreshFromActiveNoteState()
{
    if (m_noteActiveState == nullptr || !m_noteActiveState->hasActiveNote())
    {
        clearEditor();
        return;
    }

    openNoteForEditing(
        m_noteActiveState->activeNoteId(),
        m_noteActiveState->activeNoteDirectoryPath());
}

void NoteEditorDocumentSession::refreshContentController()
{
    m_noteManagementCoordinator.setContentController(
        m_noteActiveState != nullptr ? m_noteActiveState->activeHierarchyController() : nullptr);
}

void NoteEditorDocumentSession::handleNoteBodyTextLoaded(
    const quint64 sequence,
    const QString& noteId,
    const QString& text,
    const bool success,
    const QString& errorMessage)
{
    if (sequence == 0 || sequence != m_pendingLoadSequence)
    {
        return;
    }

    const QString loadedNoteId = noteId.trimmed();
    const QString loadedNoteDirectoryPath = m_pendingLoadNoteDirectoryPath;
    m_pendingLoadSequence = 0;
    m_pendingLoadNoteId.clear();
    m_pendingLoadNoteDirectoryPath.clear();
    setLoading(false);

    if (!success)
    {
        switchToBlankEditorFile();
        setActiveNoteContext(QString(), QString());
        setReadOnly(true);
        setLastError(errorMessage.trimmed().isEmpty()
                         ? QStringLiteral("Failed to load note body text.")
                         : errorMessage.trimmed());
        return;
    }

    const QString sessionFilePath = editorFilePathForNote(loadedNoteId, loadedNoteDirectoryPath);
    QString writeError;
    if (!writeEditorSourceFile(sessionFilePath, text, &writeError))
    {
        switchToBlankEditorFile();
        setActiveNoteContext(QString(), QString());
        setReadOnly(true);
        setLastError(writeError);
        return;
    }

    m_editorFileContexts.insert(
        sessionFilePath,
        {loadedNoteId, loadedNoteDirectoryPath});
    setActiveNoteContext(loadedNoteId, loadedNoteDirectoryPath);
    setLastError(QString());
    setReadOnly(false);
    setEditorFilePath(sessionFilePath);
    emit editorSourceLoaded(loadedNoteId, sessionFilePath);
}

void NoteEditorDocumentSession::handleEditorTextPersistenceFinished(
    const QString& noteId,
    const QString&,
    const bool success,
    const QString& errorMessage)
{
    setLastError(success ? QString() : errorMessage);
    emit editorSourcePersistFinished(noteId, success, errorMessage);
}

void NoteEditorDocumentSession::handleNoteActiveStateDestroyed()
{
    m_noteActiveState = nullptr;
    emit noteActiveStateChanged();
    refreshContentController();
    clearEditor();
}

QString NoteEditorDocumentSession::sessionRootPath() const
{
    if (!m_sessionRootPathForTests.trimmed().isEmpty())
    {
        return m_sessionRootPathForTests;
    }

    QString basePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (basePath.trimmed().isEmpty())
    {
        basePath = QDir::tempPath() + QStringLiteral("/WhatSon");
    }
    return QDir(basePath).filePath(QStringLiteral("note-editor-sessions"));
}

QString NoteEditorDocumentSession::blankEditorFilePath() const
{
    return QDir(sessionRootPath()).filePath(QStringLiteral("_blank.wsnsource"));
}

QString NoteEditorDocumentSession::editorFilePathForNote(
    const QString& noteId,
    const QString& noteDirectoryPath) const
{
    const QString stem = sanitizeFileStem(noteId)
        + QLatin1Char('-')
        + shortHash(noteDirectoryPath);
    return QDir(sessionRootPath()).filePath(stem + QStringLiteral(".wsnsource"));
}

bool NoteEditorDocumentSession::ensureSessionRoot(QString* errorMessage) const
{
    const QString rootPath = sessionRootPath();
    if (rootPath.trimmed().isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Editor session root path is empty.");
        }
        return false;
    }

    QDir directory;
    if (!directory.mkpath(rootPath))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to create editor session directory: %1").arg(rootPath);
        }
        return false;
    }
    return true;
}

bool NoteEditorDocumentSession::writeEditorSourceFile(
    const QString& filePath,
    const QString& text,
    QString* errorMessage) const
{
    QString ensureError;
    if (!ensureSessionRoot(&ensureError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = ensureError;
        }
        return false;
    }

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to open editor source file for write: %1").arg(filePath);
        }
        return false;
    }

    const QByteArray encodedText = text.toUtf8();
    if (file.write(encodedText) != encodedText.size())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to write complete editor source file: %1").arg(filePath);
        }
        return false;
    }

    if (!file.commit())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to commit editor source file: %1").arg(filePath);
        }
        return false;
    }
    return true;
}

bool NoteEditorDocumentSession::readEditorSourceFile(
    const QString& filePath,
    QString* outText,
    QString* errorMessage) const
{
    if (outText != nullptr)
    {
        outText->clear();
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Failed to open editor source file for read: %1").arg(filePath);
        }
        return false;
    }

    if (outText != nullptr)
    {
        *outText = QString::fromUtf8(file.readAll());
    }
    return true;
}

void NoteEditorDocumentSession::setEditorFilePath(const QString& editorFilePath)
{
    const QString normalizedEditorFilePath = normalizePath(editorFilePath);
    if (m_editorFilePath == normalizedEditorFilePath)
    {
        return;
    }

    m_editorFilePath = normalizedEditorFilePath;
    emit editorFilePathChanged();
}

void NoteEditorDocumentSession::setActiveNoteContext(
    const QString& noteId,
    const QString& noteDirectoryPath)
{
    const QString normalizedNoteId = noteId.trimmed();
    const QString normalizedNoteDirectoryPath = normalizePath(noteDirectoryPath);
    if (m_activeNoteId == normalizedNoteId
        && m_activeNoteDirectoryPath == normalizedNoteDirectoryPath)
    {
        return;
    }

    m_activeNoteId = normalizedNoteId;
    m_activeNoteDirectoryPath = normalizedNoteDirectoryPath;
    emit activeNoteChanged();
}

void NoteEditorDocumentSession::setLoading(const bool loading)
{
    if (m_loading == loading)
    {
        return;
    }

    m_loading = loading;
    emit loadingChanged();
}

void NoteEditorDocumentSession::setReadOnly(const bool readOnly)
{
    if (m_readOnly == readOnly)
    {
        return;
    }

    m_readOnly = readOnly;
    emit readOnlyChanged();
}

void NoteEditorDocumentSession::setLastError(const QString& lastError)
{
    if (m_lastError == lastError)
    {
        return;
    }

    m_lastError = lastError;
    emit lastErrorChanged();
}

void NoteEditorDocumentSession::switchToBlankEditorFile()
{
    const QString blankPath = blankEditorFilePath();
    QString writeError;
    if (!QFileInfo::exists(blankPath) && !writeEditorSourceFile(blankPath, QString(), &writeError))
    {
        setEditorFilePath(QString());
        if (!writeError.trimmed().isEmpty())
        {
            setLastError(writeError);
        }
        return;
    }
    setEditorFilePath(blankPath);
}
