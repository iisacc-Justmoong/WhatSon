# `src/app/models/file/note`

## Status
- Directory mirror generated from the current `src` tree.
- Concrete note package CRUD, mounted-hub note mutation services, local note file store, note management coordinator,
  and note version/diff storage have been deleted.

## Scope
- Mirrored source directory: `src/app/models/file/note`
- Child directories with live source files: `body`, `folder`, `header`, `package`, `support`
- Child files at the source root: none

## Child Directories
- `body` - body parser/serializer helpers, semantic tag helpers, resource tag generation, web-link support, and
  markdown style metadata.
- `folder` - raw folder block inspection semantics only.
- `header` - `.wsnhead` creation, parsing, storage, bookmark color palette, and header-local metadata helpers.
- `package` - note header/body text bootstrap helpers that no longer create a package-suffix directory.
- `support` - shared iiXml document-tree helpers used by body/header parsers.

## Current Contract
- This shard keeps reusable note text/schema helpers only.
- Body persistence to a concrete note package is disabled and returns failure.
- Note creation, note deletion, note folder clearing, local file-store CRUD, editor note-management orchestration, and
  note version snapshot/diff persistence are not present.
- Code that needs a document model must define a new owner instead of reviving deleted helpers or adding QML
  compatibility wrappers.
- Automated C++ regression coverage lives in `test/cpp/suites/*.cpp` and now covers only retained helpers such as
  body resource tag generation and raw folder-block inspection.

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  `src/app/models/file/note`
- Location:  `docs/src/app/models/file/note`
- Role: Describes the responsibility of the remaining note helper shard and the boundary of the deleted package  CRUD
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
