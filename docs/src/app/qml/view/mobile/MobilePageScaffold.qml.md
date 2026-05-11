# `src/app/qml/view/mobile/MobilePageScaffold.qml`

## Status
`MobilePageScaffold.qml` is the restored mobile route scaffold used by the adaptive/mobile workspace.

## Current Surface
- Owns a compact `NavigationBarLayout`, a body `LV.PageRouter`, and a compact `StatusBarLayout` for mobile routes.
- Exposes `compactEditorViewVisible` so the editor route can swap the settings button for the restored `View` combo.
- Forwards `editorViewModeController` into `NavigationBarLayout`.

## Guardrail
The editor view-mode selector is navigation chrome only. Future mobile editor rendering work must still start from the
LVRS `TextEditor` contract and document any route policy first.

## 한국어

- 현재 workspace에서는 adaptive/mobile layout에서 사용되는 scaffold다.
- editor route에서는 view-mode 콤보박스 표시를 위해 controller를 navigation bar로 전달한다.
