#include "app/models/editor/resource/ContentsResourceImportController.hpp"

#include "app/models/editor/resource/ContentsEditorSurfaceGuardController.hpp"
#include "app/models/editor/resource/ContentsInlineResourcePresentationController.hpp"
#include "app/models/editor/resource/ContentsResourceDropPayloadParser.hpp"
#include "app/models/editor/resource/ContentsResourceImportConflictController.hpp"
#include "app/models/editor/tags/ContentsResourceTagController.hpp"

ContentsResourceImportController::ContentsResourceImportController(QObject* parent)
    : QObject(parent)
    , m_dropPayloadParser(new ContentsResourceDropPayloadParser(this))
    , m_resourceTagController(new ContentsResourceTagController(this))
    , m_inlineResourcePresentationController(new ContentsInlineResourcePresentationController(this))
    , m_editorSurfaceGuardController(new ContentsEditorSurfaceGuardController(this))
    , m_resourceImportConflictController(new ContentsResourceImportConflictController(this))
{
    connect(
        m_editorSurfaceGuardController,
        &ContentsEditorSurfaceGuardController::resourceDropEditorSurfaceGuardActiveChanged,
        this,
        &ContentsResourceImportController::resourceDropEditorSurfaceGuardActiveChanged);
    connect(
        m_editorSurfaceGuardController,
        &ContentsEditorSurfaceGuardController::programmaticEditorSurfaceSyncActiveChanged,
        this,
        &ContentsResourceImportController::programmaticEditorSurfaceSyncActiveChanged);
    connect(
        m_resourceImportConflictController,
        &ContentsResourceImportConflictController::resourceImportConflictAlertOpenChanged,
        this,
        &ContentsResourceImportController::resourceImportConflictAlertOpenChanged);
}

ContentsResourceImportController::~ContentsResourceImportController() = default;

bool ContentsResourceImportController::resourceDropEditorSurfaceGuardActive() const noexcept
{
    return m_editorSurfaceGuardController->resourceDropEditorSurfaceGuardActive();
}

bool ContentsResourceImportController::resourceImportConflictAlertOpen() const noexcept
{
    return m_resourceImportConflictController->resourceImportConflictAlertOpen();
}

bool ContentsResourceImportController::programmaticEditorSurfaceSyncActive() const noexcept
{
    return m_editorSurfaceGuardController->programmaticEditorSurfaceSyncActive();
}

bool ContentsResourceImportController::canAcceptResourceDropUrls(const QVariant& urls)
{
    syncChildren();
    return m_resourceImportConflictController->canAcceptResourceDropUrls(urls);
}

void ContentsResourceImportController::clearPendingResourceImportConflict()
{
    syncChildren();
    m_resourceImportConflictController->clearPendingResourceImportConflict();
}

QVariantMap ContentsResourceImportController::normalizedResourceImportConflict(const QVariant& conflict)
{
    syncChildren();
    return m_resourceImportConflictController->normalizedResourceImportConflict(conflict);
}

QString ContentsResourceImportController::resourceImportConflictAlertMessage()
{
    syncChildren();
    return m_resourceImportConflictController->resourceImportConflictAlertMessage();
}

bool ContentsResourceImportController::scheduleResourceImportConflictPrompt(
    const int importMode,
    const QVariant& urls,
    const QVariant& conflict)
{
    syncChildren();
    return m_resourceImportConflictController->scheduleResourceImportConflictPrompt(importMode, urls, conflict);
}

bool ContentsResourceImportController::finalizeInsertedImportedResources(const QVariant& importedEntries)
{
    syncChildren();
    return m_resourceImportConflictController->finalizeInsertedImportedResources(importedEntries);
}

void ContentsResourceImportController::cancelPendingResourceImportConflict()
{
    syncChildren();
    m_resourceImportConflictController->cancelPendingResourceImportConflict();
}

bool ContentsResourceImportController::executePendingResourceImportWithPolicy(const int conflictPolicy)
{
    syncChildren();
    return m_resourceImportConflictController->executePendingResourceImportWithPolicy(conflictPolicy);
}

bool ContentsResourceImportController::importUrlsAsResourcesWithPrompt(const QVariant& urls)
{
    syncChildren();
    return m_resourceImportConflictController->importUrlsAsResourcesWithPrompt(urls);
}

QVariantList ContentsResourceImportController::extractResourceDropUrls(const QVariant& drop)
{
    return m_dropPayloadParser->extractResourceDropUrls(drop);
}

bool ContentsResourceImportController::sourceContainsCanonicalResourceTag(const QString& sourceText)
{
    syncChildren();
    return m_resourceTagController->sourceContainsCanonicalResourceTag(sourceText);
}

int ContentsResourceImportController::canonicalResourceTagCount(const QString& sourceText)
{
    syncChildren();
    return m_resourceTagController->canonicalResourceTagCount(sourceText);
}

bool ContentsResourceImportController::resourceTagLossDetected(
    const QString& previousSourceText,
    const QString& nextSourceText)
{
    syncChildren();
    return m_resourceTagController->resourceTagLossDetected(previousSourceText, nextSourceText);
}

