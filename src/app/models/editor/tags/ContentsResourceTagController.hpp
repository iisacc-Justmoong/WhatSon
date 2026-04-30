#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantList>

class ContentsResourceTagTextGenerator;
class ContentsStructuredDocumentMutationPolicy;

class ContentsResourceTagController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* editorSession MEMBER m_editorSession)
    Q_PROPERTY(QObject* editorTypingController MEMBER m_editorTypingController)
    Q_PROPERTY(QObject* structuredDocumentFlow MEMBER m_structuredDocumentFlow)
    Q_PROPERTY(QObject* bodyResourceRenderer MEMBER m_bodyResourceRenderer)
    Q_PROPERTY(QString editorText MEMBER m_editorText)
    Q_PROPERTY(QString documentPresentationSourceText MEMBER m_documentPresentationSourceText)
    Q_PROPERTY(QString selectedNoteId MEMBER m_selectedNoteId)
    Q_PROPERTY(QString selectedNoteBodyNoteId MEMBER m_selectedNoteBodyNoteId)
    Q_PROPERTY(QString selectedNoteBodyText MEMBER m_selectedNoteBodyText)
    Q_PROPERTY(bool showStructuredDocumentFlow MEMBER m_showStructuredDocumentFlow)
    Q_PROPERTY(QObject* view MEMBER m_view)

public:
    explicit ContentsResourceTagController(QObject* parent = nullptr);
    ~ContentsResourceTagController() override;

    Q_INVOKABLE int currentEditorCursorPosition() const;
    Q_INVOKABLE QVariantList normalizedImportedResourceEntries(const QVariant& importedEntries) const;
    Q_INVOKABLE int currentResourceInsertionSourceOffset() const;
    Q_INVOKABLE bool sourceContainsCanonicalResourceTag(const QString& sourceText) const;
    Q_INVOKABLE int canonicalResourceTagCount(const QString& sourceText) const;
    Q_INVOKABLE bool resourceTagLossDetected(
        const QString& previousSourceText,
        const QString& nextSourceText) const;
    Q_INVOKABLE bool insertImportedResourceTags(const QVariant& importedEntries);

private:
    QObject* m_editorSession = nullptr;
    QObject* m_editorTypingController = nullptr;
    QObject* m_structuredDocumentFlow = nullptr;
    QObject* m_bodyResourceRenderer = nullptr;
    QString m_editorText;
    QString m_documentPresentationSourceText;
    QString m_selectedNoteId;
    QString m_selectedNoteBodyNoteId;
    QString m_selectedNoteBodyText;
    bool m_showStructuredDocumentFlow = false;
    QObject* m_view = nullptr;
    ContentsStructuredDocumentMutationPolicy* m_resourceInsertionPolicy = nullptr;
    ContentsResourceTagTextGenerator* m_resourceTagTextGenerator = nullptr;
};
