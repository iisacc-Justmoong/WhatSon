# `src/app/models/file/hub/WhatSonHubMountValidator.cpp`

<a id="implementation-notes"></a>

## 구현 노트
- 접근이 확보된 후, 검증자는 최소 허브 패키지 계약인 `.wscontents`, `Library.wslibrary`, `.wsresources`, `*.wsstat` 및 `.wscontents` 아래의 핵심 도메인 항목을 확인합니다.
- 시작 해결과 온보딩 로딩이 이제 별도의 허브 구조 검사를 유지하는 대신 하나의 검증 경로를 공유합니다.
