import QtQuick

Item {
    id: control

    property var activeItem: null
    property int activeItemId: -1
    property string activeItemKey: ""
    property bool autoSelectFirstItem: true
    default property alias items: itemsColumn.data
    property bool keyboardNavigationEnabled: true

    signal activeChanged(var item, int itemId, int index)

    function activateFirstManagedItem() {
        const items = managedItems();
        for (let index = 0; index < items.length; ++index) {
            const item = items[index];
            if (!item || item.enabled === false || item.rowVisible === false)
                continue;
            requestActivate(item);
            return true;
        }
        return false;
    }
    function clearActiveItem() {
        if (control.activeItem === null && control.activeItemId === -1 && control.activeItemKey.length === 0)
            return;
        control.activeItem = null;
        control.activeItemId = -1;
        control.activeItemKey = "";
        control.activeChanged(null, -1, -1);
    }
    function managedItems() {
        const items = [];
        const children = itemsColumn.children;
        for (let index = 0; index < children.length; ++index) {
            const child = children[index];
            if (child && child.__isHierarchyItem === true)
                items.push(child);
        }
        return items;
    }
    function normalizedIndentLevel(item) {
        if (!item || item.indentLevel === undefined || item.indentLevel === null)
            return 0;
        const numericIndentLevel = Number(item.indentLevel);
        if (!isFinite(numericIndentLevel))
            return 0;
        return Math.max(0, Math.floor(numericIndentLevel));
    }
    function normalizedItemId(item, index) {
        if (!item)
            return -1;
        const rawItemId = item.itemId !== undefined && item.itemId !== null ? Number(item.itemId) : index;
        if (!isFinite(rawItemId))
            return Math.max(0, Math.floor(index));
        return Math.floor(rawItemId);
    }
    function normalizedItemKey(item, index) {
        if (!item)
            return "";
        const rawItemKey = item.itemKey === undefined || item.itemKey === null ? "" : String(item.itemKey).trim();
        return rawItemKey.length > 0 ? rawItemKey : String(Math.max(0, Math.floor(index)));
    }
    function notifyExpansionChanged(item) {
        control.scheduleRefreshState();
        if (control.activeItem && control.activeItem !== item && control.activeItem.rowVisible === false)
            control.requestActivate(item);
    }
    function refreshVisibility(currentItems) {
        const items = currentItems === undefined ? managedItems() : currentItems;
        const expandedAncestors = [];
        for (let index = 0; index < items.length; ++index) {
            const item = items[index];
            const indentLevel = normalizedIndentLevel(item);
            while (expandedAncestors.length > indentLevel)
                expandedAncestors.pop();

            let visible = true;
            for (let ancestorIndex = 0; ancestorIndex < expandedAncestors.length; ++ancestorIndex) {
                if (!expandedAncestors[ancestorIndex]) {
                    visible = false;
                    break;
                }
            }

            item._rowVisibleInternal = visible;
            expandedAncestors[indentLevel] = item.expanded === undefined ? false : !!item.expanded;
        }
    }
    function registerItem(item) {
        if (!item)
            return;
        if (item.hierarchyList !== control)
            item.hierarchyList = control;
        control.scheduleRefreshState();
    }
    function requestActivate(item) {
        if (!item || item.enabled === false)
            return;

        const items = managedItems();
        const index = items.indexOf(item);
        if (index < 0)
            return;
        if (item.hierarchyList !== control)
            item.hierarchyList = control;

        control.refreshVisibility(items);
        if (item.rowVisible === false)
            return;

        const nextItemId = control.normalizedItemId(item, index);
        const nextItemKey = control.normalizedItemKey(item, index);
        const changed = control.activeItem !== item || control.activeItemId !== nextItemId || control.activeItemKey !== nextItemKey;

        control.activeItem = item;
        control.activeItemId = nextItemId;
        control.activeItemKey = nextItemKey;

        if (changed)
            control.activeChanged(item, nextItemId, index);
        if (control.keyboardNavigationEnabled && !control.activeFocus)
            control.forceActiveFocus();
    }
    function scheduleNormalizeActiveItem() {
        if (!control.activeItem) {
            if (control.autoSelectFirstItem)
                control.activateFirstManagedItem();
            return;
        }

        const items = managedItems();
        if (items.indexOf(control.activeItem) >= 0 && control.activeItem.enabled !== false && control.activeItem.rowVisible !== false)
            return;
        control.clearActiveItem();
    }
    function scheduleRefreshState() {
        const items = managedItems();
        for (let index = 0; index < items.length; ++index) {
            const item = items[index];
            if (item && item.hierarchyList !== control)
                item.hierarchyList = control;
        }

        control.refreshVisibility(items);
        control.scheduleNormalizeActiveItem();
    }
    function syncManagedItems() {
        control.scheduleRefreshState();
        if (!control.activeItem && control.autoSelectFirstItem)
            control.activateFirstManagedItem();
    }

    focus: false
    implicitHeight: itemsColumn.implicitHeight
    implicitWidth: itemsColumn.implicitWidth

    Component.onCompleted: control.syncManagedItems()

    Column {
        id: itemsColumn

        spacing: 0
        width: parent ? parent.width : implicitWidth

        onChildrenChanged: {
            control.syncManagedItems();
        }
    }
}
