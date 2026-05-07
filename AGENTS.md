# WhatSon 에이전트 운영 가이드

## 목적

이 저장소는 LVRS 기반 Qt Quick 애플리케이션으로 구현된 비즈니스급 크리에이티브, 브랜딩, 정보, 지식 텍스트 허브 편집기다. 이 문서는 Codex가 매 턴 동일한 맥락과 규칙으로 작업하도록 고정 운영 기준을 정의한다.

## 언어 원칙

- 사용자에게 보내는 모든 답변은 한국어로 작성한다.
- 이 `AGENTS.md`는 한국어 운영 문서로 유지한다.
- 코드, 식별자, 커밋/PR용 기술 문서, 저장소 내부 상세 문서는 기존 프로젝트 관례에 맞춰 영어를 기본으로 한다. 단, 사용자가 명시적으로 한국어 문서를 요구하면 그 지시를 우선한다.

## 기술 기준선

- UI 스택: Qt 6.5+ QML + LVRS 1.0
- 앱 진입점: `src/app/main.cpp`
- 앱 빌드 정의: `src/app/CMakeLists.txt`, `src/app/*/CMakeLists.txt` 아래의 최상위 도메인 shard, 그리고 `src/app/cmake/{resources,defaults,runtime}/CMakeLists.txt` 아래의 빌드 전용 shard
- 데몬 진입점: `src/daemon/main.cpp`
- 루트 빌드 정의: `CMakeLists.txt` 및 `cmake/root/*/CMakeLists.txt` 아래의 그룹화된 루트 타깃 shard
- 기본 QML 루트: `src/app/qml/Main.qml`
- 라이브러리 계층 백엔드: `src/app/models/file/hierarchy/library`
- 라이브러리 계층 모델/컨트롤러: `src/app/models/file/hierarchy/library/LibraryHierarchyModel.*`,
  `src/app/models/file/hierarchy/library/LibraryHierarchyController.*`
- 라이브러리 오른쪽 패널 목록 모델: `src/app/models/file/hierarchy/library/LibraryNoteListModel.*`
- 허브 배치 store: `src/app/models/file/hub/WhatSonHubPlacementStore.*`
- 태그 depth provider: `src/app/models/file/hierarchy/tags/WhatSonHubTagsDepthProvider.*`
- 허브 런타임 store: `src/app/models/file/hub/WhatSonHubRuntimeStore.*`
- 런타임 병렬 bootstrap loader: `src/app/runtime/threading/WhatSonRuntimeParallelLoader.*`
- 태그 런타임 상태 store: `src/app/models/file/hierarchy/tags/WhatSonHubTagsStateStore.*`
- 태그 계층 모델/컨트롤러: `src/app/models/file/hierarchy/tags/TagsHierarchyModel.*`,
  `src/app/models/file/hierarchy/tags/TagsHierarchyController.*`
- 내비게이션 모드 상태/컨트롤러: `src/app/models/navigationbar/NavigationModeState.*`,
  `src/app/models/navigationbar/NavigationModeSectionController.*`,
  `src/app/models/navigationbar/NavigationModeController.*`
- 편집기 보기 모드 상태/컨트롤러: `src/app/models/navigationbar/EditorViewState.*`,
  `src/app/models/navigationbar/EditorViewSectionController.*`,
  `src/app/models/navigationbar/EditorViewModeController.*`
- 편집기 persistence 컨트롤러: `src/app/models/editor/persistence/ContentsEditorPersistenceController.*`
- 편집기 non-format 태그 helper: `src/app/models/editor/tags/ContentsAgendaBackend.*`,
  `src/app/models/editor/tags/ContentsCalloutBackend.*`,
  `src/app/models/editor/tags/ContentsStructuredTagValidator.*`,
  `src/app/models/editor/tags/WhatSonStructuredTagLinter.*`,
  `src/app/models/editor/tags/ContentsResourceTagTextGenerator.*`
- 사이드바 선택 store 인터페이스/구현: `src/app/store/sidebar/ISidebarSelectionStore.*`,
  `src/app/store/sidebar/SidebarSelectionStore.*`
- 사이드바 계층 wiring 인터페이스/컨트롤러: `src/app/models/sidebar/IHierarchyControllerProvider.*`,
  `src/app/models/sidebar/HierarchyControllerProvider.*`,
  `src/app/models/sidebar/SidebarHierarchyController.*`
