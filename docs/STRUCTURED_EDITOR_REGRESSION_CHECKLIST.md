# Structured Editor Regression Checklist

This repository does not operate automated or scripted tests. Use this checklist as manual regression coverage for
structured document-flow editor changes.

## Agenda Typing
- Typing continuously inside an existing agenda task must keep the caret in that same task instead of snapping to the
  task start.
- Pressing `Enter` inside an agenda task must still create the next task or exit the agenda according to the existing
  backend rule.
- Pressing `Enter` on an empty middle agenda task must not delete later sibling tasks.
- A task body whose visible text is only an escaped entity like `&amp;` must still count as non-empty.

## Callout Typing
- Typing continuously inside a callout must keep the caret at the edited position after each RAW rewrite/reparse.
- Pressing `Enter` twice on a trailing empty callout line must still exit the callout block.
- After a callout cursor restore, the next structured shortcut must still resolve relative to that active callout block.
- Repeating edits inside one agenda/callout block of a long structured note must not visibly degrade as unrelated blocks
  are added elsewhere in the same document.

## Structured Shortcuts
- In structured-flow mode, `Cmd+Opt+T`, `Cmd+Opt+C`, and `Cmd+Shift+H` invoked from the middle of a text block must
  insert at the live caret position, not only after the whole block.
- The same shortcuts invoked while focus is inside an agenda or callout must keep wrappers standalone instead of nesting
  a new proprietary block inside existing task/callout body content.

## Note Open Cost
- Opening a note that contains no proprietary structured tags must stay on the fast path: no agenda/callout cards
  should appear, and the editor must not instantiate legacy structured overlays in parallel with the plain-text path.
- Opening a long structured note must populate the document-native block flow without one large synchronous stall before
  the first editor frame becomes interactive.
- While the first structured render for a newly selected note is still pending, note-open must stay on the legacy
  editor/session path instead of switching into the structured host before block ownership is known.
- Once a selected note has already activated structured-flow mode, later async reparses must keep that structured
  surface mounted instead of bouncing back through the legacy editor.
- While structured-flow mode remains active, the fallback full-document inline editor must stay unloaded rather than
  remaining mounted off-screen behind `visible: false`.
- One note-open transition must not cause duplicate editor host selection sync passes just because the note-list bridge
  emitted both note-id and body-text updates in the same event loop turn.
- Showing the editor again for the already-selected note must reuse the same queued note-open sync path rather than
  scheduling a second independent minimap/presentation/gutter refresh sequence.
- One note-list rebuild must not trigger two immediate visible list snapshot refreshes from `modelReset` plus a second
  `itemsChanged()`-driven snapshot apply.
- Parser-side structured correction suggestions must not trigger direct note writes during ordinary note-open or typing.

## Large Note Lazy Load
- Selecting a very large note must not freeze the app before the editor becomes interactive.
- While the selected note body is still loading, desktop/mobile editor hosts must show the loading overlay and keep
  editor input disabled.
- A note-open turn must not sync an empty placeholder body into the editor before the lazy body read completes.
- Library/bookmarks/projects/progress note-list rows must not carry the full note body as a secondary transport just to
  support editor note-open.
- When the selected note is refreshed in place after reconcile or metadata reload, the host may keep the existing body
  visible while the replacement body is fetched asynchronously.
- If two lazy body reads for the same selected note overlap, only the newest request may update the selected note body.
- If a same-note lazy body read fails, the current selected-note body must remain intact instead of being replaced by an
  empty snapshot.
- If the current note still has `pendingBodySave`, a note switch must not proceed as if that save succeeded unless the
  persistence bridge accepted the staged snapshot first.
- Leaving a note with `pendingBodySave` should prefer the immediate fetch path rather than only a deferred stage request.
- A buffered editor save must remain attached to the note directory that was resolved when the edit was staged, even if
  the active hierarchy/content-view-model changes before the next fetch turn.
