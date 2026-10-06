# `src/app/qml/view/panels/NavigationBarLayout.qml`

<a id="status"></a>

## 상태
- 문서화 단계: 라이브 소스 트리에서 생성된 스캐폴드입니다.
- 세부 수준: 이후 딥 패스를 위해 준비된 구조적 자리 표시자입니다.

<a id="source-metadata"></a>

## 소스 메타데이터
- 소스 경로: `src/app/qml/view/panels/NavigationBarLayout.qml`
- 소스 종류: QML 뷰/컴포넌트
- 파일 이름: `NavigationBarLayout.qml`
- 대략적인 줄 수: 262

<a id="qml-surface-snapshot"></a>

## QML 표면 스냅샷
- 루트 유형: `Rectangle`

<a id="object-ids"></a>

### 개체 ID
- `navigationBar`
- `navigationBarSurface`
- `navigationBarContents`
- `propertiesBar`
- `applicationBarLoader`
- `applicationViewBarComponent`
- `applicationEditBarComponent`
- `applicationControlBarComponent`

<a id="required-properties"></a>

### 필수 속성
- 스캐폴드 생성 중에 감지된 항목이 없습니다.

<a id="signals"></a>

### 신호
- `compactAddFolderRequested`
- `compactLeadingActionRequested`
- `dayCalendarRequested`
- `monthCalendarRequested`
- `toggleDetailPanelRequested`
- `toggleSidebarRequested`
- `viewHookRequested`
- `weekCalendarRequested`
- `yearCalendarRequested`

<a id="recent-updates"></a>

## 최근 업데이트
- `pragma ComponentBehavior: Bound` 를 추가하여 중첩 모드 `Component` 분기들이 자격 없는 범위 경고 없이 `navigationBar` id 멤버를 안전하게 참조할 수 있습니다. 활성 View/Edit/Control 애플리케이션 바 컴포넌트에서 로드된 `nodesnewFolder` 폴더 추가 버튼 하나와 모드별 애플리케이션 바 슬롯 하나입니다. 활성 View/Edit/Control 애플리케이션 바 컴포넌트입니다. 이것은 활성 콘텐츠 라우트가 계층 구조나 메모 목록 라우트로 해당 기능 (affordance) 을 누출시키지 않고 컴팩트 컨텍스트 메뉴 내부에 해당 작업을 중복하지 않으면서 전용 오른쪽 테두리 상세 페이지 아이콘 버튼을 표시할 수 있게 합니다.
- 편집기 뷰 모드 선택기는 제거됩니다. 데스크톱 및 컴팩트 탐색은 편집기 뷰 모드 컨트롤러를 전달하거나 편집기 전용 뷰 크롬을 마운트하지 않아야 합니다. `panelBackground10` (`#343536`) 를 해당 피플 배경에 사용해야 하며 투명하게 두거나 페이지 캔버스 톤을 상속하지 않아야 합니다.
- View/Edit 애플리케이션 바에서 발생하는 캘린더 훅 사유는 `handleApplicationBarViewHook(...)`에서 정규화됩니다. `daily-calendar`, `weekly-calendar`, `monthly-calendar`, 또는 `yearly-calendar`가 일치하는 캘린더 요청 신호를 발생시하여 `Main.qml`가 해당 콘텐츠 오버레이를 열 수 있도록 합니다.

<a id="intended-detailed-sections"></a>

## 의도된 세부 섹션
- 책임과 비즈니스 역할
- 소유권 및 수명주기
- 공개 API 또는 외부에서 관찰된 바인딩
- 협력자 및 의존성 방향
- 데이터 흐름 및 상태 전환
- 오류 처리 및 복구 경로
- 관련된 경우 스레딩, 스케줄링 또는 UI 선호도 제약 조건
- 확장점, 불변성 및 알려진 복잡성 핫스팟
- 테스트 적용 범위 및 검증 누락

<a id="authoring-notes-for-next-pass"></a>

## 다음 패스에 대한 작성 노트
- 이 스캐폴드를 교체하기 전에 실제 구현과 인접한 헤더를 읽어보세요.
- 해당하는 경우 구체적인 신호, 슬롯, 호출 가능 항목, 지속성 부작용 및 LVRS/QML 바인딩을 문서화합니다.
- 세부 단계가 시작되면 이 파일을 동일한 디렉터리에 있는 피어 모듈과 상호 연결하세요.
