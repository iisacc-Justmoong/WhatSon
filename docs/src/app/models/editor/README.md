# `src/app/models/editor`

## Responsibility
Owns C++ editor-domain model objects that are intentionally outside QML view composition.

## Scope
- Source directory: `src/app/models/editor`
- Build shard: `src/app/models/editor/CMakeLists.txt`
- Child files:
  - `GetProperty.h`
  - `GetProperty.cpp`
  - `EditorFontFamilyProvider.hpp`
  - `EditorFontFamilyProvider.cpp`
  - `component/Break.h`
  - `component/Break.cpp`
  - `component/Callout.h`
  - `component/Callout.cpp`
  - `component/style.h`
  - `component/style.cpp`
  - `component/ResourceImageFrame.h`
  - `component/ResourceImageFrame.cpp`
  - `EditorInputCommandFilter.hpp`
  - `EditorInputCommandFilter.cpp`
  - `TagInsertionWriter.hpp`
  - `TagInsertionWriter.cpp`
  - `NoteEditorDocumentSession.hpp`
  - `NoteEditorDocumentSession.cpp`
  - `SetProperty.h`
  - `SetProperty.cpp`
  - `SetTag.h`
  - `SetTag.cpp`

## Current Contract
- The directory is registered through its own CMake shard and the root app target reaches it with
  `add_subdirectory(models/editor)`.
- Editor-domain C++ belongs here only when it is backend model/controller logic rather than view-local QML behavior.
- `SetTag` is the static `.wsnbody` RAW tag input object. It exposes a fixed allow-list of body tag templates,
  text or into a serialized `.wsnbody` document via `WhatSon::NoteBodyPersistence`. Component-specific template
  definitions can live under `component/`; `SetTag` consumes those descriptors and owns the generic mutation result
  shape. The paired `style` text wrapper is part of this allow-list; `component/style` owns its canonical `style`
  attribute values, LVRS token metrics, Body fallback opening token, marker-backed editor HTML projection, and source
  recovery helpers so note-body persistence delegates the component policy rather than embedding it inline.
- `TagInsertionWriter` is the persisted tag insertion command object. It reads a local `.wsnote`, delegates
  static tag source mutation to `SetTag`, and writes the resulting source back through `WhatSonLocalNoteFileStore` so
  the actual `.wsnbody` document is updated.
- `SetProperty` is the dynamic `.wsnbody` tag-attribute mutation object. It receives the property name as a string,
  infers the value serialization from `QVariant`, and writes string, integer, float, or boolean attribute values into
  the tag under the requested cursor position.
- `GetProperty` is the read-side `.wsnbody` tag-attribute capture object. It stores the current tag's dynamic
  attributes as in-app key/value state and exposes inferred value kinds beside the stored values.
- `EditorFontFamilyProvider` reads Qt system font families for the editor toolbar font selector, normalizes them into a
  stable sorted list, and returns LVRS context-menu item descriptors. The provider does not mutate note source itself;
  selected families are passed through `NoteEditorDocumentSession.insertStyleFontTagIntoSource(...)`.
- `component/Break` owns the standalone editor source token `</break>`. It recognizes `</break>`, `<break/>`, and
  legacy `<hr/>` aliases as a single logical break source line, renders that line as editor line-break space instead of
  literal tag text, and keeps `.wsnbody` storage normalized to `<break/>`.
  generic insertion wrapping, checkbox toggle handling, and `.wsnbody` serialization to the session/`SetTag`/
  persistence boundaries.
  `LV.CheckBox` overlays provide the interactive toggle hit targets from task body positions. Its root frame fills
  table-backed in editor HTML so Qt paints one continuous frame surface and keeps the header date right-aligned instead
  block. Enter in the last non-empty task appends a new empty task, and Enter in an empty last task removes that task
