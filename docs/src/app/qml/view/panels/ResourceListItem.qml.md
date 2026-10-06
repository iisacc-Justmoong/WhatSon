# `src/app/qml/view/panels/ResourceListItem.qml`

<a id="responsibility"></a>

## 책임

`ResourceListItem.qml`는 리소스 목록 행을 위한 전용 오른쪽 패널 카드 구성 요소입니다. 의도적으로 `NoteListItem` 구조를 재사용하지 않고 Figma `232:7892` 형상 및 상태 색상을 격리된 계약으로 유지합니다.

<a id="source-metadata"></a>

## 소스 메타데이터
- 소스 경로: `src/app/qml/view/panels/ResourceListItem.qml`
- 소스 종류: QML 뷰/컴포넌트
- 파일 이름: `ResourceListItem.qml`
- 대략적인 줄 수: 91

<a id="qml-surface-snapshot"></a>

## QML 표면 스냅샷
- 루트 유형: `Item`

<a id="object-ids"></a>

### 개체 ID
- `resourceListItem`
- `resourceHoverHandler`

<a id="public-properties"></a>

### 공개 속성
- `active`
- `pressed`
- `previewSource`
- `titleText`

<a id="visual-contract"></a>

## 시각적 계약

- 너비는 원시 고정 픽셀 리터럴 대신 `LV.Theme.inputMinWidth + LV.Theme.gap14`를 사용합니다.
- 프레임 패딩은 `LV.Theme.gap8`를 사용합니다.
- 다음이 포함된 단일 가로 행:
  - `LV.Theme.gap24 + LV.Theme.gap24` 크기의 썸네일 프레임.
  - `LV.Theme.gap10` 크기의 행 간격입니다.
  - `LV.Theme.textBody` / `LV.Theme.textBodyLineHeight`가 포함된 세미볼드 제목 텍스트.
- 배경 상태:
  - 기본값: `LV.Theme.accentTransparent`
  - 호버/눌림: `LV.Theme.panelBackground06`
  - 활성: `LV.Theme.accentBlueMuted`
- 썸네일 자리 표시자는 `LV.Theme.strokeSoft`를 사용합니다.

<a id="integration"></a>

## 통합

- `ListBarLayout.qml` 스위치는 `resourceListMode`로 대표 구성을 전환합니다.
- 리소스 행은 `ResourceListItem`를 렌더링합니다. 리소스 메모가 아닌 행은 `NoteListItem`를 유지합니다.

<a id="tests"></a>

## 테스트

자동 테스트 파일이 이 저장소에서 제거되었습니다. 런타임 검사를 통해 구성요소 형상 및 위임 바인딩을 확인합니다.
