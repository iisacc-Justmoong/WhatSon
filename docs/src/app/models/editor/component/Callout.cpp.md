# `src/app/models/editor/component/Callout.cpp`

## Responsibility

Implements the note editor callout HTML renderer.

## Design Source

- Figma file: `fQUfzeMDED9JWvh4saYiVT`
- Node: `280:7897`
- Baseline shape: `295x22` callout row with `#262728` background, `16px` vertical padding, `4px` horizontal padding,
  `12px` content gap, a `3px`
  `#d9d9d9` leading bar, and Pretendard Medium `12px / 12px` white text.

## Runtime Behavior

- The emitted root is a `width="100%"`, `height:auto` `whatson-callout` table with
  `data-frame-width-mode="fill"` and `data-frame-height-mode="hug-contents"`, so a callout owns the full editor source
  row instead of behaving like loose inline styling while its height remains content-driven.
- The leading bar is a table cell with `height:100%` and `min-height:14px`. One-line callouts match the Figma bar height,
  while wrapped text makes the row and bar grow together.
- The callout content cell uses normal whitespace and `word-break:break-word`, keeping the component's core meaning as a
  text-wrapping region.
- The block is surrounded by `<!--whatson-callout-source:...-->` comments. Persistence uses those markers to recognize
  live rendered callouts and recover the canonical `<callout>...</callout>` wrapper from edited rich text.

## 한국어

- 이 구현은 Figma `280:7897` Callout을 Qt rich text가 이해할 수 있는 table 기반 HTML로 변환한다.
- callout root는 editor width를 항상 `100%`로 채우고 `height:auto`로 콘텐츠 높이만큼만 늘어난다.
- callout root는 상하 `16px`, 좌우 `4px` padding을 둔다.
- callout은 line number 한 줄을 전부 점유하는 editor row이며, 텍스트가 wrap되면 좌측 막대도 같은 높이로 늘어난다.
- `.wsnbody`에는 여전히 `<callout>...</callout>` source wrapper만 저장된다.
