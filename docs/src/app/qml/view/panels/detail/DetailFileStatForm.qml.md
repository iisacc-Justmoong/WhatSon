# `src/app/qml/view/panels/detail/DetailFileStatForm.qml`

<a id="lvrs-token-notes"></a>

## LVRS 토큰 노트
- 파일 상태 줄 대체 경로 너비와 섹션 간격은 `LV.Theme.gapNone`를 사용합니다.

<a id="responsibility"></a>

## 책임
`DetailFileStatForm.qml`는 이전 자리 표시자 대신 실제 `fileStat` 세부 패널 표면을 렌더링합니다.

<a id="input-contract"></a>

## 입력 계약
- `fileStatController`
  - 예상 유형: `DetailFileStatController`
  - 소비된 텍스트 컬렉션:
    - `summaryLines`
    - `textMetricLines`
    - `activityLines`

<a id="visual-structure"></a>

## 시각적 구조
- `LV.Theme.gap8` / `LV.Theme.gap2`에서 가져온 수평/상단 인세트가 있는 좁은 `Form` 열 1개
- 3 개의 `LV.Theme.gap10` 로 구분된 텍스트 그룹
- 렌더링된 모든 라인은 `LV.Label { style: description }`를 사용합니다.

이는 통계 패널이 카드 그리드가 아닌 일반 `description` 인쇄 표면인 Figma 노드 `235:7734`와 일치합니다.
