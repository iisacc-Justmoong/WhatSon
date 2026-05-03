# `src/app/models/editor/resource/ContentsInlineResourcePresentationController.cpp`

## Role
`ContentsInlineResourcePresentationController.cpp` owns RichText-side inline resource presentation helpers.

## Responsibilities
- Resolves preview width from the current editor or print layout width budget.
- Converts imported resource entries into inline framed image HTML or placeholder block HTML.
- Rewrites `whatson-resource-block` placeholders inside the fallback RichText editor surface without mutating import or
  editor-session state.
- `renderEditorSurfaceHtmlWithInlineResources(...)` accepts the current `renderedResources` list explicitly so QML
  bindings update when the resource renderer changes, while still falling back to the bound
  `bodyResourceRenderer.renderedResources` property for existing callers.
- The controller is wired from `ContentsDisplayView.qml` after `ContentsBodyResourceRenderer` has resolved
  parser-owned `resource` blocks, so canonical `<resource ... />` tags authored in RAW body text render as framed image
  HTML instead of staying visible as tag text.
- If the optional tag controller cannot normalize the renderer payload through dynamic invocation, the controller
  falls back to the shared sequential-variant normalization helper before replacing placeholders.
- Image resources use the historical Figma `292:50` frame treatment inside the RichText projection instead of a bare
  `<img>` projection. Because Qt RichText table/CSS rendering does not preserve `border-radius`, the controller first
  renders a cache PNG containing the complete frame chrome and then inserts that rounded bitmap into the editor HTML:
  - fixed outer frame width `480`
  - rounded outer border radius `12`
  - neutral `panelBackground08`-equivalent border `#2C2E2F`
  - `19px` header and footer rows with divider lines
  - `8px` horizontal header/footer inset
  - `16px` more-horizontal affordance cell
  - fixed `338 x 352` media viewport
  - image object-cover crop based on renderer-provided `imageWidth`/`imageHeight` metadata or `QImageReader` metadata
  - `Pretendard` `11px` caption labels using the Figma `Caption` metadata
  - footer label resolved from `displayName`, resource path, resource id, resolved path, then source URL
