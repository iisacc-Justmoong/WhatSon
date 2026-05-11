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
- 라이브러리 계층 백엔드: `src/app/models/hierarchy/library`
- 라이브러리 계층 모델/컨트롤러: `src/app/models/hierarchy/library/LibraryHierarchyModel.*`,
  `src/app/models/hierarchy/library/LibraryHierarchyController.*`
- 라이브러리 오른쪽 패널 목록 모델: `src/app/models/hierarchy/library/LibraryNoteListModel.*`
- 허브 배치 store: `src/app/models/file/hub/WhatSonHubPlacementStore.*`
- 태그 depth provider: `src/app/models/hierarchy/tags/WhatSonHubTagsDepthProvider.*`
- 허브 런타임 store: `src/app/models/file/hub/WhatSonHubRuntimeStore.*`
- 런타임 병렬 bootstrap loader: `src/app/runtime/threading/WhatSonRuntimeParallelLoader.*`
- 태그 런타임 상태 store: `src/app/models/hierarchy/tags/WhatSonHubTagsStateStore.*`
- 태그 계층 모델/컨트롤러: `src/app/models/hierarchy/tags/TagsHierarchyModel.*`,
  `src/app/models/hierarchy/tags/TagsHierarchyController.*`
- 내비게이션 모드 상태/컨트롤러: `src/app/models/navigationbar/NavigationModeState.*`,
  `src/app/models/navigationbar/NavigationModeSectionController.*`,
  `src/app/models/navigationbar/NavigationModeController.*`
- 사이드바 선택 store 인터페이스/구현: `src/app/store/sidebar/ISidebarSelectionStore.*`,
  `src/app/store/sidebar/SidebarSelectionStore.*`
- 사이드바 계층 wiring 인터페이스/컨트롤러: `src/app/models/sidebar/IHierarchyControllerProvider.*`,
  `src/app/models/sidebar/HierarchyControllerProvider.*`,
  `src/app/models/sidebar/SidebarHierarchyController.*`
- 전역 노트 active 상태 추적 객체: `src/app/models/panel/NoteActiveStateTracker.*`
- 지속 태그 삽입 writer: `src/app/models/editor/insert/TagInsertionWriter.*`
- 아키텍처 policy lock 및 layer contract: `src/app/policy/ArchitecturePolicyLock.*`
- 런타임 bootstrap: 저장된 `.wshub` 선택을 mount할 수 있으면 앱 시작 시 workspace shell로 바로 진입할 수 있다. 저장된 시작 허브를 mount할 수 없으면 startup은 unmounted 상태를 유지하고 blueprint/sample workspace를 다시 여는 대신 onboarding으로 라우팅해야 한다. 저장된 시작 경로는 첫 workspace window가 생성되기 전에 runtime domain을 load하면 안 된다. `main.cpp`는 LVRS `AfterFirstIdle` lifecycle task를 통해 일반 full runtime load를 예약하고, 이후 `WhatSonRuntimeParallelLoader`의 Controller 상태를 적용한다. onboarding 중 명시적 허브 선택은 workspace로 전환하기 전에 선택된 허브를 load할 수 있다.

## 자동화 테스트 정책

- 이 저장소는 루트 CMake 타깃과 `test/` 아래에 in-repo 빌드 및 런타임 회귀 게이트를 유지한다.
- 유지되는 검증 진입점은 `whatson_build_regression`(빌드 게이트), `whatson_cpp_regression`(런타임 C++ 회귀 게이트), `whatson_regression`(기본 통합 게이트)이다.
- `scripts/test_*.py` 아래의 Python 테스트 스크립트는 제거되었으며 다시 도입하면 안 된다. 자동 회귀 커버리지는 CTest와 `test/`의 C++ 회귀 suite 아래에 유지한다.
- 사용자가 명시적으로 두 번째 테스트 표면을 요구하지 않는 한 `tests/*`, `scripts/test_*.py`, 또는 임시 중복 harness를 추가하지 않는다.
- 기본 작업 흐름은 인계 전 가장 작은 관련 검증 게이트를 실행하고, 모든 생성 산출물을 `build/` 아래에 둔다.
- `build/` 루트에는 임시 WhatSon 진단 로그, 스크린샷 PNG, `.wsnbody` 백업 XML, Finder metadata를 남기지 않는다.
  불가피한 임시 산출물은 소유 하위 디렉터리에 두고, 일반 회귀 흐름은 `whatson_clean_build_extras`로 루트 clutter를 정리해야 한다.

