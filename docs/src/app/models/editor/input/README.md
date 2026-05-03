# `src/app/models/editor/input`

## Responsibility
Owns editor input-policy and mutation-controller primitives that are not themselves visual surfaces.

## Current Modules
- `ContentsAgendaBlockController.qml`
  Owns aggregate agenda-block queries that do not draw the agenda row UI.
- `ContentsAgendaTaskRowController.qml`
  Owns agenda task row focus, live text snapshots, toggle forwarding, cursor geometry, and committed text emission.
- `ContentsBreakBlockController.qml`
  Owns break-block tag-management key handling and selection activation.
- `ContentsCalloutBlockController.qml`
  Owns callout text snapshots, cursor geometry, selection cleanup, committed text emission, and the explicit
  plain-Enter callout-exit tag command. It also owns empty-callout Backspace deletion. Shift+Enter and other unhandled
  Enter/Backspace variants remain native callout body text input.
- `ContentsDocumentBlockController.qml`
  Owns generic document-block delegation, atomic-block tag-management keys, and mounted delegate signal forwarding.
- `ContentsDocumentTextBlockController.qml`
  Owns structured text-block live snapshots, direct plain-text RAW block mutation, inline-tag-aware source replacement
  for styled blocks, cursor geometry, and focus handling.
- `ContentsEditorInputPolicyAdapter.qml`
  Centralizes native input, shortcut-surface, context-menu, and focus-restore gating.
- `ContentsEditorSelectionController.qml`
  Resolves editor selections and routes inline style/context-menu commands.
- `ContentsEditorTypingController.qml`
  Keeps legacy whole-editor text mutation helpers and editor authoring shortcuts that still need whole-document RAW
  cursor/selection context. Generated body-tag insertion and selected-range callout wrapping payloads are delegated to
  `src/app/models/editor/tags/ContentsRawBodyTagMutationSupport.js`, which returns direct RAW `.wsnbody` splice
  payloads instead of routing through an extra planner QObject.
- `ContentsInlineFormatEditorController.qml`
  Owns the plain-text wrapper's native input policy, selection cache, and text-edited dispatch state.

## Boundary
- Ordinary text input must continue to stay on native Qt/OS `TextEdit` handling.
- Helpers here may coordinate explicit tag-management commands or RAW mutation plans, but they must not become generic
  key overrides for ordinary note editing.
- Visual editor QML under `src/app/qml/view/contents/editor` may expose wrapper properties and signals, but live typing,
  cursor bookkeeping, selection cache, source replacement, and atomic tag-management key decisions belong to these
  controller objects.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/editor/input`` (`docs/src/app/models/editor/input/README.md`)
- 위치: `docs/src/app/models/editor/input`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
