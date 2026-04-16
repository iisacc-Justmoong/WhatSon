# Structured Editor Regression Checklist

This repository does not operate automated or scripted tests. Use this checklist as manual regression coverage for
structured document-flow editor changes.

## Debug Trace Coverage
- Selecting a note must emit one consistent trace chain across the selection bridge, idle sync controller, editor
  session, display host, selection-sync coordinator, structured-flow coordinator, presentation-refresh controller,
  structured renderer, logical-text bridge, and structured document flow.
- Typing one character in an ordinary paragraph must emit editor-host/session/typing-controller/logical-bridge trace
  events without dropping the final persisted RAW source mutation event.
- Editing an agenda or callout block must emit block wrapper plus structured document-flow trace events that show the
  rewritten RAW source range and the follow-up focus request.
- Mounting and unmounting the desktop/mobile editor hosts, structured document flow, block delegates, and inline editor
  wrapper must each emit a lifecycle trace so leaked editor surfaces can be spotted from logs alone.

## Agenda Typing
- Typing continuously inside an existing agenda task must keep the caret in that same task instead of snapping to the
  task start.
- While an agenda task editor or checkbox still owns focus, idle note-snapshot refresh and background structured
  reparse work must remain suppressed; the caret must not disappear after the host misclassifies the card as unfocused.
- Pressing `Enter` inside an agenda task must still create the next task or exit the agenda according to the existing
  backend rule.
- Pressing `Enter` on an empty middle agenda task must not delete later sibling tasks.
- A task body whose visible text is only an escaped entity like `&amp;` must still count as non-empty.

## Callout Typing
- Typing continuously inside a callout must keep the caret at the edited position after each RAW rewrite/reparse.
- While a callout editor still owns focus, host-level idle refresh paths must not reactivate and blur the live caret
  out from under the card between typing turns.
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
- Malformed semantic/body markup such as mismatched `<title>`, `<subTitle>`, `<event>`, inline-style, or
  `<paragraph>` wrappers must flip `structuredParseVerification.wellFormed` to `false` and surface an `xml` issue
  instead of silently passing lint and degrading later read projections.
- Malformed `<resource ... />` body markup must also fail that `xml` verification path with parser location context;
  the validator must not accept a partial semantic projection just because agenda/callout/break counts still look
  balanced.
- A malformed multi-line semantic body such as `<event>` on one line and a broken closing `</title>` on a later line
  must keep `structuredParseVerification.xml.issues[*].context.sourceLineNumber` near the authored source line rather
  than collapsing every XML parse error back to one synthetic line.

## Structured Shortcuts
- In structured-flow mode, `Cmd+Opt+T`, `Cmd+Opt+C`, and `Cmd+Shift+H` invoked from the middle of a text block must
  insert at the live caret position, not only after the whole block.
- The same shortcuts invoked while focus is inside an agenda or callout must keep wrappers standalone instead of nesting
  a new proprietary block inside existing task/callout body content.

## Note Open Cost
- Opening a note that contains no proprietary structured tags must stay on the fast path: no agenda/callout cards
  should appear, and the editor must not instantiate duplicate legacy overlays in parallel with the canonical
  document-flow host.
- Opening a long structured note must populate the document-native block flow without one large synchronous stall before
  the first editor frame becomes interactive.
- Opening that same long structured note must not leave every off-screen text/resource/card delegate mounted after the
  first settled layout turn.
  GammaRay or equivalent scene-graph inspection should show only the viewport window plus overscan remaining live.
- While the first structured render for a newly selected note is still pending, note-open must stay on the legacy
  editor/session path instead of switching into the structured host before block ownership is known.
- Once a selected note has already activated structured-flow mode, later async reparses must keep that structured
  surface mounted instead of bouncing back through the legacy editor.
- Any nested structured-block editor focus must propagate back to the host note editor's "input focused" guards.
  A focused paragraph/task/callout block must not let snapshot polling or background reparsing restart just because the
  outer structured-flow `FocusScope` itself is not the direct `activeFocus` item.
- Each note-entry transition must reset gutter line-number geometry and recompute it from the newly bound note body.
  Line numbers from the previously selected note must not persist until a later scroll, resize, or incidental cursor
  move happens to refresh the gutter.
