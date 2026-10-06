# `src/app/models/hierarchy/library/LibraryAll.cpp`

<a id="responsibility"></a>

## 책임

`LibraryAll`는 "All Library" 목록을 뒷받침하는 런타임 노트 버킷을 구축합니다. `.wsnindex`, `.wsnhead` 및 본문 콘텐츠를 읽고 해당 소스를 `LibraryNoteRecord`에 병합한 다음 라이브러리 계층 구조 컨트롤러에 안정적인 프로젝션을 노출합니다.

<a id="folder-uuid-role"></a>

## 폴더 UUID 역할

이제 레코드 병합 파이프라인은 다음 두 가지를 모두 수행합니다.

- 사람이 읽을 수 있는 폴더 경로
- 안정적인 폴더 UUID

인덱스가 불완전하면 `LibraryAll`는 `.wsnhead`로 대체되고 폴더 경로를 UUID 대응 항목과 정렬하므로 이후 필터는 원시 경로 대신 폴더 ID를 기준으로 메모를 일치시킬 수 있습니다.

<a id="compatibility-behavior"></a>

## 호환성 동작

- 폴더 경로만 포함된 이전 메모 데이터는 계속 로드됩니다.
- `<folder uuid="...">path</folder>`를 저장하는 최신 메모 헤더는 UUID 값을 그대로 유지합니다.
- 코드 패드를 병합하거나 UUID 목록을 복구하여 호출자가 일치하지 않는 폴더 경로 / UUID 배열을 받지 않도록 합니다.
- `.wsnhead`가 존재하는 경우, 해당 `<project>` 필드는 명시적으로 비어 있더라도 권한이 있습니다. 이렇게 하면 `index.wsnindex`에 아직 캐시되어 있을 수 있는 오래된 프로젝트 레이블이 삭제되어, 프로젝트 계층 필터가 이름이 지정된 프로젝트 버킷 아래에 "No project" 노트를 실수로 표시하지 않도록 합니다.
- 런타임 라이브러리 인덱싱은 메타데이터 전용입니다. 노트 본문 소스에서 본문 미리보기 필드나 리소스 썸네일을 파생하지 않습니다.
- 정규형 `all notes` 버킷은 이제 변경 개폐된 단일음 `upsertNote(...)`, `removeNoteById(...)` 및 `noteById(...)` 연산도 지원합니다. 이러한 작업은 로컬 노트 편집 후 부분적인 라이브러리/캘리더 새로 고침의 기반이 됩니다.

<a id="why-this-matters"></a>

## 이것이 중요한 이유

라이브러리 사이드바는 더 이상 상위 이름 변경을 의미 폴더 변경으로 처리하지 않습니다. 따라서 `LibraryAll`는 로드 시간 정규화를 통해 원래 폴더 UUID를 보존해야 합니다. 그렇지 않으면 기본 데이터가 올바르게 마이그레이션된 경우에도 계층 구조 이름 변경 후 노트 필터가 중단됩니다.
