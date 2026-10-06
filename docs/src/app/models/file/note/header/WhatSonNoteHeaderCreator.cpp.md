# `src/app/models/file/note/header/WhatSonNoteHeaderCreator.cpp`

<a id="responsibility"></a>

## 책임

이 파일은 `WhatSonNoteHeaderStore`를 `.wsnhead` XML로 다시 직렬화합니다.

<a id="folder-serialization-rules"></a>

## 폴더 직렬화 규칙

- 각 폴더 바인딩은 `<folder>` 요소로 내보내집니다.
- 유효한 UUID가 동일한 인덱스에 있으면 직렬화기는 이를 `uuid="..."`로 씁니다.
- 요소 본문은 읽을 수 있는 폴더 경로를 유지합니다.

이 이중 표현을 사용하면 응용 프로그램이 여전히 안정적인 시스템 ID에 의존하는 동안 사람이 메모 헤더를 검사할 수 있습니다.

<a id="output-expectations"></a>

## 출력 기대치

발신자는 정규화된 데이터를 가지고 도착해야 합니다. 작성자는 경로에서 누락된 UUID를 추론하려고 시도하지 않습니다. 해당 책임은 현재 폴더 트리를 알고 있는 상위 수준 서비스에 속합니다.

이제 생성자는 최상위 라이프사이클 메타데이터에서 `<lastOpened>...</lastOpened>`를 내보내므로 RAW 노트 헤더는 `lastModified`와 독립적으로 지속되는 마지막 열기 타임스탬프를 전달합니다.

<a id="scaffold-path-policy"></a>

## 비계 경로 정책

- `requiredRelativePaths()`는 의도적으로 비어 있습니다.
- 헤더 생성에는 더 이상 패키지 로컬 `.meta` 하위 디렉터리가 필요하거나 생성되지 않습니다.

<a id="progress-serialization-rules"></a>

## 진행 직렬화 규칙

- 직렬화기 는 이제 `<progress enums="...">` 속성에 `WhatSonNoteHeaderStore::progressEnums()` 를 보존하므로, 기본 `Ready/Pending/InProgress/Done` 세트로 다시 작성되지 않는 사용자 정의 노트 로컬 진행 라벨 왕복 변환 를 유지합니다.
- 음수가 아닌 진행률 값은 `<progress>` 요소 본문으로 직렬화됩니다.
- 지워진 진행 상태(`-1`)는 빈 `<progress ...></progress>` 요소로 직렬화되므로 파서는 `0`로 다시 강제하는 대신 진행 없음을 왕복 변환할 수 있습니다.

<a id="file-statistics-serialization-rules"></a>

## 파일 통계 직렬화 규칙

- 직렬화기는 이제 `.wsnhead <head>` 바로 아래에 전용 `<fileStat>` 블록을 방출합니다.
- 모든 통계는 값이 `0`일 때도 명시적으로 작성되므로, 새로운 메모는 안정적인 스키마로 시작하고 오래된 독자는 블록을 안전하게 무시할 수 있습니다.
- 현재 블록에는 다음이 포함되어 있습니다.
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