- 전역 노트 active 상태 추적 객체: `src/app/models/panel/NoteActiveStateTracker.*`
- 아키텍처 policy lock 및 layer contract: `src/app/policy/ArchitecturePolicyLock.*`
- 런타임 bootstrap: 저장된 `.wshub` 선택을 mount할 수 있으면 앱 시작 시 workspace shell로 바로 진입할 수 있다. 저장된 시작 허브를 mount할 수 없으면 startup은 unmounted 상태를 유지하고 blueprint/sample workspace를 다시 여는 대신 onboarding으로 라우팅해야 한다. 저장된 시작 경로는 첫 workspace window가 생성되기 전에 runtime domain을 load하면 안 된다. `main.cpp`는 LVRS `AfterFirstIdle` lifecycle task를 통해 일반 full runtime load를 예약하고, 이후 `WhatSonRuntimeParallelLoader`의 Controller 상태를 적용한다. onboarding 중 명시적 허브 선택은 workspace로 전환하기 전에 선택된 허브를 load할 수 있다.

## 자동화 테스트 정책

- 이 저장소는 루트 CMake 타깃과 `test/` 아래에 in-repo 빌드 및 런타임 회귀 게이트를 유지한다.
- 유지되는 검증 진입점은 `whatson_build_regression`(빌드 게이트), `whatson_cpp_regression`(런타임 C++ 회귀 게이트), `whatson_regression`(기본 통합 게이트)이다.
- `scripts/test_*.py` 아래의 Python 테스트 스크립트는 제거되었으며 다시 도입하면 안 된다. 자동 회귀 커버리지는 CTest와 `test/`의 C++ 회귀 suite 아래에 유지한다.
- 사용자가 명시적으로 두 번째 테스트 표면을 요구하지 않는 한 `tests/*`, `scripts/test_*.py`, 또는 임시 중복 harness를 추가하지 않는다.
- 기본 작업 흐름은 인계 전 가장 작은 관련 검증 게이트를 실행하고, 모든 생성 산출물을 `build/` 아래에 둔다.

### 계층 컨트롤러 소유권 (중요)

- 각 계층 타입은 `src/app/models/file/hierarchy/*` 아래의 전용 Controller를 사용해야 한다.
- 모든 계층 카테고리를 하나의 공유 Controller instance에 바인딩하지 않는다.
- 표준 매핑:
    - Library -> `LibraryHierarchyController`
    - Projects -> `ProjectsHierarchyController`
    - Bookmarks -> `BookmarksHierarchyController`
    - Tags -> `TagsHierarchyController`
    - Resources -> `ResourcesHierarchyController`
    - Progress -> `ProgressHierarchyController`
    - Event -> `EventHierarchyController`
    - Preset -> `PresetHierarchyController`
- 계층 wiring 변경은 반드시 이 one-type/one-Controller contract를 보존해야 한다.
- 런타임 계층 Controller wiring에는 flat/shared hierarchy model abstraction을 금지한다.
- model/support 코드는 `src/app/models/file/hierarchy/<domain>/`의 도메인별 디렉터리 안에 격리한다.

### 컨트롤러 구현 경계 (중요)

- 이 저장소는 MVVM을 사용하지 않는다. `ViewModel` 개념, `src/app/viewmodel`, QML view-model registry binding, 또는 `LV.ViewModels`/`LV.Controllers` 기반 런타임 lookup을 도입하지 않는다. 뷰는 모델과 직접적으로 이어진다.
- Controller는 단일 책임 C++ class여야 한다.
- `src/app/models` 아래에 QML 파일을 추가하지 말고, 해당 디렉터리를 QML module source로 등록하지 않는다.
- QML은 view construction과 view-local behavior를 직접 소유한다. 버튼 dispatch, 메뉴 열기/닫기, pointer hit-test,
  transient visual state, focus presentation, view-local callback/signal coalescing처럼 화면 표면 안에서 끝나는
  동작은 QML에서 구현한다.
- QML에 둘 수 있는 것은 화면 이벤트를 어떤 함수에 꽂을지, 클릭 뒤 어떤 UI 후처리를 한 틱 늦게 실행할지,
  disabled 버튼을 한 번 더 막을지 같은 얇은 view wiring으로 제한한다.
- 상태 보존, rollback, 다중 호출 간 정합성, parser/renderer/geometry/line-metrics처럼 재현성과 테스트성이 필요한
  정책은 C++ model/controller 계층에 남긴다.
