# `src/app/store/hub/SelectedHubStore.hpp`

<a id="role"></a>

## 역할
`SelectedHubStore`는 `ISelectedHubStore`의 기본 설정 지원 구현입니다.

<a id="interface-alignment"></a>

## 인터페이스 정렬
- 시작 확인 중에 사용되는 지속형 허브 선택 계약을 구현합니다.
- 모든 경로/선택-URL 정규화 및 유효성 검사 동작을 구체적인 클래스에 로컬로 유지합니다.
- 청사진 대체 경로를 추가하지 않고 지속 선택 상태에서 직접 시작 허브 경로를 노출합니다.