- `component/Callout` owns the visual editor projection for paired `<callout>...</callout>` source. It renders the
  Figma `280:7897` callout as a full-width editor row with `data-frame-width-mode="fill"`,
  `data-frame-height-mode="hug-contents"`, root `height:auto`, a `#262728` surface, `4px` top/bottom
  padding, `4px` left/right padding, `3px x 14px` leading bar metadata, `12px` content gap metadata, one generated
  left-aligned frame-chrome image for the leading bar, and Pretendard Medium `12/12` white body text. The emitted root
  is a block `div`, not an inline span and not a table, so the rendered surface fills the QTextDocument row without
  QML chrome. The callout text itself is isolated in a
  `data-callout-content="true"` span, letting persistence ignore the frame chrome and restore the callout wrapper after
  LVRS rich-text editing. The bar image height is generated from the wrapped content height using the active editor
  viewport width, and the bar starts at the content origin while the root frame owns the `4px` padding. The gap is its
  right margin, so wrapped text is not pushed to a lower line by separate inline spacer images. The editor session's
  frame reproject hook includes callouts, and QML schedules it with a short debounce after text changes so edited
  callout text regenerates the leading-bar image without replacing the document on every keystroke. Enter on the frame
  chrome immediately before content is planned here as a source insertion before `<callout>`, Backspace at content start
  is planned here as wrapper removal or empty-line removal, and Enter inside content is planned here as a move after the
  closing tag. Explicit empty source lines adjacent to callouts render through an invisible placeholder so they count as
  editor/gutter rows while still saving as empty source lines.
- `EditorInputCommandFilter` owns the native editor item event filter for command-style keys. It consumes only handled
  semantic boundary and paste commands, delegating RAW semantic-boundary decisions to `NoteEditorDocumentSession` and
  clipboard package creation to `ClipboardEditorPaste`. Style Return/Enter is intercepted only when it exits a style
  wrapper; ordinary typing at the style end remains styled, and generated empty lines keep their native `TextEdit`
  Return behavior.
- `NoteEditorDocumentSession` is the active note document session object. It asks the note package layer to parse the
  selected `.wsnbody` into editor-facing RAW source, projects that source into an editor HTML cache/session file for
  LVRS `TextEditor.filePath`, exposes parsed source line count as session metadata, builds imported-resource command
  insertions and static format-tag source insertions for editor command flows, and persists only canonical RAW source to
  the selected `.wsnbody`. The modified-count RAW push path consumes the contents editor wrapper's
  `editorDocumentEdited(documentText, documentRevision)` signal, which forwards LVRS
  `documentEdited(documentText, documentRevision)` for the just-edited document turn so the final input character is
  included before wrapper bindings settle. If the native editor is still in input method composition, the view defers
  both modified-count and idle RAW push requests until composition ends, then flushes the current committed editor
  document payload instead of the partial preedit boundary. Idle
  and modified-count RAW push requests are accepted only after LVRS
  `readFinished(path)` has marked the fresh current mounted session file ready; remounting a note resets that readiness
  even when the same `.wsnsource` path was ready during an earlier visit. The session then converts the current editor
  document into RAW source, rejects transient empty editor payloads over a non-empty active RAW source, and queues only
  that validated RAW source in `WhatSonEditorRawPushController`. If the same modified-count revision is already pending,
  a later validated modified-count payload for the same mounted session file refreshes that pending RAW source instead
  of being discarded; this covers Korean composition settlement where the visible text advances from a partial syllable
  boundary to the committed word before the debounce flush reaches `.wsnbody` and NoteList preview state. Empty rich-text HTML shells are rejected for idle RAW
  push so a still-blank LVRS document snapshot cannot clear a non-empty note while the mounted file is settling. The
  controller no longer stores editor HTML or rereads
  the `.wsnsource` file as sync truth. The active RAW source becomes the save/sync truth for the active note; idle
  filesystem pulls apply the filesystem diff onto that active RAW source, and editor pushes apply the editor diff onto
  the current filesystem RAW body. Neither direction may fall back to replacing the whole body when the current text has
  diverged from the loaded base. Note-departure flushes persist active/pending RAW instead of falling back to stale
  mounted session-file contents. This protects ordinary typed editor input while image paste follows the 2026-05-19
  command-result and LVRS sync flow. Imported image-resource command
  results are staged to the mounted `.wsnsource` file immediately and discard older pending pushes for that file; until
  later user typing supplies a newer validated modified-count RAW payload, note departure persists that active canonical
  source. `<style>` wrappers are invisible to the session's source-visible text mapper, so a later LVRS-normalized
  rich-text save that has the same visible characters but lacks style markers is not allowed to overwrite the active
  styled RAW source with a plain paragraph. Plain editor raw pushes that insert, delete, or replace visible text inside a
  pre-existing style wrapper are merged into the active RAW source according to the active wrapper boundary, so style
  marker positions do not drift or disappear unless a style selector or font selector command explicitly changes them.
  Text typed after a font/style selector stays inside the wrapper; Enter is the only style exit key.
  The gutter uses the session's parsed
  source line count as
  its delegate count; the QML `TextEditor` wrapper may only provide rendered placement for those source lines and must
  not let the LVRS rendered wrap-line count create additional gutter rows. When a new empty callout is inserted, the
  session returns the cursor position in decorated LVRS rich-text coordinates after generated callout chrome so the
  caret starts inside the callout content span. Backspace at that same decorated content start maps back to the loaded
  RAW source content start and delegates the wrapper/empty-line edit plan to `component/Callout` instead of deleting
  generated chrome. Callout/style Enter boundary helpers use the current editor document snapshot before mutating RAW,
  so a character removed immediately before Enter is not restored from stale active source. Backspace on an explicit
  empty source paragraph is also a session-owned boundary edit: the first key press deletes the empty RAW source line
  and refreshes parsed line count instead of only deleting a hidden placeholder.
