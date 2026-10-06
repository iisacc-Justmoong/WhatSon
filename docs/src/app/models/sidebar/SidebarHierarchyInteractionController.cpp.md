# `src/app/models/sidebar/SidebarHierarchyInteractionController.cpp`

<a id="implementation-notes"></a>

## 구현 노트
- 푸터 작업 정규화와 디스패치가 여기에서 구현되지 않았습니다. Live `LV.ListFooter`는 `SidebarHierarchyView.qml` 내부에서 직접 디스패치를 클릭하므로 생성/삭제/옵션이 컨트롤러 신호 왕복 변환에 의존하지 않습니다.
- 확장 키는 `hierarchy:<activeIndex>:<stableKey>` 로 범위 지정되어, 계층 도메인 간에 동일한 행 ID가 새어나가지 않도록 모델 새로 고침 전반에 걸쳐 행 상태를 유지합니다.
- 단일 행 및 대량 확장 요청은 메타 객체 호출에 의해 바인딩된 `HierarchyInteractionBridge`를 거쳐, 브리지가 변이를 거부하면 보존된 상태를 롤백합니다.
- `armExpansionForItem(...)`와 `requestChevronExpansionForItem(...)`는 QML에 대한 선호되는 오른쪽 체브론 진입점입니다. 그들은 C++ 내부의 안정적인 확장 키와 현재 `expanded` 값을 도출하여, 뷰가 히트 테스트된 항목과 해결된 모델 인덱스만 제공하도록 합니다.
- 확장 커밋이 성공적으로 완료된 후, 컨트롤러는 커밋 결과만 반환합니다. 푸터 액션 신호나 선택된 행 동기화 신호를 발생시키지 않으며, `SidebarHierarchyView.qml`는 다음 차례 `syncSelectedHierarchyItem(false)` 호출을 `Qt.callLater(...)`와 함께 예약합니다.
- `setHierarchyInteractionBridge(...)`는 허용된 View -> Controller edge를 확인하지만 시작 mutable-wiring 잠금은 적용하지 않습니다. 이 컨트롤러는 QML와 `SidebarHierarchyView`가 함께 생성되었습니다. 따라서 여기에서 잠금을 적용하면 `main.cpp`가 루트 객체 그래프를 고정한 후 체브론 확장 경로가 바인드 브리지 없이 남게 됩니다.
- 안정 키에 대한 첫 번째 `onListItemExpanded` 콜백은 실제 확장 요청으로 간주되어 별도의 포인터 암이 실행되지 않았더라도 즉시 커밋됩니다. 이는 LVRS 체브론 `MouseArea` 경로를 다루며, 해당 행이 클릭을 소유한 후 사이드바 수준 대체 경로가 탭을 확인합니다.
- 확장 커밋은 대기 중인 활성화 시도를 무효화하고 짧은 억제 타이머를 시작하며, 이전의 체브론 클릭 동작과 일치시키면서 정책을 C++에 유지합니다.
