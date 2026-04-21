#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

class ContentsDisplaySessionCoordinator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool editorSessionBoundToSelectedNote READ editorSessionBoundToSelectedNote WRITE setEditorSessionBoundToSelectedNote NOTIFY editorSessionBoundToSelectedNoteChanged)
    Q_PROPERTY(QString selectedNoteId READ selectedNoteId WRITE setSelectedNoteId NOTIFY selectedNoteIdChanged)
    Q_PROPERTY(QString selectedNoteBodyNoteId READ selectedNoteBodyNoteId WRITE setSelectedNoteBodyNoteId NOTIFY selectedNoteBodyNoteIdChanged)
    Q_PROPERTY(QString selectedNoteBodyText READ selectedNoteBodyText WRITE setSelectedNoteBodyText NOTIFY selectedNoteBodyTextChanged)
    Q_PROPERTY(QString editorText READ editorText WRITE setEditorText NOTIFY editorTextChanged)
    Q_PROPERTY(QString structuredFlowSourceText READ structuredFlowSourceText WRITE setStructuredFlowSourceText NOTIFY structuredFlowSourceTextChanged)

public:
    explicit ContentsDisplaySessionCoordinator(QObject* parent = nullptr);
    ~ContentsDisplaySessionCoordinator() override;

    bool editorSessionBoundToSelectedNote() const noexcept;
    void setEditorSessionBoundToSelectedNote(bool value);

    QString selectedNoteId() const;
    void setSelectedNoteId(const QString& value);

    QString selectedNoteBodyNoteId() const;
    void setSelectedNoteBodyNoteId(const QString& value);

    QString selectedNoteBodyText() const;
    void setSelectedNoteBodyText(const QString& value);

    QString editorText() const;
    void setEditorText(const QString& value);

    QString structuredFlowSourceText() const;
    void setStructuredFlowSourceText(const QString& value);

    Q_INVOKABLE QString resolvedDocumentPresentationSourceText() const;
    Q_INVOKABLE QString currentMinimapSourceText(bool structuredHostGeometryActive) const;
    Q_INVOKABLE QVariantMap normalizedDocumentSourceMutation(const QVariant& nextSourceText) const;

signals:
    void editorSessionBoundToSelectedNoteChanged();
    void selectedNoteIdChanged();
    void selectedNoteBodyNoteIdChanged();
    void selectedNoteBodyTextChanged();
    void editorTextChanged();
    void structuredFlowSourceTextChanged();

private:
    static QString normalizeString(const QVariant& value);

    bool m_editorSessionBoundToSelectedNote = false;
    QString m_selectedNoteId;
    QString m_selectedNoteBodyNoteId;
    QString m_selectedNoteBodyText;
    QString m_editorText;
    QString m_structuredFlowSourceText;
};