- Desktop and mobile note-open flows must stay behaviorally aligned through the shared C++ coordinators:
  the same note/body transition should produce the same selection-sync fallback, reconcile scheduling, and
  structured-flow activation result on both hosts.
- While structured-flow mode remains active, the fallback full-document inline editor must stay unloaded rather than
  remaining mounted off-screen behind `visible: false`.
- One note-open transition must not cause duplicate editor host selection sync passes just because the note-list bridge
  emitted both note-id and body-text updates in the same event loop turn.
- Showing the editor again for the already-selected note must reuse the same queued note-open sync path rather than
  scheduling a second independent minimap/presentation/gutter refresh sequence.
- Typing inside one existing structured block without adding/removing distant body sections must not rebuild the entire
  minimap snapshot for the whole note on each keystroke.
- One note-list rebuild must not trigger two immediate visible list snapshot refreshes from `modelReset` plus a second
  `itemsChanged()`-driven snapshot apply.
- Parser-side structured correction suggestions must not trigger direct note writes during ordinary note-open or typing.

## Large Note Lazy Load
- Selecting a very large note must not freeze the app before the editor becomes interactive.
- While the selected note body is still loading, desktop/mobile editor hosts must show the loading overlay and keep
  editor input disabled.
- After the user selects a different note-list item, the previously rendered note body must not remain mounted once the
  host has recognized that the editor session is no longer bound to that old note.
  Structured-flow visibility must follow the currently bound selected-note session, not the previous note's cached
  parser result.
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
- A blur-side deferred editor flush captured while note `A` was focused must not execute after the session has already
  rebound to note `B`.
  Selecting `B` must never persist `A`'s stale surface text into `B` just because the editor focus change settled one
  turn later.
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
- A newly created note must enter the visible note list with the same normalized preview metadata that a later
  file-store read of that note would return.
  Note creation must not fabricate a partial runtime record that leaves the note-list preview blank until some later
  reload path repairs it.

## Resource Drop Import
- Dropping one or more local files onto the desktop/mobile editor must create matching `.wsresource` package
  directories under the active hub `*.wsresources` root and append those package paths into `Resources.wsresources`.
- The duplicate-resource conflict prompt shown for drag/drop and clipboard image import must stay behaviorally aligned
  between desktop `ContentsDisplayView.qml` and mobile `MobileContentsDisplayView.qml`.
- After the import-controller split, duplicate-import alert open/close state and editor-surface guard release must
  still remain synchronized on both hosts.
- The split helper set must remain QML-lintable as `QtObject`-owned collaborators.
  `ContentsResourceImportController.qml` must not regress into anonymous child-object composition that breaks under
  `qmllint` or QML compilation.
- Accepting `overwrite` or `keep both` from that prompt must still insert canonical `<resource ... />` tags back into
  the currently bound note on both editor hosts.
- If an imported file name already exists in the current resources store, desktop/mobile must open an explicit
  duplicate-import decision alert instead of silently auto-numbering the package.
  The user must be able to choose `Overwrite`, `Keep Both`, or `Cancel Import`.
- The same drop must inject canonical `<resource ...>` source tags into the selected note body instead of only adding
  filesystem packages out-of-band.
- On macOS, a native screenshot copied to the system clipboard must also import through plain `Cmd+V`.
  The editor must not require a browser-style `image/*` MIME advertisement if `QClipboard::image()` or `pixmap()`
  already exposes the captured bitmap.
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
- Right after drag/drop or clipboard-image insertion, the same selected note must resolve the new `<resource ... />`
  block from the currently bound editor RAW buffer.
  The body image must not wait for `selectedNoteBodyText` or a later save/reopen cycle before appearing.
- A `<resource ... path=".../.wsresource" />` body slot must resolve through that package's `resource.xml` metadata to
  the actual internal asset file before image rendering.
  The inline bitmap viewer must never try to open the `.wsresource` directory path itself as if it were the payload.
