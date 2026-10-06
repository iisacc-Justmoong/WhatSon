# `src/app/runtime/threading/WhatSonRuntimeDomainSnapshots.cpp`

<a id="responsibility"></a>

## 책임

이 파일은 기본 스레드에 대한 시작 시간 런타임 스냅샷을 빌드합니다. 작업자 스레드 친화적인 형식으로 노트 레코드, 스마트 버킷 예측 및 지속 계층 파일을 로드합니다.

`WhatSonLibraryIndexedState` 는 이제 라이브러리 측 노트 투영을 위한 공유 백엔드가 되었습니다. 스냅샷 로더는 해당 백엔드를 사용하여 `all`, `draft`, `today` 를 한 번에 물질화한 다음, 마운트된 허브를 다시 파싱하는 대신 이미 색인화된 라이브러리 노트 세트에서 북마크 도메인을 유도하는 `buildBookmarks(...)` 헬퍼를 노출합니다.

<a id="folder-snapshot-role"></a>

## 폴더 스냅샷 역할

라이브러리 폴더의 경우 스냅샷 로더는 `Folders.wsfolders`를 `WhatSonFolderDepthEntry` 행으로 구문 분석하고 파일 경로와 정규화된 폴더 항목을 모두 시작 파이프라인에 노출합니다.

<a id="uuid-migration-behavior"></a>

## UUID 마이그레이션 동작

이제 스냅샷 로더는 파서의 `outUuidMigrationRequired` 신호를 전달하고 레거시 파일에 지속된 UUID가 없을 때 즉시 `Folders.wsfolders`를 다시 작성합니다.

이는 직접 `LibraryHierarchyController::loadFromWshub()` 로드에 맞춰 시작 동작을 유지합니다.

- 레거시 폴더 트리는 안정적인 UUID를 한 번 수신합니다.
- 업그레이드된 파일은 유지됩니다.
- 다음 실행에서는 다른 메모리 내 세트를 생성하는 대신 동일한 폴더 ID를 재사용합니다.

<a id="failure-policy"></a>

## 실패 정책

스냅샷 로더가 UUID 마이그레이션이 필요하지만 폴더 파일을 다시 쓸 수 없음을 감지하면 스냅샷을 실패로 표시하고 쓰기 오류를 전파합니다. 이렇게 하면 아무런 알림 없이가 세션-로컬 ID 맵을 계속 사용하는 것을 방지할 수 있습니다.

이제 허브 런타임 로드가 동일한 준비 규칙을 따릅니다. `loadHubRuntime(...)`는 `HubRuntimeSnapshot` 내부에 임시 `WhatSonHubRuntimeStore`를 구현하고 `WhatSonRuntimeParallelLoader`에 다시 단계별 복사본을 생성합니다. 라이브 런타임 저장소는 요청한 모든 도메인이 성공적으로 완료된 후에만 업데이트됩니다.

<a id="resource-snapshot-fallback"></a>

## 리소스 스냅샷 대체 경로

리소스 스냅샷 로더는 먼저 `.wscontents/Resources.wsresources`를 읽는다.
그 결과가 비어 있으면 허브의 모든 리소스 루트(`.wsresources` + `*.wsresources`)를 직접 스캔해서 flat
`.wsresource` 패키지 목록을 `snapshot.values`로 채운다.

즉 startup path 역시 raw asset file이 아니라 `.wsresource` package path를 기준으로 움직인다.
