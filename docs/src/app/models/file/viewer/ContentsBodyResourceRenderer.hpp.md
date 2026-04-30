# `src/app/models/file/viewer/ContentsBodyResourceRenderer.hpp`

## Responsibility
Declares the body-resource renderer bridge that maps note-local `<resource ...>` tags into a QML-facing render model.

## Public Contract
- `contentController`: expects a view-model that exposes `noteDirectoryPathForNoteId(QString)`.
- `fallbackContentController`: optional second resolver, typically `LibraryHierarchyController`, used when the active
  hierarchy view-model has not switched yet or does not expose the note-directory resolver contract for the currently
  selected note.
- `noteId`: selected note id whose `.wsnbody` resource tags should be rendered.
- `noteDirectoryPath`: optional explicit selected-note directory path supplied by the selection bridge.
  When present, this path wins over view-model note-directory lookups so body resource resolution stays bound to the
  note package that is already mounted in the editor session.
- `documentBlocks`: parser-owned block projection from `ContentsStructuredBlockRenderer.renderedDocumentBlocks`.
  Regular note rendering now derives resource payload only from this canonical block stream instead of reparsing RAW
  `.wsnbody` text locally.
- `maxRenderCount`: caps the number of rendered resource cards.
- `renderedResources`: normalized `QVariantList` model (`type`, `format`, `resourcePath`, `renderMode`, `source`,
  `displayName`, `previewText`, `sourceStart`, `sourceEnd`, `focusSourceOffset`).
- Type declaration must stay **non-final** because the LVRS internal type manifest registers this as a creatable Qt QML
  type, which instantiates an internal wrapper class that derives from `T`.

## Selection Modes

The renderer supports two selection sources through the same `noteId` contract:

- regular note selection: resolve note directory -> walk parser-owned `documentBlocks` -> resolve only
  `type=resource` blocks into payload entries
- direct resource-package selection: resolve `.wsresource` directory directly and render that package as a single entry

## Signals
- Emits `renderedResourcesChanged()` whenever note selection or filesystem-backed resource payload changes.
- Emits `documentBlocksChanged()` when the parser-owned block stream changes.
- Emits `fallbackContentControllerChanged()` when the secondary resolver changes.