- The same body `<resource ... />` package reference must still resolve when the authored `path="..."` is expressed
  relative to any of these contexts:
  the mounted note directory, one of that note's ancestor folders, the owning `.wscontents`, the owning `.wshub`, the
  parent directory of that `.wshub`, or a discovered `*.wsresources` root.
  Read paths must not disagree about which bitmap file the tag points at.
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
- Typing below an inline image resource with an IME composition still in progress, then blurring the editor before
  pressing space, must keep both the image block and the newly authored body text.
  Focus loss must not collapse the note back to an image-only body by replaying an older deferred editor surface.
- Pressing `Tab` in an ordinary paragraph must mutate RAW source with visible indentation spaces and keep that
  indentation rendered after the editor refresh.
  The live surface must not collapse the stored spaces so that only the cursor appears to move.
- In the legacy inline-editor path, the visible caret must remain painted whenever the nested `TextEdit` still owns
  focus, even if the wrapper `FocusScope` is not the direct `activeFocus` item.
- On the desktop RichText editor path, IME preedit text must remain visible while the word is still being composed.
  Typing a Korean word must not keep the glyphs hidden until the user presses space or another explicit commit key.
- Pressing plain `Enter` in the middle of a paragraph that already has following text must split the line and preserve
  the following-line suffix.
  The editor must not delete the next line's text or rely on a transient RichText paragraph rewrite that diverges from
  RAW source.
- Inline image rendering must still work when the active hierarchy view-model does not expose
  `noteDirectoryPathForNoteId(QString)` or when hierarchy switching momentarily leaves the content surface bound to the
  previous domain view-model.
- The same inline image rendering must also continue to work when the selected note directory is already known to the
  editor session but the hierarchy resolver has not yet rebound.
  The body renderer must prefer that mounted note-directory path instead of dropping the image frame for one turn.
- In RichText editor mode, the rendered body should stay on paragraph/image document flow that is close to Qt raw
  RichText/RTF layout, rather than one flat `<br/>` chain plus hidden spacer overlays.
- A note that contains prose plus body `<resource ... />` tags and also includes `agenda`, `callout`, or `break`
  blocks must still let the parser activate `ContentsStructuredDocumentFlow.qml` for that selected note.
  In that mixed structured case, the inline image/resource frame must come from the parser-owned document block at the
  authored slot, not from a best-effort legacy overlay fallback.
- A note whose explicit body blocks are only `<resource ... />` plus ordinary prose paragraphs must still stay inside
  `ContentsStructuredDocumentFlow.qml`.
  That note must not be diverted into a second legacy whole-note editor mode just because it has no `agenda`,
  `callout`, or `break`.
- The same rule now applies to `agenda`, `callout`, and `break` as well.
  Those tags are still ordinary note-body blocks and must remain in the same canonical document host as prose and
  resources.
- In structured-flow mode, a `type=resource` block must render through the same image frame card used elsewhere in the
  editor, and it must occupy real document height in the block column rather than an overlay aligned on top of text.
- The same structured image/divider block must count as exactly one visible gutter row.
  Scrolling through a tall image must not make the desktop gutter expand that one body block into several apparent line
  slots or a multi-row current-line marker.
- For a note whose first authored body block is a `resource`, gutter line `1` must remain visible at the top of that
  image block.
  Later prose lines must not start from `2` because the resource row was skipped during visible-line detection.
- When the first authored body block is a `resource`, clicking that resource token's left caret lane and typing must
  create a real prose block before the `<resource ... />` tag.
  A leading image must expose a real `before` caret position on its own row instead of forcing the first typed text to
  appear only after the resource block.
- While the user types on an existing logical line without adding or removing a newline, the gutter must not visibly
  oscillate between stale and recomputed positions on each keystroke.
  Line-structure-triggered gutter refresh is expected only when the current edit actually changes newline structure.
- The same-line typing path must not temporarily replace corrected structured gutter geometry with a top-packed
  fallback layout just because one intermediate content-offset or transient relayout event fired during editing.
- Structured gutter Y correction must come from the current live block-line geometry, not from a reused cached
  line-entry snapshot that can survive after the underlying layout has already corrected itself.
- While the viewport scrolls through the middle of that tall image, the gutter Y positions for later lines must stay
  aligned with the image block's real document height.
  Line numbers below the image must not overlap the bitmap, remain pinned near the top of the gutter, or jump only
  after the image fully leaves the viewport.
