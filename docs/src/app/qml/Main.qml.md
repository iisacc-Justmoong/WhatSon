# `src/app/qml/Main.qml`

## Role
`Main.qml` is the root LVRS `ApplicationWindow` and the owner of startup routing. The workspace route keeps the existing
desktop/mobile shell layout, while the content slot uses `ContentViewLayout.qml` to bind the selected note's editor
HTML session file into the LVRS text editor surface.

## Runtime Shape
- Onboarding routes and the iOS inline onboarding sequence remain owned here.
- The loaded workspace mounts the restored status bar, navigation bar, body layout, mobile hierarchy page, sidebars,
  note lists, detail panels, and calendar overlay state.
- The root resolves `editorViewModeController` and forwards it to navigation chrome so the
  Plain/Page/Print/Web/Presentation view-mode combo is visible again.
- The root resolves `editorFontFamilyProvider` and forwards it through desktop/mobile workspace shells so the editor
  toolbar font selector can open a system-font menu.
- The desktop and mobile workspace branches are both retained.

## Kept Root Responsibilities
- Instantiate `LV.ApplicationWindow`.
- Maintain LVRS padding compatibility properties.
- Route `/onboarding` and `/` through the LVRS page stack.
- Coordinate embedded onboarding transitions through `OnboardingRouteBootstrapController`.
- Keep render-quality resize policy centralized at the window root.
- Show the standalone onboarding subwindow when desktop startup has no mounted hub.

## Tests
- `test/cpp/suites/qml_contents_view_tests.cpp` verifies that `Main.qml` keeps the restored shell and forwards the
  editor view-mode controller to navigation chrome.
- `test/cpp/suites/include_path_policy_tests.cpp` keeps the restored editor view-mode controller and selector files
  present in source, docs, QML, and C++ regression tests.

## 한국어

- workspace 인터페이스는 기존 shell/layout을 유지한다.
- `Main.qml`은 온보딩/라우팅과 desktop/mobile workspace branch를 유지하되 content slot은 선택된 노트의
  editor HTML session file을 LVRS TextEditor에 연결한다.
- `editorViewModeController` 및 `NavigationEditorViewBar.qml` 기반 보기 모드 선택 계약은 네비게이션바
  콤보박스 표시를 위해 복원되어 있다.
- `editorFontFamilyProvider`는 desktop/mobile workspace shell을 거쳐 editor toolbar font selector에 전달된다.