### 계층 컨트롤러 소유권 (중요)

- 각 계층 타입은 `src/app/models/hierarchy/*` 아래의 전용 Controller를 사용해야 한다.
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
- model/support 코드는 `src/app/models/hierarchy/<domain>/`의 도메인별 디렉터리 안에 격리한다.
- Event/Preset처럼 문자열 목록 기반의 단순 계층 도메인은 전용 Controller class를 유지하되,
  반복되는 depth item parsing/serialization/selection/expansion support는
  `src/app/models/hierarchy/WhatSonNamedStringHierarchySupport.hpp`의 typed C++ helper를 공유한다.
- Projects 계층은 Library 계층처럼 child project를 허용하는 nested depth tree다. `ProjectsHierarchyController`와
  `ProjectLists.wsproj` parser/store/creator는 depth, child drop, subtree delete/reorder를 보존해야 하며 flat-only
  project list 정책으로 되돌리면 안 된다.

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
- `LibraryHierarchyController`는 계층/선택 조정자 역할에 머물러야 하며 note-list projection/cache 조립은
  `WhatSonLibraryNoteListProjection` 같은 전용 C++ collaborator에 둔다.
- note record lookup, persisted body-state 반영, note directory/body-source 조회처럼 여러 note-backed hierarchy
  Controller가 반복하는 저수준 note-record 작업은 `WhatSonHierarchyNoteRecordSupport`에 둔다.
- `ResourcesImportController`는 import orchestration과 conflict/persistence 처리를 소유하고, clipboard 이미지
  MIME/data-url 추출 정책은 `WhatSonResourceClipboardImportSupport`에 둔다.

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

- 현재 LVRS 갱신 계약에서 본문 편집기 QML의 단일 표면은 `LV.TextEditor`다.
- workspace route의 shell/layout은 기존 status bar, navigation bar, sidebar, note list, detail panel,
  mobile hierarchy scaffold 구조를 유지한다.
- editor content surface는 LVRS `TextEditor` 중심으로 유지하되, active note의 `.wsnbody` 파일 경로를
  직접 연결하지 않는다. `ContentViewLayout.qml`은 `NoteEditorDocumentSession`이 만든 parsed RAW source
  session file 경로만 `LV.TextEditor.filePath`로 소비할 수 있다.
- contents 내부 QML(`src/app/qml/view/contents`)에는 `Gutter.qml`, `TextEditor.qml`, `Minimap.qml` 세 뷰만 허용한다.
- `TextEditor.qml`의 root는 `LV.TextEditor`여야 하며 `filePath`는 `noteBodyFilePath`를 통해
  `NoteEditorDocumentSession`이 만든 parsed RAW source session file을 가리킨다. 선택된 노트가 없으면
  blank session file 또는 빈 문자열로 둔다.
- `ContentViewLayout.qml`은 contents alias를 통해 `Gutter.qml`, `TextEditor.qml`, `Minimap.qml` 세 뷰만 mount한다.
- `Gutter.qml`은 선택 노트 id/path, parsed source session file 경로, C++ `NoteEditorDocumentSession`의
  `parsedLineCount`, 그리고 `TextEditor.qml`이 전달하는 viewport offset/line height만 입력으로 받아 line
  number rail을 표시한다. 거터가 직접 파일을 읽거나 `.wsnbody`를 파싱하면 안 된다.
