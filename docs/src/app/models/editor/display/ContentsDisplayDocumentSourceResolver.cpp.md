# `ContentsDisplayDocumentSourceResolver.cpp`

- Implements the same-note source arbitration rules for note snapshot text, structured-flow source text, and the bound
  editor session text.
- The resolver is intentionally side-effect free: it computes presentation and mutation plans, while persistence and
  mount coordination remain elsewhere.
