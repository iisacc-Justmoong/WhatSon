# `src/app/qml/view/panels/detail/DetailMetadataHierarchyPicker.qml`

<a id="responsibility"></a>

## 책임
`DetailMetadataHierarchyPicker.qml` 가 `DetailContents.qml` 에 의해 사용되는 공유 폴더/태그 추가 오버레이를 소유합니다. `LV.ContextMenu` 를 상속하며, LVRS 컨텍스트 메뉴 컨트랙트를 그대로 유지하고, 완전히 확장된 계층 구조 항목 목록으로 메뉴 본문만 오버라이드합니다. 컨테이너 기하학은 여전히 활성 폼 팩터에 적응합니다:
- 데스크탑: 고정된 컨텍스트 메뉴 스타일 팝업

<a id="inputs"></a>

## 입력
- `emptyStateText`
- `manualFallbackText`
- `hierarchyItems`
- `manualFallbackEnabled`

<a id="behavior"></a>

## 행동
- `openForAnchor(anchorItem, reason)`는 재정의된 `LV.ContextMenu`를 열고 후크 이유를 상위 항목에 다시 보고합니다.
- 구성요소는 독립형 `Controls.Popup`를 래핑하지 않습니다. 대신 LVRS 컨텍스트 메뉴 애니메이션, 해제,
- 계층형 행은 `LV.HierarchyItem` 위임자 형태로 메뉴 본문 내부에 직접 렌더링되므로, 팝업은 다른 계층 패널 표면을 삽입하는 대신 컨텍스트 메뉴 쉘을 유지합니다.
- 메뉴 본문은 `Controls.ScrollBar`를 사용합니다. 이 파일은 `QtQuick.Controls`를 `Controls` 별칭으로 가져오며, QML 모듈 내부에서 로드 가능하도록 해당 유형 해상도를 명시해야 합니다.
- 제공된 모든 계층 항목은 이미 확장된 것으로 간주되며, 접이식이 불가능합니다: 선택기는 체브론을 전혀 표시하지 않으며 소스 계층 확장 상태를 변경하지 않습니다.
- 계층 행을 클릭하면 필터링된 선택기 항목이 해결되고, 메모 메타데이터를 로컬에서 변형하는 대신 `entryChosen(entry)`가 출력됩니다.
- `manualFallbackEnabled == true`일 때, 푸터 행은 `manualFallbackRequested()`를 출력하고 부모가 인라인 폴더 편집기를 다시 열 수 있도록 합니다.
- 팝업 여백, 행 높이 및 대체 경로 너비는 이제 명명된 `LV.Theme` 간격/제어 너비 토큰을 사용합니다.

<a id="tests"></a>

## 테스트

- 현재 이 저장소에는 자동화된 테스트 파일이 없습니다.
- 회귀 체크리스트:
  - 바닥글 추가 버튼이 오른쪽이나 아래쪽 가장자리 근처에 있는 경우에도 데스크탑 팝업 형상이 오버레이 내부에 고정되어야 합니다.
  - 선택기는 갈매기형 표시나 축소 어포던스 없이 소스 순서대로 모든 계층 노드를 즉시 렌더링해야 합니다.
  - 렌더링된 계층 구조 행을 클릭하면 `entryChosen(...)`를 내보내고 메뉴를 닫아야 합니다.
  - 팝업을 닫으면 다음 열기 전에 임시 앵커 상태를 지워야 합니다.