- model, controller, session, persistence, sync, parsing, domain mutation, scheduling, command-surface 책임을
  우회하기 위한 compatibility QML wrapper를 추가하지 않는다. 해당 책임은 single-purpose C++ object로 승격한다.
- 순수 view 동작을 C++ Controller signal round-trip으로 감싸는 간접 경로를 새로 만들지 않는다. view 동작이
  domain mutation을 필요로 할 때는 QML이 이미 노출된 좁은 model/controller/bridge API를 직접 호출한다.
- 각 C++ Controller는 좁은 signal/slot contract를 노출하고, domain mutation, timing, rendering, persistence 작업은 소유 model layer에 위임해야 한다.

### Model Layer, QML 의 역할 분담 (중요)

- `src/app/models/**`는 QML 파일을 포함하면 안 된다.
- `src/app/models` 아래에 남아 있는 모든 QML 파일은 C++(`.hpp/.cpp`)으로 변환 후 제거해야 할 migration debt로 취급한다.
- model layer는 QObject/Q_GADGET/QAbstractItemModel 기반 API를 QML에 노출할 수 있지만, domain logic, orchestration, controller behavior, mutable session state, formatting policy, parsing policy, mutation planning, resource import handling, input policy를 QML로 구현하면 안 된다.
- 기능이 `src/app/qml/view/**` 아래보다 낮은 레이어의 logic을 필요로 하면, `src/app/models` 아래에 또 다른 `*.qml` helper를 추가하지 말고 소유 도메인의 C++ object를 생성하거나 확장한다.
- `src/app/qml/**`만 QML의 허용 위치이며, 해당 파일은 view/presentation composition과 view-local behavior
  구현 역할에 머물러야 한다.
- qml에는 백엔드가 존재할 수 없으며, 오로지 뷰 구현에 관련한 동작만을 허용한다. 이 뷰 동작은 QML에서
  직접 구현하는 것을 기본값으로 한다.


### 편집기 Source of Truth (중요)

- `.wsnote`와 embedded `.wsnbody` content만 editor state의 write-authoritative source다.
- 모든 edit path는 다음 순서를 정확히 보존해야 한다.
  1. RAW `.wsnote/.wsnbody` source를 mutate한다.
  2. always-on parser가 최신 RAW snapshot을 reparse하게 한다.
  3. editor surface는 parsed projection에서만 render한다.
- live typing, shortcut, Enter handling, paste, drop-driven document mutation은 먼저 RAW source edit으로 변환되어야 한다.
- parser와 renderer layer는 RAW source snapshot에서 presentation state만 derive할 수 있으며, write-authoritative document model이 되면 안 된다.
- RichText/DOM/QML editor surface는 persisted note source로 자기 자신을 primary save path로 serialize하면 안 된다.
- temporary RichText fragment, DOM split, editor-surface HTML은 read-side projection detail일 뿐이며 alternate persistence 또는 formatting authority로 사용하면 안 된다.
- focus restoration, caret movement, block re-materialization은 incidental DOM 구조가 아니라 RAW source offset 또는 parser-derived block coordinate를 기준으로 계산해야 한다.
- parser blockization이 끝나면 renderer/QML host가 publish하기 전에 block projection을 하나의 interactive document block stream으로 normalize해야 하며, client-side duplicate regrouping path는 금지한다.
- 일반 note editing은 persisted backing store가 `.wsnbody`인 Apple Notes 같은 document surface로 취급한다.
- note editor는 plain prose, semantic text block, `<resource ... />`, `agenda`, `callout`, `break`를 포함한 모든 note-body element를 하나의 document host 안의 ordinary block으로 취급해야 한다.
- note session이 bound된 뒤의 표준 note document host는 `ContentsStructuredDocumentFlow.qml`이다.
- active note 선택과 편집기 세션 mount는 전역 `NoteActiveStateTracker`가 같은 update turn에서 연결해야 한다.
  view QML은 현재 표시 중인 `ContentsEditorSessionController`를 이 전역 객체에 등록할 수 있지만, active note
  본문 스냅샷에서 세션을 갱신하는 책임을 각 view가 독립적으로 재구현하면 안 된다.
- specialized block delegate는 presentation을 바꿀 수 있지만, note를 별도 editor mode 또는 다른 persistence authority로 분리하면 안 된다.
- editor-session persistence orchestration은 `src/app/models/editor/persistence` 아래에 둔다.
  `ContentsEditorPersistenceController`는 note-body snapshot buffering, immediate enqueue attempt, persistence retry/drain scheduling, pending-snapshot adoption, selected-note body read, post-save reconcile handoff를 소유한다.
