# `src/app/models/hierarchy/resources/WhatSonResourcesHierarchyParser.cpp`

<a id="responsibility"></a>

## 책임

`Resources.wsresources` 페이로드를 읽고 정규화된 `.wsresource` 패키지 경로 목록을 복원합니다.

<a id="accepted-input-forms"></a>

## 허용되는 입력 양식

- 레거시 JSON 문자열 배열.
- `resources: [...]`를 사용한 개체 루트.
- `resourcePath` 또는 레거시 `path`(대소문자를 구분하지 않는 키 일치)가 포함된 객체 배열 항목입니다.
- 루트 개체 직접 경로 키(`resourcePath` / `path`).
- `<resource ... path=...>` 또는 `<resource ... resourcePath=...>`와 같은 XML 유사 태그 텍스트입니다.
- 최종 대체 경로: 줄 기반 일반 텍스트.

`<resource ...>` 태그 내부의 속성 값은 지원되는 모든 형식으로 구문 분석됩니다.

- 큰따옴표(`path="..."`).
- 작은따옴표(`path='...'`).
- 베어 값(`resourcePath=...`).

또한 파서는 속성 키에 대해 대소문자 혼합을 허용합니다(예: `PATH`).

<a id="compatibility"></a>

## 호환성

이제 작성자 출력은 객체 배열 형식을 선호하지만 파서 호환성은 레거시 허브에 대해 이전 버전과 안전하므로 런타임 로드에 즉각적인 마이그레이션이 필요하지 않습니다.

`<resources>` 스타일 래퍼 라인은 대체 경로 라인 구문 분석 중에 무시되며 리소스 경로로 처리되지 않습니다.

<a id="regex-literal-safety-note"></a>

## 정규식 리터럴 안전 참고 사항

리소스 태그 속성 정규식 리터럴은 C++ 원시 문자열 리터럴 대신 이스케이프된 `QStringLiteral(...)` 문자열을 사용하여 속성 따옴표가 포함된 패턴에서 실수로 원시 리터럴 종결자 충돌을 방지합니다.
