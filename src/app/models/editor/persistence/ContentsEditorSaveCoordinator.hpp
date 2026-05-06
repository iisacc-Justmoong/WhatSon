#pragma once

#include <QObject>
#include <QPointer>

class ContentsEditorPersistenceController;
class ContentsEditorSessionController;

class ContentsEditorSaveCoordinator final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* contentController READ contentController WRITE setContentController NOTIFY contentControllerChanged)
    Q_PROPERTY(QObject* editorSession READ editorSession WRITE setEditorSession NOTIFY editorSessionChanged)

public:
    explicit ContentsEditorSaveCoordinator(QObject* parent = nullptr);
    ~ContentsEditorSaveCoordinator() override;

    QObject* contentController() const noexcept;
    void setContentController(QObject* contentController);

    QObject* editorSession() const noexcept;
    void setEditorSession(QObject* editorSession);

    Q_INVOKABLE bool commitEditedSourceText(const QString& text);
    Q_INVOKABLE bool scheduleEditorPersistence();
    Q_INVOKABLE bool flushPendingEditorText();
    Q_INVOKABLE bool persistEditorTextImmediately();
    Q_INVOKABLE bool persistEditorTextImmediatelyWithText(const QString& text);
    Q_INVOKABLE bool syncEditorSessionFromSelection(
        const QString& noteId,
        const QString& text,
        const QString& bodyNoteId,
        const QString& noteDirectoryPath = QString());

signals:
    void contentControllerChanged();
    void editorSessionChanged();

private slots:
    void handleEditorTextPersistenceFinished(
        const QString& noteId,
        const QString& text,
        bool success,
        const QString& errorMessage);

private:
    static QString normalizedNoteId(const QString& noteId);
    static QString normalizedNoteDirectoryPath(const QString& noteDirectoryPath);

    bool enqueueEditorPersistence(const QString& noteId, const QString& noteDirectoryPath, const QString& text, bool immediateFlush);
    ContentsEditorSessionController* session() const noexcept;

    QPointer<QObject> m_contentController;
    QPointer<ContentsEditorSessionController> m_editorSession;
    ContentsEditorPersistenceController* m_persistenceController = nullptr;
};
