# `src/app/calendar/ISystemCalendarStore.hpp`

<a id="role"></a>

## 역할
`ISystemCalendarStore`는 인터페이스 경계를 통해 로케일 및 날짜 형식 상태를 노출합니다.

<a id="contract"></a>

## 계약
- 로케일 스냅샷 접근자: 로케일 이름, UI 언어, 시간대, 날짜 형식, 첫 번째 요일
- 포맷: `snapshot`, `formatShortDate`, `formatNoteDate`
- 후크: `refreshFromSystem`, `requestStoreHook`
- 신호: `systemInfoChanged`

<a id="notes"></a>

## 메모
- 이제 계층 컨트롤러는 구체적인 저장소 대신 이 인터페이스를 관찰합니다.
- 정적 대체 경로 도우미는 개체 통신이 아닌 유틸리티 논리로 `SystemCalendarStore`에 유지됩니다.
