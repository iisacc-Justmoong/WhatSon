# `src/app/models/hierarchy/event/EventHierarchyController.cpp`

<a id="responsibility"></a>

## 책임

이 구현은 이벤트 버킷 트리를 구축하고, 사용자 변형을 이벤트 계층 구조 저장소로 전달하며, 런타임 새로 고침 이탈로부터 선택 및 확장 상태를 보호합니다.

<a id="runtime-refresh-contract"></a>

## 런타임 갱신 계약

`applyRuntimeSnapshot(...)`는 이제 런타임 로드를 전체 재설정 대신 상태 저장 업데이트로 처리합니다.

- 들어오는 이벤트 이름은 비교 전에 삭제됩니다.
- 정제된 목록이 현재 `m_eventNames`와 일치하면, 함수는 로드 상태 메타데이터만 업데이트하고 반환합니다.
- 목록이 변경된 경우, 컨트롤러는 버킷 행을 재구성하고, 안정적인 행 키로 확장을 복원하며, 동일한 키로 선택을 복원한 뒤 모델을 동기화합니다.

이는 관련되지 않은 파일 활동으로 인해 이벤트 계층 구조가 기본 닫힌 상태로 축소되는 것을 방지합니다.

<a id="controller-hook-contract"></a>

## 컨트롤러 후크 계약

`requestControllerHook()`는 더 이상 신호 전용 브리지가 아닙니다.

- `m_eventFilePath` 가 사용 가능할 경우 `reloadFromEventFilePath(...)` 를 통해 `Event.wsevent` 를 디스크에서 다시 로드합니다.
- 성공적으로 다시 로드하면 새로운 런타임 스냅샷이 적용되고 로드 상태 메타데이터가 업데이트됩니다.
- 재로드 실패가 이제 `loadStateChanged`를 통해 구체적인 파서/읽기 오류를 발생시키면서도 훅 체인 호환성을 위해 여전히 `controllerHookRequested()`를 방출합니다.

<a id="expansion-handling"></a>

## 확장 처리

- `setItemExpanded(...)`는 공유된 체브론 검증/상태 전환을 `IHierarchyController`에 위임한 다음, 유효한 확장 가능한 행이 변경될 때 이벤트 모델을 동기화합니다.
- `expandedEventItemKeys(...)`는 재구축 전에 현재 열려 있는 행을 캡처합니다.
- `restoreExpandedEventItemKeys(...)`는 `buildBucketItems(...)` 이후에 해당 개구부를 다시 적용합니다.

<a id="mutation-flow"></a>

## 돌연변이 흐름

- `renameItem(...)`, `createFolder()`, 그리고 `deleteSelectedFolder()`는 이벤트 이름 목록에 대해 작업한 후, `syncDomainStoreFromItems()`와 `syncModel()`를 호출합니다.
- `loadFromWshub(...)`는 사용 가능한 첫 번째 `Event.wsevent`를 해석하고, 이를 `WhatSonEventHierarchyStore`로 파싱한 뒤, `setEventNames(...)`를 통해 행 모델을 시드합니다.

<a id="invariants"></a>

## 불변성

- 선택 항목은 오래된 행 인덱스가 아닌 의미 체계 행 키로 유지됩니다.
- 확장은 기본 이벤트 계층 구조가 실제로 변경되어 대상 행을 제거할 때에만 초기화됩니다.
- 행이 사용 가능할 경우, 부정적이거나 잘못된 선택된 인덱스가 첫 번째 보이는 행으로 정규화되어 저장된 C++ 선택 상태가 사이드바의 기본 활성 행과 일치하도록 합니다.

<a id="count-role-compatibility"></a>

## 카운트 역할 호환성

이제 `depthItems()`에는 항상 행당 숫자 `count` 필드가 포함됩니다. 이벤트 도메인은 현재 노트 인덱스 멤버십 상태를 소유하지 않으므로 도메인 전체에서 공유 계층 모델 형태를 일관되게 유지하기 위해 값이 `0`로 방출됩니다.
