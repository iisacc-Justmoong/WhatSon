# `src/app/models/editor/component/Agenda.cpp`

## Responsibility

Implements the agenda-family static RAW tag template lookup.

## Runtime Behavior

- Tag-name lookup is case-insensitive and ignores non-alphanumeric characters, matching the normalization behavior used
  by `SetTag`.
- The component returns valid descriptors only for `agenda` and `task`; other static body tags remain owned by their
  existing editor components or by `SetTag`.
- `SetTag` consumes these descriptors and applies the generic selection wrapping, toggle-off, body-document
  serialization, and invalid-tag result behavior.

## 한국어

- 이 구현은 `agenda`와 `task` 정적 삽입 토큰의 단일 소스다.
- source text 변형, selection 처리, `.wsnbody` 재직렬화는 여기서 하지 않고 `SetTag`가 담당한다.
