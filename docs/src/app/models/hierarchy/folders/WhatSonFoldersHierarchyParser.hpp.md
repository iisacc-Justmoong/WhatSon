# `src/app/models/hierarchy/folders/WhatSonFoldersHierarchyParser.hpp`

<a id="responsibility"></a>

## 책임

이 헤더는 지속된 `Folders.wsfolders` 콘텐츠에 대한 구문 분석기를 선언합니다.

<a id="public-contract"></a>

## 공공 계약

`parse(...)`는 원시 폴더 텍스트를 허용하고 정규화된 `WhatSonFolderDepthEntry` 행으로 `WhatSonFoldersHierarchyStore`를 채웁니다.

현대 계약에서는 `outUuidMigrationRequired`도 공개합니다.

- `false`는 구문 분석된 모든 행에 이미 유효한 폴더 UUID가 있음을 의미합니다.
- `true`는 파서가 파일이 레거시이거나 형식이 아니었기 때문에 최소 하나의 UUID를 합성해야 함을 의미합니다.

소스 파일 경로를 소유한 호출자는 마이그레이션이 필요할 때 폴더 파일을 다시 작성하여 UUID ID가 세션 전반에 걸쳐 지속되도록 해야 합니다.

<a id="main-collaborators"></a>

## 주요 협력자

- `WhatSonFoldersHierarchyStore`: 정규화된 구문 분석 행을 수신합니다.
- `LibraryHierarchyController.cpp`: 직접 라이브러리 로드 중에 마이그레이션된 UUID를 유지합니다.
- `WhatSonRuntimeDomainSnapshots.cpp`: 시작 스냅샷 로드 중에 마이그레이션된 UUID를 유지합니다.
