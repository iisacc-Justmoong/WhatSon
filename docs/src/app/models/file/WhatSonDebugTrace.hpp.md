# `src/app/models/file/WhatSonDebugTrace.hpp`

## Status
- Documentation phase: maintained alongside the runtime debug trace helper.
- Detail level: documents the current trace controls and message-filtering boundary.

## Source Metadata
- Source path: `src/app/models/file/WhatSonDebugTrace.hpp`
- Source kind: C++ header
- File name: `WhatSonDebugTrace.hpp`
- Approximate line count: 360

## Responsibility
- Provides the app-local `[whatson:debug]` tracing helpers used by model, controller, startup, and editor code.
- Parses boolean trace environment flags through one shared helper so `on/off`, `true/false`, and `1/0` behave
  consistently.
- Installs the app-level Qt message filter that suppresses noisy `iiXml::*` debug messages by default while preserving
  WhatSon, LVRS, Qt warning, and fatal output.

## Trace Flags
- `WHATSON_DEBUG_MODE` controls ordinary WhatSon runtime traces.
- `WHATSON_EDITOR_TRACE` overrides editor-specific traces; when absent, it follows `WHATSON_DEBUG_MODE`.
- `WHATSON_IIXML_TRACE_MODE` controls local `iiXml` parser trace visibility. It defaults to off because the parser
  emits one `qDebug` message for many internal parse steps.

## Message Filter
- `installThirdPartyTraceMessageFilter()` is called from `src/app/main.cpp` immediately after Qt application bootstrap.
- The filter drops only `QtDebugMsg` entries whose message starts with `iiXml::` when `WHATSON_IIXML_TRACE_MODE` is not
  enabled.
- `QtWarningMsg`, `QtCriticalMsg`, `QtFatalMsg`, and all non-`iiXml` messages continue through the previous Qt message
  handler, or through `qFormatLogMessage` when no previous handler exists.

## Verification
- `test/cpp/suites/debug_trace_filter_tests.cpp` locks the suppression predicate, warning passthrough behavior, main
  startup installation call, and environment variable contract.

## Extension Notes
- Keep the filter as a narrow text-prefix gate for local dependencies that do not expose a logging API.
- Prefer adding real logging controls to local libraries when they become available instead of broadening app-side
  suppression.
