# `src/app/models/detailPanel/DetailFileStatController.cpp`

<a id="responsibility"></a>

## 책임
이 구현은 `WhatSonNoteHeaderStore` 를 2 병렬 뷰 표면으로 변환합니다.
- 직접 바인딩을 위한 숫자 카운터
- `description` 스타일 통계 패널의 Figma 모양 텍스트 줄

<a id="figma-text-rules"></a>

## Figma 텍스트 규칙
- `summaryLines`는 지속된 헤더 메타데이터와 파생된 폴더/태그 합계를 병합합니다.
  - 프로젝트
  - 총 폴더
  - 폴더 목록
  - 총 태그
  - 태그 목록
  - 생성 시간
  - 수정된 시간
- `project` 요약 라인은 지속된 헤더 필드가 비어 있을 때 `No project`를 렌더링해야 하며, 이는 detail-panel 프로젝트 선택기가 사용하는 clear 옵션과 일치합니다.
- `textMetricLines`는 Figma에서 사용하는 단일 레이블에 정확히 텍스트 개수 행을 노출합니다.
  - `Letter`
  - `Word`
  - `Sentence`
  - `Paragraph`
  - `Space`
  - `Indent`
  - `Line`
- `activityLines`는 다음을 노출합니다.
  - `Open count`
  - `Modified count`
  - `Backlink to`
  - `Backlink by`
  - `Include resources`
- `Modified count`는 버전 차이가 있는 노트 패키지 커밋에 의해 뒷받침됩니다. 노트 업데이트는 카운터가 진행되기 전에 `.wsnversion`에 캡처된 직렬화된 헤더 페이로드 차이를 생성해야 하며, 따라서 상세 표면은 타임스탬프만 카운트하거나 변경되지 않은 세이브 턴을 계산하지 않습니다.

<a id="numeric-grouping-rules"></a>

## 숫자 그룹화 규칙
- `overviewItems`
  - `totalFolders`
  - `totalTags`
  - `openCount`
  - `modifiedCount`
- `textItems`
  - `letterCount`
  - `wordCount`
  - `sentenceCount`
  - `paragraphCount`
  - `spaceCount`
  - `indentCount`
  - `lineCount`
- `relationItems`
  - `backlinkToCount`
  - `backlinkByCount`
  - `includedResourceCount`

<a id="qml-payload-shape"></a>

## QML 페이로드 형태
각 측정항목 행은 다음을 사용하여 컴팩트 맵으로 내보내집니다.
- `key`
- `label`
- `value`

현재 QML 표면이 이제 일반 텍스트 Figma 레이아웃을 따르더라도 숫자로 그룹화된 페이로드는 향후 재사용을 위해 계속 사용할 수 있습니다.

<a id="tests"></a>

## 테스트

- 폴더 디스플레이는 이제 대체 경로 요약 텍스트를 렌더링하기 전에 이스케이프된 폴더 경로 세그먼트를 디코딩하므로, 하나의 폴더 라벨에 문자 그대로 `/`가 포함되어 있을 때 상세 통계는 `\/`와 같은 영속성 마커를 노출하지 않습니다.
- 유지 관리되는 C++ 회귀 제품군은 이 보기가 의존하는 공유 폴더 경로 이스케이프 의미 체계를 잠급니다.
- 회귀 체크리스트:
  - 빈 `.wsnhead <project>` 필드가 있는 메모는 파일 통계 요약에서 `Projects: No project`를 렌더링해야 합니다.
  - 편집기에 입력하고 성공적인 변경된 본문 저장을 기다리면 `modifiedCount`가 증가해야 합니다.
  - 변경되지 않은 조정/저장 회전은 `modifiedCount`를 증가해서는 안 됩니다.
  - 오픈 카운트 쓰기는 지속적인 헤더 업데이트가 성공한 후 표시된 `.wsnhead` 메타데이터를 강제로 새로 고쳐야 합니다.
