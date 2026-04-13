# `src/app/qml/view/content/editor/ContentsImageResourceFrame.qml`

## Responsibility
Dedicated image-body frame container derived from Figma node `292:50`.

## Structure
- Top `resourceHeader`
  - left caption label, default text `Image`
  - right 3-dot menu affordance
- Center media slot
  - fixed initial design size `338 x 352`
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
  Initial viewport size derived from the Figma frame.
- `contentData`
  Default property alias targeting the central media viewport.
- `menuRequested()`
  Emitted when the 3-dot affordance is clicked.

## Styling
- Uses LVRS labels and theme colors instead of React/Tailwind output.
- Keeps the Figma frame proportions:
  - outer width `480`
  - 8px horizontal header/footer padding
  - 4px vertical header/footer padding
  - 12px rounded outer border

## Current Scope
- This file declares the frame only.
- Rendering integration into the inline resource pipeline is intentionally deferred to a follow-up change.
