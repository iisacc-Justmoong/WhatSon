# `src/app/models/hierarchy/folders/WhatSonFoldersHierarchyParser.cpp`

<a id="responsibility"></a>

## 책임

이 파서는 지속형 `Folders.wsfolders` 구조를 `WhatSonFolderDepthEntry` 행으로 변환합니다. 또한 UUID 지원이 존재하기 전에 저장된 이전 폴더 트리에 대한 호환성 업그레이드도 수행합니다.

<a id="accepted-uuid-keys"></a>

## 허용되는 UUID 키

파서는 폴더 행을 읽을 때 여러 필드 이름을 허용합니다.

- `uuid`
- `UUID`
- `folderUuid`

이렇게 하면 수동 데이터 정리 단계 없이 이전 실험과 부분적으로 마이그레이션된 파일을 읽을 수 있습니다.

<a id="upgrade-behavior"></a>

## 업그레이드 동작

- 폴더 행에 이미 유효한 64문자 영숫자 UUID가 포함된 경우 파서는 이를 유지합니다.
- 행에 UUID가 없거나 UUID가 유효하지 않은 경우 파서는 새 행을 합성합니다.
- 그럴 때마다 `outUuidMigrationRequired`가 설정되어 호출자가 업그레이드된 파일을 즉시 지속할 수 있습니다.

이렇게 하면 세션 로컬 임의 ID를 사용하여 앱을 종료하는 대신 UUID 마이그레이션이 명시적으로 이루어집니다.

<a id="structural-output"></a>

## 구조적 출력

구문 분석된 각 행은 다음을 반환합니다.

- 레거시 경로 ID,
- 폴더 라벨,
- 나무 깊이,
- 안정적인 UUID.

따라서 파서는 이름 변경이 안전한 돌연변이에 필요한 런타임 ID를 생성하는 동안 가독성을 위해 경로 인식을 유지합니다.

<a id="escaped-slash-canonicalization"></a>

## 이스케이프된 슬래시 정식화

- 이제 구문 분석된 행은 `label`를 하나의 계층 구조 수준에 대한 신뢰할 수 있는 리프 이름으로 처리합니다.
- 정규화 기간 동안 파서는 공유 폴더 경로 이스케이프 규칙을 사용하여 `entry.id` 를 `depth + parentPath + label` 에서 다시 구축합니다.
- 따라서 하나의 레이블 내의 리터럴 `/` 는 `entry.id` 내부의 `\/` 로 영속화되어 우연한 하위 계층 수준을 생성하지 않습니다.
- 이는 이미 저장된 폴더 행 중 JSON에 여전히 `"label": "Marketing/Sales"`와 `"depth": 0`와 같은 원시 슬래시 레이블이 포함되어 있는 경우, 해당 행은 정규 ID `Marketing\/Sales`를 가진 하나의 루트 노드로 다시 전송됩니다.
