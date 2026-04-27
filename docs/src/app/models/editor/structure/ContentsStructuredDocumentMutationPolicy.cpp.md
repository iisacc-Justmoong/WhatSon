# `ContentsStructuredDocumentMutationPolicy.cpp`

- Builds RAW `.wsnbody` mutation payloads for structured document block edits.
- Empty text-block Backspace is resolved here, not in a resource-specific QML route.
- For a backward deletion at the start of an empty block, the policy first checks whether the preceding RAW span is a
  self-closing `<resource ... />` tag plus trailing whitespace.
  If so, it returns that tag range as the deletion range and leaves the current empty paragraph intact for focus.
- If no preceding resource tag exists, empty-block deletion falls back to the existing adjacent newline deletion range.
