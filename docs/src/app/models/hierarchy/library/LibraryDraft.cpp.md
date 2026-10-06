# `src/app/models/hierarchy/library/LibraryDraft.cpp`

<a id="implementation-notes"></a>

## 구현 노트

- `matches(...)`는 이제 전체 재구축과 증분 노트 업데이트 모두에서 사용되는 공유 초안 멤버 자격 조건자입니다.
- `upsertNote(...)` 는 구조적 무작위 작업 감지를 수행하여 실제로 저장된 초안 기록을 변경하지 않는 본문 저장이 가짜 "changed" 신호 상위 공급 측 를 전파하지 않도록 합니다.
- `removeNoteById(...)`는 전체 파생 벡터를 교체하지 않고도 드래프트 버킷 가지치기를 허용합니다.