- `component/ResourceImageFrame` owns standalone image `<resource ... />` editor frame rendering. It implements the Figma `292:50`
  image-resource frame as a marker-wrapped atomic image object, marker-wrapped source recovery, editor-width responsive
  media sizing from the current editor viewport width, initial auto-height locking across later viewport reprojection,
  dynamic centered image placement inside that frame-width media raster, and an image-only visible rendering surface.
  The emitted object has zero margins and `vertical-align:top`, so the resource frame top aligns with the canonical
  source line origin. It does not put resource-frame background/border CSS on the `<img>` object, so text typed below
  the image cannot inherit frame styling from Qt rich text character-format carryover. It does not emit type/file-name
  display metadata or visible header/footer chrome.
- `component/style` owns the proprietary paired `<style ...>...</style>` source wrapper. It provides the static wrapper
  tokens to `SetTag`, maps canonical toolbar values (`Title`, `Title2`, `Subtitle`, `Header`, `Header2`, `Body`,
  `Description`, `Caption`, `Footnote`) to the same LVRS token-aligned Pretendard typography recipe used by the toolbar
  style-menu previews, treats an empty `style` attribute as Body, projects explicit attributes such as `font`, `weight`,
  `size`, `color`, `background`, `align`, and existing `height` into editor CSS, and emits/recovers the marker-backed
  `<span>` used by the LVRS text editor path. Font-family CSS values are quoted in that span so selected system fonts,
  including multi-word families, render as the requested family in Qt RichText instead of falling through to the editor
  default. Token-derived projection does not add per-style colors; styled spans inherit the editor body color unless a
  source `color` attribute is present.
- `NoteEditorDocumentSession.insertStyleFontTagIntoSource(...)` uses the same style-source selection mapping as the
  style selector, then inserts `<style font="...">...</style>` through `SetTag`. Active notes stage the projected editor
  HTML into the mounted `.wsnsource` file and persist the canonical `.wsnbody` body immediately.
- `NoteEditorDocumentSession.insertStyleBackgroundTagIntoSource(...)` uses the same style-source selection mapping for
  highlight color commands, validates names/hex through the bookmark color palette contract, then inserts
  `<style background="...">...</style>` through `SetTag`.
- `NoteEditorDocumentSession.highlightColorMenuItems()` exposes that bookmark palette as QML menu data for the editor
  toolbar highlight chevron.
- Minimap display backends, projection/rendering pipelines, and legacy editor view-mode controllers remain outside
  this shard unless a new documented contract explicitly reintroduces them.

## Verification Notes
- Source-tree policy coverage verifies that this shard is present, documented, and registered through
  `src/app/models/editor/CMakeLists.txt` rather than direct file entries in `src/app/CMakeLists.txt`.
  template lookup, `Break` standalone source projection, `Callout` visual block rendering and source recovery,
  `ResourceImageFrame` image-only container rendering, `SetProperty`
  dynamic attribute mutation, `GetProperty` key/value capture, `NoteEditorDocumentSession` editor-HTML mounting,
  parsed line-count reporting, imported resource source insertion, resource-frame viewport reprojection with locked
  reserialization.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/editor`` (`docs/src/app/models/editor/README.md`)
