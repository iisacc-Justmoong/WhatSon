# `src/app/models/hierarchy/bookmarks/BookmarksHierarchyController.cpp`

<a id="implementation-notes"></a>

## 구현 노트
- `setSystemCalendarStore(...)`는 이제 `ISystemCalendarStore`를 통해 바인딩됩니다.
- 로캘/날짜 형식 상태가 변경되면 책갈피 메모 목록 새로 고침이 계속 발생합니다.
- 계층 구조에 보이는 행이 있는 경우, 부정적이거나 잘못된 선택된 인덱스는 북마크 노트 목록이 새로 고쳐지기 전에 첫 번째 보이는 행으로 정규화되어 필터가 사이드바의 활성 행과 정렬됩니다.
- 북마크 행 투영은 메타데이터 전용입니다. 본문 상태 적용 및 편집기 stat-리프레시 훅은 노트 편집기/세이브 경계와 함께 제거되었습니다.
- `setItemExpanded(...)`는 공유된 체브론 검증/상태 플립을 `IHierarchyController`에 위임한 다음, `syncModel()`를 통해 북마크 프로젝션을 다시 게시합니다. 이렇게 하면 북마크 버킷 행이 다른 계층 도메인과 동일한 단일 행 체브론 확장 계약에 유지됩니다.
