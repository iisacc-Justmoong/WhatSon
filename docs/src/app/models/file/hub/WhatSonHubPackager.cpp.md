# `src/app/models/file/hub/WhatSonHubPackager.cpp`

<a id="role"></a>

## 역할
허브 스캐폴드 생성과 독립적으로 `.wshub` 패키지 루트 구체화를 구현합니다.

<a id="responsibilities"></a>

## 책임
- 패키지 생성이 항상 절대 `.wshub` 디렉터리에 위치하도록 요청된 허브 경로를 정규화합니다.
- 상위 디렉터리와 패키지 루트 디렉터리를 만듭니다.
- 빌드가 Apple 플랫폼에서 실행될 때 `WhatSonApplePackageAppearance`를 통해 Apple 패키지 프레젠테이션 힌트를 적용합니다.

<a id="behavior-notes"></a>

## 행동 참고 사항
- `.wshub` 생성은 실제 디렉터리 트리를 작성하므로 패키지 생성은 로컬이 아닌 대상을 거부합니다.
- Apple 패키지 프레젠테이션 실패는 경고로 기록되므로, 제공자 파일 시스템이 Finder 스타일 패키지 메타데이터를 지속하지 않을 때에도 온보딩이 패키지 생성을 완료할 수 있습니다.

<a id="tests"></a>

## 테스트
- `test/cpp/whatson_cpp_regression_tests.cpp`는 패키지 루트 생성에 대한 런타임 커버리지와 생성자/패키저 분할에 대한 소스 수준 회귀 커버리지를 모두 포함합니다.
