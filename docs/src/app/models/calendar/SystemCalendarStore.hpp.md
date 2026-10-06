# `src/app/calendar/SystemCalendarStore.hpp`

<a id="role"></a>

## 역할
`SystemCalendarStore`는 `ISystemCalendarStore` 뒤에 있는 구체적인 시스템 로케일 구현입니다.

<a id="interface-alignment"></a>

## 인터페이스 정렬
- `ISystemCalendarStore`의 전체 로케일/날짜 형식 지정 계약을 구현합니다.
- 삽입되지 않은 코드 경로에 대해 정적 대체 경로 형식 지정 도우미를 유지합니다.
- `main.cpp`에서 계층 구조 컨트롤러에 주입된 구체적인 소스 역할을 합니다.
