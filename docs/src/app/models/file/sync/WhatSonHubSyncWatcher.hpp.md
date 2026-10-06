# `src/app/models/file/sync/WhatSonHubSyncWatcher.hpp`

<a id="role"></a>

## 역할
마운트된 허브 동기화를 위한 파일 시스템 감시자 래퍼를 선언합니다.

<a id="contract"></a>

## 계약
- 내부 `QFileSystemWatcher`에 정규화된 디렉터리 감시 경로를 적용합니다.
- 감시된 디렉터리가 변경되면 `watchedPathChanged(...)`를 내보냅니다.
- 변경되지 않은 경로 세트로 인해 감시자 등록이 중단되지 않도록 적용된 경로를 기억합니다.

<a id="boundary"></a>

## 경계
- 감시자는 허브 서명을 계산하지 않으며 런타임 다시 로드가 필요한지 여부를 결정하지 않습니다.
