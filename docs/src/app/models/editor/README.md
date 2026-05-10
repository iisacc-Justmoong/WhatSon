# `src/app/models/editor`

## Responsibility
Owns C++ editor-domain model objects that are intentionally outside QML view composition.

## Scope
- Source directory: `src/app/models/editor`
- Build shard: `src/app/models/editor/CMakeLists.txt`
- Child files:
  - `SetTag.h`
  - `SetTag.cpp`

## Current Contract
- The directory is registered through its own CMake shard and the root app target reaches it with
  `add_subdirectory(models/editor)`.
- Editor-domain C++ belongs here only when it is backend model/controller logic rather than view-local QML behavior.
- `SetTag` is the static `.wsnbody` RAW tag input object. It exposes a fixed allow-list of body tag templates and can
  insert them into editor source text or into a serialized `.wsnbody` document via `WhatSon::NoteBodyPersistence`.
- Minimap display backends, parser/projection/rendering pipelines, persistence sessions, and legacy editor view-mode
  controllers remain outside this shard unless a new documented contract explicitly reintroduces them.

## Verification Notes
- Source-tree policy coverage verifies that this shard is present, documented, and registered through
  `src/app/models/editor/CMakeLists.txt` rather than direct file entries in `src/app/CMakeLists.txt`.
- Runtime C++ coverage verifies `SetTag` source insertion, unsupported tag rejection, and `.wsnbody` reserialization.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/editor`` (`docs/src/app/models/editor/README.md`)
- 위치: `docs/src/app/models/editor`
- 역할: 이 파일은 editor model shard의 구조, 책임, CMake 등록 계약, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 현재: `SetTag`는 정적으로 허용된 `.wsnbody` RAW 태그만 삽입하는 C++ 입력 객체다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
