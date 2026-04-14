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

## Semantic Tags
- A stored legacy `<next/>` tag must render as a real line break in both note-body read projections and editor HTML
  rendering; it must not paint literal `<next/>` text in one path while becoming a break in another.
- A stored `<title>`, `<subTitle>`, or `<eventTitle>` block must render as heading-style text in both the body
  persistence read path and the editor renderer.
- A stored `<event>...</event>` wrapper must stay transparent in read-side rendering while its child semantic blocks
  still surface their text content in order.
- Autosaving a note that already contains `<title>`, `<subTitle>`, `<eventTitle>`, `<eventDescription>`, `<event>`, or
  `<next/>` must not rewrite those tags into escaped literal text.

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
- Each note-entry transition must reset gutter line-number geometry and recompute it from the newly bound note body.
  Line numbers from the previously selected note must not persist until a later scroll, resize, or incidental cursor
  move happens to refresh the gutter.
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
- After typing in one note, switching away, and returning to that same note, the editor body must rehydrate from the
  saved RAW snapshot instead of reopening as blank.
- Note-list preview text, loaded editor text, and the saved RAW paragraph text must remain the same logical content.
  One stale presentation snapshot must not cause repeated prose duplication in RAW while the editor surface still looks
  normal.

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
- Dropping an image into a note that already contains ordinary paragraphs must switch that same note into the
  structured resource path immediately.
  The editor must not leave one legacy frame alive that renders the literal `<resource ... />` tag as visible text.
- If the live editor surface briefly lags behind note-load or drop-import source state, the selection-bridge
  `selectedNoteBodyText` snapshot must still be able to keep the note on the structured resource path.
  A same-note image block must not fall back to the legacy resource overlay just because one intermediate presentation
  source is behind by a turn.
- The same mixed-content note must remain an ordinary note editor surface.
  Inline images must render between surrounding paragraphs in the authored body column and must not replace the whole
  editor with the dedicated resource-package viewer.
- After the tag insertion completes, the same selected note must show the new resource frame at the authored body slot
  before a later manual note reopen or explicit filesystem refresh.
- A `<resource ... path=".../.wsresource" />` body slot must resolve through that package's `resource.xml` metadata to
  the actual internal asset file before image rendering.
  The inline bitmap viewer must never try to open the `.wsresource` directory path itself as if it were the payload.
- Later text in that same note must flow below the inline resource frame; the card must not overlap subsequent
  paragraphs just because the tag itself is zero-width source markup.
- Mixed prose/image note bodies should keep visible block spacing between the paragraph above, the inline image frame,
  and the paragraph below, matching the markdown-style reference flow instead of collapsing every block together.
- In the editable note surface, dropping an image must render at the authored body slot while keeping ordinary typing
  on the RAW-placeholder/editor-overlay path.
  Later paragraphs must remain below that image in ordinary body flow and must not inherit image-only centering state.
- Typing immediately after an inline image resource must keep both the `<resource ... />` RAW token and the newly
  typed prose.
  The editor must neither erase the finished word nor drop the resource block into memory-only presentation state.
- Inline image rendering must still work when the active hierarchy view-model does not expose
  `noteDirectoryPathForNoteId(QString)` or when hierarchy switching momentarily leaves the content surface bound to the
  previous domain view-model.
- In RichText editor mode, the rendered body should stay on paragraph/image document flow that is close to Qt raw
  RichText/RTF layout, rather than one flat `<br/>` chain plus hidden spacer overlays.
- A note that only contains prose plus body `<resource ... />` tags must stay on the legacy inline editor host.
  Resource presence alone must not swap the active editor implementation into `ContentsStructuredDocumentFlow.qml`.
- In structured-flow mode, a `type=resource` block must render through the same image frame card used elsewhere in the
  editor, and it must occupy real document height in the block column rather than an overlay aligned on top of text.
- The first structured resource block in a note must still resolve its inline asset payload.
  A `resourceIndex` or focus target value of `0` must not collapse to a sentinel fallback that downgrades the block to
  the empty `Document Resource` metadata card.
- A resolved bitmap resource must still upgrade into the Figma `292:50` image frame even if the renderer payload
  reaches QML with `renderMode=document`.
  Real bitmap paths/formats must win over that downgraded metadata state; the editor must not fall back to the empty
  `Document Resource` summary card while the asset itself is resolvable.
- The inline image frame must keep a transparent background. Only the border chrome from the image-resource frame may
  remain; no extra dark fill from the wrapper card should sit behind the bitmap.
- Inline image resource blocks must stretch only with their outer frame to the available editor body width.
  The inner bitmap viewport must stay centered and keep the natural bitmap width hint until the body column forces it
  smaller, while still recomputing height from the actual bitmap aspect ratio.
