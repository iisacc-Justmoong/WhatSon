# `src/app/models/file/WhatSonDebugTrace.hpp`

<a id="status"></a>

## 상태
- 문서화 단계: 런타임 디버그 추적 도우미와 함께 유지됩니다.
- 세부 수준: 현재 추적 제어 및 메시지 필터링 경계를 문서화합니다.

<a id="source-metadata"></a>

## 소스 메타데이터
- 소스 경로: `src/app/models/file/WhatSonDebugTrace.hpp`
- 소스 종류: C++ 헤더
- 파일 이름: `WhatSonDebugTrace.hpp`
- 대략적인 줄 수: 360

<a id="responsibility"></a>

## 책임
- 모델, 컨트롤러, 시작 및 편집기 코드에서 사용되는 앱 로컬 `[whatson:debug]` 추적 도우미를 제공합니다.
- 하나의 공유 헬퍼를 통해 불리언 트레이스 환경 플래그를 파싱하여 `on/off`, `true/false`, 그리고 `1/0`가 일관되게 동작하도록 합니다.
- 앱 수준의 Qt 메시지 필터를 설치하여 기본적으로 시끄러운 `iiXml::*` 디버그 메시지를 억제하고 WhatSon, LVRS, Qt 경고 및 치명적인 출력을 유지합니다.

<a id="trace-flags"></a>

## 추적 플래그
- `WHATSON_DEBUG_MODE`는 일반 WhatSon 런타임 추적을 제어합니다.
- `WHATSON_EDITOR_TRACE`는 편집기별 추적을 재정의합니다. 없으면 `WHATSON_DEBUG_MODE`를 따릅니다.
- `WHATSON_IIXML_TRACE_MODE`는 로컬 `iiXml` 파서 트레이스 가시성을 제어합니다. 파서가 여러 내부 파싱 단계에 대해 하나의 `qDebug` 메시지를 발생시키기 때문에 기본값이 꺼져 있습니다.

<a id="text-summaries"></a>

## 텍스트 요약
- `summarizeText(...)` 는 원래 문자 수와 정규화된 미리보기만 보고합니다. 편집기 추적 세부 정보 인수가 추적 기능이 비활성화되어도 종종 생성되므로 전체 소스 문자열을 정규화하거나 복사해서는 안 됩니다.

<a id="message-filter"></a>

## 메시지 필터
- `installThirdPartyTraceMessageFilter()`는 Qt 애플리케이션 부트스트랩 직후에 `src/app/main.cpp`에서 호출됩니다.
- 필터는 `WHATSON_IIXML_TRACE_MODE` 가 비활성화되지 않은 경우 `iiXml::` 로 시작하는 `QtDebugMsg` 항목만 제거합니다.
- `QtWarningMsg`, `QtCriticalMsg`, `QtFatalMsg` 및 모든 비 `iiXml` 메시지는 이전 Qt 메시지 핸들러를 통해, 또는 이전 핸들러가 존재하지 않을 경우 `qFormatLogMessage`를 통해 계속됩니다.

<a id="verification"></a>

## 검증
- `test/cpp/suites/debug_trace_filter_tests.cpp`는 억제 전제, 경고 패스스루 동작, 메인 시작 설치 호출, 환경 변수 계약 및 대형 텍스트 미리보기 전용 요약 동작을 잠급니다.

<a id="extension-notes"></a>

## 확장 노트
- 로깅 API를 노출하지 않는 로컬 종속성에 대한 좁은 텍스트 접두사 게이트로 필터를 유지합니다.
- 앱 측 억제를 확대하는 대신, 사용 가능해질 때 로컬 라이브러리에 실제 로그 제어를 추가하는 것을 선호합니다.
