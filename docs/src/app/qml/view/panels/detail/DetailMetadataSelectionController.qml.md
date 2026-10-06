# `src/app/qml/view/panels/detail/DetailMetadataSelectionController.qml`

<a id="responsibility"></a>

## 책임

이 도우미는 세부 정보 패널에서 사용되는 압축 메타데이터 목록에 대한 수정자 인식 선택 상태를 소유합니다.

<a id="selection-rules"></a>

## 선택 규칙

- 일반 클릭을 하면 선택 항목이 단일 행으로 축소되고 해당 행이 섹션의 기본 `activeIndex`로 승격됩니다.
- `Cmd/Ctrl + click`는 클릭한 행을 시각적 선택 세트 안팎으로 전환합니다.
- `Shift + click`는 `selectionAnchorIndex`에서 클릭한 행까지 연속된 범위를 선택합니다.
- `Cmd/Ctrl + Shift + click`는 연속 범위를 기존 선택 세트와 통합합니다.

<a id="primary-selection-contract"></a>

## 1차 선정 계약

- 상세 패널은 여전히 속성 컨트롤러를 통해 하나의 커밋 메타데이터 인덱스(`activeFolderIndex` / `activeTagIndex`)만 노출합니다.
- 따라서 이 컨트롤러는 시각적 다중 선택을 위해 QML -local `selectedIndices` 배열을 유지하지만, 항상 기본 행을 `section.itemTriggered(...)`를 통해 다시 라우팅하므로 삭제 및 기타 활성 행 작업이 단일 커밋된 인덱스에서 계속 작동합니다.
- 커밋된 활성 행이 현재 시각 세트에서 제거되면, 컨트롤러는 마지막으로 남은 선택된 행을 새로운 기본 활성 행으로 승격합니다.

<a id="modifier-recovery"></a>

## 수정자 회복

- 도우미는 기존 사이드바/목록 표시줄 패턴을 미러링하여 짧은 창에 대한 누르기 시간 포인터 수정자를 캐시합니다.
- 이렇게 하면 포인터업 후 LVRS 클릭 콜백이 도착할 때 빠른 `Cmd/Ctrl` 또는 `Shift` 클릭이 일반 클릭으로 저하되는 것을 방지합니다.

<a id="tests"></a>

## 테스트

- 현재 이 저장소에는 자동화된 테스트 파일이 없습니다.
- 회귀 체크리스트:
  - 메타데이터 행을 일반 클릭하면 선택된 행이 정확히 하나만 남아 있어야 합니다.
  - `Cmd/Ctrl + click`는 다른 행을 유지하면서 대상 행만 전환해야 합니다.
  - `Shift + click`는 앵커 행에서 연속 범위를 선택해야 합니다.
  - 수정자 기반 다중 선택 후 일반 클릭은 목록을 선택한 단일 행으로 다시 축소해야 합니다.
