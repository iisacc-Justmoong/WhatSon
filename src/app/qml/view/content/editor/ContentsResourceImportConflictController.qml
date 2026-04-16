pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: controller

    property var view: null
    property var resourceTagController: null
    property var editorSurfaceGuardController: null
    property var pendingResourceImportConflict: ({})
    property int pendingResourceImportMode: 0
    property var pendingResourceImportUrls: []
    property bool resourceImportConflictAlertOpen: false

    function canAcceptResourceDropUrls(urls) {
        if (!controller.view
                || !controller.view.hasSelectedNote
                || !controller.view.resourcesImportViewModel) {
            return false;
        }
        if (controller.view.showDedicatedResourceViewer || controller.view.showFormattedTextRenderer)
            return false;
        if (!Array.isArray(urls) || urls.length === 0)
            return false;
        if (controller.view.resourcesImportViewModel.canImportUrls === undefined)
            return false;
        return !!controller.view.resourcesImportViewModel.canImportUrls(urls);
    }

    function clearPendingResourceImportConflict() {
        controller.pendingResourceImportConflict = ({});
        controller.pendingResourceImportMode = controller.view ? controller.view.resourceImportModeNone : 0;
        controller.pendingResourceImportUrls = [];
        controller.resourceImportConflictAlertOpen = false;
    }

    function normalizedResourceImportConflict(conflict) {
        return conflict && typeof conflict === "object" ? conflict : ({});
    }

    function resourceImportConflictAlertMessage() {
        const conflict = controller.normalizedResourceImportConflict(controller.pendingResourceImportConflict);
        const fileName = conflict.sourceFileName !== undefined ? String(conflict.sourceFileName).trim() : "";
        const resourcePath = conflict.existingResourcePath !== undefined ? String(conflict.existingResourcePath).trim() : "";
        if (fileName.length === 0)
            return "A resource with the same name already exists. Choose how to continue.";
        if (resourcePath.length === 0)
            return "A resource named \"" + fileName + "\" already exists. Choose whether to overwrite it, keep both copies, or cancel the import.";
        return "A resource named \"" + fileName + "\" already exists at \"" + resourcePath + "\". Choose whether to overwrite it, keep both copies, or cancel the import.";
    }

    function scheduleResourceImportConflictPrompt(importMode, urls, conflict) {
        if (!controller.view)
            return false;

        if (controller.editorSurfaceGuardController
                && controller.editorSurfaceGuardController.activateResourceDropEditorSurfaceGuard !== undefined) {
            controller.editorSurfaceGuardController.activateResourceDropEditorSurfaceGuard();
        }
        controller.pendingResourceImportMode = importMode;
        controller.pendingResourceImportUrls = Array.isArray(urls) ? urls.slice(0) : [];
        controller.pendingResourceImportConflict = controller.normalizedResourceImportConflict(conflict);
        controller.view.resourceDropActive = false;
        controller.resourceImportConflictAlertOpen = true;
        return true;
    }

    function finalizeInsertedImportedResources(importedEntries) {
        if (!controller.view || !controller.resourceTagController)
            return false;

        const importedEntryCount = controller.resourceTagController.normalizedImportedResourceEntries(importedEntries).length;
        const inserted = controller.resourceTagController.insertImportedResourceTags(importedEntries);
        if (importedEntryCount > 0
                && controller.view.resourcesImportViewModel
                && controller.view.resourcesImportViewModel.reloadImportedResources !== undefined) {
            controller.view.resourcesImportViewModel.reloadImportedResources();
        }
        if (controller.editorSurfaceGuardController
                && controller.editorSurfaceGuardController.releaseResourceDropEditorSurfaceGuard !== undefined) {
            controller.editorSurfaceGuardController.releaseResourceDropEditorSurfaceGuard(inserted);
        }
        controller.view.resourceDropActive = false;
        controller.clearPendingResourceImportConflict();
        return inserted;
    }

    function cancelPendingResourceImportConflict() {
        if (controller.editorSurfaceGuardController
                && controller.editorSurfaceGuardController.releaseResourceDropEditorSurfaceGuard !== undefined) {
            controller.editorSurfaceGuardController.releaseResourceDropEditorSurfaceGuard(false);
        }
        if (controller.view)
            controller.view.resourceDropActive = false;
        controller.clearPendingResourceImportConflict();
    }

    function executePendingResourceImportWithPolicy(conflictPolicy) {
        if (!controller.view || !controller.view.resourcesImportViewModel) {
            controller.cancelPendingResourceImportConflict();
            return false;
        }

        controller.resourceImportConflictAlertOpen = false;
        let importedEntries = [];
        if (controller.pendingResourceImportMode === controller.view.resourceImportModeUrls) {
            if (controller.view.resourcesImportViewModel.importUrlsForEditorWithConflictPolicy === undefined) {
                controller.cancelPendingResourceImportConflict();
                return false;
            }
            importedEntries = controller.view.resourcesImportViewModel.importUrlsForEditorWithConflictPolicy(
                        controller.pendingResourceImportUrls,
                        conflictPolicy);
        } else if (controller.pendingResourceImportMode === controller.view.resourceImportModeClipboard) {
            if (controller.view.resourcesImportViewModel.importClipboardImageForEditorWithConflictPolicy === undefined) {
                controller.cancelPendingResourceImportConflict();
                return false;
            }
            importedEntries = controller.view.resourcesImportViewModel.importClipboardImageForEditorWithConflictPolicy(
                        conflictPolicy);
        } else {
            controller.cancelPendingResourceImportConflict();
            return false;
        }

        return controller.finalizeInsertedImportedResources(importedEntries);
    }

    function importUrlsAsResourcesWithPrompt(urls) {
        if (!controller.view
                || !controller.view.resourcesImportViewModel
                || controller.view.resourcesImportViewModel.importUrlsForEditorWithConflictPolicy === undefined) {
            return false;
        }

        const conflict = controller.view.resourcesImportViewModel.inspectImportConflictForUrls !== undefined
                ? controller.view.resourcesImportViewModel.inspectImportConflictForUrls(urls)
                : ({});
        if (conflict && conflict.conflict)
            return controller.scheduleResourceImportConflictPrompt(controller.view.resourceImportModeUrls, urls, conflict);

        if (controller.editorSurfaceGuardController
                && controller.editorSurfaceGuardController.activateResourceDropEditorSurfaceGuard !== undefined) {
            controller.editorSurfaceGuardController.activateResourceDropEditorSurfaceGuard();
        }
        const importedEntries = controller.view.resourcesImportViewModel.importUrlsForEditorWithConflictPolicy(
                    urls,
                    controller.view.resourceImportConflictPolicyAbort);
        return controller.finalizeInsertedImportedResources(importedEntries);
    }

    function pasteClipboardImageAsResource() {
        if (!controller.view
                || !controller.view.hasSelectedNote
                || controller.view.showDedicatedResourceViewer
                || controller.view.showFormattedTextRenderer) {
            return false;
        }
        if (!controller.view.resourcesImportViewModel
                || controller.view.resourcesImportViewModel.importClipboardImageForEditor === undefined) {
            return false;
        }
        if (controller.view.resourcesImportViewModel.busy !== undefined
                && controller.view.resourcesImportViewModel.busy) {
            return false;
        }
        if (controller.view.clipboardImageAvailableForPaste === undefined
                || !controller.view.clipboardImageAvailableForPaste()) {
            return false;
        }

        const conflict = controller.view.resourcesImportViewModel.inspectClipboardImageImportConflict !== undefined
                ? controller.view.resourcesImportViewModel.inspectClipboardImageImportConflict()
                : ({});
        if (conflict && conflict.conflict) {
            return controller.scheduleResourceImportConflictPrompt(
                        controller.view.resourceImportModeClipboard,
                        [],
                        conflict);
        }

        if (controller.view.resourcesImportViewModel.importClipboardImageForEditorWithConflictPolicy === undefined)
            return false;

        if (controller.editorSurfaceGuardController
                && controller.editorSurfaceGuardController.activateResourceDropEditorSurfaceGuard !== undefined) {
            controller.editorSurfaceGuardController.activateResourceDropEditorSurfaceGuard();
        }
        const importedEntries = controller.view.resourcesImportViewModel.importClipboardImageForEditorWithConflictPolicy(
                    controller.view.resourceImportConflictPolicyAbort);
        return controller.finalizeInsertedImportedResources(importedEntries);
    }
}
