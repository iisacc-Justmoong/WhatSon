# `src/app/models/file/sync/WhatSonHubSyncObservationBuilder.cpp`

<a id="role"></a>

## 역할
런타임 동기화를 위해 마운트된 `.wshub` 관찰을 구현합니다.

<a id="behavior"></a>

## 행동
- `QDirIterator`를 사용하여 허브를 재귀적으로 이동합니다.
- 상대 경로, 항목 유형, 크기 및 마지막 수정 타임스탬프에서 서명 레코드를 작성합니다.
- SHA-256를 사용하여 정렬된 서명 레코드를 해시합니다.
- 동일한 순회 패스에서 정규화된 디렉터리 감시 경로를 반환합니다.
- `.whatson` 및 그 하위 항목을 무시하여 앱 비공개 장부가 런타임 다시 로드를 트리거하지 않도록 합니다.

<a id="tests"></a>

## 테스트
- `hubSyncObservationBuilder_ignoresPrivateWhatSonBookkeeping`는 `.whatson` 변경이 관찰된 서명을 변경하지 않는 반면, 가시적인 허브 콘텐츠 변경은 변경된다는 것을 확인합니다.
