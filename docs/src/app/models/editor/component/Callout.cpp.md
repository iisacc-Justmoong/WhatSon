# `src/app/models/editor/component/Callout.cpp`

## Responsibility

Implements the note editor callout HTML renderer and source boundary-edit planner.

## Design Source

- Figma file: `fQUfzeMDED9JWvh4saYiVT`
- Node: `280:7897`
- Baseline Figma shape: `295x22` callout row with `#262728` background, `4px` frame padding, `12px` content gap, a
  `3px x 14px` `#d9d9d9` leading bar, and Pretendard Medium `12px / 12px` white text. WhatSon keeps width fill and
  height hug, and preserves the Figma `4px` frame padding, bar, gap, and text values as explicit frame metadata.

## Runtime Behavior

- The emitted root is a `width="100%"`, `height:auto` block `div.whatson-callout` text frame with
  `data-frame-width-mode="fill"` and `data-frame-height-mode="hug-contents"`, so a callout owns the full editor source
  row instead of behaving like loose inline styling while its height remains content-driven. It also carries
  `data-frame-padding-*`, `data-callout-bar-width`, `data-callout-bar-design-height`, `data-callout-bar-radius`, and
  `data-callout-content-gap` values so the visual wrapper can keep the Figma frame geometry without a QML overlay.
- Qt rich text does not render block `border-left` with enough fidelity for this frame, so the leading bar is emitted
  as one left-aligned frame-chrome image. Its PNG height is generated from the estimated wrapped content height using
  the active editor viewport width when the session provides one, so the visible `3px` bar grows with multi-line
  callout text instead of remaining a fixed icon. The PNG itself starts painting the bar at the content origin; the
  root frame owns the `4px` padding, so the bar and the first text line stay vertically aligned instead of giving the
  bar a second internal top inset. The `12px` content gap is the image's right margin, not a second editable image.
  This chrome is not stored in `.wsnbody` source; persistence strips its object replacement when Qt serializes the
  rich-text document.
- Actual callout text is wrapped by the `data-callout-content="true"` span and an inner
  `<!--whatson-callout-content-->` marker pair. Marker-based recovery extracts that content marker rather than the
  frame chrome, so nested inline spans such as italic/highlight remain intact.
- The callout frame uses normal whitespace and `word-break:break-word`, keeping the component's core meaning as a
  text-wrapping region. Wrapped text grows the rendered frame height. Because the leading bar is a generated image, the
  editor session reprojects callout frames through a short QML debounce after text changes, so the image height follows
  edited content without replacing the native document on every keystroke.
- The block is surrounded by `<!--whatson-callout-source:...-->` comments. Persistence uses those markers to recognize
  live rendered callouts and recover the canonical `<callout>...</callout>` wrapper from edited rich text.
- The component also parses paired callout source ranges and owns the callout-specific cursor rules that need knowledge
  of generated frame chrome. Backspace at the visible content start unwraps a non-empty callout or removes the whole
  empty callout source line. Enter on chrome immediately before content inserts a source line before the opening tag,
  while Enter from inside content moves the source cursor after the closing tag and creates that following line when
  needed. `NoteEditorDocumentSession` supplies the generic source-visible cursor mapper, applies the returned edit, and
  reprojects the document.

## 한국어

- 이 구현은 Figma `280:7897` Callout을 Qt rich text가 이해할 수 있는 단일 block `div` 텍스트 프레임 HTML로
  변환한다.
- callout root는 editor width를 항상 `100%`로 채우고 `height:auto`로 콘텐츠 높이만큼만 늘어난다.
- callout root는 상하 padding `4px`, 좌우 padding `4px`, Figma bar `3px x 14px`, radius `3px`, gap `12px`를
  metadata로 보존하고, 현재 editor viewport width에서 wrap된 콘텐츠 높이에 맞춘 좌측 막대 PNG를 생성한다. 이 PNG는
  자체 상단 padding을 다시 갖지 않고 콘텐츠 원점부터 막대를 그려 첫 텍스트 라인과 세로 중심을 맞춘다.
- 배경 면은 block frame 자체가 담당하고, 좌측 막대는 table이나 QML overlay가 아니라 left-aligned frame-chrome
  이미지로 렌더링된다. gap은 이 이미지의 우측 margin이며, 실제 콘텐츠는 `data-callout-content="true"` span 안에만 둔다.
- callout은 line number 한 줄을 전부 점유하는 editor row이며, 텍스트가 wrap되면 프레임 높이도 함께 늘어난다.
- 좌측 막대는 생성된 이미지이므로, 텍스트 변경 후 QML의 짧은 debounce를 거쳐 editor session의 frame reproject가
  다시 실행되어 막대 높이가 편집된 content의 wrap 높이를 따라간다. 이 갱신은 매 keystroke마다 native document를
  즉시 교체하지 않는다.
- `.wsnbody`에는 여전히 `<callout>...</callout>` source wrapper만 저장된다.
- 이 구현은 paired callout source range와 generated frame chrome을 고려한 cursor mapping도 맡는다. content 시작
  지점 Backspace는 wrapper 제거 또는 빈 callout 줄 삭제로 계획되고, content 앞 chrome 위치 Enter는 opening tag
  앞 줄 삽입으로 계획되며, content 내부 Enter는 closing tag 뒤 source 위치로 나가는 편집으로 계획된다. 세션은
  이 계획을 적용하고 editor HTML을 다시 투영한다.
