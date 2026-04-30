#include "app/models/editor/resource/ContentsResourceImportConflictController.hpp"

#include "app/models/editor/resource/ContentsEditorDynamicObjectSupport.hpp"

using namespace WhatSon::Editor::DynamicObjectSupport;

ContentsResourceImportConflictController::ContentsResourceImportConflictController(QObject* parent)
    : QObject(parent)
{
}

ContentsResourceImportConflictController::~ContentsResourceImportConflictController() = default;

bool ContentsResourceImportConflictController::resourceImportConflictAlertOpen() const noexcept
{
    return m_resourceImportConflictAlertOpen;
}

bool ContentsResourceImportConflictController::canAcceptResourceDropUrls(const QVariant& urls) const
{
    const QVariantList normalizedUrls = normalizeSequentialVariant(urls);
    if (!m_hasSelectedNote || !m_resourcesImportViewModel)
    {
        return false;
    }
    if (m_showDedicatedResourceViewer || m_showFormattedTextRenderer || normalizedUrls.isEmpty())
    {
        return false;
    }
    return invokeBool(m_resourcesImportViewModel, "canImportUrls", { normalizedUrls });
}

void ContentsResourceImportConflictController::clearPendingResourceImportConflict()
{
    m_pendingResourceImportConflict.clear();
    m_pendingResourceImportMode = m_resourceImportModeNone;
    m_pendingResourceImportUrls.clear();
    setResourceImportConflictAlertOpen(false);
}

QVariantMap ContentsResourceImportConflictController::normalizedResourceImportConflict(const QVariant& conflict) const
{
    return conflict.toMap();
}

QString ContentsResourceImportConflictController::resourceImportConflictAlertMessage() const
{
    const QString fileName = m_pendingResourceImportConflict.value(QStringLiteral("sourceFileName")).toString().trimmed();
    const QString resourcePath = m_pendingResourceImportConflict.value(QStringLiteral("existingResourcePath")).toString().trimmed();
    if (fileName.isEmpty())
    {
        return QStringLiteral("A resource with the same name already exists. Choose how to continue.");
    }
    if (resourcePath.isEmpty())
    {
        return QStringLiteral(
                   "A resource named \"%1\" already exists. Choose whether to overwrite it, keep both copies, or cancel the import.")
            .arg(fileName);
    }
    return QStringLiteral(
               "A resource named \"%1\" already exists at \"%2\". Choose whether to overwrite it, keep both copies, or cancel the import.")
        .arg(fileName, resourcePath);
}

bool ContentsResourceImportConflictController::scheduleResourceImportConflictPrompt(
    const int importMode,
    const QVariant& urls,
    const QVariant& conflict)
{
    invokeVariant(m_editorSurfaceGuardController, "activateResourceDropEditorSurfaceGuard");
    m_pendingResourceImportMode = importMode;
    m_pendingResourceImportUrls = normalizeSequentialVariant(urls);
    m_pendingResourceImportConflict = normalizedResourceImportConflict(conflict);
    if (m_view)
    {
        m_view->setProperty("resourceDropActive", false);
    }
    setResourceImportConflictAlertOpen(true);
    return true;
}

bool ContentsResourceImportConflictController::finalizeInsertedImportedResources(const QVariant& importedEntries)
{
    if (!m_resourceTagController)
    {
        return false;
    }

    const int importedEntryCount = invokeVariant(
        m_resourceTagController,
        "normalizedImportedResourceEntries",
        { importedEntries })
                                       .toList()
                                       .size();
    const bool inserted = invokeBool(
        m_resourceTagController,
        "insertImportedResourceTags",
        { importedEntries });
    if (inserted && importedEntryCount > 0)
    {
        invokeVariant(m_resourcesImportViewModel, "reloadImportedResources");
    }
    invokeVariant(
        m_editorSurfaceGuardController,
        "releaseResourceDropEditorSurfaceGuard",
        { inserted });
    if (m_view)
    {
        m_view->setProperty("resourceDropActive", false);
    }
    clearPendingResourceImportConflict();
    return inserted;
}

