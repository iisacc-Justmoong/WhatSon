# `src/app/qml/Main.qml`

<a id="role"></a>

## 역할
`Main.qml`는 루트 LVRS `ApplicationWindow`이며 데스크탑 작업 공간에 대한 시작 라우팅의 소유자입니다. 작업 공간 경로는 상태 표시줄, 탐색 표시줄, 사이드바, 목록 표시줄, 콘텐츠 슬롯, 세부 정보 패널 및 달력 오버레이 상태를 유지합니다.

<a id="kept-root-responsibilities"></a>

## 루트 책임 유지
- `LV.ApplicationWindow`를 인스턴스화합니다.
- LVRS 패딩 호환성 속성을 유지합니다.
- LVRS 페이지 스택을 통해 온보딩 및 작업 공간 상태를 라우팅합니다.
- 렌더링 품질 크기 조정 정책을 창 루트에 중앙 집중화합니다.
- 데스크탑 시작에 마운트된 허브가 없을 때 독립 실행형 온보딩 하위 창을 표시합니다.

<a id="tests"></a>

## 테스트
- `test/cpp/suites/qml_contents_view_tests.cpp`는 `Main.qml`가 삭제된 노트 편집기 화면을 재구성하지 않고 데스크탑 셸을 유지하는지 확인합니다.
- `test/cpp/suites/include_path_policy_tests.cpp`는 제거된 편집기와 소스, 문서, QML 및 C++ 회귀 테스트에서 분리된 플랫폼 앱 개체 제품군을 유지합니다.

## 한국어
- workspace 인터페이스는 데스크톱 shell/layout을 유지한다.
- `Main.qml`은 데스크톱 온보딩/라우팅과 workspace branch를 소유한다.
- 삭제된 editor view-mode controller, font provider, 모바일 route 객체는 root context로 전달하지 않는다.
