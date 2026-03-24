# `src/app/qml/view/panels/detail/DetailPanel.qml`

## Responsibility
`DetailPanel.qml` binds the C++ `detailPanelViewModel` from the LVRS registry, resolves the active state/content contract, and composes the centered header toolbar plus the active detail form surface.

## Key Contracts
- View-model lookup: `LV.ViewModels.get("detailPanelViewModel")`
- Active content contract: `resolvedActiveContentViewModel`
- Active state contract: `resolvedActiveStateName`
- Toolbar contract: `resolvedToolbarItems`

## Figma Mapping
- Toolbar frame width: `145`
- Toolbar frame height: `20`
- Gap between toolbar and contents: `10`
- The toolbar remains horizontally centered regardless of panel width.

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
- The contents area always receives both the resolved state name and the resolved active content view-model.