QString ContentsResourceImportController::renderEditorSurfaceHtmlWithInlineResources(const QString& editorHtml)
{
    syncChildren();
    return m_inlineResourcePresentationController->renderEditorSurfaceHtmlWithInlineResources(editorHtml);
}

bool ContentsResourceImportController::restoreEditorSurfaceFromPresentation()
{
    syncChildren();
    return m_editorSurfaceGuardController->restoreEditorSurfaceFromPresentation();
}

bool ContentsResourceImportController::restorePendingEditorSurfaceFromPresentationIfInputSettled()
{
    syncChildren();
    return m_editorSurfaceGuardController->restorePendingEditorSurfaceFromPresentationIfInputSettled();
}

void ContentsResourceImportController::releaseResourceDropEditorSurfaceGuard(const bool restoreSurface)
{
    syncChildren();
    m_editorSurfaceGuardController->releaseResourceDropEditorSurfaceGuard(restoreSurface);
}

bool ContentsResourceImportController::insertImportedResourceTags(const QVariant& importedEntries)
{
    syncChildren();
    return m_resourceTagController->insertImportedResourceTags(importedEntries);
}

bool ContentsResourceImportController::pasteClipboardImageAsResource()
{
    syncChildren();
    return m_resourceImportConflictController->pasteClipboardImageAsResource();
}

void ContentsResourceImportController::syncChildren()
{
    m_resourceTagController->setProperty("bodyResourceRenderer", QVariant::fromValue(m_bodyResourceRenderer));
    m_resourceTagController->setProperty("documentPresentationSourceText", m_documentPresentationSourceText);
    m_resourceTagController->setProperty("editorSession", QVariant::fromValue(m_editorSession));
    m_resourceTagController->setProperty("editorText", m_editorText);
    m_resourceTagController->setProperty("editorTypingController", QVariant::fromValue(m_editorTypingController));
    m_resourceTagController->setProperty("view", QVariant::fromValue(m_view));
    m_resourceTagController->setProperty("selectedNoteBodyNoteId", m_selectedNoteBodyNoteId);
    m_resourceTagController->setProperty("selectedNoteBodyText", m_selectedNoteBodyText);
    m_resourceTagController->setProperty("selectedNoteId", m_selectedNoteId);
    m_resourceTagController->setProperty("showStructuredDocumentFlow", m_showStructuredDocumentFlow);
    m_resourceTagController->setProperty("structuredDocumentFlow", QVariant::fromValue(m_structuredDocumentFlow));

    m_inlineResourcePresentationController->setProperty("bodyResourceRenderer", QVariant::fromValue(m_bodyResourceRenderer));
    m_inlineResourcePresentationController->setProperty("contentEditor", QVariant::fromValue(m_contentEditor));
    m_inlineResourcePresentationController->setProperty("editorHorizontalInset", m_editorHorizontalInset);
    m_inlineResourcePresentationController->setProperty("editorViewport", QVariant::fromValue(m_editorViewport));
    m_inlineResourcePresentationController->setProperty("view", QVariant::fromValue(m_view));
    m_inlineResourcePresentationController->setProperty("printPaperTextWidth", m_printPaperTextWidth);
    m_inlineResourcePresentationController->setProperty("resourceEditorPlaceholderLineCount", m_resourceEditorPlaceholderLineCount);
    m_inlineResourcePresentationController->setProperty("resourceTagController", QVariant::fromValue(static_cast<QObject*>(m_resourceTagController)));
    m_inlineResourcePresentationController->setProperty("inlineHtmlImageRenderingEnabled", m_inlineHtmlImageRenderingEnabled);
    m_inlineResourcePresentationController->setProperty("showPrintEditorLayout", m_showPrintEditorLayout);

    m_editorSurfaceGuardController->setProperty("contentEditor", QVariant::fromValue(m_contentEditor));
    m_editorSurfaceGuardController->setProperty("editorProjection", QVariant::fromValue(m_editorProjection));

    m_resourceImportConflictController->setProperty("view", QVariant::fromValue(m_view));
    m_resourceImportConflictController->setProperty("editorSurfaceGuardController", QVariant::fromValue(static_cast<QObject*>(m_editorSurfaceGuardController)));
    m_resourceImportConflictController->setProperty("hasSelectedNote", m_hasSelectedNote);
    m_resourceImportConflictController->setProperty("resourceImportConflictPolicyAbort", m_resourceImportConflictPolicyAbort);
    m_resourceImportConflictController->setProperty("resourceImportModeClipboard", m_resourceImportModeClipboard);
    m_resourceImportConflictController->setProperty("resourceImportModeNone", m_resourceImportModeNone);
    m_resourceImportConflictController->setProperty("resourceImportModeUrls", m_resourceImportModeUrls);
    m_resourceImportConflictController->setProperty("resourceTagController", QVariant::fromValue(static_cast<QObject*>(m_resourceTagController)));
    m_resourceImportConflictController->setProperty("resourcesImportViewModel", QVariant::fromValue(m_resourcesImportViewModel));
    m_resourceImportConflictController->setProperty("showDedicatedResourceViewer", m_showDedicatedResourceViewer);
    m_resourceImportConflictController->setProperty("showFormattedTextRenderer", m_showFormattedTextRenderer);
}
