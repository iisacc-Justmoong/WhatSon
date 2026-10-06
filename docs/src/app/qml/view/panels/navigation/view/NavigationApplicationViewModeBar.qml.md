# `src/app/qml/view/panels/navigation/view/NavigationApplicationViewModeBar.qml`

<a id="responsibility"></a>

## 책임
`NavigationApplicationViewModeBar.qml`는 보기 모드 응용 프로그램 도구 모음에 대한 Figma `258:7849` `ModeBar` 슬라이스를 소유합니다.

<a id="figma-mapping"></a>

## Figma 매핑
- 프레임: `258:7849` `ModeBar`
- 버튼 순서:
  - `258:7852` `CenterView` -> `singleRecordView`
  - `258:7853` `FocusMode` -> `imagefitContent`
  - `258:7854` `Presentation` -> `runshowCurrentFrame`

<a id="interaction-contract"></a>

## 상호작용 계약
- 루트 유형: `LV.HStack`
- `viewHookRequested(string reason)` 및 `requestViewHook(reason)`를 노출합니다.
- 모든 3 작업은 테두리 없는 톤과 `gap2` 패딩이 적용된 기본 `LV.IconButton` 컨트롤이며, 따라서 메타데이터 수준 아이콘 계약이 QML에서 명시적으로 유지됩니다.

<a id="regression-checklist"></a>

## 회귀 체크리스트
- 모드 버튼 순서 `CenterView -> FocusMode -> Presentation`를 유지하세요.
- `FocusMode` 아이콘을 `imagefitContent`에서 바꾸지 마십시오.
