# `src/app/models/hierarchy/preset/PresetHierarchyController.cpp`

<a id="responsibility"></a>

## 책임

이 파일은 런타임 스냅샷 새로 고침 전반에 걸쳐 미리 설정된 계층 구조 로딩, 행 모델 재구성, 지속성 동기화 및 상태 보존을 구현합니다.

<a id="runtime-refresh-contract"></a>

## 런타임 갱신 계약

`applyRuntimeSnapshot(...)`는 이제 이벤트 계층 구조 동작을 미러링합니다.

- 비교하기 전에 들어오는 사전 설정 이름을 삭제합니다.
- 정리된 목록이 `m_presetNames`와 일치하면 로드 상태 메타데이터만 업데이트됩니다.
- 목록이 변경된 경우, 사전 설정된 버킷 행을 재구성하고, 확장된 브랜치를 복원하며, 안정 키로 이전 선택을 복원하고, 모델을 동기화합니다.

이렇게 하면 노트 저장 관련 런타임 새로 고침이 사전 설정된 사이드바 트리를 축소하는 것을 중지합니다.

<a id="controller-hook-contract"></a>

## 컨트롤러 후크 계약

`requestControllerHook()`는 사전 설정된 소스 경로가 알려진 경우 파일 기반 자체 새로 고침을 수행합니다.

- `reloadFromPresetFilePath(...)`를 통해 `Preset.wspreset`를 다시 로드합니다.
- 성공하면, 컨트롤러는 스냅샷 파이프라인을 다시 적용하고 상태가 실제로 변경된 경우에만 `loadStateChanged`를 방출합니다.
- 읽기/파싱 실패 시 현재 행을 유지하고, 부하 상태에서 실패를 기록하며, 여전히 `controllerHookRequested()`를 방출하여 외부 훅 리스너가 결정론성을 유지하도록 합니다.

<a id="expansion-ownership"></a>

## 확장 소유권

- `setItemExpanded(...)`는 공유된 체브론 검증/상태 플립을 `IHierarchyController`에 위임하고, 유효한 확장 가능한 행이 변경될 때 사전 설정된 모델을 동기화합니다.
- 확장은 메모리 내에서만 직렬화됩니다; 이는 `Preset.wspreset`의 일부가 아니라 사용자 세션 상태로 의도적으로 취급됩니다.

<a id="mutation-flow"></a>

## 돌연변이 흐름

- `setPresetNames(...)`는 초기 로드에 사용되는 명령형 설정자입니다.
- `renameItem(...)`, `createFolder()`, 및 `deleteSelectedFolder()`는 현재 아이템 세트를 변환한 다음, 해당 아이템들로부터 프리셋 스토어를 다시 작성합니다.
- `syncModel()`는 행 벡터를 QML에 다시 게시하는 유일한 경로입니다.

<a id="invariants"></a>

## 불변성

- 사전 설정된 계층 구조 행은 현재 행 위치가 아닌 의미론적으로 키가 지정됩니다.
- 재구성은 사전 설정된 소스가 실질적으로 변경된 경우에만 허용됩니다.
- 행이 사용 가능할 경우, 부정적이거나 잘못된 선택된 인덱스가 첫 번째 보이는 행으로 정규화되어 런타임 새로 고침이 컨트롤러가 보이지 않는 "no selection" 상태에 남지 않도록 합니다.

<a id="count-role-compatibility"></a>

## 카운트 역할 호환성

`depthItems()`는 이제 모든 사전 설정 행에 숫자 `count` 필드를 게시합니다. 사전 설정 도메인은 현재 사전 설정 이름별로 노트 인덱스 멤버십을 전달하지 않으므로 공유 계층 페이로드 계약을 계속 충족시키면서 값이 `0`로 방출됩니다.
