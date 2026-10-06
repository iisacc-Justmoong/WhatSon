# `src/app/qml/view/panels/detail/CalendarDetailPanel.qml`

<a id="responsibility"></a>

## 책임
`CalendarDetailPanel.qml`는 달력 페이지가 콘텐츠 보기에 마운트되는 동안 사용되는 전용 세부 열 표면입니다.

<a id="current-state"></a>

## 현재 상태
- 지금은 뷰가 의도적으로 비어 있습니다.
- 별도의 캘린더 상세 표면을 보유하고 있어 캘린더 전용 상세 UI를 나중에 추가해도 `NoteDetailPanel.qml` 또는 `ResourceDetailPanel.qml`로 다시 전환되지 않습니다.

<a id="contract"></a>

## 계약
- `signal viewHookRequested`
- `function requestViewHook(reason)`
