# `src/app/qml/view/mobile/pages/MobileHierarchyPage.qml`

## Status
`MobileHierarchyPage.qml` is the adaptive/mobile workspace page mounted by `Main.qml`.

## Current Contract
- The page keeps the existing routed hierarchy, note-list, editor, and detail body components.
- The editor body uses `ContentViewLayout.qml` as the TextEditor surface and forwards `noteEditorSession` so the
  selected note's editor HTML session file can be edited.
- The editor body disables `ContentViewLayout.qml`'s gutter and minimap rails for the mobile route.
- It owns and forwards `editorViewModeController` only to mobile navigation chrome so the editor route can show the
  restored compact `View` combo.
- Sidebar, note-list, resource import, calendar overlay, and mobile route coordinator wiring remain part of the shell.

## Editor Surface
`ContentViewLayout.qml` may receive legacy shell inputs from this page, but it must not mount parser, projection,
rendering, resource editor, calendar page, or editor view-mode backend objects. File editing is limited to the
`NoteEditorDocumentSession.editorFilePath -> LV.TextEditor.filePath` binding plus the sync-finished persist hook.

## Tests
- `test/cpp/suites/qml_contents_view_tests.cpp` verifies that `Main.qml` keeps the restored mobile/desktop shell.
- `test/cpp/suites/mobile_chrome_tests.cpp` verifies this page keeps the mobile shell, forwards the restored editor
  view-mode combo contract, and disables the editor gutter/minimap rails.
- `test/cpp/suites/include_path_policy_tests.cpp` keeps the restored editor view-mode selector/controller family
  present.

## 한국어

- 현재 앱 workspace에서는 adaptive/mobile layout에서 이 mobile shell을 mount한다.
- editor route는 `ContentViewLayout.qml`을 사용하고 active note의 editor HTML session file을
  `LV.TextEditor`에 연결하며, 모바일 route에서는 거터와 미니맵 rail을 끈다.
- hierarchy, note-list, detail, calendar, sidebar, import shell 연결을 유지하면서 editor route의 navigation chrome에
  view-mode 콤보박스를 다시 표시한다.
