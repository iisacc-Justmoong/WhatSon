import QtQuick

QtObject {
    id: renameController

    required property var hierarchyInteractionBridge
    required property var hierarchyRenameField
    required property var hierarchyViewModel
    required property var hostView
    required property var standardHierarchyModel

    function normalizedInteger(value, fallbackValue) {
        const numericValue = Number(value);
        if (!isFinite(numericValue))
            return fallbackValue;
        return Math.floor(numericValue);
    }

    function beginRenameSelectedHierarchyItem() {
        const renameIndex = renameController.normalizedInteger(hostView.selectedFolderIndex, -1);
        if (!renameController.canRenameIndex(renameIndex))
            return false;
        hostView.syncSelectedHierarchyItem(false);
        hostView.editingHierarchyIndex = renameIndex;
        hostView.refreshEditingHierarchyPresentation(true);
        hostView.editingHierarchyLabel = renameController.selectedHierarchyItemLabel();
        hostView.requestViewHook("hierarchy.rename.begin");
        Qt.callLater(function () {
            if (!hostView.renameEditingActive || hostView.editingHierarchyIndex !== renameIndex || !hierarchyRenameField)
                return;
            hostView.refreshEditingHierarchyPresentation(true);
            hierarchyRenameField.text = hostView.editingHierarchyLabel;
            hierarchyRenameField.forceInputFocus();
            hierarchyRenameField.selectAll();
        });
        return true;
    }

    function canRenameIndex(index) {
        const numericIndex = Number(index);
        if (!hostView.renameContractAvailable || !isFinite(numericIndex) || !hierarchyInteractionBridge)
            return false;
        return Boolean(hierarchyInteractionBridge.canRenameItem(Math.floor(numericIndex)));
    }

    function canRenameSelectedHierarchyItem() {
        return renameController.canRenameIndex(hostView.selectedFolderIndex);
    }

    function cancelHierarchyRename() {
        if (!hostView.renameEditingActive)
            return false;
        hostView.editingHierarchyIndex = -1;
        hostView.editingHierarchyLabel = "";
        hostView.clearEditingHierarchyPresentation();
        Qt.callLater(function () {
            hostView.syncSelectedHierarchyItem(true);
        });
        return true;
    }

    function cloneHierarchyItem(sourceItem) {
        const clone = {};
        if (!sourceItem)
            return clone;
        for (const key in sourceItem)
            clone[key] = sourceItem[key];
        return clone;
    }

    function commitHierarchyRename() {
        if (!hostView.renameEditingActive)
            return false;
        const renameIndex = hostView.editingHierarchyIndex;
        const nextLabel = hierarchyRenameField ? String(hierarchyRenameField.text) : hostView.editingHierarchyLabel;
        hostView.editingHierarchyLabel = nextLabel;
        if (!renameController.canRenameIndex(renameIndex))
            return renameController.cancelHierarchyRename();
        const renamed = Boolean(hierarchyInteractionBridge && hierarchyInteractionBridge.renameItem(renameIndex, nextLabel));
        if (!renamed) {
            Qt.callLater(function () {
                if (!hierarchyRenameField)
                    return;
                hierarchyRenameField.forceInputFocus();
                hierarchyRenameField.selectAll();
            });
            return false;
        }
        hostView.editingHierarchyIndex = -1;
        hostView.editingHierarchyLabel = "";
        hostView.clearEditingHierarchyPresentation();
        hostView.requestViewHook("hierarchy.rename.commit");
        Qt.callLater(function () {
            hostView.syncSelectedHierarchyItem(true);
        });
        return true;
    }

    function leafHierarchyItemLabel(rawLabel) {
        const normalizedLabel = rawLabel === undefined || rawLabel === null ? "" : String(rawLabel).trim();
        if (!normalizedLabel.length)
            return "";
        const segments = normalizedLabel.split("/");
        for (let index = segments.length - 1; index >= 0; --index) {
            const segment = String(segments[index]).trim();
            if (segment.length)
                return segment;
        }
        return normalizedLabel;
    }

    function normalizeHierarchyModel(modelValue) {
        if (modelValue === undefined || modelValue === null)
            return [];
        if (Array.isArray(modelValue))
            return modelValue.slice();
        if (modelValue.length !== undefined)
            return Array.from(modelValue);
        return [];
    }

    function projectedHierarchyModel(modelValue) {
        const normalizedModel = renameController.normalizeHierarchyModel(modelValue);
        if (!hostView.renameEditingActive)
            return normalizedModel;
        const editingIndex = renameController.normalizedInteger(hostView.editingHierarchyIndex, -1);
        if (editingIndex < 0 || editingIndex >= normalizedModel.length)
            return normalizedModel;
        const projectedModel = normalizedModel.slice();
        const projectedItem = renameController.cloneHierarchyItem(projectedModel[editingIndex]);
        projectedItem.label = " ";
        projectedItem.key = "";
        projectedItem.itemKey = "";
        projectedModel[editingIndex] = projectedItem;
        return projectedModel;
    }

    function selectedHierarchyItemLabel() {
        const selectedIndex = renameController.normalizedInteger(hostView.selectedFolderIndex, -1);
        if (selectedIndex < 0)
            return "";
        const item = standardHierarchyModel[selectedIndex];
        if (item && item.label !== undefined && item.label !== null) {
            const projectedLabel = renameController.leafHierarchyItemLabel(item.label);
            if (projectedLabel.length)
                return projectedLabel;
        }
        if (hierarchyViewModel) {
            const modelLabel = renameController.leafHierarchyItemLabel(
                        hierarchyViewModel.hierarchyItemLabelAt(selectedIndex));
            if (modelLabel.length)
                return modelLabel;
        }
        if (hostView.activeHierarchyItem && hostView.activeHierarchyItem.text !== undefined)
            return renameController.leafHierarchyItemLabel(hostView.activeHierarchyItem.text);
        return "";
    }
}
