# `src/app/models/hierarchy/library/LibraryHierarchyControllerSupport.hpp`

<a id="responsibility"></a>

## 책임

이 헤더는 이제 공유 계층 IO 도우미에 대한 라이브러리 도메인 외관 역할을 합니다.

<a id="shared-io-delegation"></a>

## 공유 IO 위임

`LibrarySupport`는 `WhatSonHierarchyIoSupport.hpp`에서 다음 도우미를 다시 내보냅니다.

- `normalizePath(...)`
- `resolveContentsDirectories(...)`
- `readUtf8File(...)`

이는 라이브러리 도메인에서 중복된 `.wshub` 및 UTF-8 파일 로직을 제거하는 동시에 기존 호출 사이트를 안정적으로 유지합니다.

<a id="scope"></a>

## 범위

라이브러리 지원 헤더는 더 이상 공유 파일 시스템 구현의 인라인 복사본을 소유하지 않습니다. 향후 라이브러리별 구문 분석 또는 변환 논리는 라이브러리 도메인에 유지되어야 하며 일반 허브 IO는 `WhatSon::Hierarchy::IoSupport`에 유지됩니다.
