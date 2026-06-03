# `src/app/qml/Main.qml`

## Role
`Main.qml` is the root LVRS `ApplicationWindow` and the owner of startup routing for the desktop workspace.
The workspace route keeps the status bar, navigation bar, sidebar, list bar, content slot, detail panel, and calendar overlay state.

## Kept Root Responsibilities
- Instantiate `LV.ApplicationWindow`.
- Maintain LVRS padding compatibility properties.
- Route onboarding and workspace states through the LVRS page stack.
- Keep render-quality resize policy centralized at the window root.
- Show the standalone onboarding subwindow when desktop startup has no mounted hub.

## Tests
- `test/cpp/suites/qml_contents_view_tests.cpp` verifies that `Main.qml` keeps the desktop shell without rebuilding the deleted note editor surface.
- `test/cpp/suites/include_path_policy_tests.cpp` keeps removed editor and separated platform app object families absent from source, docs, QML, and C++ regression tests.

## 한국어
- workspace 인터페이스는 데스크톱 shell/layout을 유지한다.
- `Main.qml`은 데스크톱 온보딩/라우팅과 workspace branch를 소유한다.
- 삭제된 editor view-mode controller, font provider, 모바일 route 객체는 root context로 전달하지 않는다.