- 위치: `docs/src/app/models/editor`
- 역할: 이 파일은 editor model shard의 구조, 책임, CMake 등록 계약, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 현재: `component/style`은 toolbar style selector의 canonical 값, size/weight/background 중심의 LVRS token-aligned
  typography metric, bookmark color palette 기반 background 값, Body fallback token을 소유한다.
  소유하고, `SetTag`는 공통 source mutation과 결과 map 생성을 맡는다.
- 현재: `NoteEditorDocumentSession.insertStyleTagIntoSource(...)`는 selection이 접혀 있으면 현재 non-empty visible
  source line 전체를 스타일 대상으로 확장하고, 빈 줄에서는 즉시 사라지는 zero-width `<style>` wrapper를 만들지 않는다.
  유효한 style mutation은 active editor session file에 즉시 stage되고 실제 `.wsnbody` RAW body에도 바로 persist되어
  LVRS idle sync가 이전 paragraph snapshot으로 되돌리는 레이스를 막는다. source-visible text 계산에서도
  `<style>` wrapper를 invisible tag로 취급하여, LVRS가 marker 없는 동등 가시 텍스트를 내보내도 active styled
  RAW source가 plain paragraph로 평탄화되지 않는다. 기존 style wrapper 안쪽에서 만들어진 marker 없는 plain RAW
  push는 삽입, 삭제, 치환 모두 active RAW source의 wrapper 경계를 기준으로 병합하므로, style selector나 font selector
  명령이 아닌 일반 입력이나 Backspace만으로 `<style>` 위치가 밀리거나 사라지지 않는다. font/style selector 직후
  이어 입력한 텍스트는 wrapper 안에 남고, Enter만 wrapper 밖으로 나가는 semantic boundary key다.
- 현재: `TagInsertionWriter`는 `SetTag` 결과를 실제 로컬 `.wsnbody`에 저장하는 태그 삽입 command 객체다.
- 현재: `SetProperty`는 문자열 기반 동적 속성명과 자동 추론된 값 타입으로 태그 속성을 설정한다.
- 현재: `GetProperty`는 태그 속성을 조회해 인앱 키/값 상태로 저장한다.
- 현재: `EditorFontFamilyProvider`는 Qt system font family 목록을 읽어 editor toolbar font selector의
  `LV.ContextMenu` 항목으로 제공한다. 선택한 family 자체는 `NoteEditorDocumentSession.insertStyleFontTagIntoSource(...)`
  로 전달되어 `<style font="...">` source mutation으로 적용된다.
- 현재: `component/Break`는 standalone editor source token `</break>`를 소유한다. `</break>`, `<break/>`, legacy
  `<hr/>`는 같은 논리 break line으로 판정되며, 노트 에디터에는 literal tag text가 아니라 그 위치의 논리 빈 줄로
  투영된다.
  editor projection은 Figma `279:7854` 프레임과 checkbox task row로 보이지만 save 경계에서는 canonical
  `LV.CheckBox` overlay는 task body 좌표에서 파생한 hit target으로 완료 토글을 맡는다. root frame은 editor 폭을 채우고 높이는 task content에 맞춘 auto/hug이며,
- 현재: `component/Callout`은 `<callout>...</callout>` paired source를 Figma `280:7897` 기준의 full-width editor
  row로 렌더링한다. 배경은 `#262728`, 상하 padding은 `4px`, 좌우 padding은 `4px`, bar는 `3px x 14px`, gap은
  `12px`, 텍스트는 Pretendard Medium `12/12`이다. 루트는 inline span이 아니라 block `div`이고 table도 아니며,
  좌측 bar는 wrap된 content 높이에 맞춘 단일 left-aligned frame chrome 이미지로 렌더링되고, 자체 상단 여백 없이
  content 원점에서 시작해 첫 텍스트 라인과 세로 중심을 맞춘다. 텍스트 편집으로 wrap 높이가 달라지면 QML이 짧은
  debounce 뒤 editor session의 frame reproject 경로를 호출해 이 이미지를 다시 만든다. gap은 그 이미지의 우측
  margin이다. 또한 `component/Callout`은 callout source range, decorated cursor mapping, Backspace/Enter boundary
  edit 계획을 소유한다.
