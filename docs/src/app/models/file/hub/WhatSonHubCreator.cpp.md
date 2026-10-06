# `src/app/models/file/hub/WhatSonHubCreator.cpp`

<a id="role"></a>

## 역할
패키지 루트가 구체화된 후 새로운 `.wshub` 패키지에 대한 온디스크 스캐폴드를 구현합니다.

<a id="scaffold-output"></a>

## 비계 출력
- `.whatson`, `.wscontents` 및 `.wsresources` 아래에 필수 루트를 생성합니다.
- `.whatson/hub.json`를 기본 패키지 매니페스트로 작성합니다.
- 초기 통계, 라이브러리 인덱스, 태그, 폴더, 북마크, 진행 상황 및 프로젝트 목록 파일을 작성합니다.

<a id="behavior-notes"></a>

## 행동 참고 사항
- 경로가 구체화되기 전에 허브 이름이 삭제됩니다.
- 패키지 루트 생성 및 파일과 유사한 패키지 프레젠테이션은 `WhatSonHubPackager`에 위임됩니다.
- 패키지 루트가 존재한 후 스캐폴드 생성이 실패하면 생성자는 부분적으로 생성된 패키지 디렉터리를 제거합니다.
- 파일 쓰기는 로컬 경로에 대해 `QSaveFile`를 통과하므로 매니페스트 및 스캐폴드 업데이트는 지원되는 파일 시스템에서 원자성을 유지합니다.

<a id="tests"></a>

## 테스트
- `test/cpp/whatson_cpp_regression_tests.cpp`는 패키지 물리화와 스캐폴드 쓰기 사이의 분할과 패키지 루트 롤백을 포함합니다.
