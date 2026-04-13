# `src/app/file/viewer/ContentsBodyResourceRenderer.hpp`

## Responsibility
Declares the body-resource renderer bridge that maps note-local `<resource ...>` tags into a QML-facing render model.

## Public Contract
- `contentViewModel`: expects a view-model that exposes `noteDirectoryPathForNoteId(QString)`.
- `noteId`: selected note id whose `.wsnbody` resource tags should be rendered.
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
