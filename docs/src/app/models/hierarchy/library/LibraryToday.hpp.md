# `src/app/models/hierarchy/library/LibraryToday.hpp`

<a id="responsibility"></a>

## 책임

`LibraryToday`는 파생된 "Today" 스마트 버킷을 소유합니다.

<a id="public-contract"></a>

## 공공 계약

- `rebuild(...)`: 정식 올노트 벡터에서 버킷을 다시 빌드합니다.
- `matches(...)`: 재구축 및 증분 돌연변이 경로 모두에서 사용되는 날짜-멤버십 규칙을 중앙 집중화합니다.
- `upsertNote(...)`: 오늘 날짜를 기준으로 하나의 메모를 재평가하고 버킷을 업데이트합니다.
- `removeNoteById(...)`: 오늘 전체 버킷 재구축 없이 하나의 노트를 정리합니다.

<a id="tests"></a>

## 테스트
- 현재 이 저장소에는 자동화된 테스트 파일이 없습니다.
- 회귀 체크리스트:
  - 유효한 오늘 멤버십이 변경되지 않는 메모를 저장한다고 해서 전체 `Today` 버킷 재구축이 강제로 실행되어서는 안 됩니다.
  - 오늘/오늘이 아닌 경계에 걸쳐 메모를 변경하는 것은 한 번의 `upsertNote(...)` 호출을 통해 표현할 수 있어야 합니다.
