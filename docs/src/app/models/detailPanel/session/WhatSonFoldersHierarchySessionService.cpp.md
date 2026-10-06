# `src/app/models/detailPanel/session/WhatSonFoldersHierarchySessionService.cpp`

<a id="responsibility"></a>

## 책임
- 활성 노트 디렉토리에서 `Folders.wsfolders`를 해결합니다.
- 요청된 폴더 경로가 영구 폴더 계층 구조에 존재하도록 보장한 후, 상세 패널이 해당 폴더를 현재 노트 헤더에 바인딩합니다.

<a id="escaped-segment-semantics"></a>

## 이스케이프된 세그먼트 의미론
- 폴더 생성 및 조회는 이제 공유된 `WhatSon::NoteFolders::appendFolderPathSegment(...)` 헬퍼를 통해 누적 경로를 구축합니다.
- 하나의 폴더 라벨 안에 있는 문자 그대로의 `/`는 영구적인 ID에서 `\/`로 이스케이프된 상태를 유지하며, 상세 패널 쓰기 중에 실수로 부모/자식 계층 레벨로 다시 확장되지 않습니다.
- 기존 이스케이프된 폴더 항목은 정규 인코딩된 경로와 일치하므로, `Marketing\/Sales`에 메모를 재할당하면 중복된 계층 행을 생성하는 대신 동일한 폴더 uuid를 재사용합니다.

<a id="tests"></a>

## 테스트
- 유지 관리되는 C++ 회귀 스위트에 이제 이스케이프 슬래시 재사용 경로를 회귀에 대해 잠그는 세션 서비스 케이스가 포함됩니다.
