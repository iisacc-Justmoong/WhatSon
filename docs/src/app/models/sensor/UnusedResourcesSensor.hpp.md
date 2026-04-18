# `src/app/models/sensor/UnusedResourcesSensor.hpp`

## Responsibility
Declares a QObject sensor that reports resource packages present in a hub but not embedded by any note body.

## Public Surface
- `hubPath`: unpacked `.wshub` directory that becomes the scan root.
- `unusedResources`: `QVariantList` of unused package descriptors.
- `unusedResourcePaths`: convenience `QStringList` projection extracted from `unusedResources`.
- `unusedResourceCount`: count projection for list/detail panels or diagnostics.
- `lastError`: most recent validation or scan error.
- `scanUnusedResources(...)`: refreshes sensor state and returns unused entries.
- `collectUnusedResourcePaths(...)`: convenience refresh path that returns only resource paths.
- `refresh()`: explicit slot entrypoint so higher layers can re-run the scan after filesystem changes.

## Signals
- `hubPathChanged()`
- `unusedResourcesChanged()`
- `lastErrorChanged()`
- `scanCompleted(...)`
