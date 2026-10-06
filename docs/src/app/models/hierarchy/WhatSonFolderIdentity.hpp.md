# `src/app/models/hierarchy/WhatSonFolderIdentity.hpp`

<a id="responsibility"></a>

## 책임

이 헤더는 구문 분석, 지속성 및 런타임 계층 구조 논리에 사용되는 공유 폴더-UUID 규칙을 정의합니다.

<a id="public-contract"></a>

## 공공 계약

- `kUuidLength`는 `64`로 고정되어 있습니다.
- `normalizeFolderUuid(...)`는 입력을 트리밍하고 정확히 64와 일치하지 않는 영숫자 값을 거부합니다.
- `isValidFolderUuid(...)`는 정규화에 대한 부울 편의 래퍼입니다.
- `createFolderUuid()`는 대문자, 소문자 및 숫자를 사용하여 무작위 64문자 식별자를 생성합니다.

<a id="design-notes"></a>

## 디자인 노트

- 이 형식은 의도적으로 파일 시스템에 안전하고 XML‐안전하도록 구성되어 있어, 추가 이스케이프 규칙 없이 `.wsfolders`, `.wsnhead` 및 메모리 내 Qt 문자열 사이를 이동할 수 있습니다.
- 도우미는 외부 상태가 없고 많은 하위 수준 파일에 필요하기 때문에 헤더 전용입니다.
- 생성된 UUID는 RFC-4122 값이 아닙니다. 프로젝트별 안정적인 폴더 키입니다.

<a id="main-call-sites"></a>

## 주요 통화 사이트

- `WhatSonFoldersHierarchyParser.cpp` 및 `WhatSonFoldersHierarchyStore.cpp`는 누락된 UUID를 정화하거나 대체합니다.
- `WhatSonNoteHeaderStore.cpp`는 노트 헤더에 기록된 폴더 바인딩을 정규화합니다.
- `LibraryHierarchyController.cpp`와 `WhatSonLibraryFolderHierarchyMutationService.cpp`는 헬퍼를 사용하여 이름 변경 및 이동 작업 전반에 걸쳐 런타임 아이덴티티를 안정적으로 유지합니다.