- 현재: callout frame chrome 바로 왼쪽에서 Enter를 누르면 `component/Callout`이 `<callout>` 앞 source 위치의 빈
  줄 삽입을 계획한다. 이 빈 줄은 persistence projection에서 invisible placeholder로 렌더되어 거터 row를 실제로
  하나 늘리고, 저장 시에는 다시 빈 source line으로 복원된다.
  경계 Backspace와 style boundary Enter는 `NoteEditorDocumentSession`으로, 이미지 resource paste shortcut은
  `ClipboardEditorPaste`로 위임하며, 처리된 경우에만 native editor event를 consume한다.
- 현재: `NoteEditorDocumentSession`은 `.wsnbody` XML 원문이 아니라 RAW source에서 투영한 editor HTML session
  file을 `LV.TextEditor`에 연결하고, parsed source line metadata, imported-resource source insertion, static
  format-tag insertion을 제공하며, 저장 시 다시 canonical source를 거쳐 `.wsnbody`로 serialize한다. imported
  resource 삽입은 2026-05-19 image paste command-result 흐름처럼 현재 커서/선택 범위를 적용한 source/editor HTML
  결과를 반환한다. active note에서는 이 결과를 mounted `.wsnsource` file에도 즉시 기록하고, 이미지 삽입 전의
  오래된 pending push는 폐기한다. 이후 더 최신 validated modified-count RAW payload가 들어오기 전까지
  note-departure flush는 이 active canonical source를 저장한다. `readFinished(path)` 이후 contents editor wrapper의
  `editorDocumentEdited(documentText, documentRevision)`가 도착하면 QML은 forwarded LVRS payload와 revision을
  modified-count RAW push의 최신 editor document 기준으로 전달하고, 세션은 이를 canonical RAW로 변환·검증한 뒤 RAW source payload만 sync controller에 넘긴다. 새 note mount는 이전
  `readFinished(path)` 준비 상태를 재사용하지 않으며, 빈 editor 문자열이나 내용 없는 rich-text HTML shell이
  non-empty active source를 덮는 transient idle payload이면 저장하지 않는다. push/pull은 loaded RAW base와 현재 RAW의 diff를
  적용하는 방식으로만 본문을 갱신하며, diverged current body 위에 editor payload 전체를 대체 쓰기하지 않는다.
  이미지 바로 아래 첫 텍스트가 image object와 같은
  rich-text block으로 직렬화되어도 복원 경로는 object 앞/뒤 텍스트를 별도 source line으로 보존한다. idle sync와
  note-departure flush는 낡은 mounted session file 대신 active RAW source 또는 pending RAW payload를 기준으로 한다.
  거터의 실제
  row 개수는 session의 parsed source line count만 사용하며, QML `TextEditor` wrapper는 해당 source line의 렌더
  위치만 제공할 수 있다. 새 빈 callout을 삽입할 때는 생성된 callout chrome 뒤의 LVRS rich-text content 좌표를
  커서 위치로 반환해 caret이 callout 내부에서 시작하게 한다. 같은 decorated content 시작점에서 Backspace가 오면
  loaded RAW source의 callout content 시작점으로 역매핑하고 `component/Callout`의 edit 계획을 적용한다. 명시적
  빈 source paragraph의 Backspace는 첫 입력에서 빈 RAW source line을 삭제하고 parsed line count를 갱신하므로
  hidden placeholder만 삭제된 채 거터 row가 남지 않는다.
- 현재: `component/ResourceImageFrame`은 standalone image `<resource ... />` 라인을 Figma `292:50` 기준의 editor
  resource frame으로 렌더링한다. 이 frame은 source marker로 감싼 atomic image object이며, 캐시 raster의 intrinsic
  width는 현재 editor viewport 폭을 따른다. 첫 auto height는 `data-frame-display-height`로 고정되고 이후 viewport
  재투영에서는 frame 폭과 x축 중앙 offset만 다시 계산한다. 실제 이미지 표시 박스는 frame 폭 안에서 동적으로 중앙
  정렬된다. resource frame object의 margin은 0으로 고정되고 `vertical-align:top`으로 배치되므로 frame 좌상단은
  canonical resource source line의 시작 위치에 맞는다. `<img>` object에는 배경색/테두리 CSS를 싣지 않아 뒤에
  입력되는 텍스트가 resource-frame char format을 상속하지 않게 한다. resource type, `...`, file name 표시 정보는
  HTML에도 내보내지 않으며, 표시용 header/footer와 복원용 렌더 텍스트 목록도 제공하지 않는다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
