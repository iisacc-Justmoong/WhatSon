# `src/app/models/hierarchy/event/EventHierarchyController.hpp`

<a id="responsibility"></a>

## 책임

이 헤더는 전용 이벤트 계층 컨트롤러를 선언합니다. `Event.wsevent` 분류법을 LVRS 호환 계층 모델로 제시하고 사이드바 보기에 필요한 변경 가능한 후크를 노출합니다.

<a id="public-contract"></a>

## 공공 계약

- 행 모델, 선택 인덱스, 항목 수 및 로드 상태 속성을 게시합니다.
- `IHierarchyController` 위에 이름 변경, 생성, 삭제 및 확장 기능을 구현합니다.
- 직접 데이터 주입을 위해 `setEventNames(...)`와 `applyRuntimeSnapshot(...)`를 런타임 로더 기반 새로 고침에 노출합니다.
- `requestControllerHook()`를 파일 기반 훅 진입점으로 노출하여, 지속된 소스 경로에서 `Event.wsevent`를 재로드할 수 있습니다.
- `setItemExpanded(int, bool)`를 노출하여 폴드 상태가 일시적 델리게이트 인스턴스가 아니라 컨트롤러에 속하도록 합니다.
- 상속된 기능 메서드에 명시적인 `override`를 표시하여 리팩토링 중에 인터페이스 계약이 경고‐클린 및 서명‐안전 상태를 유지하도록 합니다.

<a id="refresh-rules"></a>

## 새로 고침 규칙

- 스냅샷 애플리케이션은 이벤트 계층 구조가 재구성될 때 현재 선택 및 확장 상태를 유지해야 합니다.
- 스냅샷 애플리케이션은 정제된 이벤트 이름 목록이 변경되지 않은 경우 재구성을 완전히 건너뛰어야 합니다.
- 로드 실패는 현재 표시되는 계층 구조를 파괴하지 않고 오류 표면을 업데이트합니다.

<a id="internal-state"></a>

## 내부 상태

- `m_eventNames`는 현재 정식 이벤트 이름 페이로드입니다.
- `m_items`는 `depthItems()`를 통해 공유된 `WhatSonHierarchyModel`로 직렬화된 렌더링된 버킷/항목 계층 구조입니다.
- `m_store`는 `Event.wsevent`에 대한 직렬화기/파서 연결 도메인 저장소로 유지됩니다.
- `m_eventFilePath`는 이름 바꾸기/생성/삭제 작업에 사용되는 돌연변이 대상을 식별합니다.
