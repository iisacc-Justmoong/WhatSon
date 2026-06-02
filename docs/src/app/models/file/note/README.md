# `src/app/models/file/note`

## Status
- Directory mirror generated from the current `src` tree.
- Concrete note package CRUD, mounted-hub note mutation services, local note file store, note management coordinator,
  and note version/diff storage have been deleted.

## Scope
- Mirrored source directory: `src/app/models/file/note`
- Child directories with live source files: `body`, `folder`, `header`, `package`, `support`
- Child files at the source root: none

## Child Directories
- `body` - body parser/serializer helpers, semantic tag helpers, resource tag generation, web-link support, and
  markdown style metadata.
- `folder` - raw folder block inspection semantics only.
- `header` - `.wsnhead` creation, parsing, storage, bookmark color palette, and header-local metadata helpers.
- `package` - note header/body text bootstrap helpers that no longer create a package-suffix directory.
- `support` - shared iiXml document-tree helpers used by body/header parsers.

## Current Contract
- This shard keeps reusable note text/schema helpers only.
- Body persistence to a concrete note package is disabled and returns failure.
- Note creation, note deletion, note folder clearing, local file-store CRUD, editor note-management orchestration, and
  note version snapshot/diff persistence are not present.
- Code that needs a document model must define a new owner instead of reviving deleted helpers or adding QML
  compatibility wrappers.
- Automated C++ regression coverage lives in `test/cpp/suites/*.cpp` and now covers only retained helpers such as
  body resource tag generation and raw folder-block inspection.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: `src/app/models/file/note`
- 위치: `docs/src/app/models/file/note`
- 역할: 남은 note helper shard의 책임과 삭제된 package CRUD 경계를 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
