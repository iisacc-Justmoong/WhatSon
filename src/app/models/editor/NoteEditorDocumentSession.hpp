#pragma once

#include "app/models/file/note/ContentsNoteManagementCoordinator.hpp"

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QtTypes>

class NoteActiveStateTracker;

class NoteEditorDocumentSession final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* noteActiveState READ noteActiveState WRITE setNoteActiveState NOTIFY noteActiveStateChanged)
    Q_PROPERTY(QString editorFilePath READ editorFilePath NOTIFY editorFilePathChanged)
    Q_PROPERTY(QString activeNoteId READ activeNoteId NOTIFY activeNoteChanged)
    Q_PROPERTY(QString activeNoteDirectoryPath READ activeNoteDirectoryPath NOTIFY activeNoteChanged)
    Q_PROPERTY(int parsedLineCount READ parsedLineCount NOTIFY parsedLineCountChanged)
    Q_PROPERTY(bool hasActiveNote READ hasActiveNote NOTIFY activeNoteChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(bool readOnly READ readOnly NOTIFY readOnlyChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit NoteEditorDocumentSession(QObject* parent = nullptr);
    ~NoteEditorDocumentSession() override;

    QObject* noteActiveState() const noexcept;
    void setNoteActiveState(QObject* noteActiveState);

    QString editorFilePath() const;
    QString activeNoteId() const;
    QString activeNoteDirectoryPath() const;
    int parsedLineCount() const noexcept;
    bool hasActiveNote() const noexcept;
    bool loading() const noexcept;
    bool readOnly() const noexcept;
    QString lastError() const;

    void setSessionRootPathForTests(const QString& sessionRootPath);

    Q_INVOKABLE bool openNoteForEditing(
        const QString& noteId,
        const QString& noteDirectoryPath);
    Q_INVOKABLE bool clearEditor();
    Q_INVOKABLE bool persistEditorFile(const QString& editorFilePath);

signals:
    void noteActiveStateChanged();
    void editorFilePathChanged();
    void activeNoteChanged();
    void parsedLineCountChanged();
    void loadingChanged();
    void readOnlyChanged();
    void lastErrorChanged();
    void editorSourceLoaded(
        const QString& noteId,
        const QString& editorFilePath);
    void editorSourcePersistRequested(
        const QString& noteId,
        const QString& editorFilePath);
    void editorSourcePersistFinished(
        const QString& noteId,
        bool success,
        const QString& errorMessage);

private slots:
    void refreshFromActiveNoteState();
    void refreshContentController();
    void handleNoteBodyTextLoaded(
        quint64 sequence,
        const QString& noteId,
        const QString& text,
        bool success,
        const QString& errorMessage);
    void handleEditorTextPersistenceFinished(
        const QString& noteId,
        const QString& text,
        bool success,
        const QString& errorMessage);
    void handleNoteActiveStateDestroyed();

private:
    struct EditorFileContext final
    {
        QString noteId;
        QString noteDirectoryPath;
    };

    QString sessionRootPath() const;
    QString blankEditorFilePath() const;
    QString editorFilePathForNote(
        const QString& noteId,
        const QString& noteDirectoryPath) const;

    bool ensureSessionRoot(QString* errorMessage = nullptr) const;
    bool writeEditorSourceFile(
        const QString& filePath,
        const QString& text,
        QString* errorMessage = nullptr) const;
    bool readEditorSourceFile(
        const QString& filePath,
        QString* outText,
        QString* errorMessage = nullptr) const;

    void setEditorFilePath(const QString& editorFilePath);
    void setActiveNoteContext(
        const QString& noteId,
        const QString& noteDirectoryPath);
    void setParsedLineCount(int parsedLineCount);
    void setLoading(bool loading);
    void setReadOnly(bool readOnly);
    void setLastError(const QString& lastError);
    void switchToBlankEditorFile();

    QPointer<NoteActiveStateTracker> m_noteActiveState;
    ContentsNoteManagementCoordinator m_noteManagementCoordinator;
    QHash<QString, EditorFileContext> m_editorFileContexts;
    QString m_sessionRootPathForTests;
    QString m_editorFilePath;
    QString m_activeNoteId;
    QString m_activeNoteDirectoryPath;
    QString m_pendingLoadNoteId;
    QString m_pendingLoadNoteDirectoryPath;
    quint64 m_pendingLoadSequence = 0;
    int m_parsedLineCount = 0;
    bool m_loading = false;
    bool m_readOnly = true;
    QString m_lastError;
};
