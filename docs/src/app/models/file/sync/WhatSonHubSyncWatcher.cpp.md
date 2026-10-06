# `src/app/models/file/sync/WhatSonHubSyncWatcher.cpp`

<a id="role"></a>

## 역할
마운트된 허브 동기화에 대한 감시자 경로 등록을 구현합니다.

<a id="behavior"></a>

## 행동
- 요청된 디렉터리 경로를 정규화, 중복 제거 및 정렬합니다.
- 현재 적용된 감시자 경로에 대해 증분 추가/제거 세트를 계산합니다.
- 오래된 경로를 제거하고 내부 `QFileSystemWatcher`에 새 경로를 추가합니다.
- 마운트된 허브가 설정 해제되면 파일 및 디렉터리 감시자 경로를 지웁니다.

<a id="boundary"></a>

## 경계
- 감시자는 파일 시스템 변경 힌트만 전달합니다. Debounce, polling, signature comparison 및 reload 결정은 형제 동기화 객체에 실시간으로 포함됩니다.
