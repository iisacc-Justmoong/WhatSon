# `src/app/models/file/hub/WhatSonHubParser.cpp`

<a id="responsibility"></a>

## 책임

이 파일은 `.wshub` 패키지용 부트스트랩 파서입니다. 지속형 허브 파일을 읽고 상위 수준 저장소 및 컨트롤러에서 사용되는 메모리 내 런타임 페이로드를 생성합니다.

<a id="folder-hierarchy-output"></a>

## 폴더 계층 출력

폴더 계층의 경우 파서는 이제 안정적인 `uuid` 필드를 포함하여 전체 `WhatSonFolderDepthEntry` 계약을 전달합니다. 따라서 라이브러리 계층 구조 컨트롤러 또는 돌연변이 서비스가 노트 필터링을 시작하기 전에 시작하는 동안 즉시 UUID ID를 사용할 수 있습니다.

<a id="compatibility-notes"></a>

## 호환성 참고 사항

- 폴더 UUID가 없는 레거시 허브 패키지는 하위 레벨 폴더 파서가 업그레이드하기 때문에 여전히 허용됩니다.
- 새 허브 패키지는 UUID를 끝까지 보존하므로, 한 세션에서 수행된 폴더 이름 변경은 다음 세션에서 삭제 및 재생성 이벤트가 아니라 이름 변경으로 유지됩니다.

<a id="resource-counting-rule"></a>

## 자원 계산 규칙

리소스 도메인은 이제 raw file 개수가 아니라 flat `.wsresource` package 개수를 센다.

- `resourceCount` fallback
- domain payload의 `resourcePaths`
- `resourceFileCount`

모두 허브 루트 `*.wsresources` 디렉터리의 직계 패키지 기준으로 계산된다.
