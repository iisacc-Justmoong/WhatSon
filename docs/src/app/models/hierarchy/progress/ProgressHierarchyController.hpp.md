# `src/app/models/hierarchy/progress/ProgressHierarchyController.hpp`

<a id="responsibility"></a>

## 책임

이 헤더는 진행 사이드바와 해당 노트 목록을 지원하는 진행 계층 컨트롤러를 선언합니다.

<a id="public-contract"></a>

## 공공 계약

- `itemModel`와 `noteListModel`를 모두 게시하므로, 진행 영역은 다른 도메인에서 사용하는 동일한 계층 인터페이스를 통해 사이드바 행과 필터링된 노트를 노출합니다.
- 직접 상태 주입을 위해 `setProgressState(int, QStringList)`를 노출하고, 시작/런타임를 리프레시를 위해 `applyRuntimeSnapshot(...)`를 노출합니다.
- 상세 패널 현재 노트 흐름을 위한 노트 디렉토리 조회를 노출합니다. 본문 지속성과 편집자 통계 새로고침 도구는 진행 컨트롤러 표면의 일부가 아닙니다.
- `requestControllerHook()`를 파일 기반 리프레시 훅으로 노출하여 `Progress.wsprogress`를 재파싱하고 메모 메타데이터를 재인덱싱합니다.
- 모든 상속된 기능 메서드를 명시적인 `override`와 함께 선언하고, 헤더를 `IHierarchyRenameCapability`, `IHierarchyCrudCapability` 및 `IHierarchyExpansionCapability`와 정렬하여 컴파일러 오버라이드 경고 없이 유지합니다.

<a id="stored-state"></a>

## 저장된 상태

- `m_progressStates`는 런타임 동기화를 위해 지속형 `Progress.wsprogress` 페이로드를 저장합니다.
- `m_items`는 사이드바에서 사용되는 고정된 10행 LVRS 방향 분류입니다.
- `m_allNotes`는 필터링된 노트 목록을 작성하는 데 사용되는 색인화된 허브 노트를 저장합니다.
- `m_progressFilePath` 는 런타임 새로고침 동안 소유하는 `.wshub` 를 해결하는 데 필요한 소스 파일 경로를 유지합니다.
