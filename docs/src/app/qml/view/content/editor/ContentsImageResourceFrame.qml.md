# `src/app/qml/view/content/editor/ContentsImageResourceFrame.qml`

## Responsibility
Dedicated image-body frame container derived from Figma node `292:50`.

## Structure
- Top `resourceHeader`
  - left caption label, default text `Image`
  - right 3-dot menu affordance
- Center media slot
  - uses `8px` horizontal inset from the outer frame
  - keeps the outer frame at the available editor-body width while centering the actual media viewport inside it
  - clamps the media viewport by one fitted scale derived from:
    - the available frame width after the 8px left/right inset
    - the media's natural pixel size
    - the frame-local maximum media height
  - keeps the outer frame at the full available editor-body width even when the bitmap itself remains smaller
  - still derives its effective width/height from that same fitted scale so tall images remain bounded as ordinary
    document blocks instead of expanding to their raw pixel height inside the note body
  - exposed as a default-content slot so image/video viewers can be mounted inside it later
- Bottom `resourceToolbar`
  - caption-style filename label

## Public Contract
- `resourceTitle`
  Header caption text.
- `fileNameText`
  Footer filename text.
- `menuButtonVisible`, `menuButtonEnabled`
  Control the header action affordance.
- `mediaWidthHint`, `mediaHeightHint`
  Initial media ratio hint. Parent viewers can replace these with the real bitmap pixel size once the asset loads.
- `maxMediaHeight`
  Caps the inline note-body media height. The default matches the Figma reference height (`352`), keeping image
  resources as document blocks rather than whole-editor takeovers.
- `contentData`
  Default property alias targeting the central media viewport.
- `menuRequested()`
  Emitted when the 3-dot affordance is clicked.

## Styling
- Uses LVRS labels and theme colors instead of React/Tailwind output.
- Keeps the Figma frame chrome while adapting it to the mixed-content note-body contract shown by Figma node `294:7933`:
  - border/label/menu chrome stays on neutral panel/caption tones rather than Accent
  - no background fill on the outer frame, only border chrome
  - design-time reference width `480` remains only as the implicit-width hint
  - runtime outer width follows the parent/editor block width
  - frame-width resolution must derive from the parent width or the design fallback width, not from the frame's own
    `width` binding, so the image card does not enter a `resolvedFrameWidth` binding loop while it mounts
  - runtime media viewport stays centered
  - runtime media width/height may shrink to fit the frame width budget, but must not grow past the natural bitmap size
  - runtime media height is additionally capped to the inline note-body block budget so the image frame does not
    consume the whole editor column
  - 8px horizontal header/footer/media padding
  - 4px vertical header/footer padding
  - 12px rounded outer border

## Current Scope
- This file declares the frame geometry and chrome only.
- Inline note-body rendering now composes this frame directly and supplies the real bitmap ratio from
  `ContentsResourceViewer.qml`.
