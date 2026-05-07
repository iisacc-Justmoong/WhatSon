# `src/app/models/file/note/WhatSonLocalNoteFileStore.cpp`

## Role
`WhatSonLocalNoteFileStore` is the concrete filesystem adapter for `.wsnhead`, `.wsnbody`, `.wsnversion`, and `.wsnpaint`.

It creates notes, reads materialized note directories, updates persisted body/header state, and deletes local note storage.

## Package Contract
- A note package now persists only these four files:
  - `<note-id>.wsnhead`
  - `<note-id>.wsnbody`
  - `<note-id>.wsnversion`
  - `<note-id>.wsnpaint`
- The store no longer creates or appends `.wsnhistory`.
- The store no longer creates `.meta` directories.
- `.wsnpaint` is treated as a dedicated paint-layer document, not as an attachment sidecar.

## Body Parsing Contract
- `applyBodyDocumentText(...)` is the read-side body decoder.
- The decoder now routes `.wsnbody` XML structure through `WhatSonIiXmlDocumentSupport` before inspecting `<body>` and
  `<resource ...>` nodes.
- Thumbnail metadata is derived from shared iiXml node-field helpers instead of a body/resource regular-expression
  scan.
- It now projects both:
  - `bodyPlainText` (search/list summary text)
  - `bodySourceText` (editor-facing RAW source projection from `.wsnbody`)
- `WhatSonLocalNoteDocument::normalizeBodyFields()` keeps those projections separate after reads and writes: RAW inline
  tags remain in `bodySourceText`, while `bodyPlainText`/`bodyFirstLine` are rebuilt from the canonical body projection
  for note-list preview and search consumers.
- That `bodySourceText` projection now inherits the read-side HTML-block guard from
  `WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(...)`, so rendered-editor comment wrappers and other
  suspicious non-canonical HTML block markup do not get rebound into note-open editor state.
- The read path also derives `bodyFirstLine` from `WhatSon::NoteBodyPersistence::firstLineFromBodyDocument(...)` so inline titles before the first paragraph survive indexing and editor reads consistently.
- This means empty paragraphs and whitespace-only paragraphs survive file reads instead of being normalized away.
- Formatting whitespace from an otherwise empty `<body>` no longer survives file reads, so a newly created empty note
  binds to the editor as truly empty text instead of starting from a phantom second line.
- `<resource ... resourcePath="...">` now accepts `.wsresource` package paths.
- Resource attribute parsing now also supports unquoted values with path separators (for example `path=PreviewHub.wsresources/preview.wsresource`) so note bodies can reference package paths without forcing quote normalization first.
- When the reference points to a package directory, the store resolves `resource.xml`, follows its `asset path`, and uses the real packaged asset file for preview thumbnail URLs.

## Update Contract
- `updateNote(...)` now canonicalizes body writes through `WhatSon::NoteBodyPersistence::serializeBodyDocument(...)`.
- This allows RichText editor payloads to be accepted while still writing canonical `.wsnbody` inline tags.
- During update, the store still recomputes `bodyPlainText` from the serialized `.wsnbody`, but it now keeps the
  incoming editor-authored `bodySourceText` as the authoritative RAW source instead of round-tripping that source back
  through another read-side projection first.
- The update path now also short-circuits direct body persists when the newly serialized `.wsnbody` payload is bytewise
  identical to the file already on disk and the caller did not explicitly request a separate header mutation.
  Note selection/reconcile therefore cannot bump `lastModifiedAt` or reorder list items merely by re-saving an
  unchanged body snapshot.
- Body writes now also extract inline body hashtags from the editor-visible source, merge those values into the note
  header tag list, and ensure `Tags.wstags` contains matching hierarchy entries for any new tags by parsing and
  rewriting the tracked tags hierarchy file in the same transaction.
- Before persisting `.wsnhead`, the store now also recalculates the note-local `fileStat` block from
  the current header/body state.
- Body writes force a header write as well, because the body-derived counters live in `.wsnhead`.
- `UpdateRequest` now decides whether a successful write should increment `modifiedCount`.
- Header / structural mutations still opt in to that counter by default.
- Each successful `modifiedCount` increment is now treated as a commit boundary:
  - after header/body files persist, the store captures a new `.wsnversion` snapshot
  - capture/diff persistence is delegated to `file/diff/WhatSonLocalNoteVersionStore`
  - snapshot label format: `commit:<modifiedCount>`
  - snapshot metadata stores `commitModifiedCount` and git-style unified patches for `.wsnhead` / `.wsnbody`
- Debounced editor body autosaves deliberately opt out: they still write the normalized `.wsnbody`, refresh
  body-derived stats, and update `lastModifiedAt`, but they no longer create commit snapshots because
  `incrementModifiedCount` is disabled on that path.
- Open tracking is intentionally handled by the editor-selection bridge instead of the generic read path.
- The same update contract now also splits local stat work from hub-wide backlink scans:
  - `refreshIncomingBacklinkStatistics == false` skips the expensive `.wsnbody` sweep for the edited note's
    `backlinkByCount`, but still rewrites all body-derived counters into the current header.
  - `refreshAffectedBacklinkTargets == false` skips immediate refresh of linked target headers.
- This lets latency-sensitive editor autosaves write the selected `.wsnote` package directly and leave backlink
  fan-out work to a later owner-controlled pass.

## Regression Notes
- This repository no longer maintains a dedicated scripted test for the empty-note load path; keep the behaviors below
  as documentation-only regression expectations.
  - leading/interior/trailing empty paragraph round-trip
  - whitespace-only paragraph round-trip
  - inline-tag source serialization
  - Qt Rich HTML source serialization into canonical `.wsnbody` tags
- Additional regression expectation:
  - editor body autosave must not increment `modifiedCount`; only update requests that explicitly opt in may advance that counter.
  - every update transaction that advances `modifiedCount` by exactly one must append a corresponding snapshot to
    `.wsnversion` with `commitModifiedCount == modifiedCount`
  - each captured snapshot must include unified patch metadata for both header and body payloads
  - editor hot-path writes that disable backlink refresh must still rewrite the current note's normalized body text,
    header timestamp, and non-hub-derived counters
  - non-editor file read/reconcile turns must not regenerate a different `bodySourceText` RAW snapshot solely because
    `.wsnbody` was reparsed
  - note selection with no effective body change must not touch `.wsnhead` / `.wsnbody`, must not advance
    `lastModifiedAt`, and must not reorder the note list
  - a saved body hashtag such as `#label` must materialize in three places together:
    - `.wsnbody` as `<tag>label</tag>`
    - `.wsnhead` inside the note tag list
    - `Tags.wstags` as a hierarchy entry when the tag is new to the hub
