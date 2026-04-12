# `src/app/qml/view/panels/detail/DetailPanel.qml`

## Responsibility
`DetailPanel.qml` binds the C++ `detailPanelViewModel` from the LVRS registry, resolves the active state/content contract, and composes the centered header toolbar plus the active detail form surface.

## Key Contracts
- View-model lookup: `LV.ViewModels.get("detailPanelViewModel")`
- Active content contract: `resolvedActiveContentViewModel`
- Statistics contract: `resolvedFileStatViewModel`
- Selector-copy contracts:
  - `resolvedProjectSelectionViewModel`
  - `resolvedBookmarkSelectionViewModel`
  - `resolvedProgressSelectionViewModel`
- Active state contract: `resolvedActiveStateName`
- Note-link contract: `resolvedNoteContextLinked` (from `detailPanelViewModel.noteContextLinked`)
- Toolbar contract: `resolvedToolbarItems`

## Toolbar Layout
- Gap between toolbar and contents: `LV.Theme.scaleMetric(10)`
- The toolbar remains horizontally centered regardless of panel width.
- Header height is derived from `DetailPanelHeaderToolbar.qml`'s implicit height instead of a fixed `20px` clamp.
- The panel must not force the toolbar back to the legacy `145x20` frame on mobile or desktop.
- The panel's implicit width is now a fixed LVRS-scaled default width, not a reflection of its own
  current `width`. This avoids the `RightPanel -> DetailPanel` implicitWidth binding loop that could
  destabilize the desktop detail column.

## Toolbar Metadata
The file keeps a Figma-scoped toolbar spec and uses its icon names as the canonical source even when C++ toolbar items are present.
This prevents stale backend icon strings from drifting away from the current design metadata.

Current metadata mapping:
- `155:4576` -> `Properties` -> `config`
- `155:4577` -> `FileStat` -> `chartBar`
- `155:4578` -> `Insert` -> `generaladd`
- `155:4579` -> `Layer` -> `toolwindowdependencies`
- `155:4580` -> `FileHistory` -> `toolWindowClock`
- `155:4581` -> `Help` -> `featureAnswer`

The `Properties` button keeps the canonical Figma icon name `config`, but it renders through an explicit
`configuration` icon source so the toolbar shows the gear-shaped properties symbol instead of the unrelated
composite `config.svg` asset.

## Behavior
- Toolbar clicks forward to `detailPanelViewModel.requestStateChange(stateValue)`.
- The root panel now has explicit `linked` / `detached` states.
- `linked`: show toolbar + `DetailContents`.
- `detached`: render an empty panel surface only (no toolbar, no detail form content), so stale metadata cannot be
  displayed when no note is bound.
- The contents area always receives the resolved state name, the resolved active content view-model, the explicit
  `resolvedFileStatViewModel`, the canonical `detailPanelViewModel` object, and the three detail-local selector-copy
  viewmodels, plus `noteContextLinked`.
- This keeps detail-panel selector state independent from sidebar hierarchy selection state.
- The header wrapper mirrors the toolbar's implicit size, so larger LVRS button metrics can surface without additional
  panel-side edits.