- non-format editor-body tag 책임은 `src/app/models/editor/tags` 아래에 둔다. 여기에는 agenda/task, callout, break, structured-tag lint/correction advisory state, RAW resource-tag construction이 포함된다.
- `src/app/models/file/note`는 editor persistence layer가 IO용 RAW snapshot을 선택한 뒤의 concrete `.wsnote/.wsnbody` file mutation만 소유한다. editor-session dirty buffer나 editor save-timing policy를 소유하면 안 된다.

### 입력기 권한 (중요)

- editor input layer는 OS/Qt IME 처리를 live `LV.TextEditor.editorItem` path에 맡겨야 한다.
- 명시적 tag-management command를 제외하고, editor QML은 ordinary note editing을 위한 custom text input handler를 설치하면 안 된다. 일반 `Enter`, `Backspace`, `Delete`, arrow navigation, selection extension, repeat, IME gesture는 native text-editing behavior로 유지해야 한다.
- 허용되는 tag-management input은 inline style tag, agenda/callout/break/resource tag insertion, selected atomic resource/break block management 같은 RAW `.wsnbody` tag operation으로 제한한다.
- Markdown list shortcut, markdown list Enter continuation, generic text-boundary key override는 editor input layer에서 허용하지 않는다.
- editor QML에서 `Qt.inputMethod.update(...)`, `Qt.inputMethod.show()`, `Qt.inputMethod.hide()`, 또는 bare QML `InputMethod.*` singleton을 호출하지 않는다.
- `Qt.inputMethod && ...`, `Qt.inputMethod.visible !== undefined` guard처럼 alternate input-method object를 허용하는 fallback branch를 추가하지 않는다.
- text editor wrapper는 native composition이 안정될 때까지 persistence나 programmatic sync를 defer하기 위해서만 `LV.TextEditor.editorItem.inputMethodComposing`과 `LV.TextEditor.editorItem.preeditText`를 native text state로 관찰할 수 있다.
- 이 primitive들이 LVRS로 승격되기 전까지 editor QML은 native-input session, shortcut-surface, context-menu long-press, focused programmatic-sync, ordinary text-edit focus-restore 결정을 `src/app/models/editor/input/ContentsEditorInputPolicyAdapter.qml`을 통해 라우팅해야 한다.

## LVRS 통합의 단일 Source of Truth

### CMake

app target은 이 pattern을 유지해야 한다.

```cmake
find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2)
find_package(LVRS CONFIG REQUIRED)

qt_add_executable(WhatSon main.cpp)
qt_add_qml_module(WhatSon
        URI WhatSon.App
        VERSION 1.0
        RESOURCE_PREFIX "/qt/qml"
        QML_FILES
        qml/Main.qml
)

lvrs_configure_qml_app(WhatSon)
```

### QML

모든 LVRS 기반 QML 파일은 이 import baseline을 사용한다.

```qml
import QtQuick
import LVRS 1.0 as LV
```

## 금지 변경

- LVRS 통합을 위해 manual `rpath`, manual `-F`, manual framework path를 추가하지 않는다.
- `QQmlApplicationEngine`에 LVRS-specific import/plugin path를 수동 주입하지 않는다.
- LVRS 대체물로 `QtQuick.Controls.Material` 중심 UI로 회귀하지 않는다.
- LVRS component screen에 임의의 Qt default widget styling을 섞지 않는다.

## 허용 예외

예외는 다음 조건을 모두 만족할 때만 허용한다.

1. `find_package(LVRS)`와 `lvrs_configure_qml_app()`이 재현 가능하게 실패한다.
2. 실패 로그가 명시적이고 보존되어 있다.
3. 변경이 재현 가능하며 임시 우회가 아니다.
4. 이유와 rollback condition이 PR/commit message에 문서화되어 있다.

## 현재 UI 레이아웃

