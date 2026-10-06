# `src/app/models/file/note/header/WhatSonNoteHeaderStore.hpp`

<a id="responsibility"></a>

## 책임

`WhatSonNoteHeaderStore`는 `.wsnhead` 메타데이터에 대한 정규화된 변경 가능 컨테이너입니다.

<a id="activity-metadata"></a>

## 활동 메타데이터

- `lastOpenedAt() / setLastOpenedAt(...)`는 지속되는 RAW 노트 오픈 타임스탬프를 저장합니다.
- 값은 숫자 `fileStat` 블록 내부가 아닌 다른 최상위 수명 주기 메타데이터와 함께 존재합니다.
- 이 필드가 도입된 이후 한 번도 열린 적이 없는 노트에는 빈 값이 유효합니다.

<a id="folder-binding-api"></a>

## 폴더 바인딩 API

스토어는 이제 3 수준의 폴더 액세스를 노출합니다.

- `folders() / setFolders(...)`: 레거시 경로 전용 액세스
- `folderUuids() / setFolderUuids(...)`: 원시 안정적인 폴더 ID 액세스
- `setFolderBindings(folders, folderUuids)`: 최신 발신자를 위한 정렬된 쓰기 API

최신 코드에서는 두 어레이가 동기화 상태를 유지하도록 `setFolderBindings(...)`를 선호해야 합니다.

<a id="progress-metadata-api"></a>

## 진행 메타데이터 API

- `progressEnums() / setProgressEnums(...)`는 현재 `.wsnhead` `<progress enums="{...}">` 속성에 선언된 정확한 열거 라벨을 수행합니다.
- `progress() / setProgress(...)`는 선택한 정수 값만 계속 저장합니다.
- 메모리 내 기본값은 `-1`이며 이는 `No progress`를 나타냅니다.
- 열거형 레이블 배열과 선택된 정수는 의도적으로 분리되어 호출자가 사용자 정의 노트 로컬 진행 분류를 유지하면서도 활성 선택을 변조할 수 있도록 합니다.

<a id="file-statistics-api"></a>

## 파일 통계 API

`WhatSonNoteHeaderStore`는 이제 Figma 세부 통계 패널을 뒷받침하는 숫자 `fileStat` 메타데이터 블록도 소유합니다.

- 헤더 파생 카운터:
  - `totalFolders`
  - `totalTags`
- 본문 파생 카운터:
  - `letterCount`
  - `wordCount`
  - `sentenceCount`
  - `paragraphCount`
  - `spaceCount`
  - `indentCount`
  - `lineCount`
  - `backlinkToCount`
  - `backlinkByCount`
  - `includedResourceCount`
- 런타임 카운터:
  - `openCount`
  - `modifiedCount`

모든 카운터는 음수가 아닌 정수로 고정됩니다. `incrementOpenCount()` 및 `incrementModifiedCount()`는 수명 주기 추적을 위한 돌연변이 진입점입니다.

<a id="persistence-shape"></a>

## 지속성 형태

직렬화 시간에 폴더 바인딩은 다음과 같이 작성됩니다.

```xml
<folders>
  <folder uuid="64-char-id">Research/Competitor</folder>
</folders>
```

텍스트 노드는 읽기 가능한 경로로 유지됩니다. `uuid` 속성은 안정적인 의미적 정체성을 전달합니다.

이제 수명 주기 헤더 메타데이터에는 다음도 포함됩니다.

```xml
<lastModified>2026-04-18T09:00:00Z</lastModified>
<lastOpened>2026-04-18T09:03:15Z</lastOpened>
```

통계 블록은 다음과 같이 직렬화됩니다.

```xml
<fileStat>
  <totalFolders>0</totalFolders>
  <openCount>0</openCount>
</fileStat>
```

<a id="main-collaborators"></a>

## 주요 협력자

- `WhatSonNoteHeaderParser.cpp`: XML에서 저장소를 채웁니다.
- `WhatSonNoteHeaderCreator.cpp`: 저장소를 XML로 다시 직렬화합니다.
- `WhatSonLibraryFolderHierarchyMutationService.cpp`: 폴더 트리가 변경되면 바인딩을 다시 작성합니다.
