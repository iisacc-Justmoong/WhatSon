# `src/app/qml/view/mobile/pages/MobileHierarchyPage.qml`

## Status
`MobileHierarchyPage.qml` is the adaptive/mobile workspace page mounted by `Main.qml`.

## Current Contract
- The page keeps the existing routed hierarchy, note-list, editor, and detail body components.
- The editor body uses `ContentViewLayout.qml` as the TextEditor surface and forwards `noteActiveState` so the selected
  note body file can be edited.
- It no longer owns or forwards `editorViewModeController`.
- Sidebar, note-list, resource import, calendar overlay, and mobile route coordinator wiring remain part of the shell.

## Editor Surface
`ContentViewLayout.qml` may receive legacy shell inputs from this page, but it must not mount parser, projection,
rendering, resource editor, calendar page, or editor view-mode backend objects. File editing is limited to the
`activeNoteBodyPath -> LV.TextEditor.filePath` binding.

## Tests
- `test/cpp/suites/qml_contents_view_tests.cpp` verifies that `Main.qml` keeps the restored mobile/desktop shell.
- `test/cpp/suites/mobile_chrome_tests.cpp` verifies this page keeps the mobile shell while the editor view-mode path
  stays removed.
- `test/cpp/suites/include_path_policy_tests.cpp` keeps the removed editor view-mode selector/controller family absent.

## 한국어

- 현재 앱 workspace에서는 adaptive/mobile layout에서 이 mobile shell을 mount한다.
- editor route는 `ContentViewLayout.qml`을 사용하고 active note의 `.wsnbody` 파일을 `LV.TextEditor`에 연결한다.
- hierarchy, note-list, detail, calendar, sidebar, import shell 연결은 유지하고 editor view mode만 제거 상태로 둔다.
