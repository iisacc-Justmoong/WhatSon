# `src/app/models/hierarchy/resources/ResourcesHierarchyController.hpp`

<a id="responsibility"></a>

## 책임

이 헤더는 대부분 읽기 리소스 계층 구조 컨트롤러를 선언합니다. `Resources.wsresources`에서 로드된 현재 리소스 경로 페이로드를 추적하면서 리소스를 메타데이터 기반 LVRS 계층 구조로 표시합니다.

<a id="public-contract"></a>

## 공공 계약

- 일반적인 계층 구조 행 모델, 선택, 개수 및 로드 상태 속성을 게시합니다.
- `noteListModel` ( `ResourcesListModel` )를 게시하여 리소스 도메인이 공유된 `ListBarLayout` 메모 카드 표면을 구동할 수 있도록 합니다.
- 열림/닫힘 상태를 컨트롤러가 소유하도록 `IHierarchyExpansionCapability`를 구현합니다.
- `setResourcePaths(...)` 를 직접 입력과 `applyRuntimeSnapshot(...)` 런타임 스냅샷 로드를 위해 노출합니다.
- 편집기 측 리소스 렌더링을 위해 `noteDirectoryPathForNoteId(...)`를 노출합니다: 목록 항목 ID가 `.wsresource` 패키지를 가리키면, 렌더러가 해당 패키지 디렉터리를 직접 해석할 수 있습니다.
- `deleteNoteById(...)` / `deleteNotesByIds(...)`를 노출하여 공유된 `ListBarLayout` 삭제 단축키가 라이브러리‐노트 변이 경로를 라우팅하지 않고도 리소스 도메인에서 선택된 `.wsresource` 패키지를 제거할 수 있도록 합니다.
- `requestControllerHook()`를 파일 기반 리프레시 훅으로 노출하여 소스 경로가 알려질 때 `Resources.wsresources`(및 대체 경로 패키지 스캔)를 다시 읽습니다.
- 공유 계층 구조 표면에 부합하기 때문에 여전히 이름 바꾸기/생성/삭제 진입점을 노출하지만, 구체적인 리소스 패키지가 이제 오른쪽 패널 목록에서 삭제 가능함에도 계층 구조 분류 체계는 읽기 전용로 유지됩니다.
- 상속된 이름 바꾸기/크루드/확장 진입점을 명시적인 `override` 마커와 함께 선언하여 공유 계층 계약이 경고 없이 깨끗하게 유지됩니다.
- 패키지 삭제가 성공적으로 완료된 후 `hubFilesystemMutated()`를 방출하여 hub‐sync 배선이 변경을 로컬 돌연변이로 계속 분류할 수 있도록 합니다.

<a id="refresh-rules"></a>

## 새로 고침 규칙

- 런타임 업데이트는 현재 패키지 메타데이터에 따라 행 유형/형식을 변경할 수 있습니다.
- 확장 상태는 `setResourcePaths(...)`, `applyRuntimeSnapshot(...)` 및 `requestControllerHook()` 재로드 후에도 지속되어야 합니다.

<a id="internal-state"></a>

## 내부 상태

- `m_resourcePaths`는 최신 구문 분석 파일 목록을 저장합니다.
- `m_items`는 현재 구체화된 계층 구조 행과 UI 확장 상태를 저장합니다.
- `m_noteListModel`는 현재 계층 구조 선택으로 필터링된 오른쪽 패널 리소스 카드를 저장합니다.
- `m_resourcesFilePath`는 소스 `Resources.wsresources` 파일을 식별합니다.
