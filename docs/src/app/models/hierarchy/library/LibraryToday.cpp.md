# `src/app/models/hierarchy/library/LibraryToday.cpp`

<a id="implementation-notes"></a>

## 구현 노트

- `matches(...)`는 이제 전체 재구축과 증분 노트 업데이트 모두에서 사용되는 공유 오늘 멤버쉽 조건자입니다.
- `upsertNote(...)`는 구조적 무검출을 수행하여 변경되지 않은 메모 저장이 가짜 오늘-버킷 변이로 떠오르지 않도록 합니다.
- `removeNoteById(...)`를 사용하면 전체 파생 벡터를 교체하지 않고도 오늘 메모 하나를 정리할 수 있습니다.
