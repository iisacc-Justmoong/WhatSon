.pragma library

function isObject(value) {
    return value !== null && value !== undefined && typeof value === "object";
}

function hasCurrentResourceEntry(noteListModel) {
    if (!isObject(noteListModel))
        return false;
    if (noteListModel.noteBacked === true)
        return false;
    return isObject(noteListModel.currentResourceEntry);
}

function currentResourceEntry(noteListModel) {
    if (!hasCurrentResourceEntry(noteListModel))
        return ({});
    return noteListModel.currentResourceEntry;
}

function resourceEditorVisible(noteListModel) {
    return hasCurrentResourceEntry(noteListModel);
}
