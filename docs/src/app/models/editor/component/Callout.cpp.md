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

- The emitted root is a `width="100%"`, `height:auto` `whatson-callout` text span with
  `data-frame-width-mode="fill"` and `data-frame-height-mode="hug-contents"`, so a callout owns the full editor source
  row instead of behaving like loose inline styling while its height remains content-driven.
- The leading bar is the text frame's `3px` `border-left`, not a separate editable table cell. This keeps the cursor
  inside callout content and avoids extra plain-text offsets from chrome-only cells or spacer text.
- The callout frame uses normal whitespace and `word-break:break-word`, keeping the component's core meaning as a
  text-wrapping region. Wrapped text grows the rendered frame height, so the border grows with the content.
- The block is surrounded by `<!--whatson-callout-source:...-->` comments. Persistence uses those markers to recognize
  live rendered callouts and recover the canonical `<callout>...</callout>` wrapper from edited rich text.

## 한국어

- 이 구현은 Figma `280:7897` Callout을 Qt rich text가 이해할 수 있는 단일 텍스트 프레임 HTML로 변환한다.
- callout root는 editor width를 항상 `100%`로 채우고 `height:auto`로 콘텐츠 높이만큼만 늘어난다.
- callout root는 상하 `16px`, 우측 `4px`, 좌측 `12px` padding을 둔다.
- 좌측 막대는 별도 편집 셀이 아니라 텍스트 프레임의 `border-left`이므로 막대나 간격에 커서가 들어가지 않는다.
- callout은 line number 한 줄을 전부 점유하는 editor row이며, 텍스트가 wrap되면 프레임 높이와 좌측 border도 함께 늘어난다.
- `.wsnbody`에는 여전히 `<callout>...</callout>` source wrapper만 저장된다.
