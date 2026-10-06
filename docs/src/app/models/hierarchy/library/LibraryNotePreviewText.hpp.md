# `src/app/models/hierarchy/library/LibraryNotePreviewText.hpp`

<a id="responsibility"></a>

## 책임

이 헤더는 보기 계층에서 메모 저장소를 다시 열지 않고 메타데이터 기반 레이블이 필요한 라이브러리 도메인 소비자를 위한 공유 메모 미리 보기 텍스트 규칙을 제공합니다.

<a id="shared-preview-rules"></a>

## 공유 미리보기 규칙

- `notePrimaryText(...)`는 노트 목록 `primaryText` 계약과 일치합니다.
  - `noteId`를 선호합니다
  - `project`로 대체
  - 비어 있지 않은 첫 번째 폴더 레이블로 대체
- `notePrimaryHeadline(...)`는 동일한 메타데이터 페이로드에서 첫 번째 비어 있지 않은 줄을 추출하여 캘린더 이벤트 칩과 같은 컴팩트한 소비자들이 두 번째 레이블 규칙을 고안하는 대신 노트 리스트 헤드라인 텍스트를 재사용할 수 있도록 합니다.
- 호출자는 완전히 정규화된 `LibraryNoteRecord` 데이터를 전달해야 합니다.

<a id="tests"></a>

## 테스트

- `test/cpp/suites/library_note_list_model_tests.cpp`는 메타데이터 미리보기 경로를 다룹니다.
- 회귀 체크리스트:
  - `noteId`가 포함된 메모는 프로젝트/폴더 대체 경로보다 해당 ID를 선호해야 합니다.
  - `notePrimaryHeadline(...)`는 동일한 미리보기 텍스트의 보이는 맨 윗줄을 반환해야 합니다.
  - 빈 미리보기 페이로드는 하위 소비 측 호출자가 자신의 대체 경로를 결정할 수 있도록 빈 문자열을 계속 반환해야 합니다.
