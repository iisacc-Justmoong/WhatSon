pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: controller

    property var resourceTagController: null
    property var editorSurfaceGuardController: null
    property bool hasSelectedNote: false
    property bool showDedicatedResourceViewer: false
    property bool showFormattedTextRenderer: false
    property var resourcesImportViewModel: null
    property int resourceImportModeNone: 0
    property int resourceImportModeUrls: 1
    property int resourceImportModeClipboard: 2
    property int resourceImportConflictPolicyAbort: 0
    property var clearResourceDropActiveHandler: null
    property var clipboardImageAvailableHandler: null
    property var pendingResourceImportConflict: ({})
    property int pendingResourceImportMode: 0
    property var pendingResourceImportUrls: []
    property bool resourceImportConflictAlertOpen: false

    function canAcceptResourceDropUrls(urls) {
        if (!controller.hasSelectedNote
                || !controller.resourcesImportViewModel) {
            return false;
        }
        if (controller.showDedicatedResourceViewer || controller.showFormattedTextRenderer)
            return false;
        if (!Array.isArray(urls) || urls.length === 0)
            return false;
        if (controller.resourcesImportViewModel.canImportUrls === undefined)
            return false;
        return !!controller.resourcesImportViewModel.canImportUrls(urls);
    }

    function clearPendingResourceImportConflict() {
        controller.pendingResourceImportConflict = ({});
        controller.pendingResourceImportMode = controller.resourceImportModeNone;
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
        if (controller.editorSurfaceGuardController
                && controller.editorSurfaceGuardController.activateResourceDropEditorSurfaceGuard !== undefined) {
            controller.editorSurfaceGuardController.activateResourceDropEditorSurfaceGuard();
        }
        controller.pendingResourceImportMode = importMode;
        controller.pendingResourceImportUrls = Array.isArray(urls) ? urls.slice(0) : [];
        controller.pendingResourceImportConflict = controller.normalizedResourceImportConflict(conflict);
        if (controller.clearResourceDropActiveHandler
                && typeof controller.clearResourceDropActiveHandler === "function")
            controller.clearResourceDropActiveHandler();
        controller.resourceImportConflictAlertOpen = true;
        return true;
    }

    function finalizeInsertedImportedResources(importedEntries) {
        if (!controller.resourceTagController)
            return false;

        const importedEntryCount = controller.resourceTagController.normalizedImportedResourceEntries(importedEntries).length;
        const inserted = controller.resourceTagController.insertImportedResourceTags(importedEntries);
        if (importedEntryCount > 0
                && controller.resourcesImportViewModel
                && controller.resourcesImportViewModel.reloadImportedResources !== undefined) {
            controller.resourcesImportViewModel.reloadImportedResources();
        }
        if (controller.editorSurfaceGuardController
                && controller.editorSurfaceGuardController.releaseResourceDropEditorSurfaceGuard !== undefined) {
            controller.editorSurfaceGuardController.releaseResourceDropEditorSurfaceGuard(inserted);
        }
        if (controller.clearResourceDropActiveHandler
                && typeof controller.clearResourceDropActiveHandler === "function")
            controller.clearResourceDropActiveHandler();
        controller.clearPendingResourceImportConflict();
        return inserted;
    }

    function cancelPendingResourceImportConflict() {
        if (controller.editorSurfaceGuardController
                && controller.editorSurfaceGuardController.releaseResourceDropEditorSurfaceGuard !== undefined) {
            controller.editorSurfaceGuardController.releaseResourceDropEditorSurfaceGuard(false);
        }
        if (controller.clearResourceDropActiveHandler
                && typeof controller.clearResourceDropActiveHandler === "function")
            controller.clearResourceDropActiveHandler();
        controller.clearPendingResourceImportConflict();
    }

    function executePendingResourceImportWithPolicy(conflictPolicy) {
        if (!controller.resourcesImportViewModel) {
            controller.cancelPendingResourceImportConflict();
            return false;
        }

        controller.resourceImportConflictAlertOpen = false;
        let importedEntries = [];
        if (controller.pendingResourceImportMode === controller.resourceImportModeUrls) {
            if (controller.resourcesImportViewModel.importUrlsForEditorWithConflictPolicy === undefined) {
                controller.cancelPendingResourceImportConflict();
                return false;
            }
            importedEntries = controller.resourcesImportViewModel.importUrlsForEditorWithConflictPolicy(
                        controller.pendingResourceImportUrls,
                        conflictPolicy);
        } else if (controller.pendingResourceImportMode === controller.resourceImportModeClipboard) {
            if (controller.resourcesImportViewModel.importClipboardImageForEditorWithConflictPolicy === undefined) {
                controller.cancelPendingResourceImportConflict();
                return false;
            }
            importedEntries = controller.resourcesImportViewModel.importClipboardImageForEditorWithConflictPolicy(
                        conflictPolicy);
        } else {
            controller.cancelPendingResourceImportConflict();
            return false;
        }

        return controller.finalizeInsertedImportedResources(importedEntries);
    }

    function importUrlsAsResourcesWithPrompt(urls) {
        if (!controller.resourcesImportViewModel
                || controller.resourcesImportViewModel.importUrlsForEditorWithConflictPolicy === undefined) {
            return false;
        }

        const conflict = controller.resourcesImportViewModel.inspectImportConflictForUrls !== undefined
                ? controller.resourcesImportViewModel.inspectImportConflictForUrls(urls)
                : ({});
        if (conflict && conflict.conflict)
            return controller.scheduleResourceImportConflictPrompt(controller.resourceImportModeUrls, urls, conflict);

        if (controller.editorSurfaceGuardController
                && controller.editorSurfaceGuardController.activateResourceDropEditorSurfaceGuard !== undefined) {
            controller.editorSurfaceGuardController.activateResourceDropEditorSurfaceGuard();
        }
        const importedEntries = controller.resourcesImportViewModel.importUrlsForEditorWithConflictPolicy(
                    urls,
                    controller.resourceImportConflictPolicyAbort);
        return controller.finalizeInsertedImportedResources(importedEntries);
    }

    function pasteClipboardImageAsResource() {
        if (!controller.hasSelectedNote
                || controller.showDedicatedResourceViewer
                || controller.showFormattedTextRenderer) {
            return false;
        }
        if (!controller.resourcesImportViewModel
                || controller.resourcesImportViewModel.importClipboardImageForEditor === undefined) {
            return false;
        }
        if (controller.resourcesImportViewModel.busy !== undefined
                && controller.resourcesImportViewModel.busy) {
            return false;
        }
        if (!controller.clipboardImageAvailableHandler
                || typeof controller.clipboardImageAvailableHandler !== "function"
                || !controller.clipboardImageAvailableHandler()) {
            return false;
        }

        const conflict = controller.resourcesImportViewModel.inspectClipboardImageImportConflict !== undefined
                ? controller.resourcesImportViewModel.inspectClipboardImageImportConflict()
                : ({});
        if (conflict && conflict.conflict) {
            return controller.scheduleResourceImportConflictPrompt(
                        controller.resourceImportModeClipboard,
                        [],
                        conflict);
        }

        if (controller.resourcesImportViewModel.importClipboardImageForEditorWithConflictPolicy === undefined)
            return false;

        if (controller.editorSurfaceGuardController
                && controller.editorSurfaceGuardController.activateResourceDropEditorSurfaceGuard !== undefined) {
            controller.editorSurfaceGuardController.activateResourceDropEditorSurfaceGuard();
        }
        const importedEntries = controller.resourcesImportViewModel.importClipboardImageForEditorWithConflictPolicy(
                    controller.resourceImportConflictPolicyAbort);
        return controller.finalizeInsertedImportedResources(importedEntries);
    }
}
