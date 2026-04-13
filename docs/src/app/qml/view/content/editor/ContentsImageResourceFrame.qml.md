# `src/app/qml/view/content/editor/ContentsImageResourceFrame.qml`

## Responsibility
Dedicated image-body frame container derived from Figma node `292:50`.

## Structure
- Top `resourceHeader`
  - left caption label, default text `Image`
  - right 3-dot menu affordance
- Center media slot
  - uses `8px` horizontal inset from the outer frame
  - stretches to the available frame width instead of staying at the original Figma sample width
  - derives its height from the effective media aspect ratio, falling back to the original `338 x 352` design sample
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
- `contentData`
  Default property alias targeting the central media viewport.
- `menuRequested()`
  Emitted when the 3-dot affordance is clicked.

## Styling
- Uses LVRS labels and theme colors instead of React/Tailwind output.
- Keeps the Figma frame chrome while adapting it to the note body width contract:
  - no background fill on the outer frame, only border chrome
  - design-time reference width `480` remains only as the implicit-width hint
  - runtime width follows the parent/editor block width
  - 8px horizontal header/footer/media padding
  - 4px vertical header/footer padding
  - 12px rounded outer border

## Current Scope
- This file declares the frame geometry and chrome only.
- Inline note-body rendering now composes this frame directly and supplies the real bitmap ratio from
  `ContentsResourceViewer.qml`.
