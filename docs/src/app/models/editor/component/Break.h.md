# `src/app/models/editor/component/Break.h`

## Responsibility

Declares the standalone editor break component contract.

## Current Contract

- `Break::sourceToken()` returns the canonical editor source token `</break>`.
- `Break::isSourceLine(...)` accepts standalone `</break>`, `<break/>`, and legacy `<hr/>` source lines.
- `Break::renderHtml()` returns the editor HTML fragment for that logical break line. The fragment is intentionally
  empty because surrounding source-line joins provide the visible `<br/>` spacing while preserving the logical line
  number.

## 한국어

- 이 헤더는 standalone break source line의 C++ 계약을 선언한다.
- `</break>`, `<break/>`, legacy `<hr/>`는 같은 break line으로 취급한다.
- editor projection은 태그 텍스트를 그대로 노출하지 않고 해당 논리 라인을 빈 editor line으로 투영한다.