- clipboard image paste는 예외적으로 `ContentViewLayout.qml`의 얇은 `StandardKey.Paste` command wiring을
  허용한다. 이 QML은 `ResourcesImportController.importClipboardImageForEditor()`,
  `NoteEditorDocumentSession.insertImportedResourcesIntoSource(...)`, `TextEditor.qml`의 LVRS document 반영 hook만
  순서대로 호출해야 하며, MIME 판별, resource import, RAW tag 생성, `.wsnbody` persistence 정책을 구현하면 안 된다.
- `ContentViewLayout.qml`의 note editor branch는 `ContentsEditorDisplayBackend`, page/print renderer,
  resource editor, structured-document wrapper, projection, renderer를 직접 mount하지 않는다.
- `EditorViewModeController`, `EditorViewSectionController`, `EditorViewState`, `NavigationEditorViewBar.qml`의
  Plain/Page/Print/Web/Presentation 선택 계약은 navigation bar의 view-mode 콤보박스 계약으로 유지한다.
  단, 이 콤보박스는 현재 editor surface를 `LV.TextEditor`에서 다른 renderer로 전환하는 백엔드 계약이 아니며,
  `ContentViewLayout.qml`은 계속 parsed RAW source session file만 `LV.TextEditor.filePath`로 소비해야 한다.
- 새 편집 정책은 LVRS `TextEditor` 갱신 계약이 먼저 정의된 뒤 그 계약을 중심으로 추가한다. 기존 QML 호환 wrapper,
  RichText overlay, 직접 `TextEdit` adapter, snapshot cache, projection cache, renderer bridge를 되살리지 않는다.
- `.wsnote/.wsnbody` parser/projection/rendering, tag mutation 같은 domain 책임은 QML 계약의 일부가 아니다.
  노트 본문을 편집기에 연결할 때는 C++ `NoteEditorDocumentSession`이 `.wsnbody`를 parsed RAW source로
  mount하고, LVRS `TextEditor`의 session file sync 뒤 C++ persistence가 다시 `.wsnbody`로 serialize해야 한다.
- 태그 삽입을 실제 파일에 반영할 때는 C++ `TagInsertionWriter`가 `SetTag`의 RAW source 변환 결과를 받아
  `WhatSonLocalNoteFileStore`를 통해 `.wsnbody`에 저장한다. QML은 대상 노트와 cursor/selection만 전달한다.

### 입력기 권한 (중요)

- editor input layer는 OS/Qt IME 처리를 live `LV.TextEditor` path에 맡겨야 한다. 현재 LVRS 갱신 계약에서는
  note body surface도 `LV.TextEditor`를 직접 배치하고 `filePath`는 parsed RAW source session file 경로를
  따른다.
- 현재 editor QML은 ordinary note editing을 위한 custom text input handler, tag-management key handler,
  rendered selection handler를 설치하지 않는다. 단 clipboard image paste의 `StandardKey.Paste` command wiring은
  위 편집기 Source of Truth 섹션의 제한 안에서만 허용한다.
- Markdown list shortcut, markdown list Enter continuation, generic text-boundary key override는 editor input layer에서 허용하지 않는다.
- editor QML에서 `Qt.inputMethod.update(...)`, `Qt.inputMethod.show()`, `Qt.inputMethod.hide()`, 또는 bare QML `InputMethod.*` singleton을 호출하지 않는다.
- `Qt.inputMethod && ...`, `Qt.inputMethod.visible !== undefined` guard처럼 alternate input-method object를 허용하는 fallback branch를 추가하지 않는다.
- text editor wrapper는 programmatic sync defer를 위한 native text state 관찰을 현재 계약에 포함하지 않는다.
  입력 표면의 파일 읽기/저장은 LVRS `TextEditor`의 `filePath` 기반 sync 계약에 맡기되, `.wsnbody`
  parse/serialize는 C++ `NoteEditorDocumentSession`이 담당한다.

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
- Workspace route: `Main.qml`은 기존 desktop/mobile layout shell을 유지한다. `BodyLayout.qml`의 content slot이
  `src/app/qml/view/panels/ContentViewLayout.qml`을 mount하고, 그 내부는 gutter/TextEditor/minimap으로 제한하되
  선택된 노트의 `.wsnbody`를 C++에서 parsed RAW source session file로 mount한 뒤 `LV.TextEditor.filePath`에
  연결한다.
