# WhatSon Init Record

This file records the standard Codex initialization (`/init`) procedure for this repository.

## Standard Init Commands

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=$HOME/.local/LVRS
cmake --build build --target WhatSon -j
cmake --build build --target whats_on_daemon -j
./build/src/daemon/whats_on_daemon --healthcheck
cmake --build build --target run_WhatSon
```

## Expected Results

- Configure succeeds
- `WhatSon` build succeeds
- `whats_on_daemon` build succeeds
- Healthcheck output: `status=ok`

## Notes

- Keep LVRS integration in the default pattern: `find_package(LVRS CONFIG REQUIRED)` + `lvrs_configure_qml_app()`.
- Manual link option overrides are allowed only with clear exception evidence.
