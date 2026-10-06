# `src/app/models/file/note/folder/WhatSonNoteFolderSemantics.hpp`

<a id="responsibility"></a>

## 책임
이 헤더는 노트 헤더 구문 분석, 변형 및 세부 패널 표시 전반에 걸쳐 일관성을 유지해야 하는 경량 폴더 경로 규칙을 중앙 집중화합니다.

<a id="public-helpers"></a>

## 공공 도우미
- `escapeFolderPathSegment(QString)`는 하나의 폴더 레이블 내에서 리터럴 `\` 및 `/` 문자를 이스케이프합니다.
- `folderPathSegments(QString)`는 `\/`를 리터럴 슬래시로, `\\`를 리터럴 백슬래시로 처리하면서 지속형 폴더 경로를 논리적 세그먼트로 분할합니다.
- `joinFolderPathSegments(QStringList)`는 논리 세그먼트에서 정식 지속 형식을 다시 작성합니다.
- `normalizeFolderPath(QString)`는 저장된 경로를 `.wsfolders`/노트 폴더 바인딩에서 사용하는 이스케이프 세그먼트 형식으로 정규화합니다.
- `appendFolderPathSegment(parent, label)`는 해당 레이블 내부의 `/`가 실수로 하위 계층 구조를 생성하도록 허용하지 않고 기존 지속 경로에 하나의 리터럴 폴더 레이블을 추가합니다.
- `displayFolderPath(QString)`는 일반 텍스트로 다시 디코딩된 이스케이프 구분 기호가 포함된 사용자 대상 경로 문자열을 반환합니다.
- `isHierarchicalFolderPath(QString)`는 문자 그대로 슬래시만 포함하는 단일 폴더 레이블과 실제 다중 세그먼트 계층 구조 경로를 구별합니다.
- `leafFolderName(QString)`는 정규화된 폴더 경로의 최종 표시 세그먼트만 반환합니다.
  - 예: `Research/Ideas`는 `Ideas`가 됩니다.
- `usesReservedTodayFolderSegment(const QString&)`는 예약된 `Today` 세그먼트 정책을 보호합니다.
- `inspectRawFoldersBlock(const QString&)`는 원시 `.wsnhead` 폴더 블록이 존재하는지와 구체적인 항목이 포함되어 있는지 여부를 검사합니다.

<a id="notes"></a>

## 메모
- 지속성은 이제 하나의 폴더 레이블 내부에 있는 `/`를 무조건적인 계층 구분 기호 대신 이스케이프된 콘텐츠(`\/`)로 처리합니다.
- `.wsnhead` 및 `.wsfolders`는 여전히 정규화된 전체 경로를 저장하지만 해당 경로는 이제 원시 슬래시 결합 레이블이 아닌 세그먼트 이스케이프 처리됩니다.
- 세부 정보 패널 폴더 표시에서는 `leafFolderName(...)` / `displayFolderPath(...)`를 사용하므로 사용자 표시 레이블은 지속되는 이스케이프 표시를 노출하지 않고 리터럴 `/` 문자를 유지합니다.
