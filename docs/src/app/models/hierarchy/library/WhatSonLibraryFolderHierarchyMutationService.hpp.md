# `src/app/models/hierarchy/library/WhatSonLibraryFolderHierarchyMutationService.hpp`

<a id="responsibility"></a>

## 책임

이 헤더는 영구 라이브러리 폴더 변형을 커밋하는 서비스를 선언합니다. 서비스는 폴더 트리 쓰기 경계만 소유합니다.

<a id="why-the-service-exists"></a>

## 서비스가 존재하는 이유

폴더 이름 바꾸기, 이동, 삭제 및 재정렬 작업은 이 응용 프로그램에서 시각적으로만 수행되지 않습니다.

- `Folders.wsfolders`는 사이드바 트리를 변경합니다.
- 노트 폴더 바인딩은 현재 읽기 전용 런타임 메타데이터입니다.

패키지 지속성 레이어가 삭제되는 동안 서비스는 더 이상 노트 헤더를 변경하지 않습니다.

<a id="uuid-oriented-contract"></a>

## UUID 중심 계약

현재 구현에서는 UUID 폴더를 정식 ID로 처리합니다.

- 경로 변경이 허용됩니다.
- UUID는 안정적으로 유지됩니다.
- 메모 레코드는 변경되지 않고 반환됩니다.

이는 저장된 경로가 더 이상 런타임 트리와 일치하지 않기 때문에 상위 폴더의 이름을 바꾸면 모든 하위 메모 바인딩이 무효화되는 이전 실패 모드를 해결합니다.
