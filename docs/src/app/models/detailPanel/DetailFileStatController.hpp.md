# `src/app/models/detailPanel/DetailFileStatController.hpp`

<a id="responsibility"></a>

## 책임
`DetailFileStatController`는 `fileStat` 도구 모음 상태에 대한 전용 세부 정보 패널 통계 개체입니다. 숫자 `.wsnhead <fileStat>` 필드를 QML 친화적 속성으로 미러링하고 통계 보기에서 사용되는 Figma 모양의 텍스트 블록도 노출합니다.

<a id="exported-metric-properties"></a>

## 내보낸 메트릭 속성
- `totalFolders`
- `totalTags`
- `letterCount`
- `wordCount`
- `sentenceCount`
- `paragraphCount`
- `spaceCount`
- `indentCount`
- `lineCount`
- `openCount`
- `modifiedCount`
- `backlinkToCount`
- `backlinkByCount`
- `includedResourceCount`

<a id="figma-text-surface"></a>

## Figma 텍스트 표면
- `summaryLines`
  - `Projects: ...`
  - `Total folders: ...`
  - `Folders: ...`
  - `Total tags: ...`
  - `Tags: ...`
  - `Created at: ...`
  - `Modified at: ...`
- `textMetricLines`
  - `Letter: ...`
  - `Word: ...`
  - `Sentence: ...`
  - `Paragraph: ...`
  - `Space: ...`
  - `Indent: ...`
  - `Line: ...`
- `activityLines`
  - `Open count: ...`
  - `Modified count: ...`
  - `Backlink to: ...`
  - `Backlink by: ...`
  - `Include resources: ...`

<a id="grouped-qml-surface"></a>

## 그룹화된 QML 표면
- `overviewItems`: 폴더, 태그, 열기, 수정됨
- `textItems`: 문자, 단어, 문장, 단락, 공백, 들여쓰기, 줄
- `relationItems`: 링크 대상, 링크 대상, 리소스

<a id="lifecycle"></a>

## 수명주기
- `applyHeader(...)`는 현재 로드된 메모 헤더 스냅샷을 컨트롤러에 복사합니다.
- `clearHeader()`는 표면을 비어 있거나 해결되지 않은 노트에 사용되는 0상태 계약으로 다시 재설정합니다.
- 모든 업데이트는 `statsChanged()`를 발생하므로, QML는 동일한 소스 객체의 숫자 속성와 텍스트 블록을 모두 새로 고칠 수 있습니다.