void ContentsResourceImportConflictController::cancelPendingResourceImportConflict()
{
    invokeVariant(
        m_editorSurfaceGuardController,
        "releaseResourceDropEditorSurfaceGuard",
        { false });
    if (m_view)
    {
        m_view->setProperty("resourceDropActive", false);
    }
    clearPendingResourceImportConflict();
}

bool ContentsResourceImportConflictController::executePendingResourceImportWithPolicy(const int conflictPolicy)
{
    if (!m_resourcesImportViewModel)
    {
        cancelPendingResourceImportConflict();
        return false;
    }

    setResourceImportConflictAlertOpen(false);
    QVariant importedEntries;
    if (m_pendingResourceImportMode == m_resourceImportModeUrls)
    {
        importedEntries = invokeVariant(
            m_resourcesImportViewModel,
            "importUrlsForEditorWithConflictPolicy",
            { m_pendingResourceImportUrls, conflictPolicy });
    }
    else if (m_pendingResourceImportMode == m_resourceImportModeClipboard)
    {
        importedEntries = invokeVariant(
            m_resourcesImportViewModel,
            "importClipboardImageForEditorWithConflictPolicy",
            { conflictPolicy });
    }
    else
    {
        cancelPendingResourceImportConflict();
        return false;
    }

    if (!importedEntries.isValid())
    {
        cancelPendingResourceImportConflict();
        return false;
    }
    return finalizeInsertedImportedResources(importedEntries);
}

bool ContentsResourceImportConflictController::importUrlsAsResourcesWithPrompt(const QVariant& urls)
{
    if (!m_resourcesImportViewModel)
    {
        return false;
    }

    const QVariantList normalizedUrls = normalizeSequentialVariant(urls);
    const QVariantMap conflict = invokeVariant(
        m_resourcesImportViewModel,
        "inspectImportConflictForUrls",
        { normalizedUrls })
                                     .toMap();
    if (conflict.value(QStringLiteral("conflict")).toBool())
    {
        return scheduleResourceImportConflictPrompt(m_resourceImportModeUrls, normalizedUrls, conflict);
    }

    invokeVariant(m_editorSurfaceGuardController, "activateResourceDropEditorSurfaceGuard");
    const QVariant importedEntries = invokeVariant(
        m_resourcesImportViewModel,
        "importUrlsForEditorWithConflictPolicy",
        { normalizedUrls, m_resourceImportConflictPolicyAbort });
    if (!importedEntries.isValid())
    {
        return false;
    }
    return finalizeInsertedImportedResources(importedEntries);
}

bool ContentsResourceImportConflictController::pasteClipboardImageAsResource()
{
    if (!m_hasSelectedNote || m_showDedicatedResourceViewer || m_showFormattedTextRenderer)
    {
        return false;
    }
    if (!m_resourcesImportViewModel || boolProperty(m_resourcesImportViewModel, "busy"))
    {
        return false;
    }
    if (!invokeBool(m_view, "clipboardImageAvailableForPaste"))
    {
        return false;
    }

    const QVariantMap conflict = invokeVariant(
        m_resourcesImportViewModel,
        "inspectClipboardImageImportConflict")
                                     .toMap();
    if (conflict.value(QStringLiteral("conflict")).toBool())
    {
        return scheduleResourceImportConflictPrompt(m_resourceImportModeClipboard, {}, conflict);
    }

    invokeVariant(m_editorSurfaceGuardController, "activateResourceDropEditorSurfaceGuard");
    const QVariant importedEntries = invokeVariant(
        m_resourcesImportViewModel,
        "importClipboardImageForEditorWithConflictPolicy",
        { m_resourceImportConflictPolicyAbort });
    if (!importedEntries.isValid())
    {
        return false;
    }
    return finalizeInsertedImportedResources(importedEntries);
}

void ContentsResourceImportConflictController::setResourceImportConflictAlertOpen(const bool open)
{
    if (m_resourceImportConflictAlertOpen == open)
    {
        return;
    }

    m_resourceImportConflictAlertOpen = open;
    emit resourceImportConflictAlertOpenChanged();
}
