# `src/app/file/viewer/ContentsBodyResourceRenderer.hpp`

## Responsibility
Declares the body-resource renderer bridge that maps note-local `<resource ...>` tags into a QML-facing render model.

## Public Contract
- `contentViewModel`: expects a view-model that exposes `noteDirectoryPathForNoteId(QString)`.
- `fallbackContentViewModel`: optional second resolver, typically `LibraryHierarchyViewModel`, used when the active
  hierarchy view-model has not switched yet or does not expose the note-directory resolver contract for the currently
  selected note.
- `noteId`: selected note id whose `.wsnbody` resource tags should be rendered.
- `noteDirectoryPath`: optional explicit selected-note directory path supplied by the selection bridge.
  When present, this path wins over view-model note-directory lookups so body resource resolution stays bound to the
  note package that is already mounted in the editor session.
- `bodySourceText`: optional live editor/presentation snapshot used to resolve `<resource ...>` tags before the latest
  `.wsnbody` flush finishes.
- `maxRenderCount`: caps the number of rendered resource cards.
- `renderedResources`: normalized `QVariantList` model (`type`, `format`, `resourcePath`, `renderMode`, `source`,
  `displayName`, `previewText`, `sourceStart`, `sourceEnd`, `focusSourceOffset`).
- Type declaration must stay **non-final** because `qmlRegisterType<T>()` instantiates an internal wrapper class that
  derives from `T`.

## Selection Modes

The renderer supports two selection sources through the same `noteId` contract:

- regular note selection: resolve note directory -> prefer live `bodySourceText` when present -> otherwise parse
  `<resource ...>` tags in `.wsnbody`
- direct resource-package selection: resolve `.wsresource` directory directly and render that package as a single entry

## Signals
- Emits `renderedResourcesChanged()` whenever note selection or filesystem-backed resource payload changes.
- Emits `fallbackContentViewModelChanged()` when the secondary resolver changes.
