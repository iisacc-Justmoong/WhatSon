# `src/app/models/editor/component/Callout.cpp`

## Responsibility

Implements the note editor callout HTML renderer.

## Design Source

- Figma file: `fQUfzeMDED9JWvh4saYiVT`
- Node: `280:7897`
- Baseline Figma shape: `295x22` callout row with `#262728` background, `4px` frame padding, `12px` content gap, a
  `3px x 14px` `#d9d9d9` leading bar, and Pretendard Medium `12px / 12px` white text. WhatSon keeps width fill and
  height hug, applies the runtime `16px` top/bottom padding requirement, and preserves the Figma bar/gap/text values as
  explicit frame metadata.

## Runtime Behavior

- The emitted root is a `width="100%"`, `height:auto` block `div.whatson-callout` text frame with
  `data-frame-width-mode="fill"` and `data-frame-height-mode="hug-contents"`, so a callout owns the full editor source
  row instead of behaving like loose inline styling while its height remains content-driven. It also carries
  `data-frame-padding-*`, `data-callout-bar-width`, `data-callout-bar-height`, `data-callout-bar-radius`, and
  `data-callout-content-gap` values so the visual wrapper can keep the Figma frame geometry without a QML overlay.
- Qt rich text does not render CSS padding and `border-left` with enough fidelity for this frame, so the left inset,
  leading bar, and content gap are emitted as inline frame-chrome images ahead of the content span. The leading-bar
  chrome is `46px` tall to make the requested `16px` top and bottom padding part of the actual rendered row height
  while only the middle `14px` is visible as the Figma bar. They are not stored in `.wsnbody` source; persistence strips
  their object replacements when Qt serializes the rich-text document.
- Actual callout text is wrapped by the `data-callout-content="true"` span and an inner
  `<!--whatson-callout-content-->` marker pair. Marker-based recovery extracts that content marker rather than the
  frame chrome, so nested inline spans such as italic/highlight remain intact.
- The callout frame uses normal whitespace and `word-break:break-word`, keeping the component's core meaning as a
  text-wrapping region. Wrapped text grows the rendered frame height.
- The block is surrounded by `<!--whatson-callout-source:...-->` comments. Persistence uses those markers to recognize
  live rendered callouts and recover the canonical `<callout>...</callout>` wrapper from edited rich text.

## 한국어

- 이 구현은 Figma `280:7897` Callout을 Qt rich text가 이해할 수 있는 단일 block `div` 텍스트 프레임 HTML로
  변환한다.
- callout root는 editor width를 항상 `100%`로 채우고 `height:auto`로 콘텐츠 높이만큼만 늘어난다.
- callout root는 상하 padding `16px`, 좌우 padding `4px`, Figma bar `3px x 14px`, radius `3px`, gap `12px`를
  metadata와 inline frame chrome으로 보존한다.
- 배경 면은 block frame 자체가 담당하고, 좌측 막대와 간격은 table이나 QML overlay가 아니라 inline frame-chrome
  이미지로 렌더링된다. 실제 콘텐츠는 `data-callout-content="true"` span 안에만 둔다.
- callout은 line number 한 줄을 전부 점유하는 editor row이며, 텍스트가 wrap되면 프레임 높이도 함께 늘어난다.
- `.wsnbody`에는 여전히 `<callout>...</callout>` source wrapper만 저장된다.
