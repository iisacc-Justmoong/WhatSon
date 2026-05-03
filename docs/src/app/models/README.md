# `src/app/models`

## Responsibility
`src/app/models` owns QObject-based app-domain runtime objects. Former presentation-layer controllers now live in the
owning model domains instead of a separate view-model tree.

## Current Domains
- `agenda`: agenda parsing and source-rewrite helpers used by the structured editor.
- `calendar`: calendar board state and system-calendar integration.
- `callout`: callout parsing and insertion/rewrite helpers for the structured editor.
- `content`: display, mobile-navigation, and structured-document helpers that coordinate QML-facing content surfaces.
- `editor`: parser, renderer, text, bridge, and session helpers that project RAW note source into editable views.
- `file`: persistent hub/note/resource storage, validation, import/export, sync, and hierarchy backends.
- `display`: paper/page/print presentation helpers that support editor view modes without living in the QML layer.
- `sensor`: hub inspection objects that derive lightweight read-side facts such as unused resource packages.
- `detailPanel`, `navigationbar`, `onboarding`, `panel`, and `sidebar`: QML-facing controller objects that remain
  model-domain runtime objects and must not reintroduce MVVM.

## Dependency Direction
- Model objects may depend on Qt core/gui primitives and lower file-domain helpers.
- They must not take ownership of LVRS view composition or sidebar/detail-panel routing concerns.
- Sensor-style models should prefer RAW hub files (`.wsnbody`, `.wsresource`, `resource.xml`) as their read source.
- `content/mobile` helpers should remain route/state coordinators only; they must not absorb sidebar or editor
  ownership that belongs to the model-domain controller or RAW-source layers.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models`` (`docs/src/app/models/README.md`)
- 위치: `docs/src/app/models`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
