# `src/app/qml/view/mobile/MobilePageScaffold.qml`

## Status
`MobilePageScaffold.qml` is the restored mobile route scaffold used by the adaptive/mobile workspace.

## Current Surface
- Owns a compact `NavigationBarLayout`, a body `LV.PageRouter`, and a compact `StatusBarLayout` for mobile routes.
- No longer exposes `compactEditorViewVisible`.
- No longer forwards an editor view-mode controller into `NavigationBarLayout`.

## Guardrail
Do not reintroduce the editor view-mode selector path here. Future mobile editor work must start from the LVRS
`TextEditor` contract and document the new route policy first.

## 한국어

- 현재 workspace에서는 adaptive/mobile layout에서 사용되는 scaffold다.
- editor view mode 선택 UI 및 controller 전달은 제거됐다.