- View directory(legacy `shell`과 `pages`에서 병합됨):
    - `src/app/qml/view/panels/StatusBarLayout.qml`
    - `src/app/qml/view/panels/NavigationBarLayout.qml`
    - `src/app/qml/view/panels/BodyLayout.qml`
    - `src/app/qml/view/mobile/MobilePageScaffold.qml`(shared mobile workspace scaffold)
    - `src/app/qml/view/mobile/pages/MobileHierarchyPage.qml`(routed mobile workspace page. editor route는 `ContentViewLayout.qml`을 통해 active note의 parsed source session file을 편집한다)
    - `src/app/qml/view/panels/ListBarLayout.qml`(Figma-driven list bar panel, node `73:2635`)
    - `src/app/qml/view/panels/ListBarHeader.qml`(Figma-driven list bar header, node `134:3180`)
        - `src/app/qml/view/panels/NoteListItem.qml`(Figma-driven note item card, node `119:3028`)
        - `src/app/qml/view/panels/DetailPanelLayout.qml`(Figma-driven right panel wrapper, node `134:3212`)
        - `src/app/qml/view/panels/navigation/NavigationPropertiesBar.qml`(Figma frame `PropertiesBar`, node `147:3876`)
        - `src/app/qml/view/panels/navigation/NavigationInformationBar.qml`(Figma frame `InformationBar`, node `134:3138`)
        - `src/app/qml/view/panels/navigation/NavigationModeBar.qml`(Figma frame `NavigationMode`, node `147:3875`)
        - `src/app/qml/view/panels/navigation/NavigationApplicationViewBar.qml`
        - `src/app/qml/view/panels/navigation/NavigationApplicationEditBar.qml`
        - `src/app/qml/view/panels/navigation/NavigationApplicationControlBar.qml`
            - `src/app/qml/view/panels/detail/RightPanel.qml`(Figma frame `RightPanel`, node `134:3212`)
            - `src/app/qml/view/panels/detail/DetailPanel.qml`(Figma frame `DetailPanel`, node `134:3641`)
            - `src/app/qml/view/panels/detail/DetailPanelHeaderToolbar.qml`(Figma frame `DetailPanelHeaderToolbar`, node `134:3642`)
            - `src/app/qml/view/panels/detail/DetailContents.qml`(Figma frame `DetailContents`, node `134:3649`)
                - `src/app/qml/view/body/HierarchySidebarLayout.qml`
                    - `src/app/qml/view/panels/ContentViewLayout.qml`
                    - `src/app/qml/view/contents/Gutter.qml`
                    - `src/app/qml/view/contents/TextEditor.qml`
                    - `src/app/qml/view/contents/Minimap.qml`
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
5. 빌드/검증 중 생성된 임시 진단 로그, 스크린샷, 백업 파일은 `build/` 루트에 남기지 않는다.

## 유지보수 규칙

- build-system 단순성을 우선한다.
- C++ entrypoint는 작고 집중된 상태로 유지한다.
- design token에는 LVRS `Theme`를 우선 사용한다.
- 명령 예시와 실제 output path를 docs에서 동기화한다.
- project-local C++ include는 configured include root 기준의 repository-absolute include를 사용한다. basename-only 또는 file-relative include 대신 `app/...`, `extension/...`, `test/...`를 사용한다.
- model/controller/view interface는 event-driven으로 유지한다. 모든 model, controller, view는 적어도 하나의 signal과 하나의 slot/hook entrypoint를 노출해야 한다.
- hierarchy wiring은 type-safe로 유지한다. 각 hierarchy category는 `src/app/models/hierarchy` 아래의 전용 Controller에 계속 bound되어야 한다.
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
