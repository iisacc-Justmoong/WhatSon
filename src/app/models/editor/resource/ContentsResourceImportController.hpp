#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

class ContentsEditorSurfaceGuardController;
class ContentsResourceDropPayloadParser;
class ContentsResourceImportConflictController;
class ContentsResourceTagController;

class ContentsResourceImportController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* contentEditor MEMBER m_contentEditor)
    Q_PROPERTY(QObject* editorViewport MEMBER m_editorViewport)
    Q_PROPERTY(QObject* structuredDocumentFlow MEMBER m_structuredDocumentFlow)
    Q_PROPERTY(QObject* editorSession MEMBER m_editorSession)
    Q_PROPERTY(QObject* editorTypingController MEMBER m_editorTypingController)
    Q_PROPERTY(
        QObject*
            editorInputPolicyAdapter READ editorInputPolicyAdapter WRITE setEditorInputPolicyAdapter
                NOTIFY editorInputPolicyAdapterChanged)
    Q_PROPERTY(QObject* editorProjection MEMBER m_editorProjection)
    Q_PROPERTY(QObject* bodyResourceRenderer MEMBER m_bodyResourceRenderer)
    Q_PROPERTY(bool showPrintEditorLayout MEMBER m_showPrintEditorLayout)
    Q_PROPERTY(qreal printPaperTextWidth MEMBER m_printPaperTextWidth)
    Q_PROPERTY(int editorHorizontalInset MEMBER m_editorHorizontalInset)
    Q_PROPERTY(int resourceEditorPlaceholderLineCount MEMBER m_resourceEditorPlaceholderLineCount)
    Q_PROPERTY(bool inlineHtmlImageRenderingEnabled MEMBER m_inlineHtmlImageRenderingEnabled)
    Q_PROPERTY(QString editorText MEMBER m_editorText)
    Q_PROPERTY(QString documentPresentationSourceText MEMBER m_documentPresentationSourceText)
    Q_PROPERTY(QString selectedNoteId MEMBER m_selectedNoteId)
    Q_PROPERTY(QString selectedNoteBodyNoteId MEMBER m_selectedNoteBodyNoteId)
    Q_PROPERTY(QString selectedNoteBodyText MEMBER m_selectedNoteBodyText)
    Q_PROPERTY(bool showStructuredDocumentFlow MEMBER m_showStructuredDocumentFlow)
    Q_PROPERTY(bool hasSelectedNote MEMBER m_hasSelectedNote)
    Q_PROPERTY(bool showDedicatedResourceViewer MEMBER m_showDedicatedResourceViewer)
    Q_PROPERTY(bool showFormattedTextRenderer MEMBER m_showFormattedTextRenderer)
    Q_PROPERTY(QObject* resourcesImportController MEMBER m_resourcesImportController)
    Q_PROPERTY(int resourceImportModeNone MEMBER m_resourceImportModeNone)
    Q_PROPERTY(int resourceImportModeUrls MEMBER m_resourceImportModeUrls)
    Q_PROPERTY(int resourceImportModeClipboard MEMBER m_resourceImportModeClipboard)
    Q_PROPERTY(int resourceImportConflictPolicyAbort MEMBER m_resourceImportConflictPolicyAbort)
    Q_PROPERTY(QObject* view MEMBER m_view)
    Q_PROPERTY(
        bool
            resourceDropEditorSurfaceGuardActive READ resourceDropEditorSurfaceGuardActive
                NOTIFY resourceDropEditorSurfaceGuardActiveChanged)
    Q_PROPERTY(
        bool
            resourceImportConflictAlertOpen READ resourceImportConflictAlertOpen
                NOTIFY resourceImportConflictAlertOpenChanged)
    Q_PROPERTY(
        bool
            programmaticEditorSurfaceSyncActive READ programmaticEditorSurfaceSyncActive
                NOTIFY programmaticEditorSurfaceSyncActiveChanged)

