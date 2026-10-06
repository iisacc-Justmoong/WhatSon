# `src/app/models/hierarchy/library/LibraryDraft.hpp`

<a id="responsibility"></a>

## 책임

`LibraryDraft`는 파생된 "Draft" 스마트 버킷을 소유합니다.

<a id="public-contract"></a>

## 공공 계약

- `rebuild(...)`: 표준 전체 노트 벡터에서 초안 버킷을 다시 빌드합니다.
- `matches(...)`: 재구축 및 증분 돌연변이 경로 모두에서 사용되는 초안 멤버십 규칙을 중앙 집중화합니다.
- `upsertNote(...)`: 해당 규칙에 대해 하나의 메모를 재평가하고 버킷을 제자리에 업데이트합니다.
- `removeNoteById(...)`: 전체 버킷을 다시 작성하지 않고 하나의 초안 메모를 정리합니다.

<a id="tests"></a>

## 테스트
- 현재 이 저장소에는 자동화된 테스트 파일이 없습니다.
- 회귀 체크리스트:
  - 초안 멤버십에 영향을 주지 않는 본문 전용 메모 저장은 전체 `Draft` 버킷 재구축을 강제해서는 안 됩니다.
  - 업데이트된 메모가 더 이상 일치하지 않을 때 초안 멤버십을 제거하려면 `removeNoteById(...)`를 통해 라우팅되어야 합니다.
