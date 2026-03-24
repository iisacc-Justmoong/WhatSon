import QtQuick

QtObject {
    id: noteDropController

    required property var hierarchyDragDropBridge
    required property var hierarchyTree
    required property var hostView

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
        return items;
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
                if (child.__isHierarchyItem === true
                        && noteDropController.hierarchyItemContainsPoint(child, targetX, targetY))
                    return child;
            }
            return null;
        }
        return visitHierarchyDescendants(hierarchyTree);
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
                const rawItemId = child.itemId !== undefined && child.itemId !== null
                    ? child.itemId
                    : child.resolvedItemId;
                if (Math.floor(Number(rawItemId) || -1) === resolvedIndex)
                    return child;
            }
            return null;
        }
        return visitHierarchyDescendants(hierarchyTree);
    }

    function noteDropTargetAtPosition(x, y, referenceItem) {
        const localX = Number(x) || 0;
        const localY = Number(y) || 0;
        const hierarchyPoint = referenceItem && referenceItem !== hierarchyTree && hierarchyTree.mapFromItem !== undefined
            ? hierarchyTree.mapFromItem(referenceItem, localX, localY)
            : ({
                    "x": localX,
                    "y": localY
                });
        const hierarchyItem = noteDropController.hierarchyItemAtPosition(hierarchyPoint.x, hierarchyPoint.y);
        if (!hierarchyItem)
            return ({
                    "index": -1,
                    "item": null
                });
        const rawItemId = hierarchyItem.itemId !== undefined && hierarchyItem.itemId !== null
            ? hierarchyItem.itemId
            : hierarchyItem.resolvedItemId;
        const parsedIndex = Number(rawItemId);
        if (!isFinite(parsedIndex))
            return ({
                    "index": -1,
                    "item": null
                });
        return ({
                "index": Math.max(-1, Math.floor(parsedIndex)),
                "item": hierarchyItem
            });
    }

    function noteDropIndexAtPosition(x, y, referenceItem) {
        return noteDropController.noteDropTargetAtPosition(x, y, referenceItem).index;
    }

    function canAcceptNoteDropAtPosition(x, y, noteId, referenceItem) {
        const normalizedNoteId = noteId !== undefined && noteId !== null ? String(noteId).trim() : "";
        const target = noteDropController.noteDropTargetAtPosition(x, y, referenceItem);
        if (target.index < 0 || normalizedNoteId.length === 0 || !hierarchyDragDropBridge)
            return false;
        return hierarchyDragDropBridge.canAcceptNoteDrop(target.index, normalizedNoteId);
    }

    function commitNoteDropAtPosition(x, y, noteId, referenceItem) {
        const normalizedNoteId = noteId !== undefined && noteId !== null ? String(noteId).trim() : "";
        const target = noteDropController.noteDropTargetAtPosition(x, y, referenceItem);
        if (target.index < 0 || normalizedNoteId.length === 0) {
            noteDropController.clearNoteDropPreview();
            return false;
        }
        if (!hierarchyDragDropBridge
                || !hierarchyDragDropBridge.assignNoteToFolder(target.index, normalizedNoteId)) {
            noteDropController.clearNoteDropPreview();
            return false;
        }
        noteDropController.clearNoteDropPreview();
        hostView.requestViewHook("hierarchy.noteDrop");
        return true;
    }

    function noteIdFromDragPayload(drag) {
        if (!drag)
            return "";
        const source = drag.source;
        if (source && source.noteId !== undefined && source.noteId !== null) {
            const sourceNoteId = String(source.noteId).trim();
            if (sourceNoteId.length > 0)
                return sourceNoteId;
        }
        if (drag.getDataAsString !== undefined) {
            const mimeNoteId = String(drag.getDataAsString("application/x-whatson-note-id") || "").trim();
            if (mimeNoteId.length > 0)
                return mimeNoteId;
            const plainTextNoteId = String(drag.getDataAsString("text/plain") || "").trim();
            if (plainTextNoteId.length > 0)
                return plainTextNoteId;
        }
        return "";
    }

    function updateNoteDropPreviewAtPosition(x, y, noteId, referenceItem) {
        const normalizedNoteId = noteId !== undefined && noteId !== null ? String(noteId).trim() : "";
        const target = noteDropController.noteDropTargetAtPosition(x, y, referenceItem);
        if (target.index < 0
                || normalizedNoteId.length === 0
                || !hierarchyDragDropBridge
                || !hierarchyDragDropBridge.canAcceptNoteDrop(target.index, normalizedNoteId)) {
            noteDropController.clearNoteDropPreview();
            return false;
        }
        hostView.noteDropHoverIndex = target.index;
        return true;
    }
}
