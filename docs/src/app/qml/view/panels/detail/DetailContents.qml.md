# `src/app/qml/view/panels/detail/DetailContents.qml`

## Responsibility
`DetailContents.qml` owns the state-switched body of the desktop detail panel.
The implemented primary form is the Figma `Properties` example (`155:4582`), while the other toolbar states mount dedicated placeholder forms instead of reusing the properties body.

## Root Contract
- Root `objectName`: `DetailContents`
- Root Figma node id: `155:4582`
- Inputs:
  - `activeContentViewModel`
  - `activeStateName`
- Normalized state values:
  - `properties`
  - `fileStat`
  - `insert`
  - `fileHistory`
  - `layer`
  - `help`
- Legacy aliases are intentionally not accepted. Callers must send the canonical state ids above.

## Implemented Properties Form
The `properties` state renders the Figma `Form` node (`155:4583`) with the exact section order:
1. `Projects` combo (`178:5494`, text `178:5495`, combo `178:5496`)
2. `Bookmark` combo (`155:4584`, text `155:4585`, combo `155:4586`)
3. `FoldersList` (`155:4587`, text `155:4588`, list `155:4589`)
4. `TagsList` (`155:4590`, text `155:4591`, list `155:4592`)
5. `Progress` combo (`178:5501`, text `178:5502`, combo `178:5503`)

## LVRS Reuse
- Uses `LV.ComboBox` for all compact selectors.
- Uses `LV.ListFooter` for the `addFile` / `trash` / `settings` footer controls in the Figma small lists.
- Uses LVRS typography and panel tokens instead of introducing ad-hoc colors or fonts.

## Non-Properties States
For `fileStat`, `insert`, `fileHistory`, `layer`, and `help`, the file renders a distinct placeholder form with state-specific titles and summaries.
This keeps the state switch explicit until each mode receives its final Figma form.
