# `src/app/models/editor/input`

## Responsibility
Owns editor input-policy and mutation-controller primitives that are not themselves visual surfaces.

## Current Modules
- `ContentsEditorInputPolicyAdapter.{hpp,cpp}`
  Centralizes native input, native-to-standard shortcut event normalization, platform shortcut sequence generation,
  shortcut-surface, context-menu, and focus-restore gating. It also canonicalizes macOS Option-modified body-tag keys
  such as `ç` from `Cmd+Option+C` back to `C` before tag-management dispatch.
- `ContentsInlineFormatEditorController.{hpp,cpp}`
  Owns the plain-text wrapper's native input policy, focused native-editor tag-management shortcut filter, selection
  cache, and text-edited dispatch state.
- `ContentsWysiwygEditorPolicy.{hpp,cpp}`
  Owns rendered logical selection mapping, visible text-to-RAW mutation planning, hidden RAW tag cursor normalization,
  hidden-wrapper selection trimming, visible line/paragraph range policy, and atomic resource-block selection decisions
  for the `.wsnbody` editor surface.

## Boundary
- Ordinary text input must continue to stay on native Qt/OS `TextEdit` handling.
- Helpers here may coordinate explicit tag-management commands or RAW mutation plans, but they must not become generic
  key overrides for ordinary note editing.
- Visual editor QML under `src/app/qml/view/contents/editor` may expose wrapper properties and signals, but live typing
  mutation planning, cursor bookkeeping, visible-to-RAW selection policy, hidden tag cursor normalization, selection
  cache, input policy, and source replacement stay behind C++ controller/model boundaries. Do not reintroduce QML
  helper files under this model directory or under the editor view tree for these responsibilities.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/editor/input`` (`docs/src/app/models/editor/input/README.md`)
- 위치: `docs/src/app/models/editor/input`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명하며 native shortcut event를
  표준 modifier 형태로 보정하고 macOS Option 조합의 변형 key code를 tag-management용 표준 키로 되돌리는
  정책도 C++ 입력 정책에 둔다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
