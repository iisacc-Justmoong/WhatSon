import QtQuick

QtObject {
    id: noteDropController

    required property var hierarchyDragDropBridge
    required property var hierarchyTree
    required property var hostView

    function canAcceptNoteDropAtPosition(x, y, noteIds, referenceItem) {
        const normalizedNoteIds = noteDropController.normalizeNoteIds(noteIds);
        const target = noteDropController.noteDropTargetAtPosition(x, y, referenceItem);
        if (target.index < 0 || normalizedNoteIds.length === 0 || !hierarchyDragDropBridge)
            return false;
        if (hierarchyDragDropBridge.canAcceptNoteDropList !== undefined)
            return hierarchyDragDropBridge.canAcceptNoteDropList(target.index, normalizedNoteIds);
        return hierarchyDragDropBridge.canAcceptNoteDrop(target.index, normalizedNoteIds[0]);
    }
    function clearNoteDropPreview() {
        hostView.noteDropHoverIndex = -1;
    }
    function collectHierarchyItems() {
        const items = [];
        function visitHierarchyDescendants(item) {
            if (!item || item.children === undefined || item.children === null)
                return;
            const children = item.children;
            for (let i = 0; i < children.length; ++i) {
                const child = children[i];
                if (!child || child.visible === false)
                    continue;
                if (child.__isHierarchyItem === true)
                    items.push(child);
                visitHierarchyDescendants(child);
            }
        }
        visitHierarchyDescendants(hierarchyTree);

    }
    function commitNoteDropAtPosition(x, y, noteIds, referenceItem) {
        const normalizedNoteIds = noteDropController.normalizeNoteIds(noteIds);
        const target = noteDropController.noteDropTargetAtPosition(x, y, referenceItem);
        if (target.index < 0 || normalizedNoteIds.length === 0) {
            noteDropController.clearNoteDropPreview();
            return false;
        }
        const committed = hierarchyDragDropBridge && (hierarchyDragDropBridge.assignNotesToFolder !== undefined ? hierarchyDragDropBridge.assignNotesToFolder(target.index, normalizedNoteIds) : hierarchyDragDropBridge.assignNoteToFolder(target.index, normalizedNoteIds[0]));
        if (!committed) {
            noteDropController.clearNoteDropPreview();
            return false;
        }
        noteDropController.clearNoteDropPreview();
        hostView.requestViewHook("hierarchy.noteDrop");
        return true;
    }
    function hierarchyItemAtPosition(x, y) {
        const targetX = Number(x) || 0;
        const targetY = Number(y) || 0;
        function visitHierarchyDescendants(item) {
            if (!item || item.children === undefined || item.children === null)
                return null;
            const children = item.children;
            for (let i = children.length - 1; i >= 0; --i) {
                const child = children[i];
                if (!child || child.visible === false)
                    continue;
                const matchedDescendant = visitHierarchyDescendants(child);
                if (matchedDescendant)
                    return matchedDescendant;
                if (child.__isHierarchyItem === true && noteDropController.hierarchyItemContainsPoint(child, targetX, targetY))
                    return child;
            }
            return null;
        }
        return visitHierarchyDescendants(hierarchyTree);
    }
    function hierarchyItemContainsPoint(item, x, y) {
        if (!item || item.mapToItem === undefined)
            return false;
        const mappedPoint = item.mapToItem(hierarchyTree, 0, 0);
        const itemX = Number(mappedPoint.x) || 0;
        const itemY = Number(mappedPoint.y) || 0;
        const itemWidth = Number(item.width) || 0;
        const itemHeight = Number(item.height) || 0;
        if (itemWidth <= 0 || itemHeight <= 0)
            return false;
        if (item.rowVisible !== undefined && !Boolean(item.rowVisible))
            return false;
        return x >= itemX && x <= itemX + itemWidth && y >= itemY && y <= itemY + itemHeight;
    }
    function hierarchyItemForResolvedIndex(itemId) {
        const numericIndex = Number(itemId);
        if (!isFinite(numericIndex))
            return null;
        const resolvedIndex = Math.max(-1, Math.floor(numericIndex));
        if (resolvedIndex < 0)
            return null;
        function visitHierarchyDescendants(item) {
            if (!item || item.children === undefined || item.children === null)
                return null;
            const children = item.children;
            for (let i = children.length - 1; i >= 0; --i) {
                const child = children[i];
                if (!child || child.visible === false)
                    continue;
                const matchedDescendant = visitHierarchyDescendants(child);
                if (matchedDescendant)
                    return matchedDescendant;
                if (child.__isHierarchyItem !== true)
                    continue;
                const rawItemId = child.itemId !== undefined && child.itemId !== null ? child.itemId : child.resolvedItemId;
                if (noteDropController.normalizedInteger(rawItemId, -1) === resolvedIndex)
                    return child;
            }
            return null;
        }
        return visitHierarchyDescendants(hierarchyTree);
    }
    function normalizeNoteIds(noteIds) {
        if (noteIds === undefined || noteIds === null)
            return [];

        var sourceIds = noteIds;
        if (typeof sourceIds === "string")
            sourceIds = sourceIds.split(/\r?\n/);
        else if (!Array.isArray(sourceIds) && sourceIds.length !== undefined)
            sourceIds = Array.prototype.slice.call(sourceIds);
        else if (!Array.isArray(sourceIds))
            sourceIds = [sourceIds];

        const normalized = [];
        for (let index = 0; index < sourceIds.length; ++index) {
            const normalizedNoteId = String(sourceIds[index] === undefined || sourceIds[index] === null ? "" : sourceIds[index]).trim();
            if (!normalizedNoteId.length || normalized.indexOf(normalizedNoteId) >= 0)
                continue;
            normalized.push(normalizedNoteId);
        }

    }
    function normalizedInteger(value, fallbackValue) {
        const numericValue = Number(value);
        if (!isFinite(numericValue))
            return fallbackValue;
        return Math.floor(numericValue);
    }
    function normalizedNonNegativeInteger(value) {
        const normalized = noteDropController.normalizedInteger(value, -1);
        return normalized >= 0 ? normalized : -1;
    }
    function noteDropIndexAtPosition(x, y, referenceItem) {

    }
    function noteDropTargetAtPosition(x, y, referenceItem) {
        const localX = Number(x) || 0;
        const localY = Number(y) || 0;
        const hierarchyPoint = referenceItem && referenceItem !== hierarchyTree && hierarchyTree.mapFromItem !== undefined ? hierarchyTree.mapFromItem(referenceItem, localX, localY) : ({
                "x": localX,
                "y": localY
            });
        const hierarchyItem = noteDropController.hierarchyItemAtPosition(hierarchyPoint.x, hierarchyPoint.y);
        if (!hierarchyItem)
            return ({
                    "index": -1,
                    "item": null
                });
        const rawItemId = hierarchyItem.itemId !== undefined && hierarchyItem.itemId !== null ? hierarchyItem.itemId : hierarchyItem.resolvedItemId;
        const parsedIndex = noteDropController.normalizedNonNegativeInteger(rawItemId);
        if (parsedIndex < 0)
            return ({
                    "index": -1,
                    "item": null
                });
        return ({
                "index": parsedIndex,
                "item": hierarchyItem
            });
    }
    function noteIdFromDragPayload(drag) {
        const noteIds = noteDropController.noteIdsFromDragPayload(drag);
        return noteIds.length > 0 ? noteIds[0] : "";
    }
    function noteIdsFromDragPayload(drag) {
        if (!drag)
            return [];
        const source = drag.source;
        if (source && source.draggedNoteIds !== undefined && source.draggedNoteIds !== null) {
            const draggedNoteIds = noteDropController.normalizeNoteIds(source.draggedNoteIds);
            if (draggedNoteIds.length > 0)
                return draggedNoteIds;
        }
        if (source && source.noteId !== undefined && source.noteId !== null) {
            const sourceNoteId = String(source.noteId).trim();
            if (sourceNoteId.length > 0)
                return [sourceNoteId];
        }
        if (drag.getDataAsString !== undefined) {
            const mimeNoteIds = String(drag.getDataAsString("application/x-whatson-note-ids") || "").trim();
            if (mimeNoteIds.length > 0) {
                try {
                    const parsedNoteIds = JSON.parse(mimeNoteIds);
                    const normalizedParsedNoteIds = noteDropController.normalizeNoteIds(parsedNoteIds);
                    if (normalizedParsedNoteIds.length > 0)
                        return normalizedParsedNoteIds;
                } catch (error) {}
            }
            const mimeNoteId = String(drag.getDataAsString("application/x-whatson-note-id") || "").trim();
            if (mimeNoteId.length > 0)
                return [mimeNoteId];
            const plainTextNoteIds = noteDropController.normalizeNoteIds(String(drag.getDataAsString("text/plain") || "").trim());
            if (plainTextNoteIds.length > 0)
                return plainTextNoteIds;
        }
        return [];
    }
    function updateNoteDropPreviewAtPosition(x, y, noteIds, referenceItem) {
        const normalizedNoteIds = noteDropController.normalizeNoteIds(noteIds);
        const target = noteDropController.noteDropTargetAtPosition(x, y, referenceItem);
        if (target.index < 0 || normalizedNoteIds.length === 0 || !hierarchyDragDropBridge || !(hierarchyDragDropBridge.canAcceptNoteDropList !== undefined ? hierarchyDragDropBridge.canAcceptNoteDropList(target.index, normalizedNoteIds) : hierarchyDragDropBridge.canAcceptNoteDrop(target.index, normalizedNoteIds[0]))) {
            noteDropController.clearNoteDropPreview();
            return false;
        }
        hostView.noteDropHoverIndex = target.index;
        return true;
    }
}
