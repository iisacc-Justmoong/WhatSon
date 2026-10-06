# `CMakeLists.txt`

<a id="responsibility"></a>

## 책임
- 저장소 전체 툴체인 설정, 제품 빌드 옵션 및 최상위 `add_subdirectory(...)` 그래프를 소유합니다.
- `WHATSON_BUILD_APP`가 활성화되면 데스크톱 `WhatSon` Qt 실행 가능 대상을 선언합니다.
- `src/app`가 QML 항목을 수집한 후 앱 QML 모듈을 마무리합니다.
- 루트 파일이 오케스트레이션 표면으로 유지되도록 그룹화된 대상 정의를 `cmake/root/*/CMakeLists.txt`에 위임합니다.
- 루트가 생성한 앱 대상과 동일한 디렉터리에 있어야 하는 루트 범위 `POST_BUILD` 후크를 소유합니다.

<a id="current-root-split"></a>

## 현재 루트 분할
- `cmake/root/build/CMakeLists.txt`: 회귀 게이트, CTest 통합 및 `whatson_clean_build_extras`를 유지했습니다.
- `cmake/root/dev/CMakeLists.txt`: QML 린트, QML 형식 및 clang-tidy 대상과 같은 개발자 도구입니다.
- `cmake/root/runtime/CMakeLists.txt`: 데스크톱 실행 및 상태 확인 대상.
- `cmake/root/distribution/CMakeLists.txt`: 미러 대상 설치, 내보내기, 패키지 및 시험판.

<a id="invariants"></a>

## 불변성
- 옵션 선언, 패키지 검색, 루트 대상 선언, QML 최종화 및 기본 제품 `add_subdirectory(...)` 호출을 이 루트 파일에 보관하십시오.
- 앱 소스/모듈 소유권을 `src/app` 아래로 유지하세요.
- 구성, 빌드 및 테스트 흐름에 `build/`를 재사용합니다.
- 캐시 가능한 루트 접두사를 통해 로컬 `iiXml` 및 `iiHtmlBlock` 패키지 검색 호스트 측을 유지합니다.
- 분리된 플랫폼 내보내기 또는 실행 프로그램 대상을 이 저장소에 다시 도입하지 마십시오.

<a id="verification-notes"></a>

## 확인 메모
- 구조적 루트-CMake가 변경된 후 `cmake -S . -B build`를 실행합니다.
- 루트 대상 리팩터링 후에 `cmake --build build --target whatson_build_regression -j`를 실행합니다.
- 회귀 대상 배선이 변경되면 `cmake --build build --target whatson_regression -j`를 실행합니다.