- "Fill width" for inline images must mean the note body column width only.
  The frame must not consume gutter space, minimap rail space, or any extra viewport overhang outside the body insets.
- The dedicated resource viewer may appear only when the user is directly browsing a `.wsresource` package from the
  Resources hierarchy.
  A selected note from Library/Bookmarks/Projects/Progress/Tags/Event/Preset that contains `<resource ... />` body
  tags must not trigger the same full-surface viewer path.
- When structured-flow mode is active and the last visible block is a resource or break block, a plain left click in
  the note body must reopen editing at the document end instead of leaving focus trapped on a non-editable block.
- The same click-to-append path must materialize exactly one trailing text block when the note currently ends in a
  non-text block, so the next typed character appears after the resource/divider instead of being lost or inserted
  into an earlier paragraph.
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
- Pasting an image from the system clipboard into the note body must follow the same pipeline as file drop:
  create a `.wsresource` package, insert a canonical `<resource ... />` tag into RAW, and render the new inline
  resource block in the current note without falling back to Qt's native inline-image document mutation.
- After any save/load round-trip, a standalone `<resource ... />` source line must remain a direct body-level resource
  block instead of being rewrapped into `<paragraph>` content or glued back onto the previous prose line.
- The same file-system drag must still import correctly when the OS exposes the payload as plain text or a platform
  file-url string instead of populating `drop.urls`.
  In that case the editor must still create the `.wsresource` package, insert the canonical `<resource ... />` tag,
  and must not fall back to Qt's default inline image-object drop behavior.
- The same imported-file drop must not rewrite neighboring structured tags into broken paragraph text.
  For example, an existing `<callout>...</callout>` block around the drop context must remain literal source markup
  instead of degrading into plain paragraph text plus escaped resource-tail fragments.
- Dropping an image into an ordinary prose note must not swap the active editor implementation from the legacy inline
  editor into `ContentsStructuredDocumentFlow.qml` solely because `<resource ... />` markup appeared in RAW.
- If the note is already in structured-flow mode because of agenda/callout/break blocks, dropping an image/resource
  must insert the new `<resource ... />` block at the active structured-block caret position rather than appending it
  to EOF through the legacy inline-editor cursor bridge.
- Backspace/Delete while the caret is at a raw tag boundary must remove the whole tag token in one step.
  Deleting across `<resource ... />`, `<callout>`, `</callout>`, `<agenda ...>`, `</agenda>`, or inline-style tags
  must not leave partial source remnants behind.
- When the visible caret is placed immediately after an inline-style run, the next typed character must stay outside
  the closing style tag.
  The typing bridge must not map that collapsed logical boundary back in front of `</bold>`, `</italic>`,
  `</underline>`, `</strikethrough>`, or `</highlight>`.
- Pressing `Enter` repeatedly around text that is wrapped by an inline-style tag such as `<highlight>...</highlight>`
  must keep the remaining prose as one contiguous paragraph tail.
  Structured-flow reparsing must not re-focus inside the closing style tag and peel the tail into one-character
  `<paragraph>` nodes such as `이`, `제`, `안`, `그`, `러`.
- Pressing `Enter` at the end of a highlighted run and then continuing to type plain prose must not expose literal
  RAW tails such as `/highlight>` or `</highlight>` on the editor surface.
- Ordinary typing inside a structured text block must not reconstruct RAW from RichText DOM/HTML.
  The saved `.wsnbody` must come only from RAW-source range replacement plus reparsing, not from serializing the
  rendered editor surface.
- Inline formatting shortcuts and context-menu actions must also stay on the same pipeline.
  They must rebuild proprietary RAW tags from logical/source ranges and then re-render from reparsed RAW, not from a
  serialized `QTextDocument` or editor-surface DOM snapshot.
- The right-side editor minimap must remain visible after note open and after later layout mutations.
  Adjacent editor/resource/detail content must not compress the minimap rail to zero width.
- The editor column order must remain gutter on the left, editor in the center, minimap on the right.
  Inherited layout mirroring or direction changes elsewhere in the app must not flip that order.
- The right-side minimap rail must stay right-aligned even when the editor row inherits other shell direction or
  mirroring settings; the fixed minimap column must not drift to the left edge.
- The visible minimap rail must be anchored to the editor surface edge, not only implied by `RowLayout` child order.
  Central editor growth or overlay content must not pull the live minimap back into the row interior.
- The ordinary structured document viewport must remain a sibling of the print-preview `Flickable`.
  Toggling or mounting print-preview surfaces must not re-parent the screen editor flow or its right-edge minimap rail.
