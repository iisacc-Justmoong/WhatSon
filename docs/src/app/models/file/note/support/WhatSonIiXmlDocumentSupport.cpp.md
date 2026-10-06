# `src/app/models/file/note/support/WhatSonIiXmlDocumentSupport.cpp`

<a id="responsibility"></a>

## 책임

로컬 iiXml 파서 주변에 공유 노트 패키지 어댑터를 구현합니다.

<a id="key-behavior"></a>

## 주요 행동

- `iiXml::Parser::TagParser`를 호출하기 전에 XML 선언과 최상위 수준 문서 유형 프리앰블을 제거합니다.
- 파서 소유 문서 소스 복사본을 유지하면서 iiXml `std::string_view` 슬라이스를 `QString`로 변환합니다.
- 노드 텍스트 및 속성 값에 대한 공통 XML 엔터티를 디코딩합니다.
- 노트 패키지 조회를 위해 대소문자를 구분하지 않고 구문 분석된 `TagNode` 트리를 탐색합니다.
- 대체 속성 이름 전반에 걸쳐 첫 번째 비어 있지 않은 속성 해석을 제공하여, 오래된 리소스 태그 도형이 로컬 정규식 스캔을 다시 도입하지 않고도 계속 로드될 수 있도록 합니다.
