#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

class ContentsDisplayDocumentSourceResolver : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString selectedNoteId READ selectedNoteId WRITE setSelectedNoteId NOTIFY selectedNoteIdChanged)
    Q_PROPERTY(QString selectedNoteBodyNoteId READ selectedNoteBodyNoteId WRITE setSelectedNoteBodyNoteId NOTIFY selectedNoteBodyNoteIdChanged)
    Q_PROPERTY(QString selectedNoteBodyText READ selectedNoteBodyText WRITE setSelectedNoteBodyText NOTIFY selectedNoteBodyTextChanged)
    Q_PROPERTY(bool selectedNoteBodyResolved READ selectedNoteBodyResolved WRITE setSelectedNoteBodyResolved NOTIFY selectedNoteBodyResolvedChanged)
    Q_PROPERTY(QString editorBoundNoteId READ editorBoundNoteId WRITE setEditorBoundNoteId NOTIFY editorBoundNoteIdChanged)
    Q_PROPERTY(QString editorText READ editorText WRITE setEditorText NOTIFY editorTextChanged)
    Q_PROPERTY(bool pendingBodySave READ pendingBodySave WRITE setPendingBodySave NOTIFY pendingBodySaveChanged)

public:
    explicit ContentsDisplayDocumentSourceResolver(QObject* parent = nullptr);
    ~ContentsDisplayDocumentSourceResolver() override;

    QString selectedNoteId() const;
    void setSelectedNoteId(const QString& value);

    QString selectedNoteBodyNoteId() const;
    void setSelectedNoteBodyNoteId(const QString& value);

    QString selectedNoteBodyText() const;
    void setSelectedNoteBodyText(const QString& value);

    bool selectedNoteBodyResolved() const noexcept;
    void setSelectedNoteBodyResolved(bool value);

    QString editorBoundNoteId() const;
    void setEditorBoundNoteId(const QString& value);

    QString editorText() const;
    void setEditorText(const QString& value);

    bool pendingBodySave() const noexcept;
    void setPendingBodySave(bool value);

    Q_INVOKABLE QVariantMap resolveDocumentSourcePlan() const;

signals:
    void selectedNoteIdChanged();
    void selectedNoteBodyNoteIdChanged();
    void selectedNoteBodyTextChanged();
    void selectedNoteBodyResolvedChanged();
    void editorBoundNoteIdChanged();
    void editorTextChanged();
    void pendingBodySaveChanged();

private:
    static QString normalizeNoteId(const QString& value);

    QString m_selectedNoteId;
    QString m_selectedNoteBodyNoteId;
    QString m_selectedNoteBodyText;
    bool m_selectedNoteBodyResolved = false;
    QString m_editorBoundNoteId;
    QString m_editorText;
    bool m_pendingBodySave = false;
};
