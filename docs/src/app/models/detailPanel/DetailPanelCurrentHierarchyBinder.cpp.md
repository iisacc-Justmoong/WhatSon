# `src/app/models/detailPanel/DetailPanelCurrentHierarchyBinder.cpp`

<a id="runtime-behavior"></a>

## 런타임 동작
- 삽입된 계층 구조 컨텍스트 소스에서 `activeBindingsChanged()`에 연결합니다.
- 활성 노트 목록 모델과 계층 디렉터리 분석기를 `NoteDetailPanelController`로 동기화합니다.
- 활성 계층 인덱스가 `Resources`일 때를 감지하고, 그때서야 활성 메모 목록 모델을 `ResourceDetailPanelController::setCurrentResourceListModel(...)`로 전달합니다.
- 활성 계층이 노트 지원 도메인으로 다시 전환되면 리소스-세부 사항 바인딩을 다시 지웁니다.
- 계층 구조 소스가 삭제되면 세부 패널 컨텍스트를 `nullptr` 모델로 재설정합니다.