public:
    explicit ContentsResourceImportController(QObject* parent = nullptr);
    ~ContentsResourceImportController() override;

    bool resourceDropEditorSurfaceGuardActive() const noexcept;
    bool resourceImportConflictAlertOpen() const noexcept;
    bool programmaticEditorSurfaceSyncActive() const noexcept;
    QObject* editorInputPolicyAdapter() const noexcept;
    void setEditorInputPolicyAdapter(QObject* adapter);

    Q_INVOKABLE bool canAcceptResourceDropUrls(const QVariant& urls);
    Q_INVOKABLE void clearPendingResourceImportConflict();
    Q_INVOKABLE QVariantMap normalizedResourceImportConflict(const QVariant& conflict);
    Q_INVOKABLE QString resourceImportConflictAlertMessage();
    Q_INVOKABLE bool scheduleResourceImportConflictPrompt(
        int importMode,
        const QVariant& urls,
        const QVariant& conflict);
    Q_INVOKABLE bool finalizeInsertedImportedResources(const QVariant& importedEntries);
    Q_INVOKABLE void cancelPendingResourceImportConflict();
    Q_INVOKABLE bool executePendingResourceImportWithPolicy(int conflictPolicy);
    Q_INVOKABLE bool importUrlsAsResourcesWithPrompt(const QVariant& urls);
    Q_INVOKABLE QVariantList extractResourceDropUrls(const QVariant& drop);
    Q_INVOKABLE bool sourceContainsCanonicalResourceTag(const QString& sourceText);
    Q_INVOKABLE int canonicalResourceTagCount(const QString& sourceText);
    Q_INVOKABLE bool resourceTagLossDetected(const QString& previousSourceText, const QString& nextSourceText);
    Q_INVOKABLE bool restoreEditorSurfaceFromPresentation();
    Q_INVOKABLE bool restorePendingEditorSurfaceFromPresentationIfInputSettled();
    Q_INVOKABLE void releaseResourceDropEditorSurfaceGuard(bool restoreSurface);
    Q_INVOKABLE bool insertImportedResourceTags(const QVariant& importedEntries);
    Q_INVOKABLE bool pasteClipboardImageAsResource();

signals:
    void resourceDropEditorSurfaceGuardActiveChanged();
    void resourceImportConflictAlertOpenChanged();
    void programmaticEditorSurfaceSyncActiveChanged();
    void editorInputPolicyAdapterChanged();

private:
    void syncChildren();

    QObject* m_contentEditor = nullptr;
    QObject* m_editorViewport = nullptr;
    QObject* m_structuredDocumentFlow = nullptr;
    QObject* m_editorSession = nullptr;
    QObject* m_editorTypingController = nullptr;
    QObject* m_editorInputPolicyAdapter = nullptr;
    QObject* m_editorProjection = nullptr;
    QObject* m_bodyResourceRenderer = nullptr;
    bool m_showPrintEditorLayout = false;
    qreal m_printPaperTextWidth = 0.0;
    int m_editorHorizontalInset = 0;
    int m_resourceEditorPlaceholderLineCount = 1;
    bool m_inlineHtmlImageRenderingEnabled = false;
    QString m_editorText;
    QString m_documentPresentationSourceText;
    QString m_selectedNoteId;
    QString m_selectedNoteBodyNoteId;
    QString m_selectedNoteBodyText;
    bool m_showStructuredDocumentFlow = false;
    bool m_hasSelectedNote = false;
    bool m_showDedicatedResourceViewer = false;
    bool m_showFormattedTextRenderer = false;
    QObject* m_resourcesImportController = nullptr;
    int m_resourceImportModeNone = 0;
    int m_resourceImportModeUrls = 1;
    int m_resourceImportModeClipboard = 2;
    int m_resourceImportConflictPolicyAbort = 0;
    QObject* m_view = nullptr;
    ContentsResourceDropPayloadParser* m_dropPayloadParser = nullptr;
    ContentsResourceTagController* m_resourceTagController = nullptr;
    ContentsEditorSurfaceGuardController* m_editorSurfaceGuardController = nullptr;
    ContentsResourceImportConflictController* m_resourceImportConflictController = nullptr;
};
