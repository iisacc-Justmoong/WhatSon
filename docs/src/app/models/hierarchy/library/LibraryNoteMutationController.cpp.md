# `src/app/models/hierarchy/library/LibraryNoteMutationController.cpp`

<a id="implementation-notes"></a>

## 구현 노트
- 이제 돌연변이 외관이 `qobject_cast`를 통해 `ILibraryNoteMutationCapability`를 발견합니다.
- 돌연변이 신호는 컴파일 타임의 구체적인 유형 커플링 대신 QObject 신호 배선을 통해 전달됩니다.
- 배치 삭제 및 폴더 정리 도우미는 기존 단일 메모 기능 호출을 재생하기 전에 들어오는 메모 ID를 정규화하고 중복을 제거합니다.
- 이것은 QML 배치 노트 리스트 작업을 하나의 파사드에 보관하고, `ListBarLayout.qml`를 통해 ID별 변이 루프를 흩뜨리는 대신에.
