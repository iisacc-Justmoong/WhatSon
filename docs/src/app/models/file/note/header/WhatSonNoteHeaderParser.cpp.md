# `src/app/models/file/note/header/WhatSonNoteHeaderParser.cpp`

<a id="responsibility"></a>

## 책임

이 파서는 `.wsnhead` XML를 읽고 `WhatSonNoteHeaderStore`를 채웁니다.

<a id="parser-backend"></a>

## 파서 백엔드

- `.wsnhead` 파싱은 이제 로컬 `iiXml::Parser::TagParser` 경계를 소유하고 태그 텍스트와 인라인 속성에 대한 파싱된 문서 트리 헬퍼를 노출하는 `WhatSonIiXmlDocumentSupport`를 통해 라우팅됩니다.
- XML 선언과 `<!DOCTYPE WHATSONNOTE>` 서문은 해당 공유 지원 계층에 의해 제거된 후 헤더 본문을 iiXml에 전달합니다. 이는 트리 파서가 최상위 선언 처리 대신 요소 계층 구조를 소유하기 때문입니다.
- 정규 표현식은 헤더 태그 또는 속성 추출에 대한 권한으로 더 이상 사용되지 않으며, iiXml 노드와 필드 탐색으로 대체됩니다.

<a id="folder-parsing-rules"></a>

## 폴더 구문 분석 규칙

- `<folder>Path</folder>`는 유효한 레거시 입력으로 유지됩니다.
- `<folder uuid="...">Path</folder>`는 현대적인 형태입니다.
- 폴더 UUID는 iiXml 속성 필드에서 추출되어 표시되는 폴더 경로와 함께 정규화됩니다.
- 파싱된 바인딩은 `setFolderBindings(...)`를 통해 저장되며, 경로와 UUID 목록을 별도로 변형하여 저장되지 않습니다.

<a id="migration-behavior"></a>

## 마이그레이션 동작

파서가 경로를 읽을 수 있는 바인딩으로 처리하고 UUID를 비워두기 때문에 폴더 UUID가 없는 이전 노트는 계속 로드됩니다. 폴더 이름 바꾸기 또는 메모 재지정과 같은 나중에 다시 쓰기 경로를 통해 메모 헤더를 새로운 속성 기반 형식으로 업그레이드할 수 있습니다.

<a id="progress-parsing-rules"></a>

## 진행률 구문 분석 규칙

- 숫자 `<progress>` 콘텐츠는 여전히 저장된 열거형 정수에 직접 매핑됩니다.
- Enum-label 진행 내용은 여전히 `enums={...}` 속성을 통해 확인됩니다.
- `enums={...}` 속성은 이제 `setProgressEnums(...)`를 통해 `WhatSonNoteHeaderStore`에 저장되며, 정수 해석 후에는 폐기되지 않습니다.
- 빈 `<progress>` 요소는 이제 명시적으로 지워진 상태로 처리되며 아무런 알림 없이가 `0`로 대체되는 대신 `-1`로 구문 분석됩니다.
- 해석할 수 없는 `<progress>` 텍스트는 이제 숫자 값이나 열거형 레이블로 해석되지 않는 경우 `-1` ( `No progress` ) 으로 해석되며 `0` ( `First draft` ) 로 강제로 변환되지 않습니다.

<a id="file-statistics-parsing-rules"></a>

## 파일 통계 구문 분석 규칙

- 이제 파서는 `.wsnhead <fileStat>...</fileStat>` 블록을 읽습니다.
- 누락된 통계 태그는 이전 노트 헤더와의 하위 호환성을 위해 기본적으로 `0`로 설정됩니다.
- 음수 또는 유효하지 않은 숫자 페이로드는 `0`로 다시 정규화됩니다.
- 기존 최상위 메타데이터( `project` , `folders` , `tags` , `created` , `lastModified` , `lastOpened` )는 숫자가 아닌 Figma 행에 대해 기준 원본를 유지합니다; `fileStat` 블록은 명시적인 숫자 카운터만 저장합니다.

<a id="activity-parsing-rules"></a>

## 활동 구문 분석 규칙

- `<lastOpened>`는 선택 사항이며 이전 메모 헤더의 경우 빈 문자열로 구문 분석됩니다.
- 파서는 `lastOpened`를 `WhatSonNoteHeaderStore`에 저장하여 비활성 센서가 본문/편집자 프로젝션을 로드하지 않고 RAW 노트 헤더로부터 추론할 수 있도록 합니다.
