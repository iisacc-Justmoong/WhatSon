# `src/app/models/hierarchy/bookmarks/BookmarksHierarchyController.hpp`

<a id="role"></a>

## 역할
`BookmarksHierarchyController`는 북마크 폴더와 북마크 메모 목록을 프로젝트합니다.

<a id="interface-alignment"></a>

## 인터페이스 정렬
- 이제 시스템 달력 연결이 `ISystemCalendarStore`를 대상으로 합니다.
- 북마크 계층 구조가 이제 `IHierarchyExpansionCapability`를 구현하므로, 공유 사이드바 체브론 브리지는 LVRS 행 상태를 되돌리는 대신 북마크 버킷 행에 대한 접기/펼침 요청을 커밋할 수 있습니다.
- 신체 상태 적용 및 편집기 stat-refresh API가 없습니다. 북마크 노트 목록은 메타데이터 예측으로 유지됩니다.
