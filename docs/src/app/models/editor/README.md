# `src/app/models/editor`

## Responsibility
Owns C++ editor-domain model objects that are intentionally outside QML view composition.

## Scope
- Source directory: `src/app/models/editor`
- Build shard: `src/app/models/editor/CMakeLists.txt`
- Child files:
  - `GetProperty.h`
  - `GetProperty.cpp`
  - `component/ResourceFrame.h`
  - `component/ResourceFrame.cpp`
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
  including `header`, `subheader`, and standalone `resource`, and can insert them into editor source text or into a
  serialized `.wsnbody` document via `WhatSon::NoteBodyPersistence`.
- `TagInsertionWriter` is the persisted tag insertion command object. It reads a local `.wsnote`, delegates
  static tag source mutation to `SetTag`, and writes the resulting source back through `WhatSonLocalNoteFileStore` so
  the actual `.wsnbody` document is updated.
- `SetProperty` is the dynamic `.wsnbody` tag-attribute mutation object. It receives the property name as a string,
  infers the value serialization from `QVariant`, and writes string, integer, float, or boolean attribute values into
  the tag under the requested cursor position.
- `GetProperty` is the read-side `.wsnbody` tag-attribute capture object. It stores the current tag's dynamic
  attributes as in-app key/value state and exposes inferred value kinds beside the stored values.
- `NoteEditorDocumentSession` is the active note document session object. It asks the note package layer to parse the
  selected `.wsnbody` into editor-facing RAW source, projects that source into an editor HTML cache/session file for
  LVRS `TextEditor.filePath`, exposes parsed source line count as session metadata, builds imported-resource and static
  format-tag source insertions for editor command flows, and persists LVRS sync-finished rich-text edits back through the note body
  persistence path after converting them to canonical source. The gutter uses the session's parsed source line count as
  its delegate count; the QML `TextEditor` wrapper may only provide rendered placement for those source lines and must
  not let the LVRS rendered wrap-line count create additional gutter rows.
- `component/ResourceFrame` owns standalone `<resource ... />` editor frame rendering. It implements the Figma `292:50`
  image-resource chrome as a cached single-image-object preview, marker-wrapped source recovery, editor-width
  responsive frame sizing, display-only type/file-name labels, and legacy rendered text lines that persistence may
  ignore when older rich frame residue is serialized back into plain text.
- Minimap display backends, projection/rendering pipelines, and legacy editor view-mode controllers remain outside
  this shard unless a new documented contract explicitly reintroduces them.

## Verification Notes
- Source-tree policy coverage verifies that this shard is present, documented, and registered through
  `src/app/models/editor/CMakeLists.txt` rather than direct file entries in `src/app/CMakeLists.txt`.
- Runtime C++ coverage verifies `SetTag` source insertion, persisted `TagInsertionWriter` body writes, `ResourceFrame`
  Figma chrome rendering, `SetProperty`
  dynamic attribute mutation, `GetProperty` key/value capture, `NoteEditorDocumentSession` editor-HTML mounting,
  parsed line-count reporting, imported resource source insertion, editor format-tag insertion, unsupported input rejection, and `.wsnbody`
  reserialization.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/editor`` (`docs/src/app/models/editor/README.md`)
- 위치: `docs/src/app/models/editor`
- 역할: 이 파일은 editor model shard의 구조, 책임, CMake 등록 계약, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 현재: `SetTag`는 정적으로 허용된 `.wsnbody` RAW 태그만 삽입하는 C++ 입력 객체이며 `header`,
  `subheader`, `resource`를 포함한다.
- 현재: `TagInsertionWriter`는 `SetTag` 결과를 실제 로컬 `.wsnbody`에 저장하는 태그 삽입 command 객체다.
- 현재: `SetProperty`는 문자열 기반 동적 속성명과 자동 추론된 값 타입으로 태그 속성을 설정한다.
- 현재: `GetProperty`는 태그 속성을 조회해 인앱 키/값 상태로 저장한다.
- 현재: `NoteEditorDocumentSession`은 `.wsnbody` XML 원문이 아니라 RAW source에서 투영한 editor HTML session
  file을 `LV.TextEditor`에 연결하고, parsed source line metadata, imported-resource source insertion, static
  format-tag insertion을 제공하며, 저장 시 다시 canonical source를 거쳐 `.wsnbody`로 serialize한다. 거터의 실제
  row 개수는 session의 parsed source line count만 사용하며, QML `TextEditor` wrapper는 해당 source line의 렌더
  위치만 제공할 수 있다.
- 현재: `component/ResourceFrame`은 standalone `<resource ... />` 라인을 Figma `292:50` 기준의 editor resource
  frame으로 렌더링한다. 이 frame은 source marker로 감싼 단일 image object이며 editor width 100%를 채운다.
  상단 resource type과 하단 resource file name은 입력란이 아닌 표시란이고 cached preview bitmap 안에 그려진다.
  Qt RichText 저장 왕복에서 이전 frame chrome 텍스트가 source에 섞이지 않도록 legacy 복원용 텍스트 목록도 함께
  제공한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
