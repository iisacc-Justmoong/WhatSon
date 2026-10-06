# `src/app/models/hierarchy/projects/ProjectsHierarchyController.hpp`

<a id="status"></a>

## 상태
- 문서화 단계: 라이브 소스 트리에서 생성된 스캐폴드입니다.
- 세부 수준: 이후 딥 패스를 위해 준비된 구조적 자리 표시자입니다.

<a id="source-metadata"></a>

## 소스 메타데이터
- 소스 경로: `src/app/models/hierarchy/projects/ProjectsHierarchyController.hpp`
- 소스 종류: C++ 헤더
- 파일 이름: `ProjectsHierarchyController.hpp`
- 대략적인 줄 수: 111

<a id="extracted-symbols"></a>

## 추출된 기호
- 선언된 네임스페이스 존재: 아니요
- QObject 매크로 존재: 예

<a id="classes-and-structs"></a>

### 클래스와 구조체
- `ProjectsHierarchyController`

<a id="current-public-surface-highlights"></a>

## 현재 공공 장소 하이라이트

- 프로젝트 계층 구조에 대한 이름 바꾸기, 생성/삭제, 재정렬 및 확장 기능을 구현합니다.
- `LibraryNoteListModel`를 노출하여 프로젝트 도메인이 활성 계층 선택과 일치하는 `.wsnhead` `project` 레이블의 메모를 표시할 수 있도록 합니다.
- `noteDirectoryPathForNoteId(...)`를 상세 패널 주석 헤더 쓰기용으로 노출하며, 예상되는 정규 디렉터리 해상도 계약(인덱싱된 경로가 먼저, `.wsnhead` 디렉터리 대체 경로).
- 신체 상태 업데이트 및 편집기 통계 새로 고침 API가 노트 편집기/저장 경계와 함께 제거되었습니다.
- `setItemExpanded(int, bool)`와 `setAllItemsExpanded(bool)`를 모두 노출하여 향후 확장 가능한 프로젝트 행이 사이드바 푸터 컨텍스트 메뉴와 동일한 프로젝트 소유 확장 상태를 공유할 수 있도록 합니다.
- 상속된 기능 메서드를 명시적인 `override`와 함께 선언하여 재배열/재명명/크루드 계약이 컴파일 시 체크되고 경고가 깨끗하게 유지되도록 합니다.
- `applyHierarchyMove(...)`를 명시적인 중첩 프로젝트 폴더 이동을 위한 대상 헬퍼로 선언합니다. 사이드바 드래그/드롭 표면은 최종 LVRS 모델 스냅샷의 전체 노드 재생을 사용합니다.

<a id="enums"></a>

### 열거형
- 스캐폴드 생성 중에 감지된 항목이 없습니다.

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
