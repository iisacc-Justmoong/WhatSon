# `src/app/qml/view/panels/ListBarLayout.qml`

<a id="responsibility"></a>

## 책임
`ListBarLayout.qml`는 메모 및 리소스 행에 대한 데스크탑 목록 화면입니다. 탭, 드래그, 컨텍스트 메뉴 및 뷰포트 상호 작용을 소유하고 노트 목록 검색은 C++ 브리지 개체에 위임된 상태로 유지됩니다.

<a id="composition-model"></a>

## 구성 모델
- 루트 뷰는 다중 선택 상태 및 수정자 해석을 `noteSelectionController`에 위임합니다.
- 대리인 및 피어 패널은 `requestNoteSelection(...)` 및 `syncSelectionFromCommittedState()`와 같은 래퍼 기능을 사용합니다.
- 시각적 대리인은 모델 계약에 따라 `NoteListItem`와 `ResourceListItem` 사이를 전환합니다.

<a id="viewport-contract"></a>

## 뷰포트 계약
- 표시되는 `ListView`는 `resolvedNoteListModel`에 직접 바인딩됩니다.
- 상위 쉘은 `noteListModel`를 제공합니다. 이 보기는 계층 구조 도구 모음 인덱스를 통해 재발견되지 않습니다.
- 뷰포트 모션은 `Flickable.StopAtBounds`, `LV.WheelScrollGuard` 및 재설정 후 복원 처리와 함께 데스크톱 좁은 단계 계약을 사용합니다.
- 드래그 미리보기 배지 텍스트, 행 간격 및 도구 모음 높이에서는 원시 시각적 리터럴 대신 LVRS 테마 토큰을 사용합니다.

<a id="interaction-contract"></a>

## 상호작용 계약
- `activateNoteIndex(index, noteId)`는 유일한 즉시 노트 활성화 경로입니다.
- 포인터 선택은 `requestNoteSelection(index, noteId, modifiers)`를 통해 라우팅됩니다.
- 데스크탑 내부 드래그는 즉각적입니다. 행 할당과 컨텍스트 작업은 동일한 대상 선택 규칙을 공유합니다.

<a id="tests"></a>

## 테스트
- `test/cpp/suites/qml_contents_view_tests.cpp` 및 선택 관련 C++ 회귀 테스트는 데스크탑 작업 공간 셸을 통해 목록 계약에 접근할 수 있도록 유지합니다.
