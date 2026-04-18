# `src/app/viewmodel/content/ContentsStructuredDocumentMutationPolicy.cpp`

## Responsibility
Implements source-normalized RAW mutation helpers for structured document editing.

## Current Behavior
- Normalizes QML-provided lists and wrapped values before resource insertion payload generation.
- Resolves empty-block deletion ranges against adjacent newline ownership instead of letting delegates guess.
- Builds insertion payloads that preserve surrounding newline boundaries and return caret/focus restoration offsets.
- Identifies paragraph-boundary-operable blocks from parser payload fields such as `semanticTagName`, `tagName`, and
  `type`, while limiting those operations to `paragraph` / `p` blocks only.
- Builds paragraph merge payloads by keeping the earlier paragraph as the surviving RAW block authority and folding the
  later paragraph content into it.
- Builds paragraph split payloads by inserting a new implicit newline for plain paragraphs or by cloning explicit
  wrapper prefixes/suffixes for `<paragraph>...</paragraph>`-style blocks.

## Verification Surface
- C++ regression coverage lives in `test/cpp/whatson_cpp_regression_tests.cpp`.
- The suite now asserts implicit merge, explicit merge, implicit split, and explicit split payloads so paragraph
  boundary editing stays deterministic.
