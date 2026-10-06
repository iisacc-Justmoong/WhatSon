# `src/app/models/detailPanel/ResourceDetailPanelController.cpp`

<a id="runtime-behavior"></a>

## 런타임 동작
- `setCurrentResourceListModel(QObject*)`를 통해서만 리소스 목록 모델을 허용합니다.
- 주입된 개체가 `ResourcesListModel`인 경우에만 컨텍스트를 연결된 것으로 처리합니다.
- `ResourcesListModel::currentResourceEntry()`를 내보낸 `currentResourceEntry` 스냅샷으로 미러링합니다.
- 활성 리소스 행이 변경될 때 항목 변경 사항을 다시 내보내고, 리소스 목록 모델이 제거되거나 파괴될 때 스냅샷을 초기화합니다.