- While that same structured note is scrolled, the gutter rail itself must remain scroll-coupled to the editor body.
  Line numbers and marker bars must not stay visually fixed at their pre-scroll Y positions while the document moves.
- The same scroll-coupling rule must hold for non-structured legacy inline-editor notes as well.
  Gutter Y must follow the active editor `Flickable.contentY` instead of staying pinned to a stale absolute screen
  coordinate.
- Right after typing, `Enter`, or another same-note reflow below a tall leading image/resource block, the later prose
  lines must keep their absolute document offsets.
  A transient block relayout must not reset those later gutter rows to the block-local top, hide line `1`, or stack
  lines `2+` back over the image header area for one refresh turn.
- A prose block below an inline image that contains only one authored newline-delimited line must contribute only one
  gutter number even when the rendered paragraph wraps or its delegate becomes taller than one editor line.
- The inline image frame chrome, including the selected-block outline, must stay on the neutral panel palette from the
  Figma resource-frame spec rather than switching to Accent-colored borders during normal document interaction.
- Clicking the center of an inline image/resource block must leave that block selected.
  The structured viewport background tap handler must not immediately move focus to the document tail and clear the
  block selection on the same gesture.
- Ordinary clicks inside the image/resource frame must default to that same whole-block selection state.
  Only an intentional narrow left/right edge hit may escape into before/after caret behavior; the block must not fall
  into right-edge caret mode just because the user clicked somewhere in the broad right half of the image.
- Pressing `Backspace` or `Delete` while that center-selected image/resource block owns focus must remove the exact
  canonical `<resource ... />` RAW source span for that block.
  The delete path must not require a hidden legacy text selection, and it must not leave the image card mounted after
  the tag has already been removed from RAW.
- When the caret is at the very start of a prose block that immediately follows an inline image/resource block,
  pressing plain `Backspace` must delete that preceding atomic block instead of doing nothing.
- When the caret is at the very end of a prose block that is immediately followed by an inline image/resource block,
  pressing plain `Delete` must delete that following atomic block instead of doing nothing.
- The same boundary-delete contract must still work even though the image/resource block itself is mounted as a
  non-text structured delegate.
  Resource blocks must behave like one cursor-addressable token for adjacent prose deletion, not like an isolated card
  that arrow/delete traversal cannot reach.
- When the caret is at the start/end of a prose block next to an inline image/resource block, plain `Left` / `Right`
  must be able to move focus onto that attachment token.
  Arrow traversal must not stop at the text edge as if the resource block were outside the editable document model.
- When the keyboard caret is currently on the resource block's right-edge boundary anchor, plain `Backspace` must
  delete that resource block immediately.
- When the keyboard caret is currently on the resource block's left-edge boundary anchor, plain `Delete` must delete
  that resource block immediately.
- While either left/right boundary anchor owns focus, the resource block must still render as one focused atomic block
  rather than looking like only a tiny edge caret is active.
- The before/after caret for an attachment must stay outside the image frame itself, in dedicated left/right lanes.
  Attachment focus should read as one selected token with explicit boundary carets, not as a miniature caret painted
  over the bitmap.
- After deleting a selected inline image/resource block, focus recovery must move to the nearest surviving prose block
  when one exists.
  The caret must not disappear into an unmapped newline gap between reparsed blocks just because the removed resource
  used to occupy that source offset.
- Mounting or rematerializing a structured note with inline image/resource blocks must not log
  `ReferenceError: index is not defined` from `ContentsStructuredDocumentFlow.qml`.
  Block-host delegates must receive their repeater `index` explicitly under bound-component semantics.
- Mounting an inline image frame must not log a `resolvedFrameWidth` binding loop from
  `ContentsImageResourceFrame.qml`.
  Frame width must resolve from the host/editor width contract without self-referencing the frame's own `width`.
- After selecting text inside a structured paragraph block, right-click must open the editor format context menu.
  The menu must not depend on the unloaded legacy whole-note editor selection state.
  Block height must not fabricate extra logical lines such as `4` when the authored note body still stops at line `3`.
- For a multi-line prose block below an inline image, each visible gutter number must align with the actual rendered
  text row that starts that authored line.
  The gutter must not keep a uniform equal-gap ladder that visibly drifts away from the text baseline positions.
