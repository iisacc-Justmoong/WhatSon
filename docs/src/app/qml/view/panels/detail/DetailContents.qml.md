# `src/app/qml/view/panels/detail/DetailContents.qml`

<a id="responsibility"></a>

## 책임
`DetailContents.qml`는 데스크탑 세부 패널의 상태 전환 본체를 소유합니다. 구현된 기본 양식은 Figma `Properties` 예제와 라이브 `fileStat` 통계 양식입니다.

<a id="root-contract"></a>

## 루트 계약
- 루트 `objectName`: `DetailContents`
- 입력에는 활성 콘텐츠 컨트롤러, 세부 패널 컨트롤러, 파일 상태 컨트롤러 및 계층 선택기 컨트롤러가 포함됩니다.
- 정식 상태는 `detached`, `properties`, `fileStat`, `insert`, `fileHistory`, `layer` 및 `help`입니다.
- 레거시 별칭은 허용되지 않습니다.

<a id="implemented-properties-form"></a>

## 구현된 속성 양식
- `Projects`, `Bookmark` 및 `Progress` 선택기는 전용 세부 패널 로컬 선택기 컨트롤러에서 읽습니다.
- `FoldersList` 및 `TagsList` 미러는 `.wsnhead` 메타데이터를 유지하고 `DetailPanelController`를 통해 씁니다.
- 공유 `DetailMetadataHierarchyPicker.qml`는 LVRS 상황에 맞는 메뉴를 통해 할당 가능한 계층 구조 항목을 렌더링합니다.

<a id="lvrs-reuse"></a>

## LVRS 재사용
- `LV.ComboBox`, `LV.ContextMenu`, `LV.HierarchyItem` 및 `LV.ListFooter`를 사용합니다.
- 임시 색상이나 글꼴 대신 LVRS 타이포그래피와 패널 토큰을 사용합니다.
- 컴팩트 행, 목록 카드, 바닥글 막대 및 양식 삽입은 LVRS 테마 토큰에서 크기를 가져옵니다.

<a id="file-statistics-state"></a>

## 파일 통계 상태
- `fileStat` 상태는 `DetailFileStatForm.qml`를 마운트합니다.
- 통계 화면은 지속된 `.wsnhead <fileStat>` 값과 현재 헤더 메타데이터를 렌더링합니다.

<a id="detached-state"></a>

## 분리된 상태
- `detached`는 메타데이터 선택기와 인라인 폴더 편집을 닫은 다음 양식 콘텐츠를 렌더링하지 않습니다.
