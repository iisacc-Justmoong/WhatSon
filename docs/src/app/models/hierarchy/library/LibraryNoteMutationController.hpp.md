# `src/app/models/hierarchy/library/LibraryNoteMutationController.hpp`

<a id="role"></a>

## 역할
`LibraryNoteMutationController`는 라이브러리 노트 돌연변이 명령 및 관련 신호에 대한 좁은 외관입니다.

<a id="interface-alignment"></a>

## 인터페이스 정렬
- 이제 소스 컨트롤러는 `QObject*`와 `ILibraryNoteMutationCapability`로 허용됩니다.
- 이렇게 하면 돌연변이 호출자가 전체 `LibraryHierarchyController` 콘크리트 표면으로부터 독립된 상태로 유지됩니다.
- 단일 음표 명령어(`deleteNoteById(...)`, `clearNoteFoldersById(...)`)는 여전히 포커스 노트 흐름에 대해 직접 노출됩니다.
- 배치 노트 목록 작업은 이제 `deleteNotesByIds(...)`와 `clearNoteFoldersByIds(...)`를 통해 라우팅될 수 있으며, 이들은 QML 배열을 받아 하나의 노트 ID씩 소스 기능으로 다시 팬합니다.