- Root shell: `src/app/qml/Main.qml`(`LV.ApplicationWindow`)
- View directory(legacy `shell`과 `pages`에서 병합됨):
    - `src/app/qml/view/top/StatusBarLayout.qml`
    - `src/app/qml/view/top/NavigationBarLayout.qml`
    - `src/app/qml/view/body/BodyLayout.qml`
    - `src/app/qml/view/mobile/MobilePageScaffold.qml`(Figma VStack node `174:4987`용 shared mobile workspace scaffold. compact navigation bar와 compact status/add-note bar를 page 전환 중에도 mount 상태로 유지)
    - `src/app/qml/view/mobile/pages/MobileHierarchyPage.qml`(hierarchy node `174:5026`용 Figma-driven routed mobile workspace page. `MobilePageScaffold.qml`와 shared hierarchy panel로 구성)
    - `src/app/qml/view/panels/ListBarLayout.qml`(Figma-driven list bar panel, node `73:2635`)
    - `src/app/qml/view/panels/ListBarHeader.qml`(Figma-driven list bar header, node `134:3180`)
        - `src/app/qml/view/panels/NoteListItem.qml`(Figma-driven note item card, node `119:3028`)
        - `src/app/qml/view/panels/DetailPanelLayout.qml`(Figma-driven right panel wrapper, node `134:3212`)
        - `src/app/qml/view/panels/navigation/NavigationPropertiesBar.qml`(Figma frame `PropertiesBar`, node `147:3876`)
        - `src/app/qml/view/panels/navigation/NavigationInformationBar.qml`(Figma frame `InformationBar`, node `134:3138`)
        - `src/app/qml/view/panels/navigation/NavigationModeBar.qml`(Figma frame `NavigationMode`, node `147:3875`)
        - `src/app/qml/view/panels/navigation/NavigationEditorViewBar.qml`(Figma frame `EditorViewMoide`, node `148:3888`)
        - `src/app/qml/view/panels/navigation/NavigationApplicationViewBar.qml`
        - `src/app/qml/view/panels/navigation/NavigationApplicationEditBar.qml`
        - `src/app/qml/view/panels/navigation/NavigationApplicationControlBar.qml`
            - `src/app/qml/view/panels/detail/RightPanel.qml`(Figma frame `RightPanel`, node `134:3212`)
            - `src/app/qml/view/panels/detail/DetailPanel.qml`(Figma frame `DetailPanel`, node `134:3641`)
            - `src/app/qml/view/panels/detail/DetailPanelHeaderToolbar.qml`(Figma frame `DetailPanelHeaderToolbar`, node `134:3642`)
            - `src/app/qml/view/panels/detail/DetailContents.qml`(Figma frame `DetailContents`, node `134:3649`)
                - `src/app/qml/view/body/HierarchySidebarLayout.qml`
                    - `src/app/qml/view/panels/ContentViewLayout.qml`
                    - `src/app/qml/view/contents/ContentsView.qml`
                    - `src/app/qml/view/contents/EditorView.qml`
                    - `src/app/qml/view/contents/Minimap.qml`
                    - `src/app/qml/view/contents/editor/ContentsDisplayView.qml`
                    - `src/app/qml/view/contents/editor/ContentsDisplaySurfaceHost.qml`
                    - `src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml`
                    - `src/app/qml/view/contents/editor/ContentsInlineFormatEditorController.qml`
                    - `src/app/qml/view/contents/editor/ContentsResourceEditorView.qml`
                    - `src/app/qml/view/contents/editor/ContentsResourceViewer.qml`
                    - `src/app/qml/view/contents/editor/ContentsStructuredDocumentFlow.qml`
                - `src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml`
                - `src/app/qml/view/panels/sidebar/HierarchyViewLibrary.qml`
                - `src/app/qml/view/panels/sidebar/HierarchyViewProjects.qml`
                - `src/app/qml/view/panels/sidebar/HierarchyViewBookmarks.qml`
                - `src/app/qml/view/panels/sidebar/HierarchyViewTags.qml`
                - `src/app/qml/view/panels/sidebar/HierarchyViewResources.qml`
                - `src/app/qml/view/panels/sidebar/HierarchyViewProgress.qml`
                - `src/app/qml/view/panels/sidebar/HierarchyViewEvent.qml`
                - `src/app/qml/view/panels/sidebar/HierarchyViewPreset.qml`
- Shared components:
    - `src/app/qml/components/NavigationRail.qml`
    - `src/app/qml/components/MetricCard.qml`
    - `src/app/qml/components/InfoListCard.qml`
    - `src/app/qml/components/InsightPanel.qml`
- Contents view namespace:
    - `src/app/qml/view/contents` is the single contents-view namespace.
    - Do not reintroduce `src/app/qml/contents` or `src/app/qml/view/content`.

## 시작 체크리스트

