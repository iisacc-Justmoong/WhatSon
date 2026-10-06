# `src/app/runtime/bootstrap/WhatSonQmlInternalTypeRegistrar.cpp`

<a id="responsibility"></a>

## 책임

복원된 애플리케이션 셸에 필요한 QObject 지원 내부 QML 브리지 유형에 대한 매니페스트를 빌드한 다음 LVRS `QmlTypeRegistrar`를 통해 해당 매니페스트를 등록합니다.

<a id="current-contract"></a>

## 현재 계약

- 레지스트라는 더 이상 편집기 파서, 렌더러, 프로젝션, 세션, 입력 정책, 미니맵, 라인 번호, 태그, 디스플레이 백엔드, 리소스 뷰어 또는 페이퍼 헬퍼 유형을 내보내지 않습니다.
- 콘텐츠 QML 경로는 더 이상 노트 텍스트 편집기를 마운트하지 않으며 C++ 편집기 백엔드 등록이 필요하지 않습니다.

<a id="test-coverage"></a>

## 테스트 범위

`test/cpp/suites/app_launch_support_tests.cpp`는 이 등록자를 LVRS 매니페스트 등록에 유지하고 직접 `qmlRegisterType<...>()` 블록이 반환되는 것을 방지합니다.