- In a structured paragraph block, if the RAW body already contains inline tags such as `<bold>`, `<italic>`, or
  `<highlight>`, the visible editor text must reflect those styles in place.
  The paragraph must not fall back to a plain unformatted rendering while the RAW source already contains the tags.
- The blue current-line gutter indicator must follow the caret's actual authored line inside multi-line paragraph,
  callout, and agenda blocks.
  Focusing a lower logical line inside one structured block must not leave the current-line marker pinned to that
  block's first gutter row.
- Moving the caret with arrow keys or a mouse click inside the same focused structured paragraph/callout block must
  update the blue current-line indicator immediately without requiring a focus change to another block first.
- Inside wrapped structured paragraph/callout/agenda text, the blue current-line indicator must follow the actual
  visual caret row, not merely the first row of the owning logical line.
- If the first block of the note is a resource/image block and focus later moves into a text block below it, the blue
  current-line indicator and current-line number must follow the focused text block instead of staying pinned to the
  first resource block.
- That same structured image block must also appear in the minimap as a wide filled block silhouette.
  The right rail must not render one tall image as several narrow text-like bars whose widths come from sliced label
  characters instead of the block card itself.
- A note whose RAW body contains semantic prose blocks such as `<paragraph>...</paragraph>` before or after
  `<resource ... />` must still paint those prose blocks inside the same structured-flow column.
  Entering a resource-bearing note must not leave the editor on a resource-only frame while the authored text becomes
  blank.
- If the RAW body still carries explicit semantic text tags such as `paragraph`, `title`, `subTitle`, or `eventTitle`,
  the parser must materialize each top-level tag as its own document block instead of collapsing them back into one
  generic raw text gap.
- The same top-level block ordering rule must hold across tag families.
  A note that mixes `paragraph`, `resource`, `break`, `agenda`, or `callout` tags must still emerge as one ordered
  block stream by source position, rather than one family-specific overlay list winning over the others.
- Selecting text inside the active structured paragraph below an inline image and pressing `Cmd/Ctrl+B` or the
  highlight shortcut must rewrite the selected block-local RAW source immediately.
  The same shortcut must not fail just because the legacy whole-note editor surface is not mounted in structured-flow
  mode.
- The same structured formatting shortcuts must still rewrite the selected RAW range when the visible selection briefly
  collapses to one boundary during shortcut dispatch.
  A block-local selection captured on the shortcut turn must remain the rewrite target for the immediate RAW mutation.
- While that formatted structured paragraph is still using the plain-text input engine, starting an IME composition
  must temporarily reveal the native composing text rather than leaving the stale rendered overlay painted on top.
  The structured-flow host must treat those semantic blocks as editable text blocks at the document tail.
- The first structured resource block in a note must still resolve its inline asset payload.
  A `resourceIndex` or focus target value of `0` must not collapse to a sentinel fallback that downgrades the block to
  a fabricated generic document summary tile.
- Clicking the left edge of an inline image/resource block must first enter that resource token's own `before` caret
  position.
  If no preceding prose block exists, the next committed text must materialize before that resource block in RAW.
- Clicking the right edge of an inline image/resource block must first enter that resource token's own `after` caret
  position.
  If no following prose block exists, the next committed text must materialize after that resource block in RAW.
- When an inline image/resource block itself is selected, pressing plain `Left` or `Right` must move to the same
  resource token's before/after caret lane before traversing outward to adjacent prose.
- Clicking the center of an inline image/resource block must select the block itself as one atomic structured item.
  The frame must expose a visible selected state instead of behaving like a display-only card with no block targeting.
- A generic focus restore that lands on one `<resource ... />` source span must also reselect the whole attachment
  block by default.
  Edge-anchor focus should appear only when the caller explicitly requests before/after attachment placement.
- A resolved bitmap resource must still upgrade into the Figma `292:50` image frame even if the renderer payload
  reaches QML with `renderMode=document`.
  Real bitmap paths/formats must win over that downgraded metadata state; the editor must not fabricate any generic
  document summary card while the asset itself is resolvable.