1. `CMakeLists.txt`와 `src/app/CMakeLists.txt`가 여전히 LVRS baseline pattern과 일치하는지 확인한다. root orchestration과 per-directory `add_subdirectory(...)` split을 맞춘 상태로 유지한다.
2. QML import가 일관되게 `import LVRS 1.0 as LV`인지 확인한다.
3. 새 UI는 LVRS component를 먼저 사용해 설계한다.

## 플랜 및 구현 체크리스트

1. 계산과 동작은 반드시 C++로 구현되어야 하고, 오로지 뷰만을 Qml에서 다룬다. Qml은 객체 모델링 및 메서드 정의 등 백엔드 논리를 전담할 수 없다.
2. 


## 완료 체크리스트

1. 변경된 source와 repository policy 변경을 문서가 함께 반영한다.
2. 수정된 QML은 LVRS import와 component usage 일관성을 유지한다.
3. model/controller/view contract는 signal-slot hook을 보존한다. C++에서는 `signals`+`slots`, QML view에서는 `signal` + callable hook function을 사용한다.
4. 사용자가 명시적으로 달리 지시하지 않는 한 인계 전 `build/`에서 가장 작은 관련 검증 게이트를 실행하고, 대체 build directory를 만들지 않는다.

## 유지보수 규칙

- build-system 단순성을 우선한다.
- C++ entrypoint는 작고 집중된 상태로 유지한다.
- design token에는 LVRS `Theme`를 우선 사용한다.
- 명령 예시와 실제 output path를 docs에서 동기화한다.
- project-local C++ include는 configured include root 기준의 repository-absolute include를 사용한다. basename-only 또는 file-relative include 대신 `app/...`, `extension/...`, `test/...`를 사용한다.
- model/controller/view interface는 event-driven으로 유지한다. 모든 model, controller, view는 적어도 하나의 signal과 하나의 slot/hook entrypoint를 노출해야 한다.
- hierarchy wiring은 type-safe로 유지한다. 각 hierarchy category는 `src/app/models/file/hierarchy` 아래의 전용 Controller에 계속 bound되어야 한다.
- QML에는 SRP를 강제한다. 하나의 QML 파일이 layout shell, rendering delegate, interaction logic을 함께 품기 시작하면 같은 domain directory 아래의 dedicated sibling module로 분리하고 parent screen에서 compose한다.
- 복잡한 editor surface에는 최소 reusable module을 우선한다. 예: minimap layer, splitter interaction layer. 모든 delegate를 하나의 root file에 embed하지 않는다.
- decomposition work와 quality gate를 맞춘다.
    - QML refactor 후 `cmake --build build --target whatson_qmllint -j`를 실행한다.
    - C++ refactor 후 `clang-tidy`가 설치되어 있으면 `cmake --build build --target whatson_clang_tidy -j`를 실행한다. 사용할 수 없으면 완료 보고에 누락 도구를 명시한다.
    - build-system 또는 compilation-surface 변경 후 `cmake --build build --target whatson_build_regression -j`를 실행한다.
    - C++ 또는 cross-domain behavior 변경 후 `cmake --build build --target whatson_regression -j`를 실행한다.

## Troubleshooting 기준선

- LVRS를 찾지 못하면 `CMAKE_PREFIX_PATH`에 LVRS prefix를 제공한다.
- QML module을 찾지 못하면 LVRS 설치 상태(`lib/qt6/qml/LVRS/qmldir`)를 확인한다.
- link warning은 먼저 `lvrs_configure_qml_app()`에 맡기고 manual link addition은 피한다.

## Agent 응답 및 작업 규칙

- 추정 대신 실제 repository file과 log를 근거로 사용한다.
- 변경은 작고 검증 가능하게 유지한다.
- 완료 보고에는 변경 파일, 검증 명령, 알려진 한계를 포함한다.
- 검증을 생략했거나 실행할 수 없었으면 명시적으로 말하고, 사용자가 거절했는지 환경이 막았는지 설명한다.
- 작업 중 사용자가 개입하면 최신 사용자 지시를 우선하고, 충돌하는 기존 방향은 즉시 재정렬한다. 사용자의 개입은 작업 중단 신호가 아니라 현재 목표를 더 정확히 맞추기 위한 상위 입력으로 취급한다.
- 프로젝트 내부 산출물은 기본적으로 영어를 사용한다. 단, 사용자-facing 답변과 이 `AGENTS.md`는 한국어를 사용한다.
- 큰 코드베이스 변경이나 새 요구사항이 도입되면 `AGENTS.md`를 함께 재정비한다.
