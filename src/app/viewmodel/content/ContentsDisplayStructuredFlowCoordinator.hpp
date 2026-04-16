#pragma once

#include <QObject>
#include <QString>

class ContentsDisplayStructuredFlowCoordinator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString selectedNoteId READ selectedNoteId WRITE setSelectedNoteId NOTIFY selectedNoteIdChanged)
    Q_PROPERTY(
        bool
            editorSessionBoundToSelectedNote READ editorSessionBoundToSelectedNote
                WRITE setEditorSessionBoundToSelectedNote NOTIFY editorSessionBoundToSelectedNoteChanged)
    Q_PROPERTY(
        bool
            parsedStructuredFlowRequested READ parsedStructuredFlowRequested
                WRITE setParsedStructuredFlowRequested NOTIFY parsedStructuredFlowRequestedChanged)
    Q_PROPERTY(bool renderPending READ renderPending WRITE setRenderPending NOTIFY renderPendingChanged)
    Q_PROPERTY(QString activatedNoteId READ activatedNoteId NOTIFY activatedNoteIdChanged)

public:
    explicit ContentsDisplayStructuredFlowCoordinator(QObject* parent = nullptr);
    ~ContentsDisplayStructuredFlowCoordinator() override;

    QString selectedNoteId() const;
    void setSelectedNoteId(const QString& noteId);

    bool editorSessionBoundToSelectedNote() const noexcept;
    void setEditorSessionBoundToSelectedNote(bool bound);

    bool parsedStructuredFlowRequested() const noexcept;
    void setParsedStructuredFlowRequested(bool requested);

    bool renderPending() const noexcept;
    void setRenderPending(bool pending);

    QString activatedNoteId() const;

    Q_INVOKABLE void refreshActivatedNoteId();

signals:
    void selectedNoteIdChanged();
    void editorSessionBoundToSelectedNoteChanged();
    void parsedStructuredFlowRequestedChanged();
    void renderPendingChanged();
    void activatedNoteIdChanged();

private:
    static QString normalizeNoteId(const QString& noteId);
    void setActivatedNoteId(const QString& noteId);

    QString m_selectedNoteId;
    bool m_editorSessionBoundToSelectedNote = false;
    bool m_parsedStructuredFlowRequested = false;
    bool m_renderPending = false;
    QString m_activatedNoteId;
};
