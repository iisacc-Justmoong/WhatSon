.pragma library

function isObjectLike(value) {
    return value !== null && value !== undefined && typeof value === "object";
}

function resourceEditorVisible(noteListModel) {
    if (!noteListModel || noteListModel.currentResourceEntry === undefined)
        return false;
    if (noteListModel.noteBacked !== undefined && Boolean(noteListModel.noteBacked))
        return false;
    return true;
}

function currentResourceEntry(noteListModel) {
    if (!noteListModel || noteListModel.currentResourceEntry === undefined)
        return ({});
    if (noteListModel.noteBacked !== undefined && Boolean(noteListModel.noteBacked))
        return ({});

    const resourceEntry = noteListModel.currentResourceEntry;
    return isObjectLike(resourceEntry) ? resourceEntry : ({});
}

function hasCurrentResourceEntry(noteListModel) {
    const resourceEntry = currentResourceEntry(noteListModel);
    for (const key in resourceEntry)
        return true;
    return false;
}
