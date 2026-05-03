# `src/app/register`

## Role
This directory contains the app-core registration state objects that are shared with optional entitlement modules such as the trial extension.

## Files
- `WhatSonRegisterManager.hpp` / `WhatSonRegisterManager.cpp`: persist the app authentication-complete state and expose it as a small `QObject` manager.

## Contract
- `authenticated == true` means the app has completed the product authentication flow.
- Trial-only modules may read this manager and disable trial restrictions entirely when the flag is `true`.
- Trial builds persist the authenticated state as a signed `QSettings` record that is verified with the secure-store-backed trial integrity secret.
- Legacy plain `QSettings` booleans are no longer trusted as an authenticated bypass source.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/register`` (`docs/src/app/register/README.md`)
- 위치: `docs/src/app/register`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
