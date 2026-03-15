import QtQuick

Item {
    id: control

    property var activeItem: null
    property int activeItemId: -1
    property string activeItemKey: ""
    property bool autoSelectFirstItem: true
    property int itemCount: 0
    default property alias items: itemsColumn.data
    property bool keyboardNavigationEnabled: true
    property bool manualActivationOnly: false
    property int visibleItemCount: 0

    signal activeChanged(var item, int itemId, int index)
    signal ensureVisibleRequested(real y, real height)
    signal expansionChanged(var item, bool expanded, int index)

    function activateById(itemId) {
        const item = resolveById(itemId);
        if (!item)
            return false;
        requestActivate(item, true);
        return true;
    }
    function activateByKey(itemKey) {
        const item = resolveByKey(itemKey);
        if (!item)
            return false;
        requestActivate(item, true);
        return true;
    }
    function activateFirstManagedItem() {
        const items = managedItems();
        for (let index = 0; index < items.length; ++index) {
            const item = items[index];
            if (!item || item.enabled === false || item.rowVisible === false)
                continue;
            requestActivate(item, true);
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
    function indexOfItem(item) {
        return managedItems().indexOf(item);
    }
    function isItemVisible(itemOrIndex) {
        if (typeof itemOrIndex === "number") {
            const index = Math.floor(Number(itemOrIndex));
            const items = managedItems();
            if (index < 0 || index >= items.length)
                return false;
            const indexedItem = items[index];
            return !!indexedItem && indexedItem.enabled !== false && indexedItem.rowVisible !== false;
        }

        const item = itemOrIndex;
        return !!item && item.enabled !== false && item.rowVisible !== false;
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
        const items = managedItems();
        const index = items.indexOf(item);
        control.refreshManagedItemCounts(items);
        if (index >= 0)
            control.expansionChanged(item, item ? !!item.expanded : false, index);
        control.scheduleRefreshState();
        if (control.activeItem && control.activeItem !== item && control.activeItem.rowVisible === false)
            control.requestActivate(item, true);
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
    function requestActivate(item, force) {
        if (!item || item.enabled === false)
            return;
        const activationForced = force === true;
        if (control.manualActivationOnly && !activationForced)
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
        control.ensureVisibleRequested(item.y, item.height);
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
        control.refreshManagedItemCounts(items);
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
