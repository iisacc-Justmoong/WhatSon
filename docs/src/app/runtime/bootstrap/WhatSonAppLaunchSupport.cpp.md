# `src/app/runtime/bootstrap/WhatSonAppLaunchSupport.cpp`

<a id="role"></a>

## 역할
부트스트랩 전용 실행 동작에 대한 명령줄 구문 분석을 소유합니다.

<a id="implementation-notes"></a>

## 구현 노트
- `parseLaunchOptions(...)`는 여전히 전용 온보딩 전용 실행 경로를 인식합니다.
- Workspace와 onboarding의 시작 준비는 헤더 헬퍼에 의도적으로 정의되어 있어 `main.cpp`와 C++ 회귀 테스트가 동일한 규칙을 평가합니다.
