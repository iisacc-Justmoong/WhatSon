# `src/app/models/file/sync/WhatSonHubSyncObservation.hpp`

<a id="role"></a>

## 역할
허브 동기화 관찰, 감시자 설정 및 컨트롤러 기준 비교에서 공유되는 값 개체를 선언합니다.

<a id="contract"></a>

## 계약
- `signature`는 관찰된 허브 파일 시스템 상태의 해시입니다.
- `directoryWatchPaths`는 허브 동기화 감시자에 등록되어야 하는 정규화된 디렉터리 목록입니다.
- 구조체는 수동 데이터일 뿐입니다. 파일 시스템 액세스를 수행하지 않으며 타이머나 감시자를 소유하지 않습니다.
