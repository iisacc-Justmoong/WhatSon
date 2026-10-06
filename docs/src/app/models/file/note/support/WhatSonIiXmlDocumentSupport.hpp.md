# `src/app/models/file/note/support/WhatSonIiXmlDocumentSupport.hpp`

<a id="responsibility"></a>

## 책임

노트 헤더 리더가 사용하는 노트 패키지 iiXml 지원 경계를 선언합니다.

<a id="contract"></a>

## 계약

- UTF-8 뷰 변환, XML 엔터티 디코딩, XML 프리앰블 스트리핑, 태그 이름 비교, 필드 이름 비교, 하위 항목 조회, 텍스트 추출, 속성 추출 및 문서 파싱을 보유하고 있습니다.
- 노트 패키지 파서가 각 소비 파일에서 iiXml 어댑터 코드를 복제하지 못하도록 합니다.
- 읽기 측 도우미만 노출합니다. 노트 소스를 변경하거나 편집기 본문 내용을 정규화하거나 노트 지속성 정책을 결정하지 않습니다.