- Replacing `noteListModel` and `contentViewModel` in the same event-loop turn must trigger only one settled selection
  refresh/rebind pass.
- The editor session must always know:
  - which note the user currently selected
  - which note owns the currently exposed body payload
  - whether an empty body is an explicit fallback for that selected note rather than a stale carry-over
- Editor/bridge APIs must not replace omitted `noteId` arguments with whichever note happens to be selected at that
  moment.

## Resource Drop Import
- Dropping one or more local files onto the desktop/mobile editor must create matching `.wsresource` package
  directories under the active hub `*.wsresources` root and append those package paths into `Resources.wsresources`.
- The same drop must inject canonical `<resource ...>` source tags into the selected note body instead of only adding
  filesystem packages out-of-band.
- The injected resource call must be canonical self-closing source with quoted attribute values:
  `<resource type="..." format="..." path=".../.wsresource" />`.
- Relative resource paths that contain `/` but no spaces must still survive the editor insert/save round-trip intact;
  they must not be truncated just because the original import path omitted attribute quotes.
- Resource-drop note linking must complete before the resources hierarchy runtime reload rebinds the editor session.
  A successful `.wsresource` import must not stop after `Resources.wsresources` persistence while leaving `.wsnbody`
  unchanged.
- Native file-manager drops that surface only `text/uri-list` must still be accepted; desktop/mobile hosts must not
  rely solely on `drop.urls`.
- Dropping a file while the caret sits in the middle of a paragraph must still insert the canonical `<resource ... />`
  call as its own source block instead of embedding the tag inline inside adjacent prose text.
- After the tag insertion completes, the same selected note must show the new resource frame at the authored body slot
  before a later manual note reopen or explicit filesystem refresh.
- Later text in that same note must flow below the inline resource frame; the card must not overlap subsequent
  paragraphs just because the tag itself is zero-width source markup.
- In RichText editor mode, dropping an image must render at the authored body slot as part of the RichText document
  itself, and later paragraphs must remain below that image in ordinary body flow.
- Inline image rendering must still work when the active hierarchy view-model does not expose
  `noteDirectoryPathForNoteId(QString)` or when hierarchy switching momentarily leaves the content surface bound to the
  previous domain view-model.
- In RichText editor mode, the rendered body should stay on paragraph/image document flow that is close to Qt raw
  RichText/RTF layout, rather than one flat `<br/>` chain plus hidden spacer overlays.
- After an inline image resource is present in the editor body, typing one additional character elsewhere in that note
  must not treat the RichText image object itself as a new plain-text character during diffing or persistence.
- A programmatic resource/body presentation rebuild must not persist the superseded placeholder surface back into
  `.wsnbody`; queued fallback `textEdited(...)` dispatch from the older RichText surface must be cancelled or ignored.
- If the RichText surface later emits a plain-text delta whose logical span collapses back to one RAW source offset
  inside a resource placeholder zone, that delta must be ignored and the surface must be restored from canonical
  source instead of rewriting the resource tag.
- A file drop that imports a resource must not leave escaped tail fragments such as
  `&quot;image&quot; format=&quot;...&quot; ... /&gt;` or `e=&quot;image&quot; ... /&gt;` inside `.wsnbody`.
  The saved body must contain canonical literal `<resource ... />` tags only.
- The same imported-file drop must not rewrite neighboring structured tags into broken paragraph text.
  For example, an existing `<callout>...</callout>` block around the drop context must remain literal source markup
  instead of degrading into plain paragraph text plus escaped resource-tail fragments.
- Backspace/Delete while the caret is at a raw tag boundary must remove the whole tag token in one step.
  Deleting across `<resource ... />`, `<callout>`, `</callout>`, `<agenda ...>`, `</agenda>`, or inline-style tags
  must not leave partial source remnants behind.
- The right-side editor minimap must remain visible after note open and after later layout mutations.
  Adjacent editor/resource/detail content must not compress the minimap rail to zero width.
