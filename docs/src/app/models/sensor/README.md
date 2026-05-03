# `src/app/models/sensor`

## Responsibility
Owns read-side hub inspection objects that derive sensor outputs from the unpacked `.wshub` filesystem layout.

## Scope
- Source directory: `src/app/models/sensor`
- Child files:
  - `MonthlyUnusedNote.hpp`
  - `MonthlyUnusedNote.cpp`
  - `UnusedNoteSensorSupport.hpp`
  - `UnusedNoteSensorSupport.cpp`
  - `UnusedResourcesSensor.hpp`
  - `UnusedResourcesSensor.cpp`
  - `WeeklyUnusedNote.hpp`
  - `WeeklyUnusedNote.cpp`

## Current Contract
- `UnusedNoteSensorSupport` owns the shared hub scan that walks unpacked `.wsnote` packages, parses
  `.wsnhead`, and derives the effective activity timestamp in this order:
  - `lastOpenedAt`
  - `createdAt`
  - `lastModifiedAt`
- `WeeklyUnusedNote` returns note ids whose effective activity timestamp is at least seven days old.
- `MonthlyUnusedNote` returns note ids whose effective activity timestamp is at least one calendar month old.
- Both note sensors surface the richer `unusedNotes` entry list for callers that need note paths, timestamps,
  and the fallback source that decided the inactivity window.
- `UnusedResourcesSensor` scans hub-local `.wsresource` packages and compares them against `<resource ... />`
  embeddings found in note `.wsnbody` documents.
- The sensor returns only packages that are present in the hub but unused by every note.
- The sensor treats RAW note source and package metadata as the source of truth; it does not inspect editor-side DOM
  projections.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/sensor`` (`docs/src/app/models/sensor/README.md`)
- 위치: `docs/src/app/models/sensor`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
