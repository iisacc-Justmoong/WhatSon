# `src/app/qml/view/panels/navigation/view/NavigationApplicationViewOptionBar.qml`

<a id="responsibility"></a>

## 책임
`NavigationApplicationViewOptionBar.qml`는 보기 모드 `ApplicationViewBar` 내부의 Figma `258:7963` `ViewOptionBar` 슬라이스를 소유합니다.

<a id="figma-mapping"></a>

## Figma 매핑
- 프레임: `258:7963` `ViewOptionBar`
- 버튼 순서:
  - `258:7964` `ReadOnlyToggle` -> `readerMode`
  - `258:8048` `WarpText` -> `textAutoGenerate`
  - `258:8039` `CenterView` -> `recursiveMethod`
  - `258:7965` `TextToSpeech` -> `textToSpeech`
  - `258:7966` `PaperOption` -> `fileFormat`

<a id="interaction-contract"></a>

## 상호작용 계약
- 루트 유형: `LV.HStack`
- `viewHookRequested(string reason)` 및 `requestViewHook(reason)`를 노출합니다.
- 로컬 센터 뷰 버튼 ID: `centerViewOptionButton`.
- `TextToSpeech`와 `PaperOption`는 `LV.IconMenuButton`를 의도적으로 사용합니다. 이는 Figma 메타데이터에 두 컨트롤 모두에 대한 트레일링 드롭다운 어포던스가 포함되어 있기 때문입니다.

<a id="regression-checklist"></a>

## 회귀 체크리스트
- 5개의 버튼을 정확한 Figma 메타데이터 순서로 유지하세요.
- 아이콘 이름 계약 `readerMode -> textAutoGenerate -> recursiveMethod -> textToSpeech -> fileFormat`를 유지합니다.
- `LV.Theme.gap2` 간격과 메뉴 버튼 `left=2 / right=4 / top=2 / bottom=2` 패딩 계약을 유지합니다.
