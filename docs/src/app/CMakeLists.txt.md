# `src/app/CMakeLists.txt`

<a id="responsibility"></a>

## 책임
- 루트로 선언된 `WhatSon` 대상을 사용하고 Qt Quick, LVRS, 소스 및 QML 등록을 앱 진입점에 가깝게 유지합니다.
- 하위 도메인이 소스를 등록하고 경로 및 QML 자산을 포함하는 데 사용하는 도우미 기능을 제공합니다.
- 명시적인 `add_subdirectory(...)` 항목을 통해 도메인 소유권을 위임합니다.

<a id="current-directory-split"></a>

## 현재 디렉터리 분할
- `src/app/models/calendar/CMakeLists.txt`
- `src/app/models/content/CMakeLists.txt`
- `src/app/models/detailPanel/CMakeLists.txt`
- `src/app/models/file/CMakeLists.txt`
- `src/app/models/navigationbar/CMakeLists.txt`
- `src/app/models/onboarding/CMakeLists.txt`
- `src/app/models/panel/CMakeLists.txt`
- `src/app/models/sensor/CMakeLists.txt`
- `src/app/models/sidebar/CMakeLists.txt`
- `src/app/permissions/CMakeLists.txt`
- `src/app/platform/CMakeLists.txt`
- `src/app/policy/CMakeLists.txt`
- `src/app/qml/CMakeLists.txt`
- `src/app/register/CMakeLists.txt`
- `src/app/runtime/CMakeLists.txt`
- `src/app/store/CMakeLists.txt`

<a id="build-shards"></a>

## 샤드 구축
- `src/app/cmake/resources/CMakeLists.txt`: 데스크탑 앱 아이콘 리소스, 온보딩 일러스트레이션 리소스, Apple 번들 아이콘 스테이징 및 Windows 아이콘 RC 생성.
- `src/app/cmake/defaults/CMakeLists.txt`: LVRS 기본값 및 데스크톱 Apple plist/자격 전달.
- `src/app/cmake/runtime/CMakeLists.txt`: 링크 라이브러리, 출력 속성, 호스트 Apple 프레임워크 링크, LVRS 런타임 가져오기 오버레이 및 `lvrs_configure_qml_app(WhatSon)`.

<a id="helper-surface"></a>

## 도우미 표면
- `whatson_app_register_sources(...)`: `WhatSon`에 소스를 첨부하기 전에 파일 존재 여부를 확인합니다.
- `whatson_app_register_directory_sources(...)`: 하위 디렉터리에 대한 C/C++ 소스 및 헤더를 등록합니다.
- `whatson_app_register_directory_include_directories(...)`: 재귀 헤더 소유자 디렉터리를 포함 경로로 확장합니다.
- `whatson_app_register_qml_entries(...)` 및 `whatson_app_register_directory_qml(...)`: 루트 QML 모듈 마무리를 위해 `src/app` 상대 QML 및 JS 경로를 수집합니다.

<a id="current-notes"></a>

## 현재 노트
- QML 소유권은 `src/app/qml/CMakeLists.txt`로 격리됩니다.
- 이전 노트 편집기와 본문 지속성 샤드는 새로운 문서 모델 계약이 도입될 때까지 제거된 상태로 유지됩니다.
- 데스크탑 평가판 빌드는 `src/extension/trial`에서 전용 평가판 활성화 소스를 가져오고 `WHATSON_IS_TRIAL_BUILD=1`를 정의합니다.
- 앱 대상은 `iiXml::iiXml` 및 `iiHtmlBlock::iiHtmlBlock`를 연결합니다. 루트 CMake는 패키지 검색을 소유합니다.
- 이 저장소는 더 이상 분리된 플랫폼 앱 패키징, 실행 또는 내보내기 연결을 소유하지 않습니다.

<a id="verification-notes"></a>

## 확인 메모
- 이 파일을 다루는 빌드 시스템 리팩터링은 `cmake --build build --target whatson_build_regression -j`를 실행해야 합니다.
- 컴파일 연결 가능성이 변경되면 `cmake --build build --target whatson_regression -j`도 실행하세요.
