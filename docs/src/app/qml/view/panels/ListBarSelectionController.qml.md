# `src/app/qml/view/panels/ListBarSelectionController.qml`

<a id="responsibility"></a>

## 책임

`ListBarSelectionController.qml`는 `ListBarLayout.qml`에 대한 노트 목록 다중 선택 상태 및 수정자 해석을 소유합니다.

컨트롤러는 선택 앵커 기록을 유지하고 수정자 페이로드를 정규화하며 포인터 활성화가 현재 노트 선택을 대체, 확장 또는 토글해야 하는지 결정합니다.

<a id="public-contract"></a>

## 공공 계약

- `selectionAnchorIndex`: 클릭 시퀀스 전반에 걸쳐 범위 선택 앵커가 유지됩니다.
- `selectedIndices`: 호스트 보기에서 사용되는 정규화된 시각적 다중 선택 세트입니다.
- `requestNoteSelection(index, noteId, modifiers)`: 포인터 활성화를 위한 기본 선택 진입점입니다.
- `syncSelectionFromCommittedState()`: 모델 인증 현재 인덱스에서 선택 항목을 다시 수화합니다.

<a id="selection-rules"></a>

## 선택 규칙

- `normalizedKeyboardModifiers(...)`는 이벤트 수정자를 `Qt.application.keyboardModifiers`와 병합합니다.
- `Shift`는 `selectionAnchorIndex`에서 선택을 확장합니다.
- `Cmd/Ctrl`는 모델 인증 현재 메모를 잃지 않고 멤버십을 전환합니다.
- `Cmd/Ctrl + Shift`는 현재 선택 항목을 요청된 앵커 범위와 통합합니다.
- 토글된 로우가 커밋된 현재 노트인 경우, 컨트롤러는 최신 생존 행을 대체 경로 기본 선택으로 활성화합니다.

<a id="host-dependency-direction"></a>

## 호스트 종속성 방향

- 컨트롤러는 `view.normalizeCurrentIndex(...)`, `view.currentIndexFromModel()` 및 `view.activateNoteIndex(...)`에 의존합니다.
- 호스트 뷰는 래퍼 함수를 유지하여 기존 델리게이트가 동일한 API 서피스를 호출할 수 있도록 하며, 선택 로직은 별도의 형제 객체에 존재합니다.
