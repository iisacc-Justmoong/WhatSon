# `src/app/policy/ArchitecturePolicyLock.cpp`

<a id="role"></a>

## 역할
이 파일에는 구체적인 아키텍처 계약 매트릭스와 애플리케이션 시작 경로에서 사용되는 단방향 런타임 잠금이 포함되어 있습니다.

<a id="dependency-matrix"></a>

## 종속성 매트릭스
행렬은 하드 코딩되어 있으며 의도적으로 단순합니다.
- `View`는 `Controller`에 따라 달라질 수 있습니다.
- `Controller`는 `DataModel`, `Store`, `Parser` 및 `Creator`에 종속될 수 있습니다.
- `Store`는 `DataModel`, `Parser`, `Creator` 및 `FileSystem`에 종속될 수 있습니다.
- `Parser` 및 `Creator`는 `DataModel`에 따라 달라질 수 있습니다.
- `DataModel` 및 `FileSystem`는 이 정책을 통해 나가는 종속성을 얻지 않습니다.

이는 범용 DI 프레임워크가 아닙니다. 저장소별 가드레일입니다.

<a id="runtime-lock"></a>

## 런타임 잠금
`g_architecturePolicyLocked`는 프로세스 전체의 원자 플래그입니다.
- 잠금 해제가 시작됩니다.
- 시작 코드는 종속성 주입을 수행합니다.
- `ArchitecturePolicyLock::lock()`가 깃발을 뒤집습니다.
- 늦은 배선 시도는 돌연변이를 거부하고 경고를 내보낼 수 있습니다.
- `ArchitecturePolicyLock::unlockForTests()`는 회귀 테스트가 케이스 간에 전역 잠금 상태를 재설정할 수 있도록 존재합니다.

프로덕션 구성에서는 여전히 잠금 장치를 단방향으로 처리합니다. 첫 번째 QML 장면이 완전히 활성화되기 전에 애플리케이션 배선이 완료될 것으로 예상됩니다.

<a id="verification-helpers"></a>

## 검증 도우미
- `assertDependencyAllowed(...)`는 순수한 진단 도우미입니다.
- `verifyDependencyAllowed(...)`는 `[whatson:policy][dependency] ...`를 사용하여 생산 방향 경고 경로를 추가합니다.
- `verifyMutableWiringAllowed(...)`는 `[whatson:policy][lock] ...`를 사용하여 포스트 잠금 재배선 가드를 추가합니다.
- `verifyMutableDependencyAllowed(...)`는 이제 시작 잠금 이후에 시도된 모든 변형과 불법 레이어 가장자리를 모두 거부해야 하는 설정자/배선 진입점에서 사용되는 공유 도우미입니다.

최신 브리지 및 컨트롤러 설정자 코드는 이러한 공유 도우미를 직접 사용하므로 정책은 문서로만 남아 있지 않고 실제 배선 경로에서 실행됩니다.

<a id="practical-reading"></a>

## 실용독서
이 파일을 다음과 함께 읽으십시오:
- 잠금 타이밍용 `src/app/main.cpp`.
- 뷰-컨트롤러 검증을 위한 `src/app/models/panel/HierarchyInteractionBridge.cpp`.
- 드래그/드롭 계약 확인을 위한 `src/app/models/panel/HierarchyDragDropBridge.cpp`.
- 컨트롤러-스토어 검증을 위한 `src/app/models/sidebar/SidebarHierarchyController.cpp`.
- 주요 런타임 배선 이음새에서 잠금 후 돌연변이 거부를 위한 `src/app/models/sidebar/HierarchyControllerProvider.cpp`, `src/app/models/panel/NoteListModelContractBridge.cpp` 및 `src/app/models/detailPanel/DetailCurrentNoteContextBridge.cpp`.
