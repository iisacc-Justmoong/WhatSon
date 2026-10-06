# `src/app/runtime/bootstrap/WhatSonQmlLaunchSupport.cpp`

<a id="responsibility"></a>

## 책임
앱 구성 루트에 대한 QML 루트 로딩을 구현합니다.

<a id="implementation-notes"></a>

## 구현 노트
- 요청된 루트당 단일 `lvrs::QmlRootLoadSpec`를 빌드합니다.
- `lvrs::loadQmlRootObjects(...)`를 사용하므로 초기 속성과 창 활성화는 LVRS 동작을 따릅니다.
- 수명 주기 인식 호출자에 대해 전체 LVRS `QmlRootLoadResult`를 반환합니다.
- LVRS가 로드 실패를 보고하거나 루트 개체가 생성되지 않은 경우 경고를 기록합니다.
- `QObject*` 래퍼는 `lastRootObject(...)` 위에 계층화된 호환성 도우미로만 유지됩니다.

<a id="test-coverage"></a>

## 테스트 범위
`test/cpp/suites/app_launch_support_tests.cpp`는 이 도우미가 LVRS 앱 항목 API를 사용하고 `main.cpp`가 더 이상 임시 `QQmlApplicationEngine::loadFromModule(...)`, 수동 창 활성화 또는 수동 수명 주기 루트/창 재구성을 수행하지 않는지 확인합니다.
