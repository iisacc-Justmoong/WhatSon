# `src/app/models/sidebar/SidebarHierarchyController.cpp`

<a id="implementation-notes"></a>

## 구현 노트
- 생성자는 이제 `IActiveHierarchySource` 베이스를 초기화합니다.
- 셀렉션 스토어 소유권과 공급자 연결은 동일하게 유지됩니다. 활성 바인딩 알림 표면만 커졌습니다.
- 활성 계층 인터페이스는 이제 뷰/패널 바인딩 코드에만 사용됩니다; 시작 런타임 로딩은 더 이상 사이드바 활성화를 지연된 부트스트랩 트리거로 사용하지 않습니다.
- 일관된 "toolbar index + hierarchy controller + note-list model" 튜플이 필요한 활성 계층형 소비자는 이제 `activeBindingsChanged()`를 구독할 수 있습니다. 이 구현은 선택 변경 및 제공자 매핑 새로 고침 후 합성 신호를 발생시키므로, QML 셸은 한 번에 전체 활성 바인딩 스냅샷을 새로 고침하고 계층 구조 전환 시 일시적인 교차 도메인 목록 유령을 방지할 수 있습니다.
- `hierarchyControllerForIndex(...)`와 `noteListModelForIndex(...)`는 이제 반환된 `QObject*` 바인딩에 대해 QML 경계를 넘기 전에 `QQmlEngine::CppOwnership`를 강제합니다. 이렇게 하면 계층 구조 스위치 스냅샷이 멤버가 소유한 C++ 객체를 마치 일회용 JS 소유 인스턴스인 것처럼 QML 가비지 컬렉터에 전달되는 것을 방지합니다.