- The same resolved resource payload must still survive when the C++ renderer reaches QML as a wrapped `QVariantList`
  instead of a native JS array.
  `length`, `count`, or numeric-key object façades must all preserve the inline image/resource card rather than leaving
  only the reserved placeholder lines in the editor body.
- Resource payload resolution for an ordinary note must come only from the parser-owned structured block stream.
  Updating one `<resource ... />` body slot must not depend on a second `.wsnbody` regex scan with different
  `resourceIndex`, source-span, `resourceId`, or `resourcePath` matching rules.
- If one structured resource block first matches a metadata-only placeholder entry and later matches a resolved entry
  with the real bitmap payload path, the block must prefer that resolved payload.
  The same body slot must not remain stuck on a fabricated generic document summary surface because an earlier partial match
  shared the same `resourceIndex`, source span, `resourceId`, or `resourcePath`.
- While the note is still on the legacy inline-editor surface for any transient reason, no plain-text/list/style/delete
  rewrite may reduce the number of canonical `<resource ... />` body tokens in RAW.
  The host must restore the surface from authoritative RAW instead of silently deleting one or more resource tags.
- The same protection must also hold for any host-level RAW source mutation path that bypasses the legacy typing or
  selection controllers.
  A direct `applyDocumentSourceMutation(...)` call on the legacy inline-editor surface must reject resource-tag loss
  and restore from authoritative RAW instead of persisting the damaged body.
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
  inside the single-line resource slot, that delta must be ignored and the surface must be restored from canonical
  source instead of rewriting the resource tag.
- A standalone `<resource ... />` body block must count as exactly one logical editor line across the bridge,
  structured-flow geometry, and fallback RichText placeholder projection.
- Typing in a paragraph below an inline image/resource block must never serialize Qt's RichText document scaffold
  (`<!DOCTYPE HTML ... qrichtext ...>`) into the visible note body or canonical `.wsnbody` source.
- Starting IME typing after noticing a misaligned image gutter must not let the next committed edit collapse the note
  into literal HTML markup; the body must stay prose + image blocks, and the editor must not fatal due to document
  corruption.
- A file drop that imports a resource must not leave escaped tail fragments such as
  `&quot;image&quot; format=&quot;...&quot; ... /&gt;` or `e=&quot;image&quot; ... /&gt;` inside `.wsnbody`.
  The saved body must contain canonical literal `<resource ... />` tags only.
- Pasting an image from the system clipboard into the note body must follow the same pipeline as file drop:
  create a `.wsresource` package, insert a canonical `<resource ... />` tag into RAW, and render the new inline
  resource block in the current note without falling back to Qt's native inline-image document mutation.
- The same clipboard-image import must still work while a live note-body text editor currently owns keyboard focus.
  `Cmd/Ctrl+V` inside the editor must not be swallowed by the native `TextEdit` paste path before the resource-import
  bridge can run.
- Copying an image from a browser page must still import when the clipboard exposes that payload as platform image MIME
  names such as `public.png` / `public.tiff` or as `text/html` / plain-text `data:image/...` content instead of a
  plain `image/*` + `hasImage()` pair.
- That clipboard-image import path must also honor the duplicate-name alert contract.
  Repeating a `clipboard-image.png` import must not silently create a numbered package unless the user explicitly chose
  `Keep Both` in the alert.
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
- Ordinary typing inside a structured text block must now stay on a plain-text `TextEdit` path.
  The active paragraph editor must not require `TextEdit.RichText` just to keep prose visible around inline image
  blocks.
  The saved `.wsnbody` must come only from RAW-source range replacement plus reparsing, not from serializing the
  rendered editor surface.
- Inline formatting shortcuts and context-menu actions must also stay on the same pipeline.
  They must rebuild proprietary RAW tags from logical/source ranges and then re-render from reparsed RAW, not from a
  serialized `QTextDocument` or editor-surface DOM snapshot.
- The same `Cmd/Ctrl+B`, `I`, `U`, `Shift+X`, and `Shift+E` shortcuts on the legacy whole-note editor surface must
  still insert canonical RAW tags even if Qt briefly collapses the visible selection to one boundary during shortcut
  dispatch.
  The controller must prefer the wrapper's cached inline-format selection snapshot over the transient zero-length live
  range.
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
